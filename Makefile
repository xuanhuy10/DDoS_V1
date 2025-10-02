# Makefile for DDoS CLI Raspberry Pi Environment Setup
# Usage: sudo make install

CC = gcc
CFLAGS = -Wall -O2
LIBS = -li2c1602 -lwiringPi -lpthread -lncurses -lcurl \
       `pkg-config --cflags --libs glib-2.0` \
       -L/usr/local/lib -lwolfssl -lsqlite3 -ljansson -lcjson

TARGET = cli1
SRC = cli_working.c

.PHONY: all install deps wiringpi i2c1602 tmux desktop bashrc build clean verify help

all: build

# Build binary
build: $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(LIBS)

install: deps wiringpi i2c1602 tmux desktop bashrc build

# Step 1: apt-get dependencies
deps:
	sudo apt-get update
	sudo apt-get install -y build-essential git autoconf automake libtool pkg-config unzip wget
	sudo apt-get install -y libncurses5-dev libncursesw5-dev libcurl4-openssl-dev \
		libglib2.0-dev libsqlite3-dev libjansson-dev libcjson-dev
	@echo "Done Step 1"

wiringpi:
	@echo "=== Installing WiringPi (local build) ==="
	cd WiringPi && ./build debian
	if [ -d "WiringPi/debian-template" ]; then \
		mv WiringPi/debian-template/*.deb . || true; \
	fi
	if ls *.deb 1> /dev/null 2>&1; then \
		sudo apt install -y ./*.deb || sudo dpkg -i ./*.deb || true; \
	else \
		echo "No .deb produced for WiringPi; please check WiringPi/build output."; \
	fi
	@echo "Done Step 2"

i2c1602:
	@echo "=== Installing i2c1602 (local build) ==="
	cd i2c1602-main && make && sudo make install
	@echo "Done Step 3"

clean:
	rm -f $(TARGET)

# Help target
help:
	@echo "DDoS CLI Raspberry Pi Setup Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all           - Build the CLI application (default)"
	@echo "  build         - Build the CLI application"
	@echo "  install       - Complete installation with dependencies"
	@echo "  deps          - Install system dependencies"
	@echo "  wiringpi      - Install WiringPi library"
	@echo "  i2c1602       - Install i2c1602 library"
	@echo "  tmux          - Setup tmux configuration"
	@echo "  desktop       - Setup desktop application"
	@echo "  bashrc        - Setup bash aliases"
	@echo "  clean         - Clean build files"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Usage: make install"