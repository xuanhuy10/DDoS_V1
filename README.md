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
