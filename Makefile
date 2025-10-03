

# Makefile for AntiDDoS (Raspberry Pi) setup and build
# Usage examples:
#  - make help
#  - sudo make install_all

CC = gcc
CFLAGS = -Wall -O2
LIBS = -li2c1602 -lwiringPi -lpthread -lncurses -lcurl \
       `pkg-config --cflags --libs glib-2.0` \
       -L/usr/local/lib -lwolfssl -lsqlite3 -ljansson -lcjson

CLI_TARGET = cli1
CLI_SRC = cli_working.c

# GUI source: prefer gui.c if present, else fallback to gui_working.c
GUI_TARGET = gui
GUI_SRC ?= gui.c

.PHONY: all help \
        build_cli build_gui run_cli run_gui clean \
        install_all deps create_folder \
        B1_WiringPi B2_i2c B3_desktop B4_autostart_copy B5_autostart_manual \
        B6_chmod B7_tmux_install B8_tmux_conf B9_bashrc_aliases \
        wolfssl wolfssl_check

all: help

build_cli: $(CLI_SRC)
	$(CC) -o $(CLI_TARGET) $(CLI_SRC) $(LIBS)

build_gui:
	@if [ -f "$(GUI_SRC)" ]; then \
		SRC_FILE=$(GUI_SRC); \
	elif [ -f "gui_working.c" ]; then \
		SRC_FILE=gui_working.c; \
	else \
		echo "No GUI source (gui.c/gui_working.c) found"; exit 1; \
	fi; \
	$(CC) $$SRC_FILE -o $(GUI_TARGET) -li2c1602 -lwiringPi -lpthread -lncurses -lcurl `pkg-config --cflags --libs wolfssl glib-2.0` -I/usr/local/include -L/usr/local/lib -lwolfssl

run_cli: build_cli
	sudo ./$(CLI_TARGET)

run_gui: build_gui
	sudo ./$(GUI_TARGET)

install_all: deps create_folder B1_WiringPi B2_i2c wolfssl B7_tmux_install B8_tmux_conf B3_desktop B4_autostart_copy B6_chmod B9_bashrc_aliases

# Step 1: apt dependencies
deps:
	sudo apt-get update
	sudo apt-get install -y build-essential git autoconf automake libtool pkg-config unzip wget
	sudo apt-get install -y libncurses5-dev libncursesw5-dev libcurl4-openssl-dev \
		libglib2.0-dev libsqlite3-dev libjansson-dev libcjson-dev
	@echo "Done Step 1: dependencies"

PROJECT_DIR := /home/antiddos/DDoS_V1
# Step 2: Create folders
create_folder:
	mkdir -p $(PROJECT_DIR)/HTTP_ip_table
	mkdir -p $(PROJECT_DIR)/Log/
	mkdir -p $(PROJECT_DIR)/Log/Log_Normal
	mkdir -p $(PROJECT_DIR)/Log/Log_Flood
	mkdir -p $(PROJECT_DIR)/Setting
	mkdir -p $(PROJECT_DIR)/Setting/certificates
	mkdir -p $(PROJECT_DIR)/IPV4_table
	@echo "Done Step 2: create folders"

B1_WiringPi:
	@echo "=== B1: Installing WiringPi ==="
	@if command -v gpio > /dev/null 2>&1; then \
		echo "WiringPi already installed, skipping..."; \
		gpio -v; \
	else \
		echo "WiringPi not found, installing..."; \
		sudo apt install -y git; \
		if [ ! -d "WiringPi" ]; then \
			git clone https://github.com/xuanhuy10/WiringPi.git; \
		fi; \
		cd WiringPi && chmod +x build && ./build Debian; \
		if [ -f "debian-template/wiringpi_3.16_arm64.deb" ]; then \
			mv debian-template/wiringpi_3.16_arm64.deb ..; \
		elif [ -f "WiringPi/debian-template/wiringpi_3.16_arm64.deb" ]; then \
			mv WiringPi/debian-template/wiringpi_3.16_arm64.deb .; \
		fi; \
		cd .. || true; \
		if [ -f "wiringpi_3.16_arm64.deb" ]; then \
			sudo apt install -y ./wiringpi_3.16_arm64.deb || sudo dpkg -i ./wiringpi_3.16_arm64.deb; \
		else \
			echo "Error: wiringpi_3.16_arm64.deb not found after build!"; \
			exit 1; \
		fi; \
	fi
	@echo "=== B1: WiringPi installation done ==="

B2_i2c:
	@echo "=== B2: Installing i2c library ==="
	@if [ ! -d "i2c" ]; then \
		git clone https://github.com/xuanhuy10/i2c.git; \
	fi; \
	cd i2c && make && sudo make install
	@echo "=== B2: i2c installed ==="

B3_desktop:
	@echo "=== B3: Creating desktop launcher ==="
	mkdir -p $$HOME/.local/share/applications
	printf "[Desktop Entry]\nVersion=1.0\nType=Application\nName=AntiDDoS\nExec=/bin/bash /home/antiddos/DDoS_V1/menu.sh\nIcon=/home/antiddos/DDoS_V1/icon.png\nTerminal=false\nComment=Run DDoS Menu\nCategories=Utility;\nX-KeepTerminal=false\n" > $$HOME/.local/share/applications/my_script.desktop
	@echo "=== B3: Desktop entry created ==="

B4_autostart_copy:
	@echo "=== B4: Setting autostart from desktop entry ==="
	mkdir -p $$HOME/.config/autostart
	mkdir -p $$HOME/.config/AutoStart
	cp -f $$HOME/.local/share/applications/my_script.desktop $$HOME/.config/autostart/
	@echo "=== B4: Autostart configured ==="

B5_autostart_manual:
	@echo "=== B5: Manually creating autostart desktop entry ==="
	mkdir -p $$HOME/.config/autostart
	printf "[Desktop Entry]\nType=Application\nVersion=1.0\nName=AntiDDoS\nExec=/bin/bash /home/antiddos/DDoS_V1/menu.sh\nIcon=/home/antiddos/DDoS_V1/icon.png\nTerminal=true\nComment=Run AntiDDoS Menu at login\nCategories=Utility;\nX-GNOME-Autostart-enabled=true\n" > $$HOME/.config/autostart/my_script.desktop
	@echo "=== B5: Manual autostart entry created ==="

B6_chmod:
	@echo "=== B6: Setting execute permissions ==="
	@if [ -f "$(PROJECT_DIR)/menu.sh" ]; then \
		chmod +x $(PROJECT_DIR)/menu.sh; \
	else \
		echo "Warning: $(PROJECT_DIR)/menu.sh not found"; \
	fi
	chmod +x $$HOME/.config/autostart/my_script.desktop || true
	@echo "=== B6: Permissions set ==="

B7_tmux_install:
	@echo "=== B7: Installing tmux and libraries ==="
	sudo apt-get install -y tmux
	sudo apt-get install -y libncurses5-dev libncursesw5-dev
	sudo apt-get install -y libcurl4-openssl-dev
	sudo apt-get install -y libglib2.0-dev
	@echo "=== B7: tmux and libs installed ==="

B8_tmux_conf:
	@echo "=== B8: Writing ~/.tmux.conf ==="
	printf "bind -T copy-mode q send -X cancel\n" > $$HOME/.tmux.conf
	for k in r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z 0 1 2 3 4 5 6 7 8 9; do \
		printf "bind -T copy-mode $$k send -X cancel\n" >> $$HOME/.tmux.conf; \
	done
	printf "bind -T copy-mode Enter send -X cancel\nset -g status-style bg=default\nset -g mouse on\n" >> $$HOME/.tmux.conf
	for k in a b c d e f g h i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P; do \
		printf "bind -T copy-mode $$k send -X cancel\n" >> $$HOME/.tmux.conf; \
	done
	@echo "=== B8: ~/.tmux.conf updated ==="

B9_bashrc_aliases:
	@echo "=== B9: Appending aliases to ~/.bashrc ==="
	AL_START="alias start='stty rows 59 cols 230; sleep 2; tmux attach -t menu_session'"; \
	AL_RESTART="alias restart='tmux new-session -d -s menu_session \"/bin/bash /home/antiddos/DDoS_V1/start_menu.sh\"; sleep 2; stty rows 59 cols 230; sleep 2; tmux attach -t menu_session'"; \
	grep -qxF "$$AL_START" $$HOME/.bashrc || echo "$$AL_START" >> $$HOME/.bashrc; \
	grep -qxF "$$AL_RESTART" $$HOME/.bashrc || echo "$$AL_RESTART" >> $$HOME/.bashrc; \
	@echo "Note: You need to open a new shell for aliases to take effect."

wolfssl:
	@echo "=== Install wolfSSL ==="
	@if [ ! -d "wolfssl" ]; then \
		git clone https://github.com/wolfSSL/wolfssl.git; \
	fi; \
	cd wolfssl && git reset --hard && git pull && rm -rf autom4te.cache config.status config.log && \
		./autogen.sh && ./configure --enable-all --enable-static --enable-shared && \
		make -j`nproc` && sudo make install && sudo ldconfig && \
		pkg-config --modversion wolfssl || true
	@echo "=== wolfSSL installed ==="

wolfssl_check:
	@echo "=== Check wolfSSL installation ==="
	ls /usr/local/lib | grep libwolfssl || true
	@echo "If you see libwolfssl.so.* and libwolfssl.a listed above, installation is OK."
	sudo ldconfig
	@echo "ldconfig updated"
clean:
	rm -f $(CLI_TARGET) $(GUI_TARGET)
	@echo "Cleaned binaries"

help:
	@echo "Targets:"
	@echo "  install_all          - Run full installation flow (deps, B1..B9, wolfssl)"
	@echo "  B1_WiringPi          - Install WiringPi (xuanhuy10/WiringPi, build Debian)"
	@echo "  B2_i2c               - Clone/build/install i2c library"
	@echo "  wolfssl              - Build and install wolfSSL"
	@echo "  wolfssl_check        - Verify wolfSSL libs and run ldconfig"
	@echo "  B3_desktop           - Create desktop launcher"
	@echo "  B4_autostart_copy    - Copy launcher to autostart"
	@echo "  B5_autostart_manual  - Create autostart entry manually"
	@echo "  B6_chmod             - Set execute permissions"
	@echo "  B7_tmux_install      - Install tmux and dependencies"
	@echo "  B8_tmux_conf         - Write ~/.tmux.conf"
	@echo "  B9_bashrc_aliases    - Append aliases to ~/.bashrc"
	@echo "  build_cli/run_cli    - Build/Run CLI"
	@echo "  build_gui/run_gui    - Build/Run GUI"
	@echo "  clean                - Remove built binaries"

