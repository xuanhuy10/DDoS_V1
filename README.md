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
# load aliases then start menu
source ~/.bashrc
start   # or: restart
```

#### 3. Optional step-by-step (skip if already installed)

```bash
# from project directory
make help

sudo make create_folder
sudo make B1_WiringPi      # build wiringpi_3.16_arm64.deb in project
make B2_i2c                # install i2c locally into ./local
make wolfssl               # install wolfSSL locally into ./local

sudo make B7_tmux_install  # install tmux and libs
make B8_tmux_conf          # write ~/.tmux.conf

make B3_desktop            # create desktop launcher
make B4_autostart_copy     # enable autostart
make B6_chmod              # permissions
make B9_bashrc_aliases     # add aliases

source ~/.bashrc
# build commands (printed for copy/paste)
make build_cli
make build_gui
```

#### 4. Run

```bash
start    # attach to existing menu_session if running
restart  # start/refresh menu_session and attach
```
