# 🛡️ AntiDDoS Safe Install & Usage Guide

> ⚠️ WARNING: Only use this project in an isolated, authorized testing
> environment.\
> Do **not** deploy or run this software against systems you do not own or have
> explicit permission to test.

---

### 🚀 Steps

#### 1. Clone the Repo:

```bash
git clone https://github.com/xuanhuy10/DDoS_V1.git
cd DDoS_V1
```

#### 2. Set up environment (auto)

```bash
# Automatic setup (deps + B1..B9 + wolfSSL)
sudo make install

```

#### 3. Optional: Manual step-by-step (skip if you used automatic install)

```bash
# Open the install_manual.md file and follow the steps in it.
sudo nano install_manual.md 
```

#### 4. Run

```bash
start    # attach to existing menu_session if running
restart  # start/refresh menu_session and attach
```

# AntiDDoS Project

## Overview
This project provides tools for Anti-DDoS operations, including command-line and GUI clients, and server components. It uses serial ports and UNIX domain sockets for communication.

## Prerequisites
- **C Compiler**: GCC (Linux) or MinGW (Windows)
- **Make**: For automated builds
- **Dependencies**: Standard C libraries, POSIX support (for UNIX sockets)
- **Windows Users**: Use WSL or MinGW for UNIX-like environment. Serial port and socket code may require adaptation for native Windows.

## Build Instructions
### Using Makefile (Recommended)
1. Open a terminal (or PowerShell for Windows).
2. Navigate to the project directory:
   ```powershell
   cd f:\DDoS_V1
   ```
3. Run:
   ```powershell
   make
   ```
   This will build all main binaries.

### Manual Build
Compile individual C files as needed. Example (Linux):
```sh
gcc -o cli_working cli_working.c
```
For Windows (MinGW):
```powershell
gcc -o cli_working.exe cli_working.c
```

## Usage
- Run the desired binary (e.g., `cli_working`, `1`, etc.).
- For GUI, see `GUI/README.md`.
- For server, see `server/README.md`.

## Troubleshooting
### Serial Port Errors
- **Error 9 from tcgetattr: Bad file descriptor**
  - Cause: Attempting to use `tcgetattr()` on an invalid file descriptor, usually because `open()` failed.
  - Solution: Ensure the serial device path is correct and accessible. The code now checks `open()` return value before using the descriptor.

### Socket Errors
- **Connect error 2: No such file or directory**
  - Cause: The UNIX domain socket file does not exist or the path is incorrect.
  - Solution: Ensure the server is running and the socket file exists at the expected path.

### Segmentation Faults
- Cause: Using invalid file descriptors or pointers after failed operations.
- Solution: The code now checks for errors after `open()` and `connect()` before proceeding.

## Makefile Usage
- `make` — Build all binaries
- `make clean` — Remove build artifacts

## Additional Notes
- Scripts like `menu.sh` and `start_menu.sh` are for automation; ensure they have executable permissions (`chmod +x script.sh` on Linux/WSL).
- For detailed specifications, see `other/docs/`.

## Contact
For further help, review the troubleshooting section or contact the project maintainer.
