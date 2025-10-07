# Makefile cho hướng dẫn cài đặt DDoS_V1 (manual)
# Lưu ý: một số bước cần sudo (apt, make install, dpkg, ldconfig)
# Chạy ví dụ: make all      hoặc make gui    hoặc make wolfssl

.PHONY: all dirs wiringpi i2c desktop autostart perms deps tmuxconf bashrc wolfssl copy_wolfssl gui cli clean install run_gui run_cli

PREFIX := /home/antiddos/DDoS_V1
WIRINGPI_REPO := https://github.com/xuanhuy10/WiringPi.git
I2C_REPO := https://github.com/xuanhuy10/i2c.git
WOLFSSL_REPO := https://github.com/wolfSSL/wolfssl.git

DESKTOP_NAME := my_script.desktop
DESKTOP_LOCAL := $(HOME)/.local/share/applications/$(DESKTOP_NAME)
AUTOSTART_DIR := $(HOME)/.config/autostart
AUTOSTART_FILE := $(AUTOSTART_DIR)/$(DESKTOP_NAME)
MENU_SCRIPT := $(PREFIX)/menu.sh

GUI_SRC := gui.c
GUI_BIN := gui
CLI_SRC := cli_working.c
CLI_BIN := cli1

# target mặc định
all: dirs wiringpi i2c wolfssl copy_wolfssl desktop autostart perms deps tmuxconf bashrc gui cli
	@echo "=== Hoan tat target all ==="

################################################################################
# B1: Tạo thư mục dự án và cây thư mục cần thiết
################################################################################
dirs:
	@echo "=> Tao cac thu muc duoi $(PREFIX)"
	@mkdir -p "$(PREFIX)"
	@cd "$(PREFIX)" && mkdir -p HTTP_ip_table
	@cd "$(PREFIX)" && mkdir -p Log/Log_Normal
	@cd "$(PREFIX)" && mkdir -p Log/Log_Flood
	@cd "$(PREFIX)" && mkdir -p Setting/certificates
	@echo "=> Da tao thu muc."

################################################################################
# B1 (tiếp): WiringPi
################################################################################
wiringpi:
	@echo "=> Clone / build WiringPi (o $(PREFIX))"
	@cd "$(PREFIX)" && \
	if [ -d WiringPi ]; then echo "WiringPi ton tai -> git pull"; cd WiringPi && git pull; else git clone $(WIRINGPI_REPO); fi
	@cd "$(PREFIX)/WiringPi" && ./build debian || true
	@cd "$(PREFIX)/WiringPi" && mv -f debian-template/wiringpi_3.16_arm64.deb . || true
	@echo "=> Cai wiringpi (yeu cau sudo)"
	@sudo apt update
	@sudo apt install -y "$(PREFIX)/WiringPi/wiringpi_3.16_arm64.deb" || sudo dpkg -i "$(PREFIX)/WiringPi/wiringpi_3.16_arm64.deb" || true
	@echo "=> WiringPi: xong (kiem tra output neu co loi)."

################################################################################
# B2: i2c repo
################################################################################
i2c:
	@echo "=> Clone / build i2c (o $(PREFIX))"
	@cd "$(PREFIX)" && \
	if [ -d i2c ]; then echo "i2c ton tai -> git pull"; cd i2c && git pull; else git clone $(I2C_REPO); fi
	@cd "$(PREFIX)/i2c" && make || true
	@cd "$(PREFIX)/i2c" && sudo make install || true
	@echo "=> i2c: xong."

################################################################################
# B3: Tạo file .desktop trong ~/.local/share/applications (không dùng nano)
################################################################################
desktop:
	@echo "=> Tao $(DESKTOP_LOCAL)"
	@mkdir -p "$(HOME)/.local/share/applications"
	@cat > "$(DESKTOP_LOCAL)" <<'EOF'
	[Desktop Entry]
	Version=1.0
	Type=Application
	Name=AntiDDoS
	Exec=/bin/bash /home/antiddos/DDoS_V1/menu.sh
	Icon=/home/antiddos/DDoS_V1/icon.png
	Terminal=true
	Comment=Run AntiDDoS Menu
	Categories=Utility;
	X-KeepTerminal=false
	EOF
	@chmod +x "$(DESKTOP_LOCAL)" || true
	@echo "=> Tao file .desktop xong: $(DESKTOP_LOCAL)"

################################################################################
# B4/B5: Tạo autostart và copy .desktop vào ~/.config/autostart
################################################################################
autostart: desktop
	@echo "=> Thiet lap autostart"
	@mkdir -p "$(AUTOSTART_DIR)"
	@cp -f "$(DESKTOP_LOCAL)" "$(AUTOSTART_FILE)"
	@chmod +x "$(MENU_SCRIPT)" || true
	@chmod +x "$(AUTOSTART_FILE)" || true
	@desktop-file-validate "$(AUTOSTART_FILE)" || echo "Warning: $(AUTOSTART_FILE) khong hop le hoac desktop-file-validate khong cai"
	@echo "=> Autostart file: $(AUTOSTART_FILE)"

################################################################################
# B6: Quyền thực thi cho menu.sh và file .desktop
################################################################################
perms:
	@echo "=> Cap quyen thuc thi cho menu.sh va .desktop"
	@mkdir -p "$(PREFIX)"
	@if [ ! -e "$(MENU_SCRIPT)" ]; then touch "$(MENU_SCRIPT)"; echo "#!/bin/bash" > "$(MENU_SCRIPT)"; echo "echo \"AntiDDoS menu placeholder\"" >> "$(MENU_SCRIPT)"; fi
	@chmod +x "$(MENU_SCRIPT)" || true
	@chmod +x "$(AUTOSTART_FILE)" || true
	@echo "=> Da chmod +x $(MENU_SCRIPT) va $(AUTOSTART_FILE) (neu ton tai)."

################################################################################
# B7: Cài tmux và các thư viện cần thiết
################################################################################
deps:
	@echo "=> Cai tmux va thu vien can thiet (yeu cau sudo)"
	@sudo apt update
	@sudo apt install -y tmux libncurses5-dev libncursesw5-dev libcurl4-openssl-dev libglib2.0-dev build-essential pkg-config i2c-tools git || true
	@echo "=> Cai dat phu thuoc xong."

################################################################################
# B8: Tạo file ~/.tmux.conf với binding
################################################################################
tmuxconf:
	@echo "=> Viet $(HOME)/.tmux.conf"
	@cat > "$(HOME)/.tmux.conf" <<'EOF'
# Tmux custom key bindings: hủy copy-mode bằng nhiều phím
	set -g mouse on
	set -g status-style bg=default

	bind -T copy-mode q send -X cancel
	bind -T copy-mode r send -X cancel
	bind -T copy-mode s send -X cancel
	bind -T copy-mode t send -X cancel
	bind -T copy-mode u send -X cancel
	bind -T copy-mode v send -X cancel
	bind -T copy-mode w send -X cancel
	bind -T copy-mode x send -X cancel
	bind -T copy-mode y send -X cancel
	bind -T copy-mode z send -X cancel
	bind -T copy-mode A send -X cancel
	bind -T copy-mode B send -X cancel
	bind -T copy-mode C send -X cancel
	bind -T copy-mode D send -X cancel
	bind -T copy-mode E send -X cancel
	bind -T copy-mode F send -X cancel
	bind -T copy-mode G send -X cancel
	bind -T copy-mode H send -X cancel
	bind -T copy-mode I send -X cancel
	bind -T copy-mode J send -X cancel
	bind -T copy-mode K send -X cancel
	bind -T copy-mode L send -X cancel
	bind -T copy-mode M send -X cancel
	bind -T copy-mode N send -X cancel
	bind -T copy-mode O send -X cancel
	bind -T copy-mode P send -X cancel
	bind -T copy-mode Q send -X cancel
	bind -T copy-mode R send -X cancel
	bind -T copy-mode S send -X cancel
	bind -T copy-mode T send -X cancel
	bind -T copy-mode U send -X cancel
	bind -T copy-mode V send -X cancel
	bind -T copy-mode W send -X cancel
	bind -T copy-mode X send -X cancel
	bind -T copy-mode Y send -X cancel
	bind -T copy-mode Z send -X cancel
	bind -T copy-mode 0 send -X cancel
	bind -T copy-mode 1 send -X cancel
	bind -T copy-mode 2 send -X cancel
	bind -T copy-mode 3 send -X cancel
	bind -T copy-mode 4 send -X cancel
	bind -T copy-mode 5 send -X cancel
	bind -T copy-mode 6 send -X cancel
	bind -T copy-mode 7 send -X cancel
	bind -T copy-mode 8 send -X cancel
	bind -T copy-mode 9 send -X cancel
	bind -T copy-mode Enter send -X cancel
	EOF
	@echo "=> Da viet $(HOME)/.tmux.conf"

################################################################################
# B9: Thêm alias vào ~/.bashrc nếu chưa có
################################################################################
bashrc:
	@echo "=> Them alias start/restart vao ~/.bashrc neu chua ton tai"
	@grep -qxF "alias start='stty rows 59 cols 230; sleep 2; tmux attach -t menu_session'" "$(HOME)/.bashrc" || echo "alias start='stty rows 59 cols 230; sleep 2; tmux attach -t menu_session'" >> "$(HOME)/.bashrc"
	@grep -qxF "alias restart='tmux new-session -d -s menu_session \"/bin/bash /home/antiddos/DDoS_V1/start_menu.sh\"; sleep 2; stty rows 59 cols 230; sleep 2; tmux attach -t menu_session'" "$(HOME)/.bashrc" || echo "alias restart='tmux new-session -d -s menu_session \"/bin/bash /home/antiddos/DDoS_V1/start_menu.sh\"; sleep 2; stty rows 59 cols 230; sleep 2; tmux attach -t menu_session'" >> "$(HOME)/.bashrc"
	@echo "=> Aliases da them (neu chua co). Hay chay: source ~/.bashrc"

################################################################################
# Install wolfSSL (the canonical steps you provided)
# This installs into system /usr/local by default (requires sudo)
################################################################################
wolfssl:
	@echo "=> Clone/build/install wolfSSL (o $(PREFIX)/wolfssl)"
	@mkdir -p "$(PREFIX)"
	@if [ -d "$(PREFIX)/wolfssl" ]; then \
	  echo "wolfssl repo da ton tai -> cap nhat"; \
	  cd "$(PREFIX)/wolfssl" && git reset --hard && git pull; \
	else \
	  git clone $(WOLFSSL_REPO) "$(PREFIX)/wolfssl"; \
	fi
	@cd "$(PREFIX)/wolfssl" && rm -rf autom4te.cache config.status config.log || true
	@cd "$(PREFIX)/wolfssl" && ./autogen.sh || true
	@cd "$(PREFIX)/wolfssl" && ./configure --enable-all --enable-static --enable-shared || true
	@cd "$(PREFIX)/wolfssl" && make -j$(shell nproc) || true
	@cd "$(PREFIX)/wolfssl" && sudo make install || true
	@echo "=> Chay ldconfig (sudo)"
	@sudo ldconfig || true
	@echo "=> Kiem tra phien ban wolfssl (pkg-config)"
	@pkg-config --modversion wolfssl || echo "Warning: pkg-config khong tim wolfssl (kiem tra PKG_CONFIG_PATH)."
# Makefile copy thu muc can thiet tu wolfssl
copy_wolfssl:
	@echo "Dang copy cac thu muc tu wolfssl..."
	cp -r wolfssl/certs /home/antiddos/DDoS_V1/
	cp -r wolfssl/src /home/antiddos/DDoS_V1/
	cp -r wolfssl/wolfcrypt /home/antiddos/DDoS_V1/
	@echo "Da hoan thanh copy thu muc."

################################################################################
# Biên dịch GUI & CLI
################################################################################
gui: wolfssl
	@echo "=> Bien dich gui (su dung wolfssl tren he /usr/local)"
	@gcc $(GUI_SRC) -o $(GUI_BIN) -li2c1602 -lwiringPi -lpthread -lncurses -lcurl $(shell pkg-config --cflags --libs wolfssl glib-2.0) -I/usr/local/include -L/usr/local/lib -Wl,-rpath=/usr/local/lib -lwolfssl || (echo "gcc gui thất bại"; exit 1)
	@echo "=> Bien dich gui xong: ./$(GUI_BIN)"

cli:
	@echo "=> Bien dich CLI (cli_working.c -> $(CLI_BIN))"
	@gcc $(CLI_SRC) -o $(CLI_BIN) -li2c1602 -lwiringPi -lpthread -lncurses -lcurl $(shell pkg-config --cflags --libs glib-2.0) -L/usr/local/lib -lwolfssl -lcjson -lsqlite3 -ljansson -Wl,-rpath=/usr/local/lib || (echo "gcc cli thất bại"; exit 1)
	@echo "=> Bien dich cli xong: ./$(CLI_BIN)"

################################################################################
# Run helpers
################################################################################
run_gui:
	@echo "=> Chay gui (yeu cau sudo neu can quyen I2C hoac bind port thap)"
	@sudo ./$(GUI_BIN) || ./$(GUI_BIN) || true
run_cli:
	@echo "=> Chay cli (yeu cau sudo neu can)"
	@sudo ./$(CLI_BIN) || ./$(CLI_BIN) || true
################################################################################
# Clean (xóa binary và .desktop/autostart cục bộ; không xóa repo hoặc /usr/local)
################################################################################
clean:
	@echo "=> Clean (xoa gui/cli va cac file .desktop test)"
	@rm -f ./$(GUI_BIN) ./$(CLI_BIN)
	@rm -f "$(DESKTOP_LOCAL)" "$(AUTOSTART_FILE)" || true
	@echo "=> Done clean."
################################################################################
# install: alias cho all
################################################################################
install: all
	@echo "=> Install finished. Hay kiem tra cac buoc yeu cau sudo trong log."
