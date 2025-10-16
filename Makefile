



# Makefile for AntiDDoS installation (based on your manual)
# Edit HOME_DIR if your user/home path differs (default: /home/antiddos)
HOME_DIR ?= /home/antiddos
PREFIX ?= $(HOME_DIR)/DDoS_V1
WIRINGPI_REPO ?= https://github.com/xuanhuy10/WiringPi.git
I2C_REPO ?= https://github.com/xuanhuy10/i2c.git
WOLFSSL_REPO ?= https://github.com/wolfSSL/wolfssl.git
WIRINGPI_DEB_NAME ?= wiringpi_3.16_arm64.deb

SHELL := /bin/bash
.PHONY: install help dirs deps wiringpi i2c desktop autostart permissions tmuxconf bashrc source wolfssl enable_i2c unzip_and_move build clean

install: dirs deps wiringpi i2c desktop autostart permissions tmuxconf bashrc source wolfssl enable_i2c unzip_and_move build clean 
	@echo "Installation completed (or attempted)Review output above for errors."
help:
	@echo "Makefile targets:"
	@echo "  make           -> run default all target"
	@echo "  make dirs      -> create required folders"
	@echo "  make deps      -> install apt packages (requires sudo)"
	@echo "  make wiringpi  -> clone + build wiringpi"
	@echo "  make i2c       -> clone + build i2c"
	@echo "  make wolfssl   -> clone + build wolfssl"
	@echo "  make desktop   -> install .desktop file in ~/.local/share/applications"
	@echo "  make autostart -> copy .desktop to ~/.config/autostart"
	@echo "  make permissions -> chmod +x menu scripts and desktop"
	@echo "  make tmuxconf  -> write ~/.tmux.conf"
	@echo "  make bashrc    -> append aliases to ~/.bashrc (idempotent)"
	@echo "  make build     -> compile gui and cli binaries (gcc required)"
	@echo "  make clean     -> remove cloned repos & build artifacts"

###############################################################################
# Create directories
dirs:
	@echo "Creating directories under $(PREFIX) ..."
	@mkdir -p "$(PREFIX)/HTTP_ip_table"
	@mkdir -p "$(PREFIX)/Log/Log_Normal"
	@mkdir -p "$(PREFIX)/Log/Log_Flood"
	@mkdir -p "$(PREFIX)/Setting/certificates"
	@echo "Directories created."

###########################################################################	####
# System dependencies
deps:
	@echo "Installing apt packages (will prompt for sudo password)..."
	sudo apt update
	sudo apt install -y tmux 
	sudo apt install -y libncurses5-dev libncursesw5-dev 
	sudo apt install -y libcurl4-openssl-dev 
	sudo apt install -y libglib2.0-dev 
	sudo apt install -y libsqlite3-dev
	sudo apt install -y libcjson-dev
	sudo apt install -y libjansson-dev
	sudo apt install -y git tmux libncurses5-dev libncursesw5-dev libcurl4-openssl-dev libglib2.0-dev autoconf automake libtool pkg-config gcc make 
	@echo "Base packages installed."
	@echo "#-----------DONE1-----------#"


###############################################################################
# WiringPi
wiringpi:
	@echo "Cloning WiringPi (if not present) and building .deb..."
	@if [ ! -d "$(PREFIX)/WiringPi" ]; then \
		git clone $(WIRINGPI_REPO) "$(PREFIX)/WiringPi"; \
	else \
		echo "WiringPi already cloned in $(PREFIX)/WiringPi"; \
	fi
	@cd "$(PREFIX)/WiringPi" && git fetch --all --prune && git reset --hard origin/HEAD 
	@cd "$(PREFIX)/WiringPi" && ./build debian
	@cd "$(PREFIX)/WiringPi" && \
		if [ -f debian-template/$(WIRINGPI_DEB_NAME) ]; then \
			mv -f debian-template/$(WIRINGPI_DEB_NAME) "$(PREFIX)/$(WIRINGPI_DEB_NAME)"; \
			sudo apt install -y "$(PREFIX)/$(WIRINGPI_DEB_NAME)"; \
		else \
			echo "Warning: deb file not found at debian-template/$(WIRINGPI_DEB_NAME)"; \
		fi
	@echo "#-----------DONE2-----------#"

###############################################################################
# i2c
i2c:
	@echo "Downloading and building i2c1602 (if not present)..."
	@if [ ! -d "$(PREFIX)/i2c1602-main" ]; then \
		wget -q https://github.com/vinhcatba/i2c1602/archive/refs/heads/main.zip -O main.zip; \
		unzip -q main.zip; \
		rm -f main.zip; \
	else \
		echo "i2c1602 source already exists in $(PREFIX)/i2c1602-main"; \
	fi
	@cd "$(PREFIX)/i2c1602-main" && make
	@cd "$(PREFIX)/i2c1602-main" && sudo make install
	@echo "#-----------DONE3-----------#"


###############################################################################
# Desktop .desktop file
# desktop:
# 	@echo "Writing desktop entry to $(HOME_DIR)/.local/share/applications/my_script.desktop ..."
# 	@mkdir -p "$(HOME_DIR)/.local/share/applications"
# 	@cat > "$(HOME_DIR)/.local/share/applications/my_script.desktop" <<'EOF'
# 	[Desktop Entry]
# 	Version=1.0
# 	Type=Application
# 	Name=AntiDDoS
# 	Exec=/bin/bash $(PREFIX)/menu.sh
# 	Icon=$(PREFIX)/icon.png
# 	Terminal=false
# 	Comment=Run DDoS Menu
# 	Categories=Utility;
# 	X-KeepTerminal=false
# 	EOF
# 	@echo "Desktop file created."
# 	@echo "#-----------DONE4-----------#"
desktop:
	@echo "Writing desktop entry to $(HOME_DIR)/.local/share/applications/my_script.desktop ..."
	@mkdir -p "$(HOME_DIR)/.local/share/applications"
	@echo "[Desktop Entry]" > "$(HOME_DIR)/.local/share/applications/my_script.desktop"
	@echo "Version=1.0" >> "$(HOME_DIR)/.local/share/applications/my_script.desktop"
	@echo "Type=Application" >> "$(HOME_DIR)/.local/share/applications/my_script.desktop"
	@echo "Name=AntiDDoS" >> "$(HOME_DIR)/.local/share/applications/my_script.desktop"
	@echo "Exec=/bin/bash $(PREFIX)/menu.sh" >> "$(HOME_DIR)/.local/share/applications/my_script.desktop"
	@echo "Icon=$(PREFIX)/icon.png" >> "$(HOME_DIR)/.local/share/applications/my_script.desktop"
	@echo "Terminal=false" >> "$(HOME_DIR)/.local/share/applications/my_script.desktop"
	@echo "Comment=Run DDoS Menu" >> "$(HOME_DIR)/.local/share/applications/my_script.desktop"
	@echo "Categories=Utility;" >> "$(HOME_DIR)/.local/share/applications/my_script.desktop"
	@echo "X-KeepTerminal=false" >> "$(HOME_DIR)/.local/share/applications/my_script.desktop"
	@echo "Desktop file created."
	@echo "#-----------DONE4-----------#"


###############################################################################
# Autostart
autostart: desktop
	@echo "Setting up autostart at $(HOME_DIR)/.config/autostart ..."
	@mkdir -p "$(HOME_DIR)/.config/autostart"
	@cp -f "$(HOME_DIR)/.local/share/applications/my_script.desktop" "$(HOME_DIR)/.config/autostart/"
	@echo "Copied desktop entry to autostart."
	@echo "#-----------DONE5-----------#"

###############################################################################
# Permissions for menu and desktop
permissions:
	@echo "Making menu scripts and desktop files executable..."
	@if [ -f "$(PREFIX)/menu.sh" ]; then sudo chmod +x "$(PREFIX)/menu.sh"; else echo "Warning: $(PREFIX)/menu.sh not found"; fi
	@if [ -f "$(HOME_DIR)/.config/autostart/my_script.desktop" ]; then sudo chmod +x "$(HOME_DIR)/.config/autostart/my_script.desktop"; fi
	@if [ -f "$(HOME_DIR)/.local/share/applications/my_script.desktop" ]; then chmod +x "$(HOME_DIR)/.local/share/applications/my_script.desktop"; fi
	@echo "Permissions set."
	@echo "#-----------DONE6-----------#"


###############################################################################
# tmux configuration
# tmuxconf:
# 	@echo "Writing ~/.tmux.conf (will overwrite if exists) ..."
# 	@cat > "$(HOME_DIR)/.tmux.conf" <<'EOF'
# 	# tmux custom copy-mode binds (from manual)
# 	bind -T copy-mode q send -X cancel
# 	bind -T copy-mode r send -X cancel
# 	bind -T copy-mode s send -X cancel
# 	bind -T copy-mode t send -X cancel
# 	bind -T copy-mode u send -X cancel
# 	bind -T copy-mode v send -X cancel
# 	bind -T copy-mode w send -X cancel
# 	bind -T copy-mode x send -X cancel
# 	bind -T copy-mode y send -X cancel
# 	bind -T copy-mode z send -X cancel
# 	bind -T copy-mode A send -X cancel
# 	bind -T copy-mode B send -X cancel
# 	bind -T copy-mode C send -X cancel
# 	bind -T copy-mode D send -X cancel
# 	bind -T copy-mode E send -X cancel
# 	bind -T copy-mode F send -X cancel
# 	bind -T copy-mode G send -X cancel
# 	bind -T copy-mode H send -X cancel
# 	bind -T copy-mode I send -X cancel
# 	bind -T copy-mode J send -X cancel
# 	bind -T copy-mode K send -X cancel
# 	bind -T copy-mode L send -X cancel
# 	bind -T copy-mode M send -X cancel
# 	bind -T copy-mode N send -X cancel
# 	bind -T copy-mode O send -X cancel
# 	bind -T copy-mode P send -X cancel
# 	bind -T copy-mode Q send -X cancel
# 	bind -T copy-mode R send -X cancel
# 	bind -T copy-mode S send -X cancel
# 	bind -T copy-mode T send -X cancel
# 	bind -T copy-mode U send -X cancel
# 	bind -T copy-mode V send -X cancel
# 	bind -T copy-mode W send -X cancel
# 	bind -T copy-mode X send -X cancel
# 	bind -T copy-mode Y send -X cancel
# 	bind -T copy-mode Z send -X cancel
# 	bind -T copy-mode 0 send -X cancel
# 	bind -T copy-mode 1 send -X cancel
# 	bind -T copy-mode 2 send -X cancel
# 	bind -T copy-mode 3 send -X cancel
# 	bind -T copy-mode 4 send -X cancel
# 	bind -T copy-mode 5 send -X cancel
# 	bind -T copy-mode 6 send -X cancel
# 	bind -T copy-mode 7 send -X cancel
# 	bind -T copy-mode 8 send -X cancel
# 	bind -T copy-mode 9 send -X cancel
# 	bind -T copy-mode Enter send -X cancel
# 	set -g status-style  bg=default                                                                                                                                                           
# 	set -g mouse on
# 	bind -T copy-mode a send -X cancel
# 	bind -T copy-mode b send -X cancel
# 	bind -T copy-mode c send -X cancel
# 	bind -T copy-mode d send -X cancel
# 	bind -T copy-mode e send -X cancel
# 	bind -T copy-mode f send -X cancel
# 	bind -T copy-mode g send -X cancel
# 	bind -T copy-mode h send -X cancel
# 	bind -T copy-mode i send -X cancel
# 	bind -T copy-mode j send -X cancel
# 	bind -T copy-mode k send -X cancel
# 	bind -T copy-mode l send -X cancel
# 	bind -T copy-mode m send -X cancel
# 	bind -T copy-mode n send -X cancel
# 	bind -T copy-mode o send -X cancel
# 	bind -T copy-mode p send -X cancel
# 	bind -T copy-mode q send -X cancel
# 	bind -T copy-mode r send -X cancel
# 	bind -T copy-mode s send -X cancel
# 	bind -T copy-mode t send -X cancel
# 	bind -T copy-mode u send -X cancel
# 	bind -T copy-mode v send -X cancel
# 	bind -T copy-mode w send -X cancel
# 	bind -T copy-mode x send -X cancel
# 	bind -T copy-mode y send -X cancel
# 	bind -T copy-mode z send -X cancel
# 	bind -T copy-mode A send -X cancel
# 	bind -T copy-mode B send -X cancel
# 	bind -T copy-mode C send -X cancel
# 	bind -T copy-mode D send -X cancel
# 	bind -T copy-mode E send -X cancel
# 	bind -T copy-mode F send -X cancel
# 	bind -T copy-mode G send -X cancel
# 	bind -T copy-mode H send -X cancel
# 	bind -T copy-mode I send -X cancel
# 	bind -T copy-mode J send -X cancel
# 	bind -T copy-mode K send -X cancel
# 	bind -T copy-mode L send -X cancel
# 	bind -T copy-mode M send -X cancel
# 	bind -T copy-mode N send -X cancel
# 	bind -T copy-mode O send -X cancel
# 	bind -T copy-mode P send -X cancel
# 	EOF
# 	@echo "~/.tmux.conf written."
# 	@echo "#-----------DONE7-----------#"

tmuxconf:
	@echo "Writing ~/.tmux.conf (will overwrite if exists) ..."
	@echo "# tmux custom copy-mode binds (from manual)" > "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode q send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode r send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode s send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode t send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode u send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode v send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode w send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode x send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode y send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode z send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode A send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode B send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode C send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode D send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode E send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode F send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode G send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode H send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode I send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode J send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode K send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode L send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode M send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode N send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode O send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode P send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode Q send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode R send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode S send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode T send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode U send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode V send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode W send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode X send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode Y send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode Z send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode 0 send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode 1 send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode 2 send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode 3 send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode 4 send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode 5 send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode 6 send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode 7 send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode 8 send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode 9 send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "bind -T copy-mode Enter send -X cancel" >> "$(HOME_DIR)/.tmux.conf"
	@echo "set -g status-style bg=default" >> "$(HOME_DIR)/.tmux.conf"
	@echo "set -g mouse on" >> "$(HOME_DIR)/.tmux.conf"
	@echo "~/.tmux.conf written."
	@echo "#-----------DONE7-----------#"

###############################################################################
# Append aliases to ~/.bashrc (idempotent)
# bashrc:
# 	@echo "Adding aliases to $(HOME_DIR)/.bashrc (if not already present)..."
# 	@grep -qxF "alias start='stty rows 59 cols 230; sleep 2; tmux attach -t menu_session'" "$(HOME_DIR)/.bashrc" 2>/dev/null || cat >> "$(HOME_DIR)/.bashrc" <<'EOF'
# 	alias start='stty rows 59 cols 230; sleep 2; tmux attach -t menu_session'
# 	alias restart='tmux new-session -d -s menu_session "/bin/bash $(PREFIX)/start_menu.sh"; sleep 2; stty rows 59 cols 230; sleep 2; tmux attach -t menu_session'
# 	EOF
# 	@echo "Aliases added (if they were missing)."
# 	@echo "#-----------DONE8-----------#"
bashrc:
	@echo "Adding aliases to $(HOME_DIR)/.bashrc (if not already present)..."
	@if ! grep -qxF "alias start='stty rows 59 cols 230; sleep 2; tmux attach -t menu_session'" "$(HOME_DIR)/.bashrc"; then \
		echo "alias start='stty rows 59 cols 230; sleep 2; tmux attach -t menu_session'" >> "$(HOME_DIR)/.bashrc"; \
		echo "alias restart='tmux new-session -d -s menu_session \"/bin/bash $(PREFIX)/start_menu.sh\"; sleep 2; stty rows 59 cols 230; sleep 2; tmux attach -t menu_session'" >> "$(HOME_DIR)/.bashrc"; \
	fi
	@echo "Aliases added (if they were missing)."
	@echo "#-----------DONE8-----------#"

source: bashrc
	@echo "Sourcing $(HOME_DIR)/.bashrc to apply changes..."
	@source "$(HOME_DIR)/.bashrc"
	@echo "Sourced .bashrc."
	@echo "#-----------DONE8-----------#"
###############################################################################
# WolfSSL build and copy files
wolfssl:
	@echo "Cloning and building wolfssl..."
	@if [ ! -d "$(PREFIX)/wolfssl" ]; then \
		git clone $(WOLFSSL_REPO) "$(PREFIX)/wolfssl"; \
	else \
		echo "wolfssl already cloned in $(PREFIX)/wolfssl"; \
	fi
	@cd "$(PREFIX)/wolfssl" && git reset --hard 
	@cd "$(PREFIX)/wolfssl" && git pull 
	@cd "$(PREFIX)/wolfssl" && rm -rf autom4te.cache config.status config.log 
	@cd "$(PREFIX)/wolfssl" && ./autogen.sh 
	@cd "$(PREFIX)/wolfssl" && ./configure --enable-all --enable-static --enable-shared 
	@cd "$(PREFIX)/wolfssl" && make -j$$(nproc) 
	@cd "$(PREFIX)/wolfssl" && sudo make install 
	@echo "Running ldconfig..."
	sudo ldconfig 
	@echo "Copying certs, src, wolfcrypt to $(PREFIX) ..."
	@cd "$(PREFIX)/wolfssl" && cp -r certs "$(PREFIX)/" 
	@cd "$(PREFIX)/wolfssl" && cp -r src "$(PREFIX)/"
	@cd "$(PREFIX)/wolfssl" && cp -r wolfcrypt "$(PREFIX)/" 
	@echo "wolfssl build + copy done (check output for errors)."
	@echo "#-----------DONE9-----------#"

###############################################################################
# Compile gui and cli (adjust source filenames if needed)
build:
	@echo "Compiling GUI and CLI (adjust source filenames if different)..."
	@if [ -f "$(PREFIX)/gui.c" ]; then \
		gcc -02 "$(PREFIX)/gui.c" -o "$(PREFIX)/gui" -li2c1602 -lwiringPi -lpthread -lncurses -lcurl `pkg-config --cflags --libs wolfssl glib-2.0` -I/usr/local/include -L/usr/local/lib -lwolfssl ; \
	else \
		echo "Warning: $(PREFIX)/gui.c not found, skipping gui build"; \
	fi
	@if [ -f "$(PREFIX)/cli_working.c" ]; then \
		gcc "$(PREFIX)/cli_working.c" -o "$(PREFIX)/cli" -li2c1602 -lwiringPi -lpthread -lncurses -lcurl `pkg-config --cflags --libs glib-2.0` -L/usr/local/lib -lwolfssl -lcjson -lsqlite3 -ljansson ; \
	else \
		echo "Warning: $(PREFIX)/cli_working.c not found, skipping cli build"; \
	fi
	@echo "Build step finished (check for compiler errors)."
	@echo "#-----------DONE10-----------#"
######################################### ######################################
enable_i2c:
	@echo "Starting raspi-config to enable I2C..."
	sudo raspi-config nonint do_i2c 0
	@echo "I2C has been enabled successfully!"
	@echo "#-----------DONE11-----------#"
unzip_and_move:
	sudo unzip -o /home/antiddos/DDoS_V1/zcu102_zynq_lib.zip -d /tmp/lab_extract
	sudo cp -r /tmp/lab_extract/* /
	sudo rm -rf /tmp/lab_extract
	@echo "#-----------DONE12-----------#"

###############################################################################
# Clean (remove clones/build artifacts)
clean:
	@echo "Cleaning cloned repos and build artifacts under $(PREFIX) ..."
	# @rm -rf "$(PREFIX)/WiringPi" "$(PREFIX)/i2c" "$(PREFIX)/wolfssl"
	# @rm -f "$(PREFIX)/$(WIRINGPI_DEB_NAME)"
    source "$(HOME_DIR)/.bashrc"
	@echo "Clean done."
	@echo "#-----------DONE OK-----------#"

###############################################################################
# ensure PREFIX exists for earlier tasks
$(PREFIX):
	@echo "Makefile done loading."
