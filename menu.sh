#!/bin/bash


SESSION_NAME="menu_session"


if ! tmux ls | grep -q menu_session; then
    tmux new-session -d -s menu_session "/bin/bash /home/antiddos/DDoS_V1/start_menu.sh"
fi

