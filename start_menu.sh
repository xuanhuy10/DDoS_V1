#!/bin/bash

clear
# stty rows 24 cols 80
# stty rows 59 cols 230
#stty rows 50 cols 132
exec < /dev/tty
stop_previous() {
    local pname=$1
    pid=$(pgrep -f $pname) 
    if [ -n "$pid" ]; then
        kill $pid
        wait $pid 2>/dev/null
    else
       echo " "
    fi
}


display_logo2() {
  echo -e "******************************************************************************************************************************************"
  echo -e "***                                                                                                                                     **"
  echo -e "***                                       _    ____ ____    ____                       _             ____        __                     **"
  echo -e "***                                      / \\  / ___/ ___|  / ___| _   _ ___ _ __   ___| |_          |  _ \\  ___ / _|                    **"
  echo -e "***                                     / _ \\| |   \\___ \\  \\___ \\| | | / __| '_ \\ / _ \\ __|  _____  | | | |/ _ \\ |_                     **"
  echo -e "***                                    / ___ \\ |___ ___) |  ___) | |_| \\__ \\ | | |  __/ |_  |_____| | |_| |  __/  _|                    **"
  echo -e "***                                   /_/   \\_\\____|____/  |____/ \\__, |___/_| |_|\\___|\\__|         |____/ \\___|_|                      **"
  echo -e "***                                                               |___/                                                                 **"
  echo -e "***                                                                                                                                     **"
  echo -e "******************************************************************************************************************************************"
  echo -e "                                                                                            ***** DDoS Defender by Acronics Solutions ****"
}


# display_logo2() {
#   echo -e "\n******************************************************************************************************************************************"
#   echo -e "**                                                                                                                                      **"
#   echo -e "**\t\t\t\t ____  ____       ____        ____        __                _                                           **"
#   echo -e "**\t\t\t\t|  _ '|  _ '  ___/ ___|      |  _ '  ___ / _| ___ _ __   __| | ___ _ __                                 **"
#   echo -e "**\t\t\t\t| | | | | | |/ _ '___ '  ___ | | | |/ _ ' |_ / _ ' '_ ' / _\` |/ _ ' '__|                                **"
#   echo -e "**\t\t\t\t| |_| | |_| | (_) |_ ) )|___|| |_| | |/_/  _|  __/ | | | (_| |  __/ |                                   **"
#   echo -e "**\t\t\t\t|____/|____/ '___/____/      |____/ '___|_|  '___|_| |_|'__,_|'___|_|                                   **"
#   echo -e "**                                                                                                                                      **"
#   echo -e "******************************************************************************************************************************************"
#   echo -e "                                                                                            ***** DDoS Defender by Acronics Solutions ****"
# }

show_menu() {
 echo -e "******************************************************************************************************************************************"
  echo -e "*                                                                                                                                        *"
  echo -e "*                                                            MAIN PROGRAM MENU                                                           *"
  echo -e "*                                                                                                                                        *"
  echo -e "******************************************************************************************************************************************"
  echo -e "*                                                                                                                                        *"
  echo -e "*              Welcome to the Anti-DDoS System Control Program!                                                                          *"
  echo -e "*                                                                                                                                        *"
  echo -e "*              Please select one of the following options:                                                                               *"
  echo -e "*                                                                                                                                        *"
  echo -e "*                      1: RUN GUI                                                                                                        *"
  echo -e "*                         Launch the graphical user interface to monitor and control system activities.                                  *"
  echo -e "*                                                                                                                                        *"
  echo -e "*                      2: RUN CLI                                                                                                        *"
  echo -e "*                         Use the command-line interface to monitor and control system activities.                                       *"
  echo -e "*                                                                                                                                        *"
  echo -e "*                      3: EXIT                                                                                                           *"
  echo -e "*                         Close the program and return to the terminal.                                                                  *"
  echo -e "*                                                                                                                                        *"
  echo -e "*              For assistance, refer to the user manual or contact support.                                                              *"
  echo -e "*                                                                                                                                        *"
  echo -e "******************************************************************************************************************************************"

}

read_file_and_choose_mode() {
    if [ -f "/home/antiddos/DDoS_V1/Setting/mode.conf" ]; then
        mode=$(cat /home/antiddos/DDoS_V1/Setting/mode.conf)
        case $mode in
            1)
                choice=1
                ;;
            2)
                choice=2
                ;;
            *)
                echo "Please enter a valid choice manually."
                choice=""
                ;;
        esac
    else
        echo "File not found. Please enter a valid choice manually."
    fi
}

countdown_timer() {
    local seconds=$1
    while [ $seconds -gt 0 ]; do
        echo -ne "\r$seconds seconds...Please choice(1-3):"
        sleep 1
        ((seconds--))
    done
    echo "\n"
}

while true; do
    clear
    
    # stty rows 59 cols 230
    display_logo2
    show_menu
    echo -e "******************************************************************************************************************************************"

    countdown_timer 30 &
    read -t 30 -p "****** Please cho(1-3): " choice
    kill $! 2>/dev/null # Stop the countdown timer

    if [ -z "$choice" ]; then
        echo "No input. Loading previous mode..."
        read_file_and_choose_mode
    fi

    echo -e "******************************************************************************************************************************************"

    case $choice in
        1)
            stop_previous cli
            echo "Running GUI..."
            sleep 2
            cd /home/antiddos/DDoS_V1
            sudo ./gui
            echo " "
            echo "Switching..."
            sleep 2
            ;;
        2)
            stop_previous gui
            echo "Running CLI..."
            sleep 2
            cd /home/antiddos/DDoS_V1
            sudo ./cli
            echo " "
            echo "Switching..."
            sleep 2
            ;;
        3)
            stop_previous gui
            stop_previous cli.sh
            echo "Exiting..."
            exit 0
            ;;
        *)
            echo "Invalid choice. Please choose a valid option (1-3)."
            ;;
    esac

done
