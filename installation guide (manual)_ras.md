	**installation guide (manual)**

B1

<!-- # Tạo thư mục  -->
mkdir /home/antiddos/DDoS_V1/HTTP_ip_table
mkdir /home/antiddos/DDoS_V1/Log
mkdir /home/antiddos/DDoS_V1/Log/Log_Normal
mkdir /home/antiddos/DDoS_V1/Log/Log_Flood
mkdir /home/antiddos/DDoS_V1/Setting
mkdir /home/antiddos/DDoS_V1/Setting/certificates
sudo apt install git 
git clone https://github.com/xuanhuy10/WiringPi.git
cd WiringPi
./build debian
mv debian-template/wiringpi_3.16_arm64.deb .
sudo apt install ./wiringpi_3.16_arm64.deb


<!-- #cd ../ (home/antiddos) -->

B2
git clone https://github.com/xuanhuy10/i2c.git
cd i2c
make
sudo make install
<!-- #cd ra ngoài thư mục 
#cd  -->
B3 Tạo App :

sudo nano ~/.local/share/applications/my_script.desktop

<!-- # copy vào my_script.desktop -->

[Desktop Entry]

Version=1.0

Type=Application

Name=AntiDDoS

Exec=/bin/bash /home/antiddos/DDoS_V1/menu.sh   

Icon=/home/antiddos/DDoS_V1/icon.png

Terminal=false

Comment=Run DDoS Menu

Categories=Utility;

X-KeepTerminal=false

B4
<!-- # Tạo thư mục autostart (nếu chưa có) -->
mkdir -p ~/.config/autostart
<!-- \#copy nội dung file từ script.destop sang  autostart : -->
cp ~/.local/share/applications/my_script.desktop ~/.config/autostart/
B5
<!-- \# nếu như thực hiện xong B4 thì bỏ qua B5  -->
mkdir -p ~/.config/AutoStart
nano ~/.config/autostart/my_script.desktop
<!-- #nội dung  -->

[Desktop Entry]

Type=Application

Version=1.0

Name=AntiDDoS

Exec=/bin/bash /home/antiddos/DDoS_V1/menu.sh

Icon=/home/antiddos/DDoS_V1/icon.png

Terminal=true

Comment=Run AntiDDoS Menu at login

Categories=Utility;

X-GNOME-Autostart-enabled=true

B6

<!-- # cấp quyền thực thi  -->

chmod +x /home/antiddos/DDoS_V1/menu.sh

chmod +x ~/.config/autostart/my_script.desktop

<!-- \#check

source ~/.bashrc -->



B7

<!-- \# Cài tmux -->

sudo apt install tmux

<!-- //Cai thu vien can thiet -->

sudo apt install libncurses5-dev libncursesw5-dev

sudo apt install libcurl4-openssl-dev

sudo apt install libglib2.0-dev

B8  
<!-- \# tạo File tmux  -->
sudo nano ~/.tmux.conf
<!-- \#them noi dung duoi day vao  -->

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
set -g status-style      bg=default                                                                                                                                                           
set -g mouse on

bind -T copy-mode a send -X cancel

bind -T copy-mode b send -X cancel

bind -T copy-mode c send -X cancel

bind -T copy-mode d send -X cancel

bind -T copy-mode e send -X cancel

bind -T copy-mode f send -X cancel

bind -T copy-mode g send -X cancel

bind -T copy-mode h send -X cancel

bind -T copy-mode i send -X cancel

bind -T copy-mode j send -X cancel

bind -T copy-mode k send -X cancel

bind -T copy-mode l send -X cancel

bind -T copy-mode m send -X cancel

bind -T copy-mode n send -X cancel

bind -T copy-mode o send -X cancel

bind -T copy-mode p send -X cancel

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



B9
<!-- # sua file  -->
sudo nano ~/.bashrc
 
<!--# Thêm 2 dòng này vòa cuối file -->

alias start='stty rows 59 cols 230; sleep 2; tmux attach -t menu_session'

alias restart='tmux new-session -d -s menu_session "/bin/bash /home/antiddos/DDoS_V1/start_menu.sh"; sleep 2; stty rows 59 cols 230; sleep 2; tmux attach -t menu_session'

<!-- # ( để ý đường dẫn khi coppy dô bị thưa dấu cách, phai co dấu cách chỗ bin/bash và home mới đúng : /bin/bash/ home/antiddos/DDoS_V1/start_menu.sh ) -->

**install wolfssl**

B1 

git clone https://github.com/wolfSSL/wolfssl.git

cd wolfssl

B2
<!-- vẫn ở thư mục home/antiddos/DDoS_V1/wolfssl -->
git reset --hard

git pull

rm -rf autom4te.cache config.status config.log

B3 
<!-- nếu gặp lỗi  ./autogen.sh
./autogen.sh: 60: autoreconf: not found -->
<!-- vẫn ở thư mục home/antiddos/DDoS_V1/wolfssl -->
sudo apt update
sudo apt install autoconf automake libtool

./autogen.sh

./configure --enable-all --enable-static --enable-shared

B4
<!-- vẫn ở thư mục home/antiddos/DDoS_V1/wolfssl -->
make -j$(nproc)

sudo make install

pkg-config --modversion wolfssl
# kiểm tra thư viện

ls /usr/local/lib | grep libwolfssl


<!-- #Lệnh này liệt kê tất cả file trong thư mục /usr/local/lib rồi lọc ra những file có chứa libwolfssl nếu thấy libwolfssl.so.43 libwolfssl.a nhưng chưa nhận dieenjj được cần lệnh dưới là oke   -->
<!-- #Cập nhật đường dẫn thư viện (ldconfig)  -->
sudo ldconfig


B5
# nhớ kéo file cert, src,wolfcrypt trong wolfssl ra ngoai file wolfssl
	cp -r wolfssl/certs /home/antiddos/DDoS_V1/
	cp -r wolfssl/src /home/antiddos/DDoS_V1/
	cp -r wolfssl/wolfcrypt /home/antiddos/DDoS_V1/

<!-- ////////////////////////////////////////////////////////////////////////////////////////////////////////////////// -->

<!-- \# sau khi xong thì biên dịch   -->
<!-- biên dịch GUI : -->
gcc gui.c -o gui -li2c1602 -lwiringPi -lpthread -lncurses -lcurl `pkg-config --cflags --libs wolfssl glib-2.0` -I/usr/local/include -L/usr/local/lib -lwolfssl
<!-- Biên dịch CLI : -->
gcc cli_working.c -o cli1 -li2c1602 -lwiringPi -lpthread -lncurses -lcurl `pkg-config --cflags --libs glib-2.0` -L/usr/local/lib -lwolfssl -lcjson -lsqlite3 -ljansson