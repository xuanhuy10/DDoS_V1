    #include <stdio.h>
    #include <string.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <termios.h>
    #include <unistd.h>
    #include <time.h>
    #include <regex.h>
    #include <ctype.h>
    #include <stdlib.h>
    #include <signal.h>
    #include <i2c1602.h>
    #include <pthread.h>
    #include <wiringPi.h>
    #include <wiringPiI2C.h>
    #include <netinet/ip.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <net/ethernet.h>
    #include <net/if.h>
    #include <linux/if_packet.h>
    #include <dirent.h>
    #include <stdbool.h>
    #include <sys/stat.h>
    #include <sys/statvfs.h>
    #include <unistd.h>
    #include <ncurses.h>
    #include <curses.h>
    #include <sys/wait.h>
    #include <sys/types.h>
    #include <glib.h>
    #include <sqlite3.h>
    #include <curl/curl.h>
    #include <cjson/cJSON.h>
    #include <jansson.h>
    #include <stddef.h>
    #include <libgen.h>
    #include <string.h>

    // #include "test_attack.h"
    // #include "test_attack.c"

    #define PRINT_BUFFER_SIZE 1024
    GHashTable *hash_table = NULL;
    GQueue *queue = NULL;
    // ...existing code...
    // char print_buffer[PRINT_BUFFER_SIZE];
    // int print_buffer_pos = 0;
    // ...existing code...// ...existing code...
    // extern char print_buffer[PRINT_BUFFER_SIZE];
    // extern int print_buffer_pos;
    // ...existing code...

    ///////////////////////////////////RECONFIGURE//////////////////////////////////////////////
    typedef enum
    {
        CONFIG_NONE,
        CONFIG_SYN_THRESHOLD,
        CONFIG_ACK_THRESHOLD,
        CONFIG_UDP_THRESHOLD,
        CONFIG_UDP_1S_THRESHOLD,
        CONFIG_DNS_THRESHOLD,
        CONFIG_ICMP_THRESHOLD,
        CONFIG_ICMP_1S_THRESHOLD,
        CONFIG_IPSEC_IKE_THRESHOLD,
        CONFIG_SYN_DEFENDER,
        CONFIG_LAND_DEFENDER,
        CONFIG_UDP_DEFENDER,
        CONFIG_DNS_DEFENDER,
        CONFIG_ICMP_DEFENDER,
        CONFIG_IPSEC_IKE_DEFENDER,
        CONFIG_TCP_FRAG_DEFENDER,
        CONFIG_UDP_FRAG_DEFENDER,
        CONFIG_HTTP_DEFENDER,
        CONFIG_HTTPS_DEFENDER,
        CONFIG_PORT_DEFENDER,
        CONFIG_IPV4_PROTECT,
        CONFIG_IPV6_PROTECT,
        CONFIG_IPV4_BLOCK,
        CONFIG_IPV6_BLOCK,
        CONFIG_REMOVE_IPV4_BLOCK,
        CONFIG_REMOVE_IPV6_BLOCK,
        CONFIG_REMOVE_IPV4_PROTECT,
        CONFIG_REMOVE_IPV6_PROTECT,
        SETTING_TIME_WHITE_LIST,
        SETTING_ATTACKER_IP_TABLE,
        CONFIG_IPV4_HTTP,
        CONFIG_IPV6_HTTP,
        CONFIG_REMOVE_IPV4_HTTP,
        CONFIG_REMOVE_IPV6_HTTP,
        CONFIG_IPV4_VPN,
        CONFIG_IPV6_VPN,
        CONFIG_REMOVE_IPV6_VPN,
        CONFIG_REMOVE_IPV4_VPN
    } ConfigType;

    ConfigType current_config_type = CONFIG_NONE;

    /////////////////////////////////PORT MIRRORING//////////////////////////////////////////////
    typedef struct
    {
        char interface_name[20];
        int is_mirroring;
        int monitor_target_id; // InterfaceToMonitorInterfaceId
        char mirror_setting[20];
        char mirror_type[128];
        char value[256];
    } PortMirroringConfig;

    //////////////////////////////ADMIN MODE//////////////////////////////////////////////
    #define MAX_PATH_LENGTH 256
    typedef struct
    {
        char ca_cert[MAX_PATH_LENGTH];
        char cert_file[MAX_PATH_LENGTH];
        char key_file[MAX_PATH_LENGTH];
    } IPSecurityConfig;

    ///////////////////////////IP SECURITY//////////////////////////////////////////////
    typedef struct
    {
        char ipsec_iface[64];
        char local_gw[64];
        char remote_gw[64];
        char profile_name[64];
        char description_profile[64];
        char local_subnet[64];
        char remote_subnet[64];

        // IKEv2 Settings
        char ikev2_version[64];
        char ike_mode[64];
        char esp_ah_proto[128];
        char ike_reauth[64];
        char enc_algo[64];
        char hash_algo[64];
        char rekey_time[64];

        // Phase 1 Settings
        char p1_encryption[128];
        char p1_hash[128];
        char p1_dh_group[64];
        char p1_rekey_time[64];

        // Phase 2 Settings
        char p2_encryption[128];
        char p2_hash[128];
        char p2_dh_group[64];
        char p2_rekey_time[64];

        int user_id;      // thêm
        int interface_id; // thêm
        int connectioncount;

    } NodeConfig;
    NodeConfig config;

    // FOR em

    static void send_syn_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer);
    static void send_syn_threshold(int serial_port, int port, int value, int enable,
                                char *ID, char *Value, char *buffer);
    static void send_ack_threshold(int serial_port, int port, int value, int enable,
                                char *ID, char *Value, char *buffer);
    static void send_udp_threshold(int serial_port, int port, int value, int enable,
                                char *ID, char *Value, char *buffer);
    static void send_udp_threshold_ps(int serial_port, int port, int value, int enable,
                                    char *ID, char *Value, char *buffer);
    static void send_dns_threshold(int serial_port, int port, int value, int enable,
                                char *ID, char *Value, char *buffer);
    static void send_icmp_threshold(int serial_port, int port, int value, int enable,
                                    char *ID, char *Value, char *buffer);
    static void send_icmp_threshold_ps(int serial_port, int port, int value, int enable,
                                    char *ID, char *Value, char *buffer);
    static void send_ike_threshold(int serial_port, int port, int value, int enable,
                                char *ID, char *Value, char *buffer);
    static void send_land_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer);
    static void send_udp_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer);
    static void send_dns_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer);
    static void send_icmp_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer);
    static void send_ike_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer);
    static void send_tcpfrag_enable_disable(int serial_port, int port, int value, int enable,
                                            char *ID, char *Value, char *buffer);
    static void send_udpfrag_enable_disable(int serial_port, int port, int value, int enable,
                                            char *ID, char *Value, char *buffer);
    static void send_http_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer);
    static void send_https_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer);
    static void send_whitelist_timeout(int serial_port, int port, int value, int enable,
                                    char *ID, char *Value, char *buffer);

    //////////////////////////////GENERAL SETTING//////////////////////////////////////////////

    static int current_port = 0; // Global variable to track current port

    // Dong_bo_Port_Write
    #define key6_1 '6'   // SYN enable/disable
    #define keyC_1 'C'   // UDP threshold
    #define keyD_1 'D'   // UDP threshold per second
    #define keyB_1 'B'   // UDP enable/disable
    #define key7_1 '7'   // SYN threshold
    #define key8_1 '8'   // ACK threshold
    #define keyF_1 'F'   // DNS threshold
    #define keyH_1 'H'   // ICMP threshold
    #define keyI_1 'I'   // ICMP threshold per second
    #define keyK_1 'K'   // IPSec IKE threshold
    #define keyA_1 'A'   // LAND enable/disable
    #define keyE_1 'E'   // DNS enable/disable
    #define keyG_1 'G'   // ICMP enable/disable
    #define keyJ_1 'J'   // IPSec IKE enable/disable
    #define keyN_1 'N'   // TCP Fragment enable/disable
    #define keyO_1 'O'   // UDP Fragment enable/disable
    #define keyP_1 'P'   // HTTP enable/disable
    #define keyFF_1 0xFF // HTTPS enable/disable
    #define key9_1 '9'   // Whitelist timeout
    #define key1_1 '1'
    #define key2_1 '2'
    #define key3_1 '3'
    #define key4_1 '4'                                                                                                                                                                                                                                                                                                                
    #define key5_1 '5'
    #define keyY_1 'Y'
    #define keyN_1 'N'
    #define enter_1 '\r'

    IPSecurityConfig g_config;
    #define CONFIG_FILE_PORT "port_config.txt"
    #define AUTO_MANUAL_CONFIG_FILE "/home/antiddos/DDoS_V1/Setting/config_auto_manual.conf"
    #define DB_PATH "./GUI/server/database/sysnetdef.db"
    // // // #define DB_PATH "/home/acs/DDoS_HUY/GUI_HUY/server/database/sysnetdef.db"
    // COLOR
    #define C1 "\033[1;31m"      // RED
    #define C2 "\033[1;32m"      // GREEN
    #define C3 "\033[1;33m"      // YELLOW
    #define C4 "\033[1;34m"      // BLUE
    #define C5 "\033[1;35m"      // MAGETA
    #define C6 "\033[1;36m"      // CYAN
    #define C7 "\033[1;37m"      // WHITE
    #define C8 "\033[38;5;66m"   // LightBlue
    #define C9 "\033[38;5;110m"  // CyanBlue
    #define C10 "\033[38;5;114m" // PastelGreen
    #define C11 "\033[38;5;151m" // LightGreen
    #define C12 "\033[38;5;188m" // SoftYellow
    #define C13 "\033[38;5;145m" // LightPurple
    #define C14 "\033[38;5;250m" // LightGray
    #define C15 "\033[38;5;244m" // Gray
    #define C16 "\033[38;5;159m" // MintBlue
    #define C17 "\033[38;5;196m" // Bright Red
    #define BOLD "\x1B[1m"
    #define UNDERLINE "\x1B[4m"
    #define RE "\033[0m" // Reset
    //
    // Function prototypes
    void SetHTTPSDefender(int serial_port);
    void new_menu(int serial_port);
    void port_mirroring_menu(int serial_port);
    void AdminConfigMenu(int serial_port);
    void IP_Security_Menu(int serial_port);
    void IP_Security_Step2_Menu(int serial_port);
    void NodeSpecificSetting_Menu(int serial_port);
    void SetIPSecInterface(int serial_port);
    void SetLocalGateway(int serial_port);
    void SetRemoteGateway(int serial_port);
    void GeneralSetting_Menu(int serial_port);
    void Phase2Settings_Form(int serial_port);
    void Phase1Settings_Form(int serial_port);
    void IKEv2Settings_Form(int serial_port);
    void ChoiceCACertificationFile(int serial_port);
    void ChoiceCertificationFile(int serial_port);
    void ChoicePrivateKeyFile(int serial_port);
    void SetIPSecName(int serial_port);
    void SetIPSecDescription(int serial_port);
    void SetIKEv2Version(int serial_port);
    void SetIKEMode(int serial_port);
    void SetESP_AHProtocol(int serial_port);
    void SetIKEReauth(int serial_port);
    void SetEncryptionAlgo(int serial_port);
    void SetHashAlgo(int serial_port);
    void SetRekeyTime(int serial_port);
    void ClearInputBuffer();
    void GetCurrentTime(char *buffer, size_t size);
    void SaveIPSecProfileToDB(int serial_port);
    void ShowIPSecProfilePreview();
    void SaveIPSecProfileToFile();
    void IPSecMenu(int serial_port);
    void SetRemoteSubnet(int serial_port);
    void SetLocalSubnet(int serial_port);
    void ShowIPSecProfilePreviewFromDB();
    void Delete_and_config_ipsec_profile(int serial_port);
    void UpdateIPSecProfileInDB(int serial_port);
    void DeleteIPSecProfileFromDB(int serial_port);
    void ShowIPSecProfileFromDB();
    void AddconnectionIPSec(int serial_port);
    void Add_new_connection_IPSec(int serial_port);

    void Display_table_2(int port);
    void send_reset(int serial_port);
    void *key_listener(void *arg);
    int configure_serial_port(const char *device, int baud_rate);
    int send_data(int serial_port, const char *data, size_t size);
    char *receive_data(int serial_port);
    char *receive_data2(int serial_port);
    void ModeStart_cnt(int serial_port);
    void ModeStart(int serial_port);
    void options_mode1(int serial_port);
    int kbhit(void);
    void mode_select_login(int serial_port);
    void reconfig(int serial_port);
    void change_info_acc_admin_mode(int serial_port);
    void display_logo1();
    void display_logo2();
    void ReturnMode2(int serial_port);
    void ReturnMode2b(int serial_port);
    void ReturnMode3();
    // void send_ipv4_address(int serial_port);
    // void send_ipv6_address(int serial_port);
    // void send_ipv4_address_http_add(int serial_port , temp_ipv4_address);
    // void send_ipv6_address_http_add(int serial_port);
    // void send_ipv4_address_http_remove(int serial_port);
    // void send_ipv6_address_http_remove(int serial_port);

    // ...existing code...
    void send_ipv4_address(int serial_port, const char *ip_address);
    void send_ipv6_address(int serial_port, const char *ipv6_address);
    void send_ipv4_address_http_add(int serial_port, const char *ip_address);
    void send_ipv4_address_http_remove(int serial_port, const char *ip_address);
    void send_ipv6_address_http_add(int serial_port, const char *ipv6_address);
    void send_ipv6_address_http_remove(int serial_port, const char *ipv6_address);
    // ...existing code...
    int validate_ip_address(const char *ip_address);
    int validate_ipv6_address(const char *ip_address);
    void SaveEEPROM(int serial_port);
    void printf_uart1(int serial_port);
    int send_array(int serial_port);
    void send_duration_time(int serial_port);
    void printf_uart(int serial_port);
    void send_user_time(int serial_port);
    int validate_time_format(const char *time_string);
    void input_and_send_account(int serial_port);
    void input_and_send_account1(int serial_port);
    void input_and_send_account2(int serial_port);
    void check_account(int serial_port);
    void check_username_change_pass(int serial_port);
    void delete_account(int serial_port);
    void add_acount(int serial_port);
    void input_and_send_username(int serial_port);
    void input_and_send_password(int serial_port);
    void load_default(int serial_port);
    //
    void SetDateTime(int serial_port);
    void SetDefenderPort(int serial_port);
    void SetPortDefender(int serial_port);
    void SetIPv4Target(int serial_port);
    void Check_Table_IPv4();
    void SetIPv6Target(int serial_port);
    void SetIPv4Block(int serial_port);
    void SetIPv6Block(int serial_port);
    void RemoveIPv4Block(int serial_port);
    void RemoveIPv6Block(int serial_port);
    void SetSynDefender(int serial_port);
    void SetSynonymousDefender(int serial_port);
    void SetUDPDefender(int serial_port);
    void SetDNSDefender(int serial_port);
    void SetICMPDefender(int serial_port);
    void SetIPSecDefender(int serial_port);
    void SetTCPFragDefender(int serial_port);
    void SetUDPFragDefender(int serial_port);
    void SetHTTPDefender(int serial_port);
    void SetHTTPDefender(int serial_port);
    void set_HTTP_IP_Table(int serial_port);
    void remove_ip_HTTP_from_hash(const char *ip);
    void remove_ip_from_file(const char *filename, const char *ip);
    void display_port_mirroring_config_from_db(int serial_port, int show_prompt);
    void display_profiles(sqlite3 *db);
    void parse_using_time(const char *input, char *output, size_t output_size);
    //
    void SetTimeflood(int serial_port);
    void SetSynThresh(int serial_port);
    void SetAckThresh(int serial_port);
    void SetTimeDelete(int serial_port);
    void SetUDPThresh(int serial_port);
    void SetUDPThresh1s(int serial_port);
    void SetDNSThresh(int serial_port);
    void SetICMPThresh(int serial_port);
    void SetICMPThresh1s(int serial_port);
    void SetIPSecThresh(int serial_port);
    void AddIPv4VPN(int serial_port);
    void RemoveIPv4VPN(int serial_port);
    void AddIPv6VPN(int serial_port);
    void RemoveIPv6VPN(int serial_port);
    void SetDurationTime(int serial_port);
    void InputDestMAC(char *mac);
    void InputSourceMAC(char *mac);
    char InputDestIP(char *ip);
    void InputProtocol(int *protocol, char *protocol_str);
    void InputDestMAC(char *mac);
    char InputSourceIP(char *ip);
    void InputSourceMAC(char *mac);
    void InputDestPort(char *port);
    void InputSourcePort(char *port);
    void RemoveIPv4HTTPBlock(int serial_port);
    void RemoveIPv6HTTPBlock(int serial_port);
    void SetIPv4HTTPBlock(int serial_port);
    void SetIPv6HTTPBlock(int serial_port);

    // PORT MIRRORING
    void Delete_port_mirroring(int serial_port);
    void Update_port_mirroring(int serial_port);
    void ConfigTypePacket(int serial_port, PortMirroringConfig *cfg);
    void Select_traffic_mirroring_mode(int serial_port, PortMirroringConfig *cfg);
    void Add_port_mirroring(int serial_port);
    void save_port_mirroring_to_db(const PortMirroringConfig *cfg);
    // void display_port_mirroring_config_api(int serial_port);
    // Display
    void send_Threshold_value(int serial_port, int value, const char *label, char *ID, char *Value, char *buffer);
    void Send_enable_value(int serial_port, int enable, const char *label, char *ID, char *Value, char *buffer);

    // VPN Tables
    void Display_IPv4_vpn_table();
    void Display_IPv6_vpn_table();
    // HTTP Tables
    void Display_IP_http_table();
    void Display_http_ipv4_table();
    void Display_http_ipv6_table();
    // Blocked Tables
    void Display_IPv4_block_table();
    void Display_IPv6_block_table();
    // Protected Tables
    void Display_IPv4_Protected_Table();
    void Display_IPv6_Protected_Table();
    // VPN
    void update_vpn_ipv4(const char *ip);
    void update_vpn_ipv6(const char *ip);
    // HTTP
    void update_http_ipv4(const char *ip);
    void update_http_ipv6(const char *ip);
    // Blocked
    void update_blocked_ipv4(const char *ip, const char *port);
    void update_blocked_ipv6(const char *ip, const char *port);
    // Protected
    void update_protected_ipv4_port(const char *ip, const char *new_port);
    void update_protected_ipv6_port(const char *ip, const char *new_port);
    // VPN
    void delete_vpn_ipv4(const char *ip);
    void delete_vpn_ipv6(const char *ip);
    // HTTP
    void delete_http_ipv4(const char *ip);
    void delete_http_ipv6(const char *ip);
    // Blocked
    void delete_blocked_ipv4(const char *ip);
    void delete_blocked_ipv6(const char *ip);
    // Protected
    void delete_protected_ipv4(const char *ip);
    void delete_protected_ipv6(const char *ip);
    // SAVE CONFIGURATION
    void ConfirmAndSaveConfig(int serial_port);
    void clear_input();
    void select_multi_port_menu(int serial_port);
    void select_multi_port(int serial_port, sqlite3 *db, int profile_id);
    // void select_multi_port(int serial_port, sqlite3 *db, int profile_id);void select_multi_port(int serial_port);
    //
    void UpdateDefenseProfileField(int port, const char *field, int value);
    void SetSynThresh(int serial_port);
    void DisplayAccount(int serial_port);
    void reset_account(int serial_port);
    void change_root(int serial_port);
    void setload_default(int serial_port);
    //
    void user_mode(int serial_port);
    void admin_mode(int serial_port);
    void user_change_info(int serial_port);
    void change_user_pass(int serial_port);
    //
    void Mode_Condition_SDCard(int serial_port);
    void display_memory();
    void display_log_files(const char *dir_path);
    void create_new_log_file();
    void delete_log_file(const char *dir_path);
    void read_threshold_timecounter_from_file();
    void write_threshold_time_counter_to_file();
    void update_threshold_time_counter();
    void read_threshold_from_file();
    void write_threshold_to_file();
    void update_threshold_SDCard();
    void read_config_mode_save_logfile();
    void write_config_mode_save_logfile();
    void update_mode_auto_manual();
    void display_setting_admin();
    void display_setting_user();
    void display_setting_user1();
    void Mode_Condition_SDCard_User(int serial_port);
    void Mode_Condition_SDCard_Admin(int serial_port);
    //
    void print_payload(const unsigned char *buffer, int size);
    void process_packet(unsigned char *buffer, int size);
    void update_lcd(const char *message);
    void *lcd_thread_function(void *arg);
    void scroll_text1(const char *text1, const char *text2, int delay_ms);
    void scroll_text(const char *text, int delay_ms);
    //
    void get_current_date(char *date_str);
    void get_custom_datetime(char *date_str);
    void send_time(int serial_port);
    // void remove_old_logs(void);
    int open_and_check_dir(const char *dir_path);
    void *memory_check_thread_function(void *arg);
    void *run(void *arg);
    void previous_mode_fc();
    //
    void *packet_queue_processing_thread(void *arg);
    void enqueue_packet(unsigned char *packet, int size);
    void *logging_thread_function(void *arg);
    // void *log_buffer_thread(void *arg);
    void check_connect_eth();
    // void open_attacker_log_file();
    // void close_attacker_log_file();
    // void log_attacker_ip(const char *src_ip);
    void load_ips_from_file(const char *filename);
    void flush_batch_to_file(const char *filename);
    void create_http_filelog(const char *filename);
    //
    void display_table(int serial_port);
    void display_account(int serial_port);
    void process_ip(const char *filename, const char *ip);
    void send_http_ipv4_start(int serial_port, const char *filename);
    void send_http_ipv6_start(int serial_port, const char *filename);
    void send_data_sync_time(int serial_port);
    void send_ips_via_uart(const char *filename);
    void uart_send(const char *data, int serial_port);
    //
    #define LCD_ADDR 0x27
    #define BUFFER_SIZE 512
    #define TARGET_MAC {0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA}
    #define TARGET_MAC_ATTACK {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
    #define LOG_FLOOD_DIR "Log/Log_Flood"
    #define LOG_NORMAL_DIR "Log/Log_Normal"
    #define ATTACKER_LOG_DIR "HTTP_ip_table"
    #define LOGFILE_HTTP_IPv4 "HTTP_ip_table/http_ipv4.log"
    #define LOGFILE_HTTP_IPv6 "HTTP_ip_table/http_ipv6.log"
    #define MAX_LOG_DAYS 5
    #define SAMPLING_RATE 1

    #define KRED "\x1B[31m"
    #define KGRN "\x1B[32m"
    #define KYEL "\x1B[33m"
    #define RESET "\x1B[0m" // Reset
    //
    #define MAX_IP_LEN 42
    #define MAX_IPS 65536
    #define BATCH_SIZE 1
    #define BUFFER_SIZE_SEND_IP_VIA_UART 8192
    //
    int serial_port;
    float Threshold_SD;
    int Threshold_time_counter;
    const char *previous_mode = "/home/antiddos/DDoS_V1/Setting/mode.conf";

    const char *threshold_logfile = "/home/antiddos/DDoS_V1/Setting/threshold_logfile.conf";
    const char *time_counter = "/home/antiddos/DDoS_V1/Setting/time_counter.conf";
    #define CONFIG_FILE "/home/antiddos/DDoS_V1/Setting/config_auto_manual.conf"
    volatile bool auto_delete_logs;
    volatile int stop_scrolling = 0;
    bool reset_program = false;
    char bw1[16];
    static unsigned char prev_time[4] = {0}; // Luu th?i gian c?a g i tru?c d
    static unsigned int bw_accumulated = 0;
    char uds_msg[256];
    char name_logfile[32];
    /////////////
    static char temp_ipv4_address[16] = "";
    static char temp_ipv6_address[64] = "";
    static char full_ipv6_address[64] = "";

    //
    I2C16x2 lcd;
    unsigned char target_mac[6] = TARGET_MAC;
    unsigned char target_mac_attack[6] = TARGET_MAC_ATTACK;
    static char current_attack[255] = "";
    time_t last_packet_time;
    pthread_mutex_t log_mutex;          // Mutex for synchronizing log access
    pthread_mutex_t lcd_mutex;          // Mutex for synchronizing LCD updates
    pthread_mutex_t packet_queue_mutex; // Mutex for packet queue
    pthread_cond_t packet_queue_cond;   // Condition variable for packet queue
    pthread_t run_thread;
    pthread_mutex_t run_mutex = PTHREAD_MUTEX_INITIALIZER; // run
    // FILE *current_log_file = NULL;                         // Bien luu tru con tro logfile
    // FILE *attacker_log_file = NULL;
    FILE *current_log_file_flood = NULL;  // Bien luu tru con tro logfile
    FILE *current_log_file_normal = NULL; // Bien luu tru con tro logfile
    GHashTable *ip_table;
    GQueue *batch_queue;
    // LCD update queue

    char key_enter = '\r';
    char key_admin = '>';
    char key_check_account = '/';
    char key_show_info = '*';

    #define QUEUE_SIZE 1024
    typedef struct
    {
        char messages[QUEUE_SIZE][255];
        int front;
        int rear;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
    } MessageQueue;
    MessageQueue lcd_queue;

    // struct MemoryStruct
    // {
    //   char *memory;
    //   size_t size;
    // };

    // Packet queue
    #define PACKET_QUEUE_SIZE 4096
    typedef struct
    {
        unsigned char packets[PACKET_QUEUE_SIZE][BUFFER_SIZE];
        int front;
        int rear;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
    } PacketQueue;
    PacketQueue packet_queue;

    // Buffer for logging
    // #define LOG_BUFFER_SIZE (1024 * 42768)
    // char log_buffer[LOG_BUFFER_SIZE];
    // int log_buffer_pos = 0;

    // Buffer printf terminal
    // #define PRINT_BUFFER_SIZE 1024
    char print_buffer[PRINT_BUFFER_SIZE];

    int print_buffer_pos = 0;
    char name_logfile_flood[64];
    char name_logfile_normal[64];
    //
    bool full_sd = false;
    bool full_sd1;
    // bool full_sd2 = false;
    bool is_run_running = false;
    bool is_connected = true;
    bool show_disconnected_message = false;
    bool is_idle = false;
    bool is_idle2 = false;
    bool stop_writing;
    bool detect_attack = false;
    bool empty_log_normal = false;
    bool close_normal_log = false;
    bool close_flood_log = false;
    //
    int count_tancong = 0;
    int count_tong = 0;
    int count_bth = 0;
    //
    // bi?n luu tr?ng thái c?u hình vào database
    static char last_update_field[64] = "";
    static int last_update_value = -1;

    ////////////////////////////////////////////////////
    void display_table(int serial_port)
    {
        sleep(1);
        char key_table = '<';
        write(serial_port, &key_table, sizeof(key_table));
        usleep(100000);
        write(serial_port, &key_enter, sizeof(key_enter));
        usleep(100000);
        printf_uart(serial_port);
    }

    void display_account(int serial_port)
    {
        char key = '>';
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        write(serial_port, &key_enter, sizeof(key_enter));
        usleep(100000);
        printf_uart(serial_port);
    }

    //
    void change_user_pass(int serial_port)
    {
        char key_mode = '|';
        char enter[] = {'\r'};
        char password[17];
        int valid_input = 0;
        while (!valid_input)
        {
            printf(C6 "\r\n\t\t|" RE " " C3 "New Password: " RE);
            scanf("%16s", password);
            if (strlen(password) > 16)
            {
                printf(C3 "The account or password exceeds 16 characters. Please re-enter.\n" RE);
            }
            else
            {
                valid_input = 1;
            }
        }

        write(serial_port, &key_mode, sizeof(key_mode));
        usleep(100000);
        write(serial_port, &key_enter, sizeof(key_enter));
        usleep(200000);
        int n_password = strlen(password);
        for (int i = 0; i < n_password; i++)
        {
            usleep(50000);
            char data[2] = {password[i], '\0'};
            send_data(serial_port, data, sizeof(data) - 1);
        }
        sleep(1);
        write(serial_port, enter, sizeof(enter));
    }

    void user_change_info(int serial_port)
    {
        // char key_mode = '|';
        // write(serial_port, &key_mode, sizeof(key_mode));
        // usleep(100000);
        // write(serial_port, &key_enter, sizeof(key_enter));
        // usleep(100000);
        change_user_pass(serial_port);
    }

    void user_mode(int serial_port)
    {
    start:
        char key = 0;
        char enter = '\r';
        display_table(serial_port);
        usleep(100000);
        printf(C6 "\r\n\t\t+===========+=============================================================================" RE " " C3 "Session-User" RE " " C6 "=====================================================================================+" RE "");
        printf(C6 "\r\n\t\t|" RE " " C3 "DISPLAY" RE "   " C6 "|" RE "                                                                                                                                                                                " C6 "|" RE);
        printf(C6 "\r\n\t\t|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                                                                                                  " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "Re-configure Anti-DDoS" RE "                                                                                                                                                         " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 "Change password" RE "                                                                                                                                                                " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "3." RE "    " C6 "|" RE " " C6 "View log file status and change time counter" RE "                                                                                                                                   " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "Z." RE "    " C6 "|" RE " " C17 "Exit" RE "                                                                                                                                                                           " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C3 "\r\n   SETTING     --> Please choose Mode: " RE);

        while (1)
        {
            scanf("%c", &key);

            if (key == '1' || key == '2' || key == '4' || key == '3')
            {
                break;
            }
            if (key != '1' || key != '2' || key != '3' || key != '4')
            {
                printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Please choose Mode: " RE);
            }
        }

        if (key == '1')
        {
            sleep(1);
            reconfig(serial_port);
            system("clear");
            goto start;
        }
        else if (key == '2')
        {
            sleep(1);
            user_change_info(serial_port);
            system("clear");
            goto start;
        }
        else if (key == '3')
        {
            //   sleep(1);
            Mode_Condition_SDCard_User(serial_port);
            goto start;
        }
        else if (key == 'Z' || key == 'z')
        {
        }
    }

    void admin_mode(int serial_port)
    {
    start:
        char key = 0;
        char enter = '\r';
        display_table(serial_port);
        usleep(100000);
        printf(C6 "\r\n\t\t+===========+=============================================================================" RE " " C3 "Session-Admin" RE " " C6 "====================================================================================+" RE);
        printf(C6 "\r\n\t\t|" RE " " C3 "DISPLAY" RE "   " C6 "|" RE "                                                                                                                                                                                " C6 "|" RE);
        printf(C6 "\r\n\t\t|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                                                                                                  " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "Re-configure Anti-DDoS" RE "                                                                                                                                                         " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 "Change account information" RE "                                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "3." RE "    " C6 "|" RE " " C6 "Setting SD card and change time counter" RE "                                                                                                                                        " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "4." RE "    " C6 "|" RE " " C17 "Exit" RE "                                                                                                                                                                           " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C3 "\r\n   SETTING    | --> Please choose Mode: " RE);

        while (1)
        {
            scanf("%c", &key);
            if (key == '1' || key == '2' || key == '4' || key == '3')
            {
                break;
            }
            if (key != '1' || key != '2' || key != '3' || key != '4')
            {
                printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Please choose Mode: " RE);
            }
        }

        if (key == '1')
        {
            sleep(1);
            // reconfig(serial_port);
            reconfig(serial_port);
            system("clear");
            goto start;
        }
        else if (key == '2')
        {
            sleep(1);
            change_info_acc_admin_mode(serial_port);
            system("clear");
            goto start;
        }
        else if (key == '3')
        {
            // sleep(1);
            Mode_Condition_SDCard_Admin(serial_port);
            goto start;
        }
        else if (key == 'Z' || key == 'z')
        {
            // system("clear");
            // ModeStart(serial_port);
        }
    }

    // void display_logo1()
    // {
    //   printf("\r\n *************************************************************************************************************************************************************************************************************");
    //   printf("\r\n **                                                                                                                                                                                                         **");
    //   printf("\r\n **                                                                        ____  ____       ____        ____        __                _                                                                     **");
    //   printf("\r\n **                                                                       |  _ '|  _ '  ___/ ___|      |  _ '  ___ / _| ___ _ __   __| | ___ _ __                                                           **");
    //   printf("\r\n **                                                                       | | | | | | |/ _ '___ '  ___ | | | |/ _ ' |_ / _ ' '_ ' / _` |/ _ ' '__|                                                          **");
    //   printf("\r\n **                                                                       | |_| | |_| | (_) |_ ) )|___|| |_| | |/_/  _|  __/ | | | (_| |  __/ |                                                             **");
    //   printf("\r\n **                                                                       |____/|____/ '___/____/      |____/ '___|_|  '___|_| |_|'__,_|'___|_|                                                             **");
    //   printf("\r\n **                                                                                                                                                                                                         **");
    //   printf("\r\n *************************************************************************************************************************************************************************************************************");
    //   printf("\r\n                                                                                                                                                                ***** DDoS Defender by Acronics Solutions ****");
    // }

    void display_logo1()
    {
        printf(C6 "\r\n *************************************************************************************************************************************************************************************************************" RE);
        printf(C6 "\r\n **                                                                                                                                                                                                         " RE C6 "**" RE);
        printf(C6 "\r\n **                                                                  " C6 " _    ____ ____    ____                       _             ____        __                                                             " RE C6 "**" RE);
        printf(C6 "\r\n **                                                                  " C6 "/ \\  / ___/ ___|  / ___| _   _ ___ _ __   ___| |_          |  _ \\  ___ / _|                                                            " RE C6 "**" RE);
        printf(C6 "\r\n **                                                                 " C2 "/ _ \\| |   \\___ \\  \\___ \\| | | / __| '_ \\ / _ \\ __|  _____  | | | |/ _ \\ |_                                                             " RE C6 "**" RE);
        printf(C6 "\r\n **                                                                " C2 "/ ___ \\ |___ ___) |  ___) | |_| \\__ \\ | | |  __/ |_  |_____| | |_| |  __/  _|                                                            " RE C6 "**" RE);
        printf(C6 "\r\n **                                                               " C3 "/_/   \\_\\____|____/  |____/ \\__, |___/_| |_|\\___|\\__|         |____/ \\___|_|                                                              " RE C6 "**" RE);
        printf(C6 "\r\n **                                                                                           " C3 "|___/                                                                                                         " RE C6 "**" RE);
        printf(C6 "\r\n **                                                                                                                                                                                                         " RE C6 "**" RE);
        printf(C6 "\r\n *************************************************************************************************************************************************************************************************************" RE);
        printf(C17 "\r\n                                                                                                                                                                ***** DDoS Defender by " C3 "Acronics Solutions" RE C17 " ****\n" RE);
    }
    void ModeStart_cnt(int serial_port)
    {
        char key1;
        char key2 = '1';
        bool check = false;
        char key;
        char enter = '\r';
        for (int i = Threshold_time_counter; i >= 0; i--)
        {
            system("clear");
            display_logo1();
            printf(C6 "\r\n                                                                                                                                                                                                             " C6 "|" RE);
            printf(C6 "\r\n ================+===========+===============================================================================================================================================================================+" RE);
            printf(C6 "\r\n     " C3 "DISPLAY" RE "     " C6 "|" RE "           " C6 "|" RE "                                                                                                                                                                               " C6 "|" RE);
            printf(C6 "\r\n\t\t |" RE " " C3 "Key Enter" RE " " C6 "|" RE "                  " C3 "Mode" RE "                                                                                                                                                         " C6 "|" RE);
            printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
            printf(C6 "\r\n\t\t |" RE "     " C3 "1:" RE "    " C6 "|" RE " " C6 "Show flow information or display it automatically after" RE " " C3 "%ds" RE "                                                                                                        " C6 "|" RE,
                i);
            printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
            printf(C6 "\r\n\t\t |" RE "     " C3 "2:" RE "    " C6 "|" RE " " C6 "View current configurations" RE "                                                                                                                                                   " C6 "|" RE);
            printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
            printf(C6 "\r\n\t\t |" RE "     " C3 "3:" RE "    " C6 "|" RE " " C6 "Config Setting" RE "                                                                                                                                                                " C6 "|" RE);
            printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
            printf(C6 "\r\n ================+===========================================================================================================================================================================================+\n" RE);
            printf(C3 "\r    SETTING" RE "     " C6 "|" RE " " C3 "--> Please choose Mode: " RE);
            sleep(1);

            if (kbhit())
            {
                key = getchar();
                if (key == '1' || key == '2' || key == '3')
                {
                    break;
                }
            }
            key = '1';
        }
        // if (detect_attack)
        // {
        //   printf("\033[10B"); // Xu?ng 10 d ng
        //   printf("\033[10D"); // Qua tr i 10 c?t
        //   printf("\033[1;36m \n The attack has been detected!!!\033[0m");
        //   printf("\033[1;36m \n Return to the monitoring page? \033[0m");
        //   while (1)
        //   {
        //     scanf("%c", &key1);

        //     if (key == 'y' || key == 'Y' || key == 'n' || key == 'N')
        //     {
        //       break;
        //     }
        //     if (key != 'y' || key != 'Y' || key != 'n' || key != 'N')
        //     {
        //       printf("\r    --> Please choose : ");
        //     }
        //   }

        //   if (key == 'y' || key == 'Y')
        //   {
        //     printf_uart1(serial_port);
        //   }
        // }
        if (key == '2')
        {
            system("clear");
            display_logo1();
            mode_select_login(serial_port);
            ModeStart(serial_port);
        }
        else if (key == '3')
        {
            system("clear");
            display_logo1();
            // write(serial_port, &key_check_account, sizeof(key_check_account));
            // usleep(100000);
            // write(serial_port, &key_enter, sizeof(key_enter));
            // usleep(100000);
            check_account(serial_port);
            ModeStart(serial_port);
        }
        else if (key == '1')
        {
            system("clear");
            // display_logo1();
            printf_uart1(serial_port);
            ModeStart(serial_port);
        }
    }

    void ModeStart(int serial_port)
    {
    start:
        system("clear");
        display_logo1();
        char key;
        char enter = '\r';
        printf(C6 "\r\n                                                                                                                                                                                                             |" RE);
        printf(C6 "\r\n ================+===========+===============================================================================================================================================================================+" RE);
        printf(C6 "\r\n     " C3 "DISPLAY" RE "     " C6 "|" RE "           " C6 "|" RE "                                                                                                                                                                               " C6 "|" RE);
        printf(C6 "\r\n\t\t |" RE " " C3 "Key Enter" RE " " C6 "|" RE "                  " C3 "Mode" RE "                                                                                                                                                         " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "1:" RE "    " C6 "|" RE " " C6 "Show flow information" RE "                                                                                                                                                         " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t " RE "     " C3 "2:" RE "    " C6 "|" RE " " C6 "View current configurations" RE "                                                                                                                                                   " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "3:" RE "    " C6 "|" RE " " C6 "Config Setting" RE "                                                                                                                                                                " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n ================+===========================================================================================================================================================================================+\n" RE);

        while (1)
        {
            scanf("%c", &key);
            if (key == '1' || key == '2' || key == '3')
            {
                break;
            }
            if (key != '1' || key != '2' || key != '3')
            {
                printf(C3 "\r     SETTING" RE "     " C6 "|" RE " " C3 "--> Please choose Mode: " RE);
            }
        }

        if (key == '2')
        {
            system("clear");
            display_logo1();
            mode_select_login(serial_port);
            goto start;
        }
        else if (key == '3')
        {
            system("clear");
            display_logo1();
            // write(serial_port, &key_check_account, sizeof(key_check_account));
            // usleep(100000);
            // write(serial_port, &key_enter, sizeof(key_enter));
            // usleep(100000);
            check_account(serial_port);
            goto start;
        }
        else if (key == '1')
        {
            system("clear");
            // display_logo1();
            for (int i = 0; i < 5; i++)
            {
                // G?i h m sinh g i tin m?u, ho?c g?i process_packet v?i d? li?u m?u
            }
            printf_uart1(serial_port);
            goto start;
        }
    }

    void options_mode1(int serial_port)
    {
        // system("clear");
        display_logo1();
    start:
        char key;
        char enter = '\r';
        printf(C6 "\r\n                                                                                                                                                                                                             |" RE);
        printf(C6 "\r\n ================+===========+===============================================================================================================================================================================+" RE);
        printf(C6 "\r\n     " C3 "DISPLAY" RE "     " C6 "|" RE "           " C6 "|" RE "                                                                                                                                                                               " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE " " C3 "Key Enter" RE " " C6 "|" RE "                  " C3 "Mode" RE "                                                                                                                                                         " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "1:" RE "    " C6 "|" RE " " C6 "Return mode start" RE "                                                                                                                                                             " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "2:" RE "    " C6 "|" RE " " C6 "Continue to show flow information" RE "                                                                                                                                             " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "3:" RE "    " C6 "|" RE " " C6 "Show current configuration" RE "                                                                                                                                                    " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "4:" RE "    " C6 "|" RE " " C6 "Monitor SD card status" RE "                                                                                                                                                        " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n ----------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n ================+===========================================================================================================================================================================================+\n" RE);
        printf(C3 "\r\n     SETTING" RE "     " C6 "|" RE " " C3 "--> Please choose Mode: " RE);

        while (1)
        {
            scanf("%c", &key);
            if (key == '4')
            {
                system("clear");
                display_logo1();
                Mode_Condition_SDCard(serial_port);
                break;
            }
            else if (key == '1' || key == '2' || key == '3')
            {
                break;
            }
            if (CONFIG_IPV6_BLOCK != '1' || key != '2' || key != '3' || key != '4')
            {
                printf(C3 "\r     SETTING" RE "     " C6 "|" RE " " C3 "--> Please choose Mode: " RE);
            }
        }

        if (key == '1')
        {
            ModeStart(serial_port);
        }
        else if (key == '2')
        {
            system("clear");
            // display_logo1();
            printf_uart1(serial_port);
        }
        else if (key == '3')
        {
            system("clear");
            display_logo1();
            mode_select_login(serial_port);
            goto start;
        }
    }
    void mode_select_login(int serial_port)
    {
        display_table(serial_port);
        // sleep(1);
        char key;
        char enter = '\r';
        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n" C3 "Setting" RE "     " C6 "|" RE " " C6 "Do you want login to configure? (" C2 "Y" RE "/" C1 "N" RE ")?: " RE);
        while (1)
        {
            scanf("%c", &key);
            if (key == 'y' || key == 'Y' || key == 'n' || key == 'N')
            {
                break;
            }
            else if (key != 'y' || key != 'Y' || key != 'n' || key != 'N')
            {
                printf("\r\n" C3 "Setting" RE "     " C6 "|" RE " " C6 "Do you want login to configure? (" C2 "Y" RE "/" C1 "N" RE ")?: " RE);
            }
        }

        if (key == 'y' || key == 'Y')
        {
            system("clear");
            display_logo1();
            // write(serial_port, &key_check_account, sizeof(key_check_account));
            // usleep(100000);
            // write(serial_port, &key_enter, sizeof(key_enter));
            // usleep(100000);
            check_account(serial_port);
            sleep(1.5);
        }
        else if (key == 'n' || key == 'N')
        {
            system("clear");
            display_logo1();
        }
    }
    void IPSecMenu(int serial_port)
    {
        char key = 0;
        while (1)
        {
            system("clear");
            display_logo1();
            printf("\r\n");
            printf(C6 "==============================================================================================================================================================================================================\n" RE);
            printf(C6 "|                                                                                                                                                                                                            |\n" RE);
            printf(C6 "|" RE "" C3 "%*s%-*s" RE "" C6 "|" RE "\n" RE "",
                (200 - (int)strlen("IPSEC MENU")) / 2, "",
                (209 - (int)strlen("IPSEC MENU")) / 2 + (int)strlen("IPSEC MENU"),
                "IPSEC MENU");
            printf(C6 "|                                                                                                                                                                                                            |\n" RE);
            printf(C6 "==============================================================================================================================================================================================================\n" RE);

            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                                                                                                                  " C6 "|" RE "\n" RE);
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "Display profile IPSec" RE "                                                                                                                                                                                    " C6 "|" RE "\n" RE);
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 "New Profile IPSec" RE "                                                                                                                                                                                    " C6 "|" RE "\n" RE);
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C3 "3." RE "    " C6 "|" RE " " C6 "Update && Delete Profile IPSec" RE "                                                                                                                                                                      " C6 "|" RE "\n" RE);
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C17 "Z." RE "    " C6 "|" RE " " C17 "Exit" RE "                                                                                                                                                                      " C6 "|" RE "\n" RE);
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C3 "Please choose:  " RE);

            while (1)
            {
                scanf(" %c", &key);
                if (key == '1' || key == '2' || key == '3' || key == 'Z' || key == 'z')
                {
                    break;
                }
                if (key != '1' && key != '2' && key != '3' && key != 'Z' && key != 'z')
                {
                    printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Your choice: " RE);
                }
            }

            if (key == '1')
            {
                system("clear");
                display_logo1();
                // ShowIPSecProfilePreview();
                ShowIPSecProfilePreviewFromDB();
                // IP_Security_Menu(serial_port);
            }
            else if (key == '2')
            {
                system("clear");
                display_logo1();
                // new_menu(serial_port);
                IP_Security_Menu(serial_port);
            }
            else if (key == '3')
            {
                system("clear");
                display_logo1();

                Delete_and_config_ipsec_profile(serial_port);
                // IP_Security_Menu(serial_port);
            }
            else if (key == 'Z' || key == 'z')
            {
                system("clear");
                display_logo1();
                // ModeStart_cnt(serial_port);
                AdminConfigMenu(serial_port);
            }
            else
            {
                printf(C3 "\nInvalid choice. Press Enter to continue..." RE);
                int c;
                while ((c = getchar()) != '\n' && c != EOF)
                    ; // flush rest
                getchar();
            }
        }
    }

    void Delete_and_config_ipsec_profile(int serial_port)
    {
        char key = 0;
        while (1)
        {
            system("clear");
            display_logo1();
            printf("\r\n");
            printf(C6 "==============================================================================================================================================================================================================\n" RE);
            printf(C6 "|                                                                                                                                                                                                            |\n" RE);
            printf(C6 "|" RE "" C3 "%*s%-*s" RE "" C6 "|" RE "\n" RE "",
                (200 - (int)strlen("UPDATE & DELETE PROFILE IPSEC")) / 2, "",
                (209 - (int)strlen("UPDATE & DELETE PROFILE IPSEC")) / 2 + (int)strlen("UPDATE & DELETE PROFILE IPSEC"),
                "UPDATE & DELETE PROFILE IPSEC");
            printf(C6 "|                                                                                                                                                                                                            |\n" RE);
            printf(C6 "==============================================================================================================================================================================================================\n" RE);

            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                                                                                                                  " C6 "|" RE "\n" RE);
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "Update Profile IPSec" RE "                                                                                                                                                                                    " C6 "|" RE "\n" RE);
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 "Delete Profile IPSec" RE "                                                                                                                                                                                    " C6 "|" RE "\n" RE);
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C17 "Z." RE "    " C6 "|" RE " " C17 "Exit" RE "                                                                                                                                                                      " C6 "|" RE "\n" RE);
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C3 "Please choose:  " RE);
            while (1)
            {
                scanf(" %c", &key);
                if (key == '1' || key == '2' || key == 'Z' || key == 'z')
                {
                    break;
                }
                if (key != '1' && key != '2' && key != 'Z' && key != 'z')
                {
                    printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Your choice: " RE);
                }
            }

            if (key == '1')
            {
                system("clear");
                display_logo1();
                // IPSecMenu(serial_port);
                ShowIPSecProfileFromDB();
                UpdateIPSecProfileInDB(serial_port);
                // IP_Security_Menu(serial_port);
            }
            else if (key == '2')
            {
                system("clear");
                display_logo1();
                ShowIPSecProfileFromDB();
                DeleteIPSecProfileFromDB(serial_port);
            }
            else if (key == 'Z' || key == 'z')
            {
                system("clear");
                display_logo1();
                return;
            }
            else
            {
                printf(C3 "\nInvalid choice. Press Enter to continue..." RE);
                int c;
                while ((c = getchar()) != '\n' && c != EOF)
                    ; // flush rest
                getchar();
            }
        }
    }
    void  UpdateIPSecProfileInDB(int serial_port)
    {
        sqlite3 *db;
        char sql[1024];
        int rc;
        int id;

        printf("Enter IPSecProfileId to update: ");
        scanf("%d", &id);
        getchar(); // clear buffer

        char newName[128], newDesc[256];

        printf("Enter new ProfileName: ");
        fgets(newName, sizeof(newName), stdin);
        newName[strcspn(newName, "\n")] = 0;

        printf("Enter new ProfileDescription: ");
        fgets(newDesc, sizeof(newDesc), stdin);
        newDesc[strcspn(newDesc, "\n")] = 0;

        rc = sqlite3_open(DB_PATH, &db);
        if (rc != SQLITE_OK)
        {
            printf("Cannot open database: %s\n", sqlite3_errmsg(db));
            return;
        }

        snprintf(sql, sizeof(sql),
                "UPDATE IPSecProfiles SET ProfileName=?, ProfileDescription=?, LastModified=datetime('now') WHERE IPSecProfileId=?;");

        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        sqlite3_bind_text(stmt, 1, newName, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, newDesc, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, id);

        rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE)
        {
            printf("Profile updated successfully.\n");
        }
        else
        {
            printf("Update failed: %s\n", sqlite3_errmsg(db));
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

//     void UpdateIPSecProfileInDB()
// {
//     sqlite3 *db;
//     sqlite3_stmt *stmt;
//     char sql[2048];
//     int rc, id;

//     printf("Enter IPSecProfileId to update: ");
//     scanf("%d", &id);
//     getchar(); // clear buffer

//     // ===== Input từ user =====
//     int userId, ikeReauth, rekeyTime, enable, connCount;
//     char name[128], desc[256], localGW[64], remoteGW[64];
//     char ikeVer[16], mode[32], proto[32];
//     char encrypt[64], hash[64];
//     char subnetLocal[64], subnetRemote[64], usingTime[64];

//     printf("UserId: "); scanf("%d", &userId); getchar();
//     printf("ProfileName: "); fgets(name, sizeof(name), stdin); name[strcspn(name, "\n")] = 0;
//     printf("ProfileDescription: "); fgets(desc, sizeof(desc), stdin); desc[strcspn(desc, "\n")] = 0;
//     printf("LocalGateway: "); fgets(localGW, sizeof(localGW), stdin); localGW[strcspn(localGW, "\n")] = 0;
//     printf("RemoteGateway: "); fgets(remoteGW, sizeof(remoteGW), stdin); remoteGW[strcspn(remoteGW, "\n")] = 0;
//     printf("IKEVersion: "); fgets(ikeVer, sizeof(ikeVer), stdin); ikeVer[strcspn(ikeVer, "\n")] = 0;
//     printf("Mode: "); fgets(mode, sizeof(mode), stdin); mode[strcspn(mode, "\n")] = 0;
//     printf("ESPAHProtocol: "); fgets(proto, sizeof(proto), stdin); proto[strcspn(proto, "\n")] = 0;
//     printf("IKEReauthTime: "); scanf("%d", &ikeReauth); getchar();
//     printf("EncryptionAlgorithm: "); fgets(encrypt, sizeof(encrypt), stdin); encrypt[strcspn(encrypt, "\n")] = 0;
//     printf("HashAlgorithm: "); fgets(hash, sizeof(hash), stdin); hash[strcspn(hash, "\n")] = 0;
//     printf("ReKeyTime: "); scanf("%d", &rekeyTime); getchar();
//     printf("Enable (0/1): "); scanf("%d", &enable); getchar();
//     printf("UsingTime: "); fgets(usingTime, sizeof(usingTime), stdin); usingTime[strcspn(usingTime, "\n")] = 0;
//     printf("SubnetLocalGateway: "); fgets(subnetLocal, sizeof(subnetLocal), stdin); subnetLocal[strcspn(subnetLocal, "\n")] = 0;
//     printf("SubnetRemoteGateway: "); fgets(subnetRemote, sizeof(subnetRemote), stdin); subnetRemote[strcspn(subnetRemote, "\n")] = 0;
//     printf("ConnectionCount: "); scanf("%d", &connCount); getchar();

//     // ===== Open DB =====
//     rc = sqlite3_open(DB_PATH, &db);
//     if (rc != SQLITE_OK)
//     {
//         printf("Cannot open database: %s\n", sqlite3_errmsg(db));
//         return;
//     }

//     snprintf(sql, sizeof(sql),
//         "UPDATE IPSecProfiles SET "
//         "UserId=?, ProfileName=?, ProfileDescription=?, LocalGateway=?, RemoteGateway=?, "
//         "IKEVersion=?, Mode=?, ESPAHProtocol=?, IKEReauthTime=?, EncryptionAlgorithm=?, HashAlgorithm=?, "
//         "ReKeyTime=?, Enable=?, LastModified=datetime('now'), UsingTime=?, "
//         "SubnetLocalGateway=?, SubnetRemoteGateway=?, ConnectionCount=? "
//         "WHERE IPSecProfileId=?;");

//     rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
//     if (rc != SQLITE_OK)
//     {
//         printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
//         sqlite3_close(db);
//         return;
//     }

//     // ===== Gán giá trị (bind) =====
//     sqlite3_bind_int(stmt, 1, userId);
//     sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);
//     sqlite3_bind_text(stmt, 3, desc, -1, SQLITE_STATIC);
//     sqlite3_bind_text(stmt, 4, localGW, -1, SQLITE_STATIC);
//     sqlite3_bind_text(stmt, 5, remoteGW, -1, SQLITE_STATIC);
//     sqlite3_bind_text(stmt, 6, ikeVer, -1, SQLITE_STATIC);
//     sqlite3_bind_text(stmt, 7, mode, -1, SQLITE_STATIC);
//     sqlite3_bind_text(stmt, 8, proto, -1, SQLITE_STATIC);
//     sqlite3_bind_int(stmt, 9, ikeReauth);
//     sqlite3_bind_text(stmt, 10, encrypt, -1, SQLITE_STATIC);
//     sqlite3_bind_text(stmt, 11, hash, -1, SQLITE_STATIC);
//     sqlite3_bind_int(stmt, 12, rekeyTime);
//     sqlite3_bind_int(stmt, 13, enable);
//     sqlite3_bind_text(stmt, 14, usingTime, -1, SQLITE_STATIC);
//     sqlite3_bind_text(stmt, 15, subnetLocal, -1, SQLITE_STATIC);
//     sqlite3_bind_text(stmt, 16, subnetRemote, -1, SQLITE_STATIC);
//     sqlite3_bind_int(stmt, 17, connCount);
//     sqlite3_bind_int(stmt, 18, id);

//     rc = sqlite3_step(stmt);
//     if (rc == SQLITE_DONE)
//         printf("✅ Profile updated successfully.\n");
//     else
//         printf("❌ Update failed: %s\n", sqlite3_errmsg(db));

//     sqlite3_finalize(stmt);
//     sqlite3_close(db);
// }

    void DeleteIPSecProfileFromDB(int serial_port)
    {
        sqlite3 *db;
        char sql[256];
        int rc;
        int id;

        printf("Enter IPSecProfileId to delete: ");
        scanf("%d", &id);
        getchar(); // clear buffer

        rc = sqlite3_open(DB_PATH, &db);
        if (rc != SQLITE_OK)
        {
            printf("Cannot open database: %s\n", sqlite3_errmsg(db));
            return;
        }

        snprintf(sql, sizeof(sql), "DELETE FROM IPSecProfiles WHERE IPSecProfileId=?;");

        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        sqlite3_bind_int(stmt, 1, id);

        rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE)
        {
            printf("Profile deleted successfully.\n");
        }
        else
        {
            printf("Delete failed: %s\n", sqlite3_errmsg(db));
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    void AdminConfigMenu(int serial_port)
    {
        char key = 0;
        while (1)
        {
            system("clear");
            display_logo1();
            printf("\r\n");
            printf(C6 "==============================================================================================================================================================================================================\n" RE);
            printf(C6 "|                                                                                                                                                                                                            |\n" RE);
            printf(C6 "|" RE "" C3 "%*s%-*s" RE "" C6 "|" RE "\n" RE "",
                (200 - (int)strlen("Admin Config Menu")) / 2, "",
                (209 - (int)strlen("Admin Config Menu")) / 2 + (int)strlen("Admin Config Menu"),
                "Admin Config Menu");
            printf(C6 "|                                                                                                                                                                                                            |\n" RE);
            printf(C6 "==============================================================================================================================================================================================================\n" RE);

            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                                                                                                                  " C6 "|" RE "\n" RE);
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "IP Security" RE "                                                                                                                                                                                    " C6 "|" RE "\n" RE);
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 "Config Setting (Advanced)" RE "                                                                                                                                                                      " C6 "|" RE "\n" RE);
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C17 "Z." RE "    " C6 "|" RE " " C17 "Exit" RE "                                                                                                                                                                      " C6 "|" RE "\n" RE);
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C3 "Please choose:  " RE);

            while (1)
            {
                scanf(" %c", &key);
                if (key == '1' || key == '2' || key == 'Z' || key == 'z')
                {
                    break;
                }
                if (key != '1' && key != '2' && key != 'Z' && key != 'z')
                {
                    printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Your choice: " RE);
                }
            }

            if (key == '1')
            {
                system("clear");
                display_logo1();
                IPSecMenu(serial_port);
                // IP_Security_Menu(serial_port);
            }
            else if (key == '2')
            {
                system("clear");
                display_logo1();
                new_menu(serial_port);
            }
            else if (key == 'Z' || key == 'z')
            {
                system("clear");
                display_logo1();
                ModeStart_cnt(serial_port);
                return;
            }
            else
            {
                printf(C3 "\nInvalid choice. Press Enter to continue..." RE);
                int c;
                while ((c = getchar()) != '\n' && c != EOF)
                    ; // flush rest
                getchar();
            }
        }
    }

    // void ShowIPSecProfilePreviewFromDB()
    // {
    //     sqlite3 *db;
    //     sqlite3_stmt *stmt;
    //     int rc;

    //     rc = sqlite3_open(DB_PATH, &db);
    //     if (rc != SQLITE_OK) {
    //         printf("Cannot open database: %s\n", sqlite3_errmsg(db));
    //         return;
    //     }

    //     const char *sql = "SELECT IPSecProfileId, ProfileName, ProfileDescription, "
    //                       "LocalGateway, RemoteGateway, IKEVersion, Mode, ESPAHProtocol, "
    //                       "IKEReauthTime, EncryptionAlgorithm, HashAlgorithm, ReKeyTime, Enable "
    //                       "FROM IPSecProfiles ORDER BY IPSecProfileId;";

    //     rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    //     if (rc != SQLITE_OK) {
    //         printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    //         sqlite3_close(db);
    //         return;
    //     }

    //     printf("\n%-5s %-20s %-25s %-15s %-15s %-6s %-6s %-10s %-10s %-10s %-10s %-8s %-6s\n",
    //            "ID", "ProfileName", "Description", "LocalGW", "RemoteGW", "IKEVer",
    //            "Mode", "ESP/AH", "Reauth", "Encrypt", "Hash", "ReKey", "Enable");
    //     printf("============================================================================================================================\n");

    //     while (sqlite3_step(stmt) == SQLITE_ROW) {
    //         int id = sqlite3_column_int(stmt, 0);
    //         const char *name = (const char *)sqlite3_column_text(stmt, 1);
    //         const char *desc = (const char *)sqlite3_column_text(stmt, 2);
    //         const char *local = (const char *)sqlite3_column_text(stmt, 3);
    //         const char *remote = (const char *)sqlite3_column_text(stmt, 4);
    //         const char *ikever = (const char *)sqlite3_column_text(stmt, 5);
    //         const char *mode = (const char *)sqlite3_column_text(stmt, 6);
    //         const char *esp = (const char *)sqlite3_column_text(stmt, 7);
    //         int reauth = sqlite3_column_int(stmt, 8);
    //         const char *enc = (const char *)sqlite3_column_text(stmt, 9);
    //         const char *hash = (const char *)sqlite3_column_text(stmt, 10);
    //         int rekey = sqlite3_column_int(stmt, 11);
    //         int enable = sqlite3_column_int(stmt, 12);

    //         printf("%-5d %-20s %-25s %-15s %-15s %-6s %-6s %-10s %-10d %-10s %-10s %-8d %-6d\n",
    //                id, name ? name : "-", desc ? desc : "-", local ? local : "-", remote ? remote : "-",
    //                ikever ? ikever : "-", mode ? mode : "-", esp ? esp : "-", reauth, enc ? enc : "-",
    //                hash ? hash : "-", rekey, enable);
    //     }

    //     sqlite3_finalize(stmt);
    //     sqlite3_close(db);

    //     printf("\nPress Enter to continue...");
    //     getchar();
    // }

    // void ShowIPSecProfilePreviewFromDB()
    // {
    //     sqlite3 *db;
    //     sqlite3_stmt *stmt;
    //     int rc;

    //     rc = sqlite3_open(DB_PATH, &db);
    //     if (rc != SQLITE_OK) {
    //         printf("Cannot open database: %s\n", sqlite3_errmsg(db));
    //         return;
    //     }

    //     const char *sql = "SELECT * FROM IPSecProfiles ORDER BY IPSecProfileId;";

    //     rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    //     if (rc != SQLITE_OK) {
    //         printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    //         sqlite3_close(db);
    //         return;
    //     }

    //     int col_count = sqlite3_column_count(stmt);

    //     // In header
    //     for (int i = 0; i < col_count; i++) {
    //         const char *col_name = sqlite3_column_name(stmt, i);
    //         printf("%-15s", col_name);
    //     }
    //     printf("\n");
    //     for (int i = 0; i < col_count; i++) printf("---------------");
    //     printf("\n");

    //     // In dữ liệu
    //     while (sqlite3_step(stmt) == SQLITE_ROW) {
    //         for (int i = 0; i < col_count; i++) {
    //             const unsigned char *val = sqlite3_column_text(stmt, i);
    //             printf("%-15s", val ? (const char *)val : "-");

    //         }
    //         printf("\n");
    //     }

    //     sqlite3_finalize(stmt);
    //     sqlite3_close(db);
    //      // Xóa buffer trước khi chờ Enter
    //     int c;
    //     while ((c = getchar()) != '\n' && c != EOF) { }

    //     printf("\nPress Enter to continue...");
    //     getchar(); // Lúc này sẽ thực sự chờ Enter
    // }

    // void ShowIPSecProfilePreviewFromDB()
    // {
    //     sqlite3 *db;
    //     sqlite3_stmt *stmt;
    //     int rc;

    //     rc = sqlite3_open(DB_PATH, &db);
    //     if (rc != SQLITE_OK)
    //     {
    //         printf("Cannot open database: %s\n", sqlite3_errmsg(db));
    //         return;
    //     }

    //     const char *sql =
    //         "SELECT IPSecProfileId, ProfileName, ProfileDescription, "
    //         " LocalGateway, RemoteGateway, IKEVersion, Mode, "
    //         "ESPAHProtocol, IKEReauthTime, EncryptionAlgorithm, HashAlgorithm, "
    //         "ReKeyTime, Enable, CreateTime, LastModified, "
    //         "SubnetLocalGateway, SubnetRemoteGateway, ConnectionCount "
    //         "FROM IPSecProfiles ORDER BY IPSecProfileId;";

    //     rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    //     if (rc != SQLITE_OK)
    //     {
    //         printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    //         sqlite3_close(db);
    //         return;
    //     }

    //     while (sqlite3_step(stmt) == SQLITE_ROW)
    //     {
    //         system("clear");
    //         display_logo1();

    //         printf(C6 "\n========================= " C3 "IPSec Profile Preview" RE " =========================\n" RE);

    //         printf(C3 "Profile ID             :" RE " " C6 "%s\n" RE,
    //                sqlite3_column_text(stmt, 0) ? (const char *)sqlite3_column_text(stmt, 0) : "-");
    //         printf(C3 "Profile Name           :" RE " " C6 "%s\n" RE,
    //                sqlite3_column_text(stmt, 1) ? (const char *)sqlite3_column_text(stmt, 1) : "[ Not Set ]");
    //         printf(C3 "Description            :" RE " " C6 "%s\n" RE,
    //                sqlite3_column_text(stmt, 2) ? (const char *)sqlite3_column_text(stmt, 2) : "[ Not Set ]");
    //         printf(C3 "Local Gateway IP       :" RE " " C6 "%s\n" RE,
    //                sqlite3_column_text(stmt, 4) ? (const char *)sqlite3_column_text(stmt, 4) : "[ Not Set ]");

    //         printf(C6 "\n========================= " C3 "IKE Configuration" RE " =========================\n" RE);

    //         printf(C3 "Remote Gateway IP      :" RE " " C6 "%s\n" RE,
    //                sqlite3_column_text(stmt, 5) ? (const char *)sqlite3_column_text(stmt, 5) : "[ Not Set ]");
    //         printf(C3 "Local Subnet IP        :" RE " " C6 "%s\n" RE,
    //                sqlite3_column_text(stmt, 15) ? (const char *)sqlite3_column_text(stmt, 15) : "[ Not Set ]");
    //         printf(C3 "Remote Subnet IP       :" RE " " C6 "%s\n" RE,
    //                sqlite3_column_text(stmt, 16) ? (const char *)sqlite3_column_text(stmt, 16) : "[ Not Set ]");
    //         printf(C3 "IKEv2 Version          :" RE " " C6 "%s\n" RE,
    //                sqlite3_column_text(stmt, 6) ? (const char *)sqlite3_column_text(stmt, 6) : "[ Not Set ]");
    //         printf(C3 "IKE Mode               :" RE " " C6 "%s\n" RE,
    //                sqlite3_column_text(stmt, 7) ? (const char *)sqlite3_column_text(stmt, 7) : "[ Not Set ]");
    //         printf(C3 "ESP/AH Protocol        :" RE " " C6 "%s\n" RE,
    //                sqlite3_column_text(stmt, 8) ? (const char *)sqlite3_column_text(stmt, 8) : "[ Not Set ]");
    //         printf(C3 "IKE Reauth Time        :" RE " " C6 "%s\n" RE,
    //                sqlite3_column_text(stmt, 9) ? (const char *)sqlite3_column_text(stmt, 9) : "[ Not Set ]");
    //         printf(C3 "Encryption Algorithm   :" RE " " C6 "%s\n" RE,
    //                sqlite3_column_text(stmt, 10) ? (const char *)sqlite3_column_text(stmt, 10) : "[ Not Set ]");
    //         printf(C3 "Hash Algorithm         :" RE " " C6 "%s\n" RE,
    //                sqlite3_column_text(stmt, 11) ? (const char *)sqlite3_column_text(stmt, 11) : "[ Not Set ]");
    //         printf(C3 "Re-key Time            :" RE " " C6 "%s\n" RE,
    //                sqlite3_column_text(stmt, 12) ? (const char *)sqlite3_column_text(stmt, 12) : "[ Not Set ]");

    //         printf(C6 "==========================================================================\n" RE);

    //         printf("\nPress Enter to continue to next profile...");
    //         int c;
    //         while ((c = getchar()) != '\n' && c != EOF) {}
    //     }

    //     sqlite3_finalize(stmt);
    //     sqlite3_close(db);
    // }
    void ShowIPSecProfilePreviewFromDB()
    {
        sqlite3 *db;
        sqlite3_stmt *stmt;
        int rc;

        rc = sqlite3_open(DB_PATH, &db);
        if (rc != SQLITE_OK)
        {
            printf("Cannot open database: %s\n", sqlite3_errmsg(db));
            return;
        }

        const char *sql =
            "SELECT IPSecProfileId, UserId, ProfileName, ProfileDescription, "
            "LocalGateway, RemoteGateway, IKEVersion, Mode, "
            "ESPAHProtocol, IKEReauthTime, EncryptionAlgorithm, HashAlgorithm, "
            "ReKeyTime, Enable, CreateTime, LastModified, UsingTime, "
            "SubnetLocalGateway, SubnetRemoteGateway, ConnectionCount "
            "FROM IPSecProfiles ORDER BY IPSecProfileId;";

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        int col_count = sqlite3_column_count(stmt);

        // Tính độ rộng lớn nhất của tên cột
        int max_field_len = 0;
        for (int i = 0; i < col_count; i++)
        {
            int len = strlen(sqlite3_column_name(stmt, i));
            if (len > max_field_len)
                max_field_len = len;
        }

        // In từng profile
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            printf("+-%-*s-+-%-50s-+\n", max_field_len, "================", "==================================================");
            printf("| %-*s | %-50s |\n", max_field_len, "Field", "Value");
            printf("+-%-*s-+-%-50s-+\n", max_field_len, "================", "==================================================");

            for (int i = 0; i < col_count; i++)
            {
                const unsigned char *val = sqlite3_column_text(stmt, i);
                const char *text = val ? (const char *)val : "-";
                printf("| %-*s | %-50s |\n", max_field_len, sqlite3_column_name(stmt, i), text);
            }

            printf("+-%-*s-+-%-50s-+\n\n", max_field_len, "================", "==================================================");
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);

        // Chờ Enter
        printf("\nPress Enter to continue...");
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        getchar();
    }


    void ShowIPSecProfileFromDB()
    {
        sqlite3 *db;
        sqlite3_stmt *stmt;
        int rc;

        rc = sqlite3_open(DB_PATH, &db);
        if (rc != SQLITE_OK)
        {
            printf("Cannot open database: %s\n", sqlite3_errmsg(db));
            return;
        }

        const char *sql =
            "SELECT IPSecProfileId, ProfileName, ProfileDescription, "
            "LocalGateway, RemoteGateway, IKEVersion, Mode, "
            "ESPAHProtocol, IKEReauthTime, EncryptionAlgorithm, HashAlgorithm, "
            "ReKeyTime, Enable, CreateTime, LastModified, "
            "SubnetLocalGateway, SubnetRemoteGateway, ConnectionCount "
            "FROM IPSecProfiles ORDER BY IPSecProfileId;";

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            system("clear");
            display_logo1();

            printf(C6 "\n========================= " C3 "IPSec Profile Preview" RE " =========================\n" RE);

            printf(C3 "Profile ID             :" RE " " C6 "%s\n" RE,
                sqlite3_column_text(stmt, 0) ? (const char *)sqlite3_column_text(stmt, 0) : "-");
            printf(C3 "Profile Name           :" RE " " C6 "%s\n" RE,
                sqlite3_column_text(stmt, 1) ? (const char *)sqlite3_column_text(stmt, 1) : "[ Not Set ]");
            printf(C3 "Description            :" RE " " C6 "%s\n" RE,
                sqlite3_column_text(stmt, 2) ? (const char *)sqlite3_column_text(stmt, 2) : "[ Not Set ]");
            printf(C3 "Local Gateway IP       :" RE " " C6 "%s\n" RE,
                sqlite3_column_text(stmt, 4) ? (const char *)sqlite3_column_text(stmt, 4) : "[ Not Set ]");

            printf(C6 "\n========================= " C3 "IKE Configuration" RE " =========================\n" RE);

            printf(C3 "Remote Gateway IP      :" RE " " C6 "%s\n" RE,
                sqlite3_column_text(stmt, 5) ? (const char *)sqlite3_column_text(stmt, 5) : "[ Not Set ]");
            printf(C3 "Local Subnet IP        :" RE " " C6 "%s\n" RE,
                sqlite3_column_text(stmt, 15) ? (const char *)sqlite3_column_text(stmt, 15) : "[ Not Set ]");
            printf(C3 "Remote Subnet IP       :" RE " " C6 "%s\n" RE,
                sqlite3_column_text(stmt, 16) ? (const char *)sqlite3_column_text(stmt, 16) : "[ Not Set ]");
            printf(C3 "IKEv2 Version          :" RE " " C6 "%s\n" RE,
                sqlite3_column_text(stmt, 6) ? (const char *)sqlite3_column_text(stmt, 6) : "[ Not Set ]");
            printf(C3 "IKE Mode               :" RE " " C6 "%s\n" RE,
                sqlite3_column_text(stmt, 7) ? (const char *)sqlite3_column_text(stmt, 7) : "[ Not Set ]");
            printf(C3 "ESP/AH Protocol        :" RE " " C6 "%s\n" RE,
                sqlite3_column_text(stmt, 8) ? (const char *)sqlite3_column_text(stmt, 8) : "[ Not Set ]");
            printf(C3 "IKE Reauth Time        :" RE " " C6 "%s\n" RE,
                sqlite3_column_text(stmt, 9) ? (const char *)sqlite3_column_text(stmt, 9) : "[ Not Set ]");
            printf(C3 "Encryption Algorithm   :" RE " " C6 "%s\n" RE,
                sqlite3_column_text(stmt, 10) ? (const char *)sqlite3_column_text(stmt, 10) : "[ Not Set ]");
            printf(C3 "Hash Algorithm         :" RE " " C6 "%s\n" RE,
                sqlite3_column_text(stmt, 11) ? (const char *)sqlite3_column_text(stmt, 11) : "[ Not Set ]");
            printf(C3 "Re-key Time            :" RE " " C6 "%s\n" RE,
                sqlite3_column_text(stmt, 12) ? (const char *)sqlite3_column_text(stmt, 12) : "[ Not Set ]");

            printf(C6 "==========================================================================\n" RE);

            printf("\nPress Enter to continue to next profile...");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }


    void ShowIPSecProfilePreview()
    {
        system("clear");
        display_logo1();
        printf(C6 "\n========================= " C3 "IPSec Profile Preview" RE " =========================\n" RE);
        printf(C3 "Profile Name           :" RE " " C6 "%s\n" RE, strlen(config.profile_name) ? config.profile_name : "[ Not Set ]");
        printf(C3 "Description            :" RE " " C6 "%s\n" RE, strlen(config.description_profile) ? config.description_profile : "[ Not Set ]");
        printf(C3 "IPSec Interface        :" RE " " C6 "%s\n" RE, strlen(config.ipsec_iface) ? config.ipsec_iface : "[ Not Set ]");
        printf(C3 "Local Gateway IP       :" RE " " C6 "%s\n" RE, strlen(config.local_gw) ? config.local_gw : "[ Not Set ]");
        printf(C6 "\n========================= " C3 "IKE Configuration" RE " =========================\n" RE);
        printf(C3 "Remote Gateway IP      :" RE " " C6 "%s\n" RE, strlen(config.remote_gw) ? config.remote_gw : "[ Not Set ]");
        printf(C3 "Local Subnet IP      :" RE " " C6 "%s\n" RE, strlen(config.remote_subnet) ? config.remote_subnet : "[ Not Set ]");
        printf(C3 "Remote Subnet IP      :" RE " " C6 "%s\n" RE, strlen(config.local_subnet) ? config.local_subnet : "[ Not Set ]");
        printf(C3 "IKEv2 Version          :" RE " " C6 "%s\n" RE, strlen(config.ikev2_version) ? config.ikev2_version : "[ Not Set ]");
        printf(C3 "IKE Mode               :" RE " " C6 "%s\n" RE, strlen(config.ike_mode) ? config.ike_mode : "[ Not Set ]");
        printf(C3 "ESP/AH Protocol        :" RE " " C6 "%s\n" RE, strlen(config.esp_ah_proto) ? config.esp_ah_proto : "[ Not Set ]");
        printf(C3 "IKE Reauth Time        :" RE " " C6 "%s\n" RE, strlen(config.ike_reauth) ? config.ike_reauth : "[ Not Set ]");
        printf(C3 "Encryption Algorithm   :" RE " " C6 "%s\n" RE, strlen(config.enc_algo) ? config.enc_algo : "[ Not Set ]");
        printf(C3 "Hash Algorithm         :" RE " " C6 "%s\n" RE, strlen(config.hash_algo) ? config.hash_algo : "[ Not Set ]");
        printf(C3 "Re-key Time            :" RE " " C6 "%s\n" RE, strlen(config.rekey_time) ? config.rekey_time : "[ Not Set ]");
        printf(C6 "==========================================================================\n" RE);
    }

    void IP_Security_Menu(int serial_port)
    {

        char key;
        char path[256];
        while (1)
        {
            system("clear");
            display_logo1();
            printf("\r\n");
            printf(C6 "==============================================================================" RE " " C3 "IP Security (Step 1: CERTIFICATION)" RE "" C6 "============================================================================================" RE "\n");

            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C3 " CA Certification File :" RE " " C6 "%s\n" RE "",
                strlen(g_config.ca_cert) ? g_config.ca_cert : "[ Not Set ]");
            printf(C3 " Certification File    :" RE " " C6 "%s\n" RE "",
                strlen(g_config.cert_file) ? g_config.cert_file : "[ Not Set ]");
            printf(C3 " Private Key File      :" RE " " C6 "%s\n" RE "",
                strlen(g_config.key_file) ? g_config.key_file : "[ Not Set ]");
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                                                                                                                  " C6 "|" RE "\n");
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "Import CA  File" RE "                                                                                                                                                                   " C6 "|" RE "\n");
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 "Import Certification File" RE "                                                                                                                                                                      " C6 "|" RE "\n");
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C3 "3." RE "    " C6 "|" RE " " C6 "Import Private Key File" RE "                                                                                                                                                                        " C6 "|" RE "\n");
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C3 "S." RE "    " C6 "|" RE " " C3 "Save" RE "                                                                                                                                                                                           " C6 "|" RE "\n");
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            // printf(C6 "|" RE "     " C2 "N." RE "    " C6 "|" RE " " C2 "Next" RE "                                                                                                                                                                                           " C6 "|" RE "\n");
            // printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C17 "Z." RE "    " C6 "|" RE " " C17 "Exit" RE "                                                                                                                                                                                           " C6 "|" RE "\n");
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

            printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Your choice: " RE);
            while (1)
            {
                int c;
                while ((c = getchar()) != '\n' && c != EOF)
                    ; // d?n s?ch stdin
                scanf(" %c", &key);
                if (key == '1' || key == '2' || key == '3' || key == 'S' || key == 's' || key == 'Z' || key == 'z')
                {
                    break;
                }
                if (key != '1' && key != '2' && key != '3' && key != 'S' && key != 's' && key != 'Z' && key != 'z')
                {
                    printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Your choice: " RE);
                }
            }
            if (key == '1')
            {
                system("clear");
                display_logo1();
                ChoiceCACertificationFile(serial_port);
                // ChoiceCACertificationFile(int serial_port);
            }
            else if (key == '2')
            {
                system("clear");
                display_logo1();
                ChoiceCertificationFile(serial_port);
                // ChoiceCertificationFile(int serial_port);
            }
            else if (key == '3')
            {
                system("clear");
                display_logo1();
                ChoicePrivateKeyFile(serial_port);
                // ChoicePrivateKeyFile(int serial_port);
            }
            else if (key == 'S' || key == 's')
            {
                if (strlen(g_config.ca_cert) == 0 ||
                    strlen(g_config.ca_cert) == 0 ||
                    strlen(g_config.key_file) == 0)
                {
                    printf("error: you must import CA CE prive file ");
                    usleep(10000);
                }

                else
                {
                    printf(C2 "Saving...\n" RE);
                    // IP_Security_Step2_Menu(serial_port);
                    AddconnectionIPSec(serial_port);
                    usleep(10000);
                }
            }
            // else if (key == 'N' || key == 'n')
            // {
            //     printf(C2 "Going to Step 2...\n" RE);
            //     IP_Security_Step2_Menu(serial_port);
            //     sleep(1);

            //     break;
            // }
            else if (key == 'Z' || key == 'z')
            {
                printf(C3 "Back to previous menu...\n" RE);
                sleep(1);
                AdminConfigMenu(serial_port);
                break;
            }
        }
    }

    void ChoiceCACertificationFile(int serial_port)
    {
        system("clear");
        display_logo1();
        char path[MAX_PATH_LENGTH] = {0};
        char input[64];
        int index = 0;
        char files[50][MAX_PATH_LENGTH]; // luu t?i da 50 file tìm th?y
        char key;

    Start:
        system("clear");
        display_logo1();
        printf("\r\n");
        printf(C6 "==================================================================================================================" RE " " C3 " Import CA File " RE " " C6 "==================================================================================================" RE "\n\n");
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                                                                                                                  " C6 "|" RE "\n" RE);
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 " Enter path manually " RE "                                                                                                                                                                          " C6 "|" RE "\n" RE);
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 " Browse current directory " RE "                                                                                                                                                                     " C6 "|" RE "\n" RE);
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE "     " C17 "Z." RE "   " C6 "|" RE " " C17 "Exit" RE "                                                                                                                                                                                          " C6 "|" RE "\n" RE);
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C3 "Please choose:  " RE);

        while (1)
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ; // d?n s?ch stdin
            scanf(" %c", &key);
            if (key == '1' || key == '2' || key == 'Z' || key == 'z')
                break;

            printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Your choice: " RE);
        }

        if (key == 'Z' || key == 'z')
        {
            printf("Returning to previous menu...\n");
            usleep(10000);
            return; // thoát h?n menu
        }

        if (key == '1')
        {
            while (1)
            {
                printf("Please enter the path to the CA Certificate file (.der) or press Z to go back: ");
                scanf("%s", input);

                if (strcmp(input, "z") == 0 || strcmp(input, "Z") == 0)
                    goto Start;

                strncpy(path, input, MAX_PATH_LENGTH);

                if (strstr(path, ".der") != NULL)
                {
                    strncpy(g_config.ca_cert, path, MAX_PATH_LENGTH);
                    printf("CA Certificate file set to: %s\n", g_config.ca_cert);
                    usleep(10000);
                    return;
                }
                else
                {
                    printf("Invalid file format! Must be .der\n");
                    usleep(10000);
                }
            }
        }
        else if (key == '2')
        {
            // Browse files
            while (1)
            {
                index = 0;
                DIR *d;
                struct dirent *dir;
                d = opendir(".");
                if (d)
                {
                    printf("\nAvailable .der files in current directory:\n");
                    while ((dir = readdir(d)) != NULL)
                    {
                        if (strstr(dir->d_name, ".der") != NULL)
                        {
                            printf("%d. %s\n", index + 1, dir->d_name);
                            strncpy(files[index], dir->d_name, MAX_PATH_LENGTH);
                            index++;
                        }
                    }
                    closedir(d);
                }

                if (index == 0)
                {
                    printf("No .der files found! Please enter manually (or press Z to go back): ");
                    scanf("%s", input);
                    if (strcmp(input, "z") == 0 || strcmp(input, "Z") == 0)
                        goto Start;

                    strncpy(path, input, MAX_PATH_LENGTH);
                    if (strstr(path, ".der") != NULL)
                    {
                        strncpy(g_config.ca_cert, path, MAX_PATH_LENGTH);
                        printf("CA Certificate file set to: %s\n", g_config.ca_cert);
                        usleep(10000);
                        return;
                    }
                    else
                    {
                        printf("Invalid file format! Must be .der\n");
                        usleep(10000);
                    }
                }
                else
                {
                    printf("Enter number (1-%d) to select file or Z to go back: ", index);
                    scanf("%s", input);
                    if (strcmp(input, "z") == 0 || strcmp(input, "Z") == 0)
                        goto Start;

                    int sel = atoi(input);
                    if (sel >= 1 && sel <= index)
                    {
                        strncpy(path, files[sel - 1], MAX_PATH_LENGTH);
                        strncpy(g_config.ca_cert, path, MAX_PATH_LENGTH);
                        printf("CA Certificate file set to: %s\n", g_config.ca_cert);
                        usleep(10000);
                        return;
                    }
                    else
                    {
                        printf("Invalid selection.\n");
                        usleep(10000);
                    }
                }
            }
        }
    }

    void ChoiceCertificationFile(int serial_port)
    {
        system("clear");
        display_logo1();
        char path[MAX_PATH_LENGTH] = {0};
        char input[64];
        int index = 0;
        char files[50][MAX_PATH_LENGTH]; // luu t?i da 50 file tìm th?y
        char key;

    Start:
        system("clear");
        display_logo1();
        printf("\r\n");
        printf(C6 "==================================================================================================================" RE " " C3 " Import CE File " RE " " C6 "==================================================================================================" RE "\n\n");
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                                                                                                                  " C6 "|" RE "\n" RE);
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 " Enter path manually " RE "                                                                                                                                                                          " C6 "|" RE "\n" RE);
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 " Browse current directory " RE "                                                                                                                                                                     " C6 "|" RE "\n" RE);
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE "     " C17 "Z." RE "   " C6 "|" RE " " C17 "Exit" RE "                                                                                                                                                                                          " C6 "|" RE "\n" RE);
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C3 "Please choose:  " RE);

        while (1)
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ; // d?n s?ch stdin
            scanf(" %c", &key);
            if (key == '1' || key == '2' || key == 'Z' || key == 'z')
                break;

            printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Your choice: " RE);
        }

        if (key == 'Z' || key == 'z')
        {
            printf("Returning to previous menu...\n");
            usleep(10000);
            return; // thoát h?n menu
        }

        if (key == '1')
        {
            while (1)
            {
                printf("Please enter the path to the CA Certificate file (.der) or press Z to go back: ");
                scanf("%s", input);

                if (strcmp(input, "z") == 0 || strcmp(input, "Z") == 0)
                    goto Start;

                strncpy(path, input, MAX_PATH_LENGTH);

                if (strstr(path, ".der") != NULL)
                {
                    strncpy(g_config.cert_file, path, MAX_PATH_LENGTH);
                    printf("CA Certificate file set to: %s\n", g_config.cert_file);
                    usleep(10000);
                    return;
                }
                else
                {
                    printf("Invalid file format! Must be .der\n");
                    usleep(10000);
                }
            }
        }
        else if (key == '2')
        {
            // Browse files
            while (1)
            {
                index = 0;
                DIR *d;
                struct dirent *dir;
                d = opendir(".");
                if (d)
                {
                    printf("\nAvailable .der files in current directory:\n");
                    while ((dir = readdir(d)) != NULL)
                    {
                        if (strstr(dir->d_name, ".der") != NULL)
                        {
                            printf("%d. %s\n", index + 1, dir->d_name);
                            strncpy(files[index], dir->d_name, MAX_PATH_LENGTH);
                            index++;
                        }
                    }
                    closedir(d);
                }

                if (index == 0)
                {
                    printf("No .der files found! Please enter manually (or press Z to go back): ");
                    scanf("%s", input);
                    if (strcmp(input, "z") == 0 || strcmp(input, "Z") == 0)
                        goto Start;

                    strncpy(path, input, MAX_PATH_LENGTH);
                    if (strstr(path, ".der") != NULL)
                    {
                        strncpy(g_config.cert_file, path, MAX_PATH_LENGTH);
                        printf("CA Certificate file set to: %s\n", g_config.cert_file);
                        usleep(10000);
                        return;
                    }
                    else
                    {
                        printf("Invalid file format! Must be .der\n");
                        usleep(10000);
                    }
                }
                else
                {
                    printf("Enter number (1-%d) to select file or Z to go back: ", index);
                    scanf("%s", input);
                    if (strcmp(input, "z") == 0 || strcmp(input, "Z") == 0)
                        goto Start;

                    int sel = atoi(input);
                    if (sel >= 1 && sel <= index)
                    {
                        strncpy(path, files[sel - 1], MAX_PATH_LENGTH);
                        strncpy(g_config.cert_file, path, MAX_PATH_LENGTH);
                        printf("CA Certificate file set to: %s\n", g_config.cert_file);
                        usleep(10000);
                        return;
                    }
                    else
                    {
                        printf("Invalid selection.\n");
                        usleep(10000);
                    }
                }
            }
        }
    }

    void ChoicePrivateKeyFile(int serial_port)
    {

        system("clear");
        display_logo1();
        char path[MAX_PATH_LENGTH] = {0};
        char input[64];
        int index = 0;
        char files[50][MAX_PATH_LENGTH]; // luu t?i da 50 file tìm th?y
        char key;

    Start:
        system("clear");
        display_logo1();
        printf("\r\n");
        printf(C6 "==================================================================================================================" RE " " C3 " Import Private File " RE " " C6 "==================================================================================================" RE "\n\n");
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                                                                                                                  " C6 "|" RE "\n" RE);
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 " Enter path manually " RE "                                                                                                                                                                          " C6 "|" RE "\n" RE);
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 " Browse current directory " RE "                                                                                                                                                                     " C6 "|" RE "\n" RE);
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE "     " C17 "Z." RE "   " C6 "|" RE " " C17 "Exit" RE "                                                                                                                                                                                          " C6 "|" RE "\n" RE);
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C3 "Please choose:  " RE);

        while (1)
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ; // d?n s?ch stdin
            scanf(" %c", &key);
            if (key == '1' || key == '2' || key == 'Z' || key == 'z')
                break;

            printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Your choice: " RE);
        }

        if (key == 'Z' || key == 'z')
        {
            printf("Returning to previous menu...\n");
            usleep(10000);
            return; // thoát h?n menu
        }

        if (key == '1')
        {
            while (1)
            {
                printf("Please enter the path to the CA Certificate file (.der) or press Z to go back: ");
                scanf("%s", input);

                if (strcmp(input, "z") == 0 || strcmp(input, "Z") == 0)
                    goto Start;

                strncpy(path, input, MAX_PATH_LENGTH);

                if (strstr(path, ".der") != NULL)
                {
                    strncpy(g_config.key_file, path, MAX_PATH_LENGTH);
                    printf("CA Certificate file set to: %s\n", g_config.key_file);
                    usleep(10000);
                    return;
                }
                else
                {
                    printf("Invalid file format! Must be .der\n");
                    usleep(10000);
                }
            }
        }
        else if (key == '2')
        {
            // Browse files
            while (1)
            {
                index = 0;
                DIR *d;
                struct dirent *dir;
                d = opendir(".");
                if (d)
                {
                    printf("\nAvailable .der files in current directory:\n");
                    while ((dir = readdir(d)) != NULL)
                    {
                        if (strstr(dir->d_name, ".der") != NULL)
                        {
                            printf("%d. %s\n", index + 1, dir->d_name);
                            strncpy(files[index], dir->d_name, MAX_PATH_LENGTH);
                            index++;
                        }
                    }
                    closedir(d);
                }

                if (index == 0)
                {
                    printf("No .der files found! Please enter manually (or press Z to go back): ");
                    scanf("%s", input);
                    if (strcmp(input, "z") == 0 || strcmp(input, "Z") == 0)
                        goto Start;

                    strncpy(path, input, MAX_PATH_LENGTH);
                    if (strstr(path, ".der") != NULL)
                    {
                        strncpy(g_config.key_file, path, MAX_PATH_LENGTH);
                        printf("CA Certificate file set to: %s\n", g_config.key_file);
                        usleep(10000);
                        return;
                    }
                    else
                    {
                        printf("Invalid file format! Must be .der\n");
                        usleep(10000);
                    }
                }
                else
                {
                    printf("Enter number (1-%d) to select file or Z to go back: ", index);
                    scanf("%s", input);
                    if (strcmp(input, "z") == 0 || strcmp(input, "Z") == 0)
                        goto Start;

                    int sel = atoi(input);
                    if (sel >= 1 && sel <= index)
                    {
                        strncpy(path, files[sel - 1], MAX_PATH_LENGTH);
                        strncpy(g_config.key_file, path, MAX_PATH_LENGTH);
                        printf("CA Certificate file set to: %s\n", g_config.key_file);
                        usleep(10000);
                        return;
                    }
                    else
                    {
                        printf("Invalid selection.\n");
                        usleep(10000);
                    }
                }
            }
        }
    }
    void AddconnectionIPSec(int serial_port)
    {
        char key;
        while (1)
        {
            system("clear");
            display_logo1();
            printf("\r\n");
            printf(C6 "============================================================================================" RE " " C3 "Add Connection IPSec" RE " " C6 "====================================================================================" RE "\n\n");

            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                                                                                                                  " C6 "|" RE "\n");
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

            printf(C6 "|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "Add new IPSec Profile" RE "                                                                                                                                                                          " C6 "|" RE "\n");
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            // printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

            printf(C6 "|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "Add new connection in IPSec Profile" RE "                                                                                                                         " C6 "|" RE "\n");
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE "     " C17 "Z." RE "    " C6 "|" RE " " C17 "Exit" RE "                                                                                                                                                                                           " C6 "|" RE "\n");
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

            printf(C3 "Please choose:  " RE);

            scanf(" %c", &key);
            switch (key)
            {
            case '1':
                // IP_Security_Menu(serial_port);
                IP_Security_Step2_Menu(serial_port);
                break;
            case '2':
                // IP_Security_Step2_Menu(serial_port);
                Add_new_connection_IPSec(serial_port);
                break;
            case 'Z':
            case 'z':
                AdminConfigMenu(serial_port);
                return;
            default:
                printf(C3 "Invalid choice!\n" RE);
                break;
            }
        }
    }

    void Add_new_connection_IPSec(int serial_port)
    {
    }

    void IP_Security_Step2_Menu(int serial_port)
    {
        char key;
        while (1)
        {
            system("clear");
            display_logo1();
            printf("\r\n");
            printf(C6 "============================================================================================" RE " " C3 "IP Security Setting (Step 2)" RE " " C6 "====================================================================================" RE "\n\n");

            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            printf(C6 "|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                                                                                                                  " C6 "|" RE "\n");
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

            printf(C6 "|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "Node Specific Setting" RE "                                                                                                                                                                          " C6 "|" RE "\n");
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

            printf(C6 "|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 "IKEv2 Setting" RE "                                                                                                                                                                                " C6 "|" RE "\n");
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

            // printf(C6 "|" RE "     " C2 "N." RE "    " C6 "|" RE " " C2 "Next" RE "                                                                                                                                                                                           " C6 "|" RE "\n");
            // printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

            printf(C6 "|" RE "     " C17 "Z." RE "    " C6 "|" RE " " C17 "Exit" RE "                                                                                                                                                                                           " C6 "|" RE "\n");
            printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

            printf(C3 "Please choose:  " RE);

            scanf(" %c", &key);
            switch (key)
            {
            case '1':
                NodeSpecificSetting_Menu(serial_port);
                break;
            case '2':
                IKEv2Settings_Form(serial_port);
                break;
            case 'Z':
            case 'z':
                IP_Security_Menu(serial_port);
                return;
            default:
                printf(C3 "Invalid choice!\n" RE);
                break;
            }
        }
    }
    void ClearInputBuffer()
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }

    void NodeSpecificSetting_Menu(int serial_port)
    {
    start:
        char key;
        system("clear");
        display_logo1();

        // Title
        printf("\r\n");
        printf(C6 "=============================================================================================== " C3 "Node Specific Setting" RE " " C6 "========================================================================================" RE "\n\n");

        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C3 "Interface Name           :" RE " " C6 "%s\n" RE "",
            strlen(config.profile_name) ? config.profile_name : "[ Not Set ]");
        printf(C3 "Description Profile      :" RE " " C6 "%s\n" RE "",
            strlen(config.description_profile) ? config.description_profile : "[ Not Set ]");
        printf(C3 "IPSec Interface          :" RE " " C6 "%s\n" RE "",
            strlen(config.ipsec_iface) ? config.ipsec_iface : "[ Not Set ]");
        printf(C3 "Local Gateway IP Address :" RE " " C6 "%s\n" RE "",
            strlen(config.local_gw) ? config.local_gw : "[ Not Set ]");
        printf(C3 "Remote Gateway IP Address:" RE " " C6 "%s\n" RE "",
            strlen(config.remote_gw) ? config.remote_gw : "[ Not Set ]");
        printf(C3 "Local Subnet IP Address:" RE " " C6 "%s\n" RE "",
            strlen(config.local_subnet) ? config.local_subnet : " [ Not Set ]");
        printf(C3 "Remote Gateway IP Address:" RE " " C6 "%s\n" RE "",
            strlen(config.remote_subnet) ? config.remote_subnet : "[ Not Set ]");
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

        // Menu
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                                                                                                                  " C6 "|" RE "\n");
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

        printf(C6 "|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "Set IPSec Profile name" RE "                                                                                                                                                                            " C6 "|" RE "\n");
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

        printf(C6 "|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 "Set IPSec Profile Description" RE "                                                                                                                                                                   " C6 "|" RE "\n");
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE "     " C3 "3." RE "    " C6 "|" RE " " C6 "Set IPSec Interface" RE "                                                                                                                                                                            " C6 "|" RE "\n");
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

        printf(C6 "|" RE "     " C3 "4." RE "    " C6 "|" RE " " C6 "Set Local Gateway IP Address" RE "                                                                                                                                                                   " C6 "|" RE "\n");
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE "     " C3 "5." RE "    " C6 "|" RE " " C6 "Set Remote Gateway IP Address" RE "                                                                                                                                                                  " C6 "|" RE "\n");
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE "     " C3 "6." RE "    " C6 "|" RE " " C6 "Set Local Subnet IP Address" RE "                                                                                                                                                                   " C6 "|" RE "\n");
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE "     " C3 "7." RE "    " C6 "|" RE " " C6 "Set Remote Subnet IP Address" RE "                                                                                                                                                                  " C6 "|" RE "\n");
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

        printf(C6 "|" RE "     " C2 "N." RE "    " C6 "|" RE " " C2 "Next" RE "                                                                                                                                                                                           " C6 "|" RE "\n");
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

        printf(C6 "|" RE "     " C17 "B." RE "    " C6 "|" RE " " C17 "Back" RE "                                                                                                                                                                                           " C6 "|" RE "\n");
        printf(C6 "+-----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

        printf(C3 "Please choose:  " RE);

        while (1)
        {
            if (scanf(" %c", &key) != 1)
            {
                ClearInputBuffer();
                continue;
            }
            if (key == '1' || key == '2' || key == '3' ||
                key == '4' || key == '5' || key == '6' || key == '7' || key == 'N' || key == 'n' ||
                key == 'B' || key == 'b')
            {
                break;
            }
            if (key != '1' && key != '2' && key != '3' &&
                key != '4' && key != '5' && key != '6' && key != '7' && key != 'N' && key != 'n' &&
                key != 'B' && key != 'b')
            {
                printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Please choose Mode: " RE);
            }
        }
        if (key == '1')
        {
            system("clear");
            display_logo1();
            SetIPSecName(serial_port);
            goto start;
            // SetIPSecInterface(serial_port);
        }
        else if (key == '2')
        {
            system("clear");
            display_logo1();
            SetIPSecDescription(serial_port);
            goto start;
            // SetLocalGateway(serial_port);
        }
        else if (key == '3')
        {
            system("clear");
            display_logo1();
            SetIPSecInterface(serial_port);
            goto start;
        }
        else if (key == '4')
        {
            system("clear");
            display_logo1();
            SetLocalGateway(serial_port);
            goto start;
        }
        else if (key == '5')
        {
            system("clear");
            display_logo1();
            SetRemoteGateway(serial_port);
            goto start;
        }
        else if (key == '6')
        {
            system("clear");
            display_logo1();
            SetLocalSubnet(serial_port);
            goto start;
        }
        else if (key == '7')
        {
            system("clear");
            display_logo1();
            SetRemoteSubnet(serial_port);
            goto start;
        }
        else if (key == 'N' || key == 'n')
        {
            IKEv2Settings_Form(serial_port);
        }
        else if (key == 'B' || key == 'b')
        {
            IP_Security_Step2_Menu(serial_port);
            // TODO: copy file vào /etc/myapp/certs ho?c ch? luu tên
            sleep(1);
        }
    }

    void SetIPSecDescription(int serial_port)
    {
        system("clear");
        display_logo1();
        char iface[50];
        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf("\r" C6 "|" RE "    " C3 "SETTING" RE "    " C6 "|" RE " " C6 "Set IPSec Name" RE "                                                                                                                                                                         " C6 "|" RE "\n");
        printf(C6 "==============================================================================================================================================================================================================\n" RE);

        printf(C3 " Please enter IPSec Interface or Press" RE " " C2 "Z" RE " " C3 "to cancel : " RE);
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
        if (fgets(iface, sizeof(iface), stdin) != NULL)
        {
            iface[strcspn(iface, "\n")] = 0;

            if ((strlen(iface) == 1) && (iface[0] == 'Z' || iface[0] == 'z'))
            {
                printf(C3 " Cancelled (pressed Y). Returning to menu...\n" RE);
                return;
            }
            else if (strlen(iface) == 0)
            {
                strcpy(config.description_profile, "Sample description");
            }
            else
            {
                strcpy(config.description_profile, iface);
            }
        }

        printf(C2 " Press Enter to go Back..." RE);
        getchar();
    }

    void SetIPSecName(int serial_port)
    {
        system("clear");
        display_logo1();
        char iface[50];
        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf("\r" C6 "|" RE "    " C3 "SETTING" RE "    " C6 "|" RE " " C6 "Set IPSec Name" RE "                                                                                                                                                                         " C6 "|" RE "\n");
        printf(C6 "==============================================================================================================================================================================================================\n" RE);

        printf(C3 " Please enter IPSec Interface or Press" RE " " C2 "Z" RE " " C3 "to cancel : " RE);
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ; // xáo bộ nhớ tạm
        if (fgets(iface, sizeof(iface), stdin) != NULL)
        {
            iface[strcspn(iface, "\n")] = 0;

            if ((strlen(iface) == 1) && (iface[0] == 'Z' || iface[0] == 'z'))
            {
                printf(C3 " Cancelled (pressed Y). Returning to menu...\n" RE);
                return;
            }
            else if (strlen(iface) == 0)
            {

                strcpy(config.profile_name, "IpSec Profile 0");
            }
            else
            {
                strcpy(config.profile_name, iface);
            }
        }
        printf(C2 " Press Enter to go Back..." RE);
        getchar();
    }

    void SetIPSecInterface(int serial_port)
    {
        system("clear");
        display_logo1();
        char iface[50];

        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf("\r" C6 "|" RE "    " C3 "SETTING" RE "    " C6 "|" RE " " C6 "Set IPSec Interface" RE "                                                                                                                                                                         " C6 "|" RE "\n");
        printf(C6 "==============================================================================================================================================================================================================\n" RE);

        printf(C3 " Please enter IPSec Interface or Press" RE " " C2 "Z" RE " " C3 "to cancel\n" RE);
        // Clear buffer
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;

        if (fgets(iface, sizeof(iface), stdin) != NULL)
        {
            iface[strcspn(iface, "\n")] = 0;

            if ((strlen(iface) == 1) && (iface[0] == 'Z' || iface[0] == 'z'))
            {
                // printf(C3 " Cancelled (pressed Z). Returning to menu...\n" RE);
                return;
            }
            else if (strlen(iface) == 0)
            {
                strcpy(config.ipsec_iface, "eth1");
            }
            else
            {
                strcpy(config.ipsec_iface, iface);
            }
        }

        printf(C2 " Press Enter to go Back..." RE);
        getchar();
    }

    // Hàm Set Local Gateway IP Address
    void SetLocalGateway(int serial_port)
    {
        system("clear");
        display_logo1();
        char localIP[50];

        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf("\r" C6 "|" RE "    " C3 "SETTING" RE "    " C6 "|" RE " " C6 "Set Local Gateway IP Address" RE "                                                                                                                                                                " C6 "|" RE "\n");
        printf(C6 "==============================================================================================================================================================================================================\n" RE);

        printf(C3 " Please enter Local Gateway IP or Press" RE " " C2 "Z" RE " " C3 "to cancel\n" RE);

        // Clear buffer
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;

        if (fgets(localIP, sizeof(localIP), stdin) != NULL)
        {
            localIP[strcspn(localIP, "\n")] = 0;
            if ((strlen(localIP) == 1) && (localIP[0] == 'Z' || localIP[0] == 'z'))
            {
                // printf(C3 " Cancelled (pressed Y). Returning to menu...\n" RE);
                return;
            }
            else if (strlen(localIP) == 0)
            {
                strcpy(config.local_gw, "1.1.1.1");
            }
            else if (validate_ip_address(localIP))
            {
                strcpy(config.local_gw, localIP);
            }
            else
            {
                printf(C3 " Invalid IP Address format! Please try again.\n" RE);
                usleep(1000000); // Pause for 1 second to let user read the message
                // printf(C3 " Please enter Local Gateway IP or Press" RE " " C2 "Z" RE " " C3 "to cancel\n" RE);
            }
        }

        // printf(C2 " Press Enter to go Back..." RE);
        // getchar();
    }

    // Hàm Set Remote Gateway IP Address
    void SetRemoteGateway(int serial_port)
    {
        system("clear");
        display_logo1();

        char remoteIP[50];

        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf("\r" C6 "|" RE "    " C3 "SETTING" RE "    " C6 "|" RE " " C6 "Set Remote Gateway IP Address" RE "                                                                                                                                                               " C6 "|" RE "\n");
        printf(C6 "==============================================================================================================================================================================================================\n" RE);

        printf(C3 " Please enter Remote Gateway IP or Press" RE " " C2 "Y" RE " " C3 "to cancel\n" RE);

        // Clear buffer
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;

        if (fgets(remoteIP, sizeof(remoteIP), stdin) != NULL)
        {
            remoteIP[strcspn(remoteIP, "\n")] = 0;
            if ((strlen(remoteIP) == 1) && (remoteIP[0] == 'Y' || remoteIP[0] == 'y'))
            {
                printf(C3 " Cancelled (pressed Y). Returning to menu...\n" RE);
                return;
            }
            else if (strlen(remoteIP) == 0)
            {
                strcpy(config.remote_gw, "1.1.1.1");
            }
            else if (validate_ip_address(remoteIP))
            {
                strcpy(config.remote_gw, remoteIP);
            }
            else
            {
                printf(C3 " Invalid IP Address format! Please try again.\n" RE);
                usleep(1000000); // Pause for 1 second to let user read the message
                // printf(C3 " Please enter Local Gateway IP or Press" RE " " C2 "Z" RE " " C3 "to cancel\n" RE);
            }
            // strcpy(config.remote_gw, remoteIP);
        }
        printf(C2 " Press Enter to go Back..." RE);
        getchar();
    }

    void SetLocalSubnet(int serial_port)
    {
        system("clear");
        display_logo1();

        char LocalIP[50];

        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf("\r" C6 "|" RE "    " C3 "SETTING" RE "    " C6 "|" RE " " C6 "Set local Subnet IP Address" RE "                                                                                                                                                               " C6 "|" RE "\n");
        printf(C6 "==============================================================================================================================================================================================================\n" RE);

        printf(C3 " Please enter Local Subnet IP or Press" RE " " C2 "Y" RE " " C3 "to cancel\n" RE);

        // Clear buffer
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;

        if (fgets(LocalIP, sizeof(LocalIP), stdin) != NULL)
        {
            LocalIP[strcspn(LocalIP, "\n")] = 0;
            if ((strlen(LocalIP) == 1) && (LocalIP[0] == 'Y' || LocalIP[0] == 'y'))
            {
                printf(C3 " Cancelled (pressed Y). Returning to menu...\n" RE);
                return;
            }
            else if (strlen(LocalIP) == 0)
            {
                strcpy(config.local_subnet, "192.168.1.1/24");
            }
            else if (validate_ip_address(LocalIP))
            {
                strcpy(config.local_subnet, LocalIP);
            }
            else
            {
                printf(C3 " Invalid IP Address format! Please try again.\n" RE);
                usleep(1000000); // Pause for 1 second to let user read the message
                // printf(C3 " Please enter Local Gateway IP or Press" RE " " C2 "Z" RE " " C3 "to cancel\n" RE);
            }
            // strcpy(config.remote_gw, remoteIP);
        }
        printf(C2 " Press Enter to go Back..." RE);
        getchar();
    }

    void SetRemoteSubnet(int serial_port)
    {
        system("clear");
        display_logo1();

        char remoteIP[50];

        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf("\r" C6 "|" RE "    " C3 "SETTING" RE "    " C6 "|" RE " " C6 "Set Remote Subnet IP Address" RE "                                                                                                                                                               " C6 "|" RE "\n");
        printf(C6 "==============================================================================================================================================================================================================\n" RE);

        printf(C3 " Please enter Remote Subnet IP or Press" RE " " C2 "Y" RE " " C3 "to cancel\n" RE);

        // Clear buffer
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;

        if (fgets(remoteIP, sizeof(remoteIP), stdin) != NULL)
        {
            remoteIP[strcspn(remoteIP, "\n")] = 0;
            if ((strlen(remoteIP) == 1) && (remoteIP[0] == 'Y' || remoteIP[0] == 'y'))
            {
                printf(C3 " Cancelled (pressed Y). Returning to menu...\n" RE);
                return;
            }
            else if (strlen(remoteIP) == 0)
            {
                strcpy(config.remote_subnet, "192.168.1.1/24");
            }
            else if (validate_ip_address(remoteIP))
            {
                strcpy(config.remote_subnet, remoteIP);
            }
            else
            {
                printf(C3 " Invalid IP Address format! Please try again.\n" RE);
                usleep(1000000); // Pause for 1 second to let user read the message
                // printf(C3 " Please enter Local Gateway IP or Press" RE " " C2 "Z" RE " " C3 "to cancel\n" RE);
            }
            // strcpy(config.remote_gw, remoteIP);
        }
        printf(C2 " Press Enter to go Back..." RE);
        getchar();
    }
    // ================= IKEv2 Settings =================

    // ================= IKEv2 Settings =================
    void IKEv2Settings_Form(int serial_port)
    {
        char key;
    start:
        system("clear");
        display_logo1();
        printf(C6 "\n=====================================================================================" RE " " C3 "IKEv2 Settings" RE " " C6 "==========================================================================================================\n" RE);
        printf(C6 "+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 " %-40s :" RE " %-192s \n", C3 "IKEv2 Version" RE, strlen(config.ikev2_version) ? config.ikev2_version : C6 "[ Not Set ]" RE);
        printf(C6 " %-40s :" RE " %-192s \n", C3 "Mode" RE, strlen(config.ike_mode) ? config.ike_mode : C6 "[ Not Set ]" RE);
        printf(C6 " %-40s :" RE " %-192s \n", C3 "ESP/AH Protocol" RE, strlen(config.esp_ah_proto) ? config.esp_ah_proto : C6 "[ Not Set ]" RE);
        printf(C6 " %-40s :" RE " %-192s \n", C3 "IKE Reauth Time" RE, strlen(config.ike_reauth) ? config.ike_reauth : C6 "[ Not Set ]" RE);
        printf(C6 " %-40s :" RE " %-192s \n", C3 "Encryption Algorithm" RE, strlen(config.enc_algo) ? config.enc_algo : C6 "[ Not Set ]" RE);
        printf(C6 " %-40s :" RE " %-192s \n", C3 "Hash Algorithm" RE, strlen(config.hash_algo) ? config.hash_algo : C6 "[ Not Set ]" RE);
        printf(C6 " %-40s :" RE " %-192s \n", C3 "Re-key Time" RE, strlen(config.rekey_time) ? config.rekey_time : C6 "[ Not Set ]" RE);
        printf(C6 "+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

        printf(C6 "+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

        printf(C6 "+=============================================================================================================================================================================================================+\n" RE);
        printf(C6 "|" RE " " C3 "Key" RE " " C6 "|" RE " " C3 "Option" RE "                                                                                                                                                                                                " C6 "|" RE "\n" RE);
        printf(C6 "+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE "  " C3 "1" RE "  " C6 "|" RE " " C6 "Set IKEv2 Version" RE "                                                                                                                                                                                     " C6 "|" RE "\n" RE);
        printf(C6 "|" RE "  " C3 "2" RE "  " C6 "|" RE " " C6 "Set Mode" RE "                                                                                                                                                                                              " C6 "|" RE "\n" RE);
        printf(C6 "|" RE "  " C3 "3" RE "  " C6 "|" RE " " C6 "Set ESP/AH Protocol" RE "                                                                                                                                                                                   " C6 "|" RE "\n" RE);
        printf(C6 "|" RE "  " C3 "4" RE "  " C6 "|" RE " " C6 "Set IKE Reauth Time" RE "                                                                                                                                                                                   " C6 "|" RE "\n" RE);
        printf(C6 "|" RE "  " C3 "5" RE "  " C6 "|" RE " " C6 "Set Encryption Algorithm" RE "                                                                                                                                                                              " C6 "|" RE "\n" RE);
        printf(C6 "|" RE "  " C3 "6" RE "  " C6 "|" RE " " C6 "Set Hash Algorithm" RE "                                                                                                                                                                                    " C6 "|" RE "\n" RE);
        printf(C6 "|" RE "  " C3 "7" RE "  " C6 "|" RE " " C6 "Set Re-key Time" RE "                                                                                                                                                                                       " C6 "|" RE "\n" RE);
        printf(C6 "+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
        printf(C6 "|" RE "  " C2 "S" RE "  " C6 "|" RE " " C2 "Save" RE "                                                                                                                                                                                                  " C6 "|" RE "\n" RE);
        printf(C6 "|" RE "  " C17 "Z" RE "  " C6 "|" RE " " C17 "Exit" RE "                                                                                                                                                                                                  " C6 "|" RE "\n" RE);
        printf(C6 "+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n\n" RE);
        printf(C3 "Please choose:  " RE);

        while (1)
        {
            scanf(" %c", &key);
            if (key == '1' || key == '2' || key == '3' ||
                key == '4' || key == '5' || key == '6' || key == '7' ||
                key == 'S' || key == 's' || key == 'Z' || key == 'z')
            {
                break;
            }
            if (key != '1' && key != '2' && key != '3' &&
                key != '4' && key != '5' && key != '6' && key != '7' &&
                key != 'S' && key != 's' && key != 'Z' && key != 'z')
            {
                printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Please choose Mode: " RE);
            }
        }
        if (key == '1')
        {
            system("clear");
            display_logo1();
            SetIKEv2Version(serial_port);
            goto start;
        }
        else if (key == '2')
        {
            system("clear");
            display_logo1();
            SetIKEMode(serial_port);
            goto start;
        }
        else if (key == '3')
        {
            system("clear");
            display_logo1();
            SetESP_AHProtocol(serial_port);
            goto start;
        }
        else if (key == '4')
        {
            system("clear");
            display_logo1();
            SetIKEReauth(serial_port);
            goto start;
        }
        else if (key == '5')
        {
            system("clear");
            display_logo1();
            SetEncryptionAlgo(serial_port);
            goto start;
        }
        else if (key == '6')
        {
            system("clear");
            display_logo1();
            SetHashAlgo(serial_port);
            goto start;
        }
        else if (key == '7')
        {
            system("clear");
            display_logo1();
            SetRekeyTime(serial_port);
            goto start;
        }

        else if (key == 'S' || key == 's')
        {
            system("clear");
            display_logo1();
            ShowIPSecProfilePreview();
            SaveIPSecProfileToDB(serial_port);
            SaveIPSecProfileToFile();
            usleep(1000000);
        }
        else if (key == 'Z' || key == 'z')
        {
            system("clear");
            display_logo1();
            NodeSpecificSetting_Menu(serial_port);
        }
        else
        {
            system("clear");
            display_logo1();
            IKEv2Settings_Form(serial_port);
        }
    }

    void SetIKEv2Version(int serial_port)
    {
        ClearInputBuffer();
        // printf(C3 "Enter IKEv2 Version (" C17 "Z" RE " " C3 "to cancel): " RE);
        // if (fgets(config.ikev2_version, sizeof(config.ikev2_version), stdin) != NULL)
        // {
        //     config.ikev2_version[strcspn(config.ikev2_version, "\n")] = 0;
        //     if ((strlen(config.ikev2_version) == 1) &&
        //         (config.ikev2_version[0] == 'Z' || config.ikev2_version[0] == 'z'))
        //     {
        //         printf(C3 " Cancelled. Returning to menu...\n" RE);
        //     }
        // }
        // Gán mặc định luôn
        strcpy(config.ikev2_version, "v2");
    }

    void SetIKEMode(int serial_port)
    {
        ClearInputBuffer();
        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf("\r" C6 "|" RE "    " C3 "SETTING" RE "    " C6 "|" RE " " C6 "Set IKEv2 MODE" RE "                                                                                                                                                                " C6 "|" RE "\n");
        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf(C6 " [1] " RE "Tunnel Mode (available)   " C6 "|" RE "   [2] " RE "Transport Mode " C3 "(not available)\n" RE);
        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf(C3 " Please select IKEv2 Mode or Press" RE " " C2 "Z" RE " " C3 "to cancel\n" RE);

        char choice[10];
        if (fgets(choice, sizeof(choice), stdin) != NULL)
        {
            choice[strcspn(choice, "\n")] = 0;

            if ((strlen(choice) == 1) &&
                (choice[0] == 'Z' || choice[0] == 'z'))
            {
                printf(C3 " Cancelled. Returning to menu...\n" RE);
                return;
            }

            if (strcmp(choice, "1") == 0)
            {
                strcpy(config.ike_mode, "tunnel ");
                printf(C2 " IKEv2 Mode set to Tunnel Mode\n" RE);
            }
            else if (strcmp(choice, "2") == 0)
            {
                printf(C3 " Transport Mode is not supported yet. Defaulting to Tunnel Mode.\n" RE);
                strcpy(config.ike_mode, "transport ");
            }
            else
            {
                printf(C3 " Invalid choice! Defaulting to Tunnel Mode.\n" RE);
                strcpy(config.ike_mode, "tunnel ");
            }
        }
    }

    void SetESP_AHProtocol(int serial_port)
    {
        //     ClearInputBuffer();
        //     printf(C3 "Enter ESP/AH Protocol (" C17 "Z" RE " " C3 "to cancel): " RE);
        //     if (fgets(config.esp_ah_proto, sizeof(config.esp_ah_proto), stdin) != NULL)
        //     {
        //         config.esp_ah_proto[strcspn(config.esp_ah_proto, "\n")] = 0;
        //         if ((strlen(config.esp_ah_proto) == 1) &&
        //             (config.esp_ah_proto[0] == 'Z' || config.esp_ah_proto[0] == 'z'))
        //         {
        //             printf(C3 " Cancelled. Returning to menu...\n" RE);
        //         }
        //     }
        ClearInputBuffer();
        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf("\r" C6 "|" RE "    " C3 "SETTING" RE "    " C6 "|" RE " " C6 "Set ESP / AH Protocol" RE "                                                                                                                                                                " C6 "|" RE "\n");
        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf(C6 " [1] " RE "ESP Protocol   " C6 "|" RE "   [2] " RE "AH Protocol " C3 "\n" RE);
        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf(C3 " Please select IKEv2 Mode or Press" RE " " C2 "Z" RE " " C3 "to cancel\n" RE);
        char choice[10];

        if (fgets(choice, sizeof(choice), stdin) != NULL)
        {
            choice[strcspn(choice, "\n")] = 0;

            if ((strlen(choice) == 1) &&
                (choice[0] == 'Z' || choice[0] == 'z'))
            {
                printf(C3 " Cancelled. Returning to menu...\n" RE);
                return;
            }

            if (strcmp(choice, "1") == 0)
            {
                strcpy(config.esp_ah_proto, "ESP ");
                // printf(C2 " IKEv2 Mode set to Tunnel Mode\n" RE);
            }
            else if (strcmp(choice, "2") == 0)
            {
                // printf(C3 " Transport Mode is not supported yet. Defaulting to Tunnel Mode.\n" RE);
                strcpy(config.esp_ah_proto, "AH ");
            }
            else
            {
                printf(C3 " Invalid choice! \n" RE);
                strcpy(config.esp_ah_proto, "ESP ");
            }
        }
    }

    // void SetIKEReauth(int serial_port)
    // {
    //     printf(C3 "Enter IKE Reauth Time (" C17 "Z" RE " " C3 "to cancel): " RE);
    //     if (fgets(config.ike_reauth, sizeof(config.ike_reauth), stdin) != NULL)
    //     {
    //         ClearInputBuffer();
    //         config.ike_reauth[strcspn(config.ike_reauth, "\n")] = 0;
    //         if ((strlen(config.ike_reauth) == 1) &&
    //             (config.ike_reauth[0] == 'Z' || config.ike_reauth[0] == 'z'))
    //         {
    //             printf(C3 " Cancelled. Returning to menu...\n" RE);
    //         }
    //     }
    // }

    void SetIKEReauth(int serial_port)
    {
        ClearInputBuffer();
        char input[32];
        int reauth_time = 0;
        // printf(C3 "Enter Re-key Time (" C17 "Z" RE " " C3 "to cancel): " RE);
        // if (fgets(config.rekey_time, sizeof(config.rekey_time), stdin) != NULL)
        // {
        //     config.rekey_time[strcspn(config.rekey_time, "\n")] = 0;
        //     if ((strlen(config.rekey_time) == 1) &&
        //         (config.rekey_time[0] == 'Z' || config.rekey_time[0] == 'z'))
        //     {
        //         printf(C3 " Cancelled. Returning to menu...\n" RE);
        //     }
        // }\\ClearInputBuffer();

        while (1)
        {
            printf(C3 "Enter IKE Reauth Time in seconds (" C17 "Z" RE " " C3 "to cancel, press Enter for default 30s): " RE);

            if (fgets(input, sizeof(input), stdin) != NULL)
            {
                input[strcspn(input, "\n")] = 0; // Xóa ký tự xuống dòng

                // Người dùng hủy
                if ((strlen(input) == 1) && (input[0] == 'Z' || input[0] == 'z'))
                {
                    printf(C3 " Cancelled. Returning to menu...\n" RE);
                    return;
                }

                // Nếu bỏ trống => mặc định 30s
                if (strlen(input) == 0)
                {
                    reauth_time = 30;
                    snprintf(config.ike_reauth, sizeof(config.ike_reauth), "%d", reauth_time);
                    printf(C2 " IKE Reauth Time set to default %d seconds\n" RE, reauth_time);
                    break;
                }

                // Chuyển sang số
                reauth_time = atoi(input);

                if (reauth_time > 0)
                {
                    snprintf(config.ike_reauth, sizeof(config.ike_reauth), "%d", reauth_time);
                    printf(C2 " IKE Reauth Time set to %d seconds\n" RE, reauth_time);
                    break;
                }
                else
                {
                    printf(C3 " Invalid input! Must be a positive integer.\n" RE);
                }
            }
        }
    }

    void SetEncryptionAlgo(int serial_port)
    {
        // ClearInputBuffer();
        // printf(C3 "Enter Encryption Algorithm (" C17 "Z" RE " " C3 "to cancel): " RE);
        // if (fgets(config.enc_algo, sizeof(config.enc_algo), stdin) != NULL)
        // {
        //     config.enc_algo[strcspn(config.enc_algo, "\n")] = 0;
        //     if ((strlen(config.enc_algo) == 1) &&
        //         (config.enc_algo[0] == 'Z' || config.enc_algo[0] == 'z'))
        //     {
        //         printf(C3 " Cancelled. Returning to menu...\n" RE);
        //     }
        // }
        ClearInputBuffer();
        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf("\r" C6 "|" RE "             " C3 "SETTING" RE "             " C6 "|" RE " " C6 "Encryption Algorithm" RE "                                                                                                                                        " C6 "|" RE "\n");
        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf(C6 " [1] " RE "AES-128\n");
        printf(C6 " [2] " RE "AES-192\n");
        printf(C6 " [3] " RE "AES-256\n");
        printf(C6 " [4] " RE "3DES\n");
        printf(C6 " [5] " RE "DES\n");
        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf(C3 " Please select option or Press" RE " " C2 "Z" RE " " C3 "to cancel\n" RE);

        char choice[10];
        if (fgets(choice, sizeof(choice), stdin) != NULL)
        {
            choice[strcspn(choice, "\n")] = 0;

            if ((strlen(choice) == 1) &&
                (choice[0] == 'Z' || choice[0] == 'z'))
            {
                printf(C3 " Cancelled. Returning to menu...\n" RE);
                return;
            }

            if (strcmp(choice, "1") == 0)
            {
                strcpy(config.enc_algo, "aes-128");
            }
            else if (strcmp(choice, "2") == 0)
            {
                strcpy(config.enc_algo, "aes-192");
            }
            else if (strcmp(choice, "3") == 0)
            {
                strcpy(config.enc_algo, "aes-256");
            }
            else if (strcmp(choice, "4") == 0)
            {
                strcpy(config.enc_algo, "3des");
            }
            else if (strcmp(choice, "5") == 0)
            {
                strcpy(config.enc_algo, "des");
            }
            else
            {
                // printf(C3 " Invalid choice! Defaulting to ESP Protocol.\n" RE);
                strcpy(config.enc_algo, "aes-256");
            }
        }
    }

    void SetHashAlgo(int serial_port)
    {
        // ClearInputBuffer();
        // printf(C3 "Enter Hash Algorithm (" C17 "Z" RE " " C3 "to cancel): " RE);
        // if (fgets(config.hash_algo, sizeof(config.hash_algo), stdin) != NULL)
        // {
        //     config.hash_algo[strcspn(config.hash_algo, "\n")] = 0;
        //     if ((strlen(config.hash_algo) == 1) &&
        //         (config.hash_algo[0] == 'Z' || config.hash_algo[0] == 'z'))
        //     {
        //         printf(C3 " Cancelled. Returning to menu...\n" RE);
        //     }
        // }
        ClearInputBuffer();
        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf("\r" C6 "|" RE "             " C3 "SETTING" RE "             " C6 "|" RE " " C6 "Set Hash Algorithm" RE "                                                                                                                                        " C6 "|" RE "\n");
        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf(C6 " [1] " RE "MD5\n");
        printf(C6 " [2] " RE "SHA-1\n");
        printf(C6 " [3] " RE "SHA-256\n");
        printf(C6 " [4] " RE "SHA-384\n");
        printf(C6 " [5] " RE "SHA-512\n");
        printf(C6 "==============================================================================================================================================================================================================\n" RE);
        printf(C3 " Please select option or Press" RE " " C2 "Z" RE " " C3 "to cancel\n" RE);

        char choice[10];
        if (fgets(choice, sizeof(choice), stdin) != NULL)
        {
            choice[strcspn(choice, "\n")] = 0;

            if ((strlen(choice) == 1) &&
                (choice[0] == 'Z' || choice[0] == 'z'))
            {
                printf(C3 " Cancelled. Returning to menu...\n" RE);
                return;
            }

            if (strcmp(choice, "1") == 0)
            {
                strcpy(config.hash_algo, "md5");
            }
            else if (strcmp(choice, "2") == 0)
            {
                strcpy(config.hash_algo, "sha-1");
            }
            else if (strcmp(choice, "3") == 0)
            {
                strcpy(config.hash_algo, "sha-256");
            }
            else if (strcmp(choice, "4") == 0)
            {
                strcpy(config.hash_algo, "sha-384");
            }
            else if (strcmp(choice, "5") == 0)
            {
                strcpy(config.hash_algo, "sha-512");
            }
            else
            {
                // printf(C3 " Invalid choice! Defaulting to ESP Protocol.\n" RE);
                strcpy(config.hash_algo, "SHA-256");
            }
        }
    }

    void SetRekeyTime(int serial_port)
    {
        ClearInputBuffer();
        char input[32];
        int reauth_time = 0;
        // printf(C3 "Enter Re-key Time (" C17 "Z" RE " " C3 "to cancel): " RE);
        // if (fgets(config.rekey_time, sizeof(config.rekey_time), stdin) != NULL)
        // {
        //     config.rekey_time[strcspn(config.rekey_time, "\n")] = 0;
        //     if ((strlen(config.rekey_time) == 1) &&
        //         (config.rekey_time[0] == 'Z' || config.rekey_time[0] == 'z'))
        //     {
        //         printf(C3 " Cancelled. Returning to menu...\n" RE);
        //     }
        // }\\ClearInputBuffer();

        while (1)
        {
            printf(C3 "Enter IKE Reauth Time in seconds (" C17 "Z" RE " " C3 "to cancel, press Enter for default 30s): " RE);

            if (fgets(input, sizeof(input), stdin) != NULL)
            {
                input[strcspn(input, "\n")] = 0; // Xóa ký tự xuống dòng

                // Người dùng hủy
                if ((strlen(input) == 1) && (input[0] == 'Z' || input[0] == 'z'))
                {
                    printf(C3 " Cancelled. Returning to menu...\n" RE);
                    return;
                }

                // Nếu bỏ trống => mặc định 30s
                if (strlen(input) == 0)
                {
                    reauth_time = 30;
                    snprintf(config.rekey_time, sizeof(config.rekey_time), "%d", reauth_time);
                    printf(C2 " IKE Reauth Time set to default %d seconds\n" RE, reauth_time);
                    break;
                }

                // Chuyển sang số
                reauth_time = atoi(input);

                if (reauth_time > 0)
                {
                    snprintf(config.rekey_time, sizeof(config.rekey_time), "%d", reauth_time);
                    printf(C2 " IKE Reauth Time set to %d seconds\n" RE, reauth_time);
                    break;
                }
                else
                {
                    printf(C3 " Invalid input! Must be a positive integer.\n" RE);
                }
            }
        }
    }

    // Hàm l?y th?i gian hi?n t?i d?ng string
    void GetCurrentTime(char *buffer, size_t size)
    {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        strftime(buffer, size, "%Y/%m/%d %H:%M:%S", t);
    }

    // Hàm lưu profile xuống DB
    // Hàm lưu profile xuống DB
    void SaveIPSecProfileToDB(int serial_port)
    {
        sqlite3 *db;
        sqlite3_stmt *stmt;
        int rc;

        rc = sqlite3_open(DB_PATH, &db);
        if (rc != SQLITE_OK)
        {
            printf("Cannot open database: %s\n", sqlite3_errmsg(db));
            return;
        }

        const char *sql =
            "SELECT IPSecProfileId, ProfileName, ProfileDescription, "
            " LocalGateway, RemoteGateway, IKEVersion, Mode, "
            "ESPAHProtocol, IKEReauthTime, EncryptionAlgorithm, HashAlgorithm, "
            "ReKeyTime, Enable, CreateTime, LastModified, "
            "SubnetLocalGateway, SubnetRemoteGateway, ConnectionCount "
            "FROM IPSecProfiles ORDER BY IPSecProfileId;";

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        display_logo1();
        printf(C6 "\n========================= " C3 "IPSec Profiles List" RE " =========================\n" RE);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            printf(C6 "\n---------------- Profile ----------------\n" RE);

            printf(C3 "Profile ID             :" RE " %s\n",
                sqlite3_column_text(stmt, 0) ? (const char *)sqlite3_column_text(stmt, 0) : "-");
            printf(C3 "Profile Name           :" RE " %s\n",
                sqlite3_column_text(stmt, 1) ? (const char *)sqlite3_column_text(stmt, 1) : "[ Not Set ]");
            printf(C3 "Description            :" RE " %s\n",
                sqlite3_column_text(stmt, 2) ? (const char *)sqlite3_column_text(stmt, 2) : "[ Not Set ]");
            printf(C3 "Local Gateway IP       :" RE " %s\n",
                sqlite3_column_text(stmt, 4) ? (const char *)sqlite3_column_text(stmt, 4) : "[ Not Set ]");

            printf(C6 "\n>>> IKE Configuration\n" RE);
            printf(C3 "Remote Gateway IP      :" RE " %s\n",
                sqlite3_column_text(stmt, 5) ? (const char *)sqlite3_column_text(stmt, 5) : "[ Not Set ]");
            printf(C3 "Local Subnet IP        :" RE " %s\n",
                sqlite3_column_text(stmt, 15) ? (const char *)sqlite3_column_text(stmt, 15) : "[ Not Set ]");
            printf(C3 "Remote Subnet IP       :" RE " %s\n",
                sqlite3_column_text(stmt, 16) ? (const char *)sqlite3_column_text(stmt, 16) : "[ Not Set ]");
            printf(C3 "IKEv2 Version          :" RE " %s\n",
                sqlite3_column_text(stmt, 6) ? (const char *)sqlite3_column_text(stmt, 6) : "[ Not Set ]");
            printf(C3 "IKE Mode               :" RE " %s\n",
                sqlite3_column_text(stmt, 7) ? (const char *)sqlite3_column_text(stmt, 7) : "[ Not Set ]");
            printf(C3 "ESP/AH Protocol        :" RE " %s\n",
                sqlite3_column_text(stmt, 8) ? (const char *)sqlite3_column_text(stmt, 8) : "[ Not Set ]");
            printf(C3 "IKE Reauth Time        :" RE " %s\n",
                sqlite3_column_text(stmt, 9) ? (const char *)sqlite3_column_text(stmt, 9) : "[ Not Set ]");
            printf(C3 "Encryption Algorithm   :" RE " %s\n",
                sqlite3_column_text(stmt, 10) ? (const char *)sqlite3_column_text(stmt, 10) : "[ Not Set ]");
            printf(C3 "Hash Algorithm         :" RE " %s\n",
                sqlite3_column_text(stmt, 11) ? (const char *)sqlite3_column_text(stmt, 11) : "[ Not Set ]");
            printf(C3 "Re-key Time            :" RE " %s\n",
                sqlite3_column_text(stmt, 12) ? (const char *)sqlite3_column_text(stmt, 12) : "[ Not Set ]");

            printf(C6 "-----------------------------------------\n" RE);
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);

        printf("\nPress Enter to continue...");
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
    }

    // Hàm chuyển chuỗi thành chữ thường và bỏ dấu gạch ngang
    void normalize_algo(char *str)
    {
        char *src = str, *dst = str;
        for (; *src; src++)
        {
            if (*src == '-')
                continue; // bỏ dấu '-'
            *dst++ = (char)tolower((unsigned char)*src);
        }
        *dst = '\0';
    }

    // void SaveIPSecProfileToFile()
    // {
    //     FILE *fp_in, *fp_out;
    //     char line[512];
    //     char esp_enc[64], esp_hash[64];
    //     char ike_lifetime[32], key_lifetime[32];
    //     char local_gw[64], remote_gw[64];
    //     char cert_file[128];

    //     fp_in = fopen("/zynq_lib/etc/ipsec.conf", "r");
    //     if (fp_in == NULL)
    //     {
    //         perror("Error opening original config");
    //         return;
    //     }

    //     fp_out = fopen("/zynq_lib/etc/ipsec.conf.tmp", "w");
    //     if (fp_out == NULL)
    //     {
    //         perror("Error opening temp config");
    //         fclose(fp_in);
    //         return;
    //     }

    //     // Lấy tham số từ config
    //     snprintf(ike_lifetime, sizeof(ike_lifetime),
    //              "%s", strlen(config.ike_reauth) ? config.ike_reauth : "6m");
    //     snprintf(key_lifetime, sizeof(key_lifetime),
    //              "%s", strlen(config.rekey_time) ? config.rekey_time : "2m");
    //     snprintf(esp_enc, sizeof(esp_enc),
    //              "%s", strlen(config.enc_algo) ? config.enc_algo : "aes128");
    //     snprintf(esp_hash, sizeof(esp_hash),
    //              "%s", strlen(config.hash_algo) ? config.hash_algo : "sha256");
    //     snprintf(local_gw, sizeof(local_gw),
    //              "%s", strlen(config.local_gw) ? config.local_gw : "1.1.1.1");
    //     snprintf(remote_gw, sizeof(remote_gw),
    //              "%s", strlen(config.remote_gw) ? config.remote_gw : "1.1.1.1");
    //     snprintf(cert_file, sizeof(cert_file),
    //              "%s", strlen(g_config.cert_file) ? g_config.cert_file : "peerCert_1.der");

    //     // Chuẩn hóa
    //     normalize_algo(esp_enc);
    //     normalize_algo(esp_hash);
    //     normalize_algo(local_gw);
    //     normalize_algo(remote_gw);
    //     normalize_algo(ike_lifetime);
    //     normalize_algo(key_lifetime);

    //     int conn_count = 0;
    //     int in_conn_block = 0;

    //     while (fgets(line, sizeof(line), fp_in))
    //     {
    //         // Nhận diện khối conn
    //         if (strncmp(line, "conn ", 5) == 0)
    //         {
    //             // Nếu là conn %default thì bỏ qua
    //             if (strstr(line, "conn %default"))
    //             {
    //                 in_conn_block = 0;
    //                 continue;
    //             }

    //             conn_count++;
    //             in_conn_block = (conn_count == 1); // lúc này sẽ là net12-net12
    //         }

    //         if (in_conn_block)
    //         {
    //             if (strstr(line, "ikelifetime="))
    //             {
    //                 fprintf(fp_out, "        ikelifetime=%sm\n", ike_lifetime);
    //                 continue;
    //             }
    //             else if (strstr(line, "keylife="))
    //             {
    //                 fprintf(fp_out, "        keylife=%sm\n", key_lifetime);
    //                 continue;
    //             }
    //             else if (strstr(line, "esp="))
    //             {
    //                 fprintf(fp_out, "        esp=%s-%s\n", esp_enc, esp_hash);
    //                 continue;
    //             }
    //             else if (strstr(line, "leftcert="))
    //             {
    //                 fprintf(fp_out, "        leftcert=%s\n", cert_file);
    //                 continue;
    //             }
    //         }
    //         // Khối conn thứ 2: chỉ thay left và right
    //         else if (in_conn_block && conn_count == 2)
    //         {
    //             if (strstr(line, "left="))
    //             {
    //                 fprintf(fp_out, "        left=%s\n", local_gw);
    //                 continue;
    //             }
    //             else if (strstr(line, "right="))
    //             {
    //                 fprintf(fp_out, "        right=%s\n", remote_gw);
    //                 continue;
    //             }
    //         }

    //         // Ghi nguyên gốc
    //         fputs(line, fp_out);
    //     }

    //     fclose(fp_in);
    //     fclose(fp_out);

    //     if (rename("/zynq_lib/etc/ipsec.conf.tmp", "/zynq_lib/etc/ipsec.conf") != 0)
    //     {
    //         perror("Error renaming temp file");
    //     }
    //     else
    //     {
    //         printf("✔ Updated IPSec profile in /zynq_lib/etc/ipsec.conf\n");
    //     }
    // }

    void SaveIPSecProfileToFile()
    {
        FILE *fp_in, *fp_out;
        char line[512];
        char esp_enc[64], esp_hash[64];
        char ike_lifetime[32], key_lifetime[32];
        char local_gw[64], remote_gw[64];
        char cert_file[128];
        char local_subnet[64], remote_subnet[64];

        fp_in = fopen("/zynq_lib/etc/ipsec.conf", "r");
        if (!fp_in)
        {
            perror("Error opening original config");
            return;
        }

        fp_out = fopen("/zynq_lib/etc/ipsec.conf.tmp", "w");
        if (!fp_out)
        {
            perror("Error opening temp config");
            fclose(fp_in);
            return;
        }

        // Lấy tham số từ config
        snprintf(ike_lifetime, sizeof(ike_lifetime),
                "%s", strlen(config.ike_reauth) ? config.ike_reauth : "6m");
        snprintf(key_lifetime, sizeof(key_lifetime),
                "%s", strlen(config.rekey_time) ? config.rekey_time : "2m");
        snprintf(esp_enc, sizeof(esp_enc),
                "%s", strlen(config.enc_algo) ? config.enc_algo : "aes128");
        snprintf(esp_hash, sizeof(esp_hash),
                "%s", strlen(config.hash_algo) ? config.hash_algo : "sha256");
        snprintf(local_gw, sizeof(local_gw),
                "%s", strlen(config.local_gw) ? config.local_gw : "1.1.1.1");
        snprintf(remote_gw, sizeof(remote_gw),
                "%s", strlen(config.remote_gw) ? config.remote_gw : "1.1.1.1");
        snprintf(cert_file, sizeof(cert_file),
                "%s", strlen(g_config.cert_file) ? g_config.cert_file : "peerCert_1.der");

        snprintf(local_subnet, sizeof(local_subnet),
                "%s", strlen(config.local_subnet) ? config.local_subnet : "192.168.1.1/24");
        snprintf(remote_subnet, sizeof(remote_subnet),
                "%s", strlen(config.remote_subnet) ? config.remote_subnet : "192.168.1.1/24");

        normalize_algo(esp_enc);
        normalize_algo(esp_hash);
        normalize_algo(local_gw);
        normalize_algo(remote_gw);
        normalize_algo(ike_lifetime);
        normalize_algo(key_lifetime);
        normalize_algo(local_subnet);
        normalize_algo(remote_subnet);

        int conn_count = 0;

        while (fgets(line, sizeof(line), fp_in))
        {
            // Nhận diện khối conn
            if (strncmp(line, "conn ", 5) == 0)
            {
                conn_count++;
            }

            // Khối conn 1: thay các thông số cấu hình
            if (conn_count == 0)
            {
                if (strstr(line, "ikelifetime="))
                {
                    fprintf(fp_out, "        ikelifetime=%sm\n", ike_lifetime);
                    continue;
                }
                else if (strstr(line, "keylife="))
                {
                    fprintf(fp_out, "        keylife=%sm\n", key_lifetime);
                    continue;
                }
                else if (strstr(line, "esp="))
                {
                    fprintf(fp_out, "        esp=%s-%s\n", esp_enc, esp_hash);
                    continue;
                }
                else if (strstr(line, "leftcert="))
                {
                    fprintf(fp_out, "        leftcert=%s\n", cert_file);
                    continue;
                }
            }
            // Khối conn 2: chỉ thay left và right
            else if (conn_count == 1)
            {
                if (strstr(line, "left="))
                {
                    fprintf(fp_out, "        left=%s\n", local_gw);
                    continue;
                }
                else if (strstr(line, "right="))
                {
                    fprintf(fp_out, "        right=%s\n", remote_gw);
                    continue;
                }
                else if (strstr(line, "leftsubnet="))
                {
                    fprintf(fp_out, "        leftsubnet=%s\n", local_subnet);
                    continue;
                }
                else if (strstr(line, "rightsubnet="))
                {
                    fprintf(fp_out, "        rightsubnet=%s\n", remote_subnet);
                    continue;
                }
            }

            // Ghi nguyên dòng
            fputs(line, fp_out);
        }

        fclose(fp_in);
        fclose(fp_out);

        if (rename("/zynq_lib/etc/ipsec.conf.tmp", "/zynq_lib/etc/ipsec.conf") != 0)
        {
            perror("Error renaming temp file");
        }
        else
        {
            printf("✔ Updated IPSec profile in /zynq_lib/etc/ipsec.conf\n");
        }
    }

    // void inject_fake_packet(int type) {
    //     unsigned char packet[90];
    //     if (type == 1) memcpy(packet, attack_packet, 90);
    //     else memcpy(packet, normal_packet, 90);
    //     enqueue_packet(packet, 90); //  ?y v o queue th?t
    // }
    void check_account(int serial_port)
    {
    start:
        // input_and_send_account(serial_port);

        // gi? l?p d? li?u nh?n du?c t? UART l  "A"
        const char *data = "A";
        usleep(10000);

        if (data == NULL)
        {
            printf(C2 "Error receiving data\n" RE);
            return;
        }

        if ((strchr(data, 'F') != NULL) || (strchr(data, 'f') != NULL))
        {
            printf(C6 "\r\n\t\t=============================================================================================================================================================================================+" RE);
            printf(C6 "\r\n\t\t|" RE " " C3 "Warning: Incorrect password/username. Retry!" RE "                                                                           " RE);
            usleep(10000);
            goto start;
        }
        else if ((strchr(data, 'Y') != NULL) || (strchr(data, 'y') != NULL))
        {
            printf(C6 "\r\n\t\t=============================================================================================================================================================================================+" RE);
            printf(C6 "\r\n\t\t|" RE " " C17 "Wrong password 3 times." RE "                                                                                                " C6 "|" RE);
            printf(C6 "\r\n\t\t|                                                                                                                        |" RE);
            printf(C6 "\r\n\t\t+------------------------------------------------------------------------------------------------------------------------+" RE);
            usleep(500000);
            // system("clear");
        }
        else if ((strchr(data, 'U') != NULL))
        {
            printf(C6 "\r\n\t\t=============================================================================================================================================================================================+" RE);
            printf(C6 "\r\n\t\t|" RE " " C2 "User login successfully !!!                         " RE);
            printf(C6 "\r\n----------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
            sleep(1);
            system("clear");
            display_logo1();
            user_mode(serial_port);
        }
        else if ((strchr(data, 'A') != NULL))
        {
            printf(C6 "\r\n\t\t=============================================================================================================================================================================================+" RE);
            printf(C6 "\r\n\t\t|" RE " " C2 "Admin login successfully !!!                                                                                           " RE);
            printf(C6 "\r\n----------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
            sleep(1);
            system("clear");
            display_logo1();
            AdminConfigMenu(serial_port);
            admin_mode(serial_port);
        }
    }

    void check_username_change_pass(int serial_port)
    {
        // char key_mode = '(';
        // write(serial_port, &key_mode, sizeof(key_mode));
        // usleep(100000);
        // write(serial_port, &key_enter, sizeof(key_enter));
        // usleep(100000);
        input_and_send_account2(serial_port);

        char *data = receive_data(serial_port);

        if (data == NULL)
        {
            printf(C2 "Error receiving data\n" RE);
            return;
        }
        else if ((strchr(data, 'Y') != NULL))
        {
            system("clear");
            display_logo1();
            display_account(serial_port);
            printf(C2 "\r\n\t\t\t\t\t\t Change Successfully !!!                                                                                            " RE);
        }
        else if ((strchr(data, 'X') != NULL))
        {
            system("clear");
            display_logo1();
            display_account(serial_port);
            printf(C3 "\r\n\t\t\t\t\t\t Change Error !!!                                                                                            " RE);
        }
        ReturnMode3();
    }

    void reconfig(int serial_port)
    {
    start:
        // system("clear");
        // display_logo1();
        char key = 0;
        char enter = '\r';
        printf(C6 "\r\n *************************************************************************************************************************************************************************************************************" RE);
        printf("\r\n");
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf(C3 "\r\n ==> Mode 2 is selected" RE "                                                                                                                                                                                      " C6 "|" RE);
        printf(C6 "\r\n");
        printf(C6 " ===============+===========+================================================================================================================================================================================+\r\n" RE);
        printf("    " C3 "DISPLAY" RE "     " C6 "|" RE "           " C6 "|" RE "                                                                                                                                                                                " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                                                                                                  " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "Setting Anti by Port mode(*)." RE "                                                                                                                                                  " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "           " C6 "|" RE " 	" C3 "->(Info: When Port protection mode(*) is enabled, IP protected mode (**) is disabled and vice versa)." RE "                                                                        " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 "Setting IPv4 Server to protect(**)." RE "                                                                                                                                            " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "3." RE "    " C6 "|" RE " " C6 "Setting IPv6 Server to protect(**)." RE "                                                                                                                                            " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "4." RE "    " C6 "|" RE " " C6 "Setting IPv4 server Block(**)." RE "                                                                                                                                                 " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "5." RE "    " C6 "|" RE " " C6 "Setting IPv6 server Block(**)." RE "                                                                                                                                                 " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "6." RE "    " C6 "|" RE " " C6 "Setting Anti-SYN flood." RE "                                                                                                                                                        " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "7." RE "    " C6 "|" RE " " C6 "Setting SYN flood attack detection threshold." RE "                                                                                                                                  " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "           " C6 "|" RE " 	" C3 "->(Info: The default value is: 1000 PPS)." RE "                                                                                                                                    " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "8." RE "    " C6 "|" RE " " C6 "Setting ACK flood attack detection threshold." RE "                                                                                                                                  " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "           " C6 "|" RE " 	" C3 "->(Info: The default value is: 1000 PPS)." RE "                                                                                                                                    " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "9." RE "    " C6 "|" RE " " C6 "Setting the time to automatically delete the connection session information in the white list." RE "                                                                                 " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "           " C6 "|" RE " 	" C3 "->(Info: The default value is: 30 second)." RE "                                                                                                                                   " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "A." RE "    " C6 "|" RE " " C6 "Setting Anti-LAND Attack." RE "                                                                                                                                                      " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "B." RE "    " C6 "|" RE " " C6 "Setting Anti-UDP flood." RE "                                                                                                                                                        " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "C." RE "    " C6 "|" RE " " C6 "Setting UDP flood attack detection threshold." RE "                                                                                                                                  " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "           " C6 "|" RE " 	" C3 "->(Info: The default value is: 1000 PPS)." RE "                                                                                                                                    " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "D." RE "    " C6 "|" RE " " C6 "Setting threshold of valid UDP packer per second allowed." RE "                                                                                                                      " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "           | 	" C3 "->(Info: The default value is: 1000 PPS)." RE "                                                                                                                                    " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "E." RE "    " C6 "|" RE " " C6 "Setting Anti-DNS Amplification attack." RE "                                                                                                                                         " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "F." RE "   " C6 "|" RE " " C6 "etting DNS Amplification attack detection threshold." RE "                                                                                                                            " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "           | 	" C3 "->(Info: The default value is: 1000 PPS)." RE "                                                                                                                                    " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "G." RE "    " C6 "|" RE " " C6 "Setting Anti-ICMP flood." RE "                                                                                                                                                       " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "H." RE "    " C6 "|" RE " " C6 "Setting ICMP flood attack detection threshold." RE "                                                                                                                                 " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "           " C6 "|" RE " 	" C3 "->(Info: The default value is: 1000 PPS)." RE "                                                                                                                                    " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "I." RE "    " C6 "|" RE " " C6 "Setting threshold of valid ICMP packer per second allowed." RE "                                                                                                                     " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "           " C6 "|" RE " 	" C3 "->(Info: The default value is: 1000 PPS)." RE "                                                                                                                                    " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "J." RE "    " C6 "|" RE " " C6 "Setting Anti-IPSec IKE flood." RE "                                                                                                                                                  " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "K." RE "    " C6 "|" RE " " C6 "Setting IPSEC IKE flood attack detection threshold." RE "                                                                                                                            " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "           " C6 "|" RE " 	" C3 "->(Info: The default value is: 1000 PPS)." RE "                                                                                                                                    " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "L." RE "    " C6 "|" RE " " C6 "Setting Anti-TCP fragmentation flood." RE "                                                                                                                                          " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "M." RE "    " C6 "|" RE " " C6 "Setting Anti-UDP fragmentation flood." RE "                                                                                                                                          " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "N." RE "    " C6 "|" RE " " C6 "Setting HTTP GET flood." RE "                                                                                                                                                        " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "O." RE "    " C6 "|" RE " " C6 "Setting HTTP/HTTPs IP Table." RE "                                                                                                                                                   " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "Q." RE "    " C6 "|" RE " " C6 "Setting HTTPS GET flood." RE "                                                                                                                                                       " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "Z." RE "    " C6 "|" RE " " C17 "=> Exit." RE "                                                                                                                                                                       " C6 "|\r\n" RE);
        printf(C6 "----------------+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf("    " C3 "SETTING" RE "    | " C3 "--> Your choice:" RE "");

        // Updated valid keys (removed '1', '5', 'L', 'M', 'S', 'T')
        while (1)
        {
            scanf("%c", &key);
            if (key == '1' || key == '2' || key == '3' ||
                key == '4' || key == '5' || key == '6' || key == '7' ||
                key == '8' || key == '9' || key == 'A' || key == 'a' ||
                key == 'B' || key == 'b' || key == 'C' || key == 'c' ||
                key == 'D' || key == 'd' || key == 'E' || key == 'e' ||
                key == 'F' || key == 'f' || key == 'G' || key == 'g' ||
                key == 'H' || key == 'h' || key == 'I' || key == 'i' ||
                key == 'J' || key == 'j' || key == 'K' || key == 'k' ||
                key == 'L' || key == 'l' || key == 'M' || key == 'm' ||
                key == 'N' || key == 'n' || key == 'O' || key == 'o' ||
                key == 'Z' || key == 'z' || key == 'Q' || key == 'q' || key == 'X' || key == 'x')

            {
                break;
            }
            // Also update the invalid key check accordingly.
            if (key != '1' && key != '2' && key != '3' &&
                key != '4' && key != '5' && key != '6' && key != '7' &&
                key != '8' && key != '9' && key != 'A' && key != 'a' &&
                key != 'B' && key != 'b' && key != 'C' && key != 'c' &&
                key != 'D' && key != 'd' && key != 'E' && key != 'e' &&
                key != 'F' && key != 'f' && key != 'G' && key != 'g' &&
                key != 'H' && key != 'h' && key != 'I' && key != 'i' &&
                key != 'J' && key != 'j' && key != 'K' && key != 'k' &&
                key != 'L' && key != 'l' && key != 'M' && key != 'm' &&
                key != 'N' && key != 'n' && key != 'O' && key != 'o' &&
                key != 'Z' && key != 'z' && key != 'Q' && key != 'q' && key != 'X' && key != 'x')
            {
                printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Your choice: " RE);
            }
        }

        usleep(500000);
        /* Removed branch for key '1'
        if (key == '1')
        {
        system("clear");
        display_logo1();
        display_table(serial_port);
        SetDateTime(serial_port);
        goto start;
        }
        */
        if (key == '1')
        {
            // system("clear");
            // display_logo1();
            Display_table_2(current_port);
            SetDefenderPort(serial_port);
            goto start;
        }
        else if (key == '2')
        {
            // system("clear");
            // display_logo1();
            Display_IPv4_Protected_Table();
            SetIPv4Target(serial_port);
            goto start;
        }

        else if (key == '3')
        {
            // system("clear");
            // display_logo1();
            // display_table(serial_port);
            // Display_table_2(current_port);
            Display_IPv6_Protected_Table();
            SetIPv6Target(serial_port);
            goto start;
        }
        else if (key == '4')
        {
            // system("clear");
            // display_logo1();
            Display_IPv4_block_table();
            SetIPv4Block(serial_port);
            goto start;
        }
        else if (key == '5')
        {
            // system("clear");
            // display_logo1();
            Display_IPv6_block_table();
            SetIPv6Block(serial_port);
            goto start;
        }
        else if (key == '6')
        {
            // system("clear");
            // display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetSynDefender(serial_port);
            goto start;
        }
        else if (key == '7')
        {
            // system("clear");
            // display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetSynThresh(serial_port);
            goto start;
        }
        else if (key == '8')
        {
            // system("clear");
            // display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetAckThresh(serial_port);
            goto start;
        }
        else if (key == '9')
        {
            // system("clear");
            // display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetTimeDelete(serial_port);
            goto start;
        }
        else if (key == 'A' || key == 'a')
        {
            // system("clear");
            // display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetSynonymousDefender(serial_port);
            goto start;
        }
        else if (key == 'B' || key == 'b')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetUDPDefender(serial_port);
            goto start;
        }
        else if (key == 'C' || key == 'c')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetUDPThresh(serial_port);
            goto start;
        }
        else if (key == 'D' || key == 'd')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);n
            Display_table_2(current_port);
            SetUDPThresh1s(serial_port);
            goto start;
        }
        else if (key == 'E' || key == 'e')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetDNSDefender(serial_port);
            goto start;
        }
        else if (key == 'F' || key == 'f')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetDNSThresh(serial_port);
            goto start;
        }
        else if (key == 'G' || key == 'g')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetICMPDefender(serial_port);
            goto start;
        }
        else if (key == 'H' || key == 'h')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetICMPThresh(serial_port);
            goto start;
        }
        else if (key == 'I' || key == 'i')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetICMPThresh1s(serial_port);
            goto start;
        }
        else if (key == 'J' || key == 'j')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetIPSecDefender(serial_port);
            goto start;
        }
        else if (key == 'K' || key == 'k')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetIPSecThresh(serial_port);
            goto start;
        }
        else if (key == 'L' || key == 'l')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetTCPFragDefender(serial_port);
            goto start;
        }
        else if (key == 'M' || key == 'm')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetUDPFragDefender(serial_port);
            goto start;
        }
        else if (key == 'N' || key == 'n')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetHTTPDefender(serial_port);
            goto start;
        }
        else if (key == 'O' || key == 'o')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            // Display_table_2(current_port);
            set_HTTP_IP_Table(serial_port);
            goto start;
        }
        else if (key == 'Q' || key == 'q')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            Display_table_2(current_port);
            SetHTTPSDefender(serial_port);
            goto start;
        }
        else if (key == 'Z' || key == 'z')
        {
            system("clear");
            display_logo1();
            new_menu(serial_port);
        }

        else
        {
            // system("clear");
            // display_logo1();
            // new_menu(serial_port);
            reconfig(serial_port);
        }
    }

    void SetDateTime(int serial_port)
    {
        char key_mode = '1';
        display_table(serial_port);
        write(serial_port, &key_mode, sizeof(key_mode));
        usleep(100000);
        write(serial_port, &key_enter, sizeof(key_enter));
        usleep(100000);
        printf("\r\n\t\t");
        printf("\r\n\t\t");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf(C3 "\r\n   Time - Date" RE "  " C6 "|" RE "               " C3 "Setting Time-Date for System Anti-DDoS" RE "                                                                                                                                       " C6 "|" RE);
        printf(C6 "\r\n\t\t+============================================================================================================================================================================================+" RE);
        printf(C6 "\r\n\t\t|" RE " " C3 "Enter the time in the format (YYYY-MM-DD HH:MM:SS) " RE);
        send_user_time(serial_port);
        ReturnMode2(serial_port);
    }
    void SaveEEPROM(int serial_port)
    {

        char key;
        char enter = '\r';
        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n");
        printf("\r\n " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to finish configuring now" RE " (" C2 "Y" RE "/" C1 "N" RE ")" C3 "?: " RE);
        while (1)
        {
            scanf("%c", &key);
            if (key == 'y' || key == 'Y' || key == 'n' || key == 'N')
            {
                write(serial_port, &key, sizeof(key));
                usleep(100000);
                write(serial_port, &enter, sizeof(enter));
                usleep(1000000);
                break;
            }
            if (key != 'y' || key != 'Y' || key != 'n' || key != 'N')
            {
                printf("\r   " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to finish configuring now" RE " (" C2 "Y" RE "/" C1 "N" RE ")" C6 "?: " RE);
            }
        }
        char *data = receive_data(serial_port);
        if (data == NULL)
        {
            printf(C3 "Error receiving data\n" RE);
            return;
        }
        // ////printf("Received message: %s\n", data);
        if ((strchr(data, 'y') != NULL) || (strchr(data, 'Y') != NULL))
        {
            printf_uart1(serial_port);
        }
        else if ((strchr(data, 'n') != NULL) || (strchr(data, 'N') != NULL))
        {
            ModeStart(serial_port);
        }
    }

    void SetDefenderPort(int serial_port)
    {
        current_config_type = CONFIG_PORT_DEFENDER;
        char key;
        int enable_value = -1;
        printf("\r\n");
        printf(C6 "\r\n================+================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to enable protect by interface port %d now" RE " (" C2 "Y" RE "/" C1 "N" RE ")" C6 "? " RE "", current_port);
        while (1)
        {
            scanf(" %c", &key);

            if (key == 'y' || key == 'Y')
            {
                last_update_value = 1;
                break;
            }
            else if (key == 'n' || key == 'N')
            {
                last_update_value = 0;
                break;
            }
            else
            {
                printf("\r  " C3 "SETTING" RE "       " C6 "|" RE " " C6 "Invalid input. Please enter" RE " " C2 "Y" RE " or " C1 "N" RE "" C6 ": " RE);
            }
        }

        ConfirmAndSaveConfig(serial_port);
    }

    void SetPortDefender(int serial_port)
    {
        char key_mode = '3';
        char key;
        char key1 = '1';
        char key0 = '0';
        char enter = '\r';
        write(serial_port, &key_mode, sizeof(key_mode));
        usleep(100000);
        write(serial_port, &key_enter, sizeof(key_enter));
        usleep(100000);
        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r   " C3 "SETTING" RE "   " C6 "|" RE " " C6 "Enter port number for client side (Internet side) for Port %d (1/2):  " RE "", current_port);
        while (1)
        {
            scanf("%c", &key);
            if (key == '1')
            {
                write(serial_port, &key0, sizeof(key0));
                usleep(100000);
                write(serial_port, &enter, sizeof(enter));
                usleep(1000000);
                break;
            }
            else if (key == '2')
            {
                write(serial_port, &key1, sizeof(key1));
                usleep(100000);
                write(serial_port, &enter, sizeof(enter));
                usleep(1000000);
                break;
            }
            if (key != '1' || key != '2')
            {
                printf("\r   " C3 "SETTING" RE "   " C6 "|" RE " " C6 "Enter port number for client side (Internet side) for Port %d (1/2):" RE "  ", current_port);
            }
        }

        if (key == '1' || key == '2')
        {
            ReturnMode2(serial_port);
        }
    }
    void SetTimeflood(int serial_port)
    {
        char key_mode = '5';
        write(serial_port, &key_mode, sizeof(key_mode));
        usleep(100000);
        write(serial_port, &key_enter, sizeof(key_enter));
        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter attack detection time (s): " RE "");
        send_array(serial_port);
        ReturnMode2(serial_port);
    }

    void SetSynDefender(int serial_port)
    {
        current_config_type = CONFIG_SYN_DEFENDER;
        char key;
        int enable_value = -1;
        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "       " C6 "|" RE " " C6 "Do you want to enable SYN flood protect for port %d now" RE " (" C2 "Y" RE "/" C1 "N" RE ")" C6 "?:  " RE "", current_port);
        while (1)
        {
            scanf(" %c", &key);
            if (key == 'y' || key == 'Y')
            {
                enable_value = 1;
                break;
            }
            else if (key == 'n' || key == 'N')
            {
                enable_value = 0;
                break;
            }
            else
            {
                printf("\r  " C3 "SETTING" RE "       " C6 "|" RE " " C6 "Please enter" RE " " C2 "Y" RE " or " C1 "N" RE ": ");
            }
        }
        // Luu gi  tr? t?m d? g?i sau
        last_update_value = enable_value;
        strcpy(last_update_field, "SYNFloodEnable");
        ConfirmAndSaveConfig(serial_port);
    }

    void send_Threshold_value(int serial_port, int value, const char *label, char *ID, char *Value, char *buffer)
    {
        char enter = '\r';
        char array[10];
        snprintf(array, sizeof(array), "%d", value);
        int n = strlen(array);
        for (int i = 0; i < n; i++)
        {
            write(serial_port, &array[i], sizeof(char));
            printf(C3 "%c" RE "", array[i]);
            usleep(100000);
        }
        write(serial_port, &enter, sizeof(enter));
        usleep(1000000);
        int t = 0;
        while (1)
        {
            char *resp = receive_data(serial_port);
            printf(C3 "\nReceived message: %s\n" RE "", resp);
            if ((strchr(resp, 'Y') != NULL))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nSYN_EN_DIS done\n");
                break;
            }
            else if ((strchr(resp, 'N') != NULL) || (t == 10))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                break;
            }
            t++;
            usleep(500000);
        }
    }

    void Send_enable_value(int serial_port, int enable, const char *label, char *ID, char *Value, char *buffer)
    {
        char yn_char = (enable == 1) ? 'Y' : 'N';
        char enter = '\r';
        int t = 0;

        write(serial_port, &yn_char, sizeof(yn_char));
        usleep(1000000);
        write(serial_port, &enter, sizeof(enter));
        usleep(1000000);
        // char *resp = receive_data(serial_port);
        // printf("\nReceived message: %s\n", resp);

        while (1)
        {
            char *resp = receive_data(serial_port);
            printf(C3 "\nReceived message: %s\n" RE "", resp);
            if ((strchr(resp, 'Y') != NULL))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nSYN_EN_DIS done\n");
                break;
            }
            else if ((strchr(resp, 'N') != NULL) || (t == 10))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nSYN_EN_DIS error\n");
                break;
            }
            t++;
            usleep(2000000); // ch? 0.2s tru?c l?n d?c ti?p
        }
    }

    int get_ports_in_latest_time(int current_port, int *port_list, int *port_count,
                                char *latest_start, char *latest_end, char *profile_name)
    {
        sqlite3 *db;
        sqlite3_stmt *stmt;
        *port_count = 0;
        latest_start[0] = '\0';
        latest_end[0] = '\0';
        profile_name[0] = '\0';

        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "DEBUG:" RE " " C6 "Cannot open DB" RE " " C3 "%s\n" RE "", sqlite3_errmsg(db));
            return -1;
        }

        const char *sql =
            "SELECT DefenseProfileUsingTime, DefenseProfileName FROM DefenseProfiles "
            "WHERE DefenseProfileUsingTime IS NOT NULL";

        // =====================================================
        // B1: Tìm profile + th?i gian m?i nh?t c?a current_port (b? end)
        // =====================================================
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        {
            printf(C3 "DEBUG:" RE " " C6 "Cannot prepare SQL:" RE " " C3 "%s\n" RE "", sqlite3_errmsg(db));
            sqlite3_close(db);
            return -1;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *json_text = (const char *)sqlite3_column_text(stmt, 0);
            const char *pname = (const char *)sqlite3_column_text(stmt, 1);
            if (!json_text || !pname)
                continue;

            cJSON *root = cJSON_Parse(json_text);
            if (!root || !cJSON_IsArray(root))
            {
                if (root)
                    cJSON_Delete(root);
                continue;
            }

            int arr_size = cJSON_GetArraySize(root);
            for (int i = 0; i < arr_size; i++)
            {
                cJSON *entry = cJSON_GetArrayItem(root, i);
                cJSON *iface_obj = cJSON_GetObjectItem(entry, "name"); // interface name
                cJSON *start_obj = cJSON_GetObjectItem(entry, "date"); // start time
                if (!iface_obj || !cJSON_IsString(iface_obj))
                    continue;
                if (!start_obj || !cJSON_IsString(start_obj))
                    continue;

                int port_num = 0;
                if (sscanf(iface_obj->valuestring, "eth%d", &port_num) != 1)
                    continue;
                if (port_num != current_port)
                    continue;

                const char *start = start_obj->valuestring;

                // ch?n b?n ghi m?i nh?t cho current_port d?a trên start thôi
                if (strlen(latest_start) == 0 || strcmp(start, latest_start) > 0)
                {
                    strcpy(latest_start, start);
                    snprintf(profile_name, sizeof(profile_name), "%s", pname);
                }
            }
            cJSON_Delete(root);
        }
        sqlite3_finalize(stmt);

        if (strlen(latest_start) == 0)
        {
            printf(C3 "DEBUG:" RE " " C6 "No latest entry found for port" RE " " C3 "%d\n" RE "", current_port);
            sqlite3_close(db);
            return -1;
        }
        // ...existing code...
        // B2: Tìm t?t c? port cùng profile_name (KHÔNG l?c theo date)
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        {
            printf(C3 "DEBUG:" RE " " C6 "Cannot prepare SQL second time:" RE " " C3 "%s\n" RE "", sqlite3_errmsg(db));
            sqlite3_close(db);
            return -1;
        }

        *port_count = 0;

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *json_text = (const char *)sqlite3_column_text(stmt, 0);
            const char *pname = (const char *)sqlite3_column_text(stmt, 1);
            if (!json_text || !pname)
                continue;
            if (strcmp(pname, profile_name) != 0)
                continue;

            cJSON *root = cJSON_Parse(json_text);
            if (!root || !cJSON_IsArray(root))
            {
                if (root)
                    cJSON_Delete(root);
                continue;
            }

            int arr_size = cJSON_GetArraySize(root);
            for (int i = 0; i < arr_size; i++)
            {
                cJSON *entry = cJSON_GetArrayItem(root, i);
                cJSON *iface_obj = cJSON_GetObjectItem(entry, "name");
                if (!iface_obj || !cJSON_IsString(iface_obj))
                    continue;

                int port_num = 0;
                if (sscanf(iface_obj->valuestring, C6 "eth" RE "" C3 "%d " RE "", &port_num) != 1)
                    continue;

                // L?y t?t c? các port thu?c profile_name (không l?c theo date)
                int exists = 0;
                for (int k = 0; k < *port_count; ++k)
                    if (port_list[k] == port_num)
                    {
                        exists = 1;
                        break;
                    }

                if (!exists)
                    port_list[(*port_count)++] = port_num;
            }
            cJSON_Delete(root);
        }
        sqlite3_finalize(stmt);

        sqlite3_close(db);
        return (*port_count > 0) ? 0 : -1;
    }

    void print_ports_in_profile(int *port_list, int port_count, const char *latest_start, const char *latest_end)
    {
        printf(C3 "\nPorts in the same profile (" C6 "start=" RE "" C3 "%s" RE ", " C6 "end=" RE "" C3 "%s" RE "): " RE "", latest_start, latest_end);
        for (int i = 0; i < port_count; ++i)
            printf(C6 "eth" RE "" C3 "%d " RE "", port_list[i]);
        printf(C3 "\n" RE);
    }

    void ConfirmAndSaveConfig(int serial_port)
    {
        // system("clear");
        // display_logo1();
        char key;
        char enter = '\r';
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to save this configuration?" RE " (" C2 "Y" RE "/" C1 "N" RE "): ");
        while (1)
        {
            // char buffer[256] = "";
            // char ID[32] = "";
            // char Value[64] = "";
            // scanf(" %c", &key);

            // char latest_start[64], latest_end[64], profile_name[64];
            // int port_list[16], port_count = 0;

            // int ret = get_ports_in_latest_time(current_port, port_list, &port_count, latest_start, latest_end, profile_name);
            // printf(C3 "DEBUG:" RE " " C6 "get_ports_in_latest_time returned" RE " " C3 "%d\n" RE "", ret);
            // printf(C3 "DEBUG:" RE " " C6 "Profile=" RE "" C3 "%s" RE ", " C6 "start=" RE "" C3 "%s" RE ", " C6 "end=" RE "" C3 "%s" RE ", " C6 "port_count=" RE "" C3 "%d" RE "\n", profile_name, latest_start, latest_end, port_count);
            // for (int i = 0; i < port_count; i++)
            //     printf(C3 "DEBUG:" RE " " C6 "port_list[" C3 "%d" RE "]=" C6 "eth" RE "" C3 "%d" RE "\n", i, port_list[i]);

            // print_ports_in_profile(port_list, port_count, latest_start, latest_end);
            // if (key == 'y' || key == 'Y')
            // {
            //     for (int i = 0; i < port_count; ++i)
            //     {
            //         int port = port_list[i];
            //         char port_char = '1' + (port - 1);
            char buffer[256] = "";
            char ID[32] = "";
            char Value[64] = "";
            scanf(" %c", &key);

            if (key == 'y' || key == 'Y')
            {
                char latest_start[64], latest_end[64], profile_name[64];
                int port_list[16], port_count = 0;

                // th? l?y danh sách port
                int ret = get_ports_in_latest_time(
                    current_port, port_list, &port_count,
                    latest_start, latest_end, profile_name);

                printf(C3 "DEBUG:" RE " get_ports_in_latest_time returned %d\n", ret);
                printf(C3 "DEBUG:" RE " Profile=%s, start=%s, end=%s, port_count=%d\n",
                    profile_name, latest_start, latest_end, port_count);

                // n?u không có port_list thì ch? làm v?i current_port
                int effective_count = (port_count > 0) ? port_count : 1;

                for (int i = 0; i < effective_count; i++)
                {
                    int port = (port_count > 0) ? port_list[i] : current_port;
                    char port_char = '1' + (port - 1);

                    // SYN Threshold
                    if (current_config_type == CONFIG_SYN_THRESHOLD)
                    {
                        char key_mode = '7';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);
                        send_Threshold_value(serial_port, last_update_value, "SYN", ID, Value, buffer);
                        usleep(500000);
                    }
                    // ACK Threshold
                    else if (current_config_type == CONFIG_ACK_THRESHOLD)
                    {
                        char key_mode = '8';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);
                        send_Threshold_value(serial_port, last_update_value, "ACK", ID, Value, buffer);
                        usleep(500000);
                    }
                    // UDP Threshold
                    else if (current_config_type == CONFIG_UDP_THRESHOLD)
                    {
                        char key_mode = 'C';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);
                        send_Threshold_value(serial_port, last_update_value, "UDP", ID, Value, buffer);
                        usleep(500000);
                    }
                    // UDP 1S Threshold
                    else if (current_config_type == CONFIG_UDP_1S_THRESHOLD)
                    {
                        char key_mode = 'D';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);
                        send_Threshold_value(serial_port, last_update_value, "UDP1S", ID, Value, buffer);
                        usleep(500000);
                    }
                    // DNS Threshold
                    else if (current_config_type == CONFIG_DNS_THRESHOLD)
                    {
                        char key_mode = 'F';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);
                        send_Threshold_value(serial_port, last_update_value, "DNS", ID, Value, buffer);
                        usleep(100000);
                    }
                    // ICMP Threshold
                    else if (current_config_type == CONFIG_ICMP_THRESHOLD)
                    {
                        char key_mode = 'H';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);
                        send_Threshold_value(serial_port, last_update_value, "ICMP", ID, Value, buffer);
                        usleep(100000);
                    }
                    // ICMP1S Threshold
                    else if (current_config_type == CONFIG_ICMP_1S_THRESHOLD)
                    {
                        char key_mode = 'I';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);
                        send_Threshold_value(serial_port, last_update_value, "ICMP1S", ID, Value, buffer);
                        usleep(100000);
                    }
                    // IPSEC_IKE Threshold
                    else if (current_config_type == CONFIG_IPSEC_IKE_THRESHOLD)
                    {
                        char key_mode = 'K';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);
                        send_Threshold_value(serial_port, last_update_value, "IPSEC_IKE", ID, Value, buffer);
                        usleep(100000);
                    }
                    /////////////////////////////////////////////////////////////////////////////////EN/DIS//////////////////////////////////////////////////////////////
                    // SYN DEFENDER
                    else if (current_config_type == CONFIG_SYN_DEFENDER)
                    {
                        char key_mode = '6';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);
                        Send_enable_value(serial_port, last_update_value, "SYN Attack Protect", ID, Value, buffer);
                        usleep(100000);
                    }
                    // LAND DEFENDER
                    else if (current_config_type == CONFIG_LAND_DEFENDER)
                    {
                        char key_mode = 'A';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);
                        Send_enable_value(serial_port, last_update_value, "LAND Defender Protect", ID, Value, buffer);
                        usleep(100000);
                    }
                    // UDP DEFENDER
                    else if (current_config_type == CONFIG_UDP_DEFENDER)
                    {
                        char key_mode = 'B';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);

                        Send_enable_value(serial_port, last_update_value, "UDP Defender Protect", ID, Value, buffer);
                        usleep(100000);
                    }
                    // DNS DEFENDER
                    else if (current_config_type == CONFIG_DNS_DEFENDER)
                    {
                        char key_mode = 'E';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);

                        Send_enable_value(serial_port, last_update_value, "DNS Defender Protect", ID, Value, buffer);
                        usleep(100000);
                    }
                    // ICMP DEFENDER
                    else if (current_config_type == CONFIG_ICMP_DEFENDER)
                    {
                        char key_mode = 'G';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);
                        Send_enable_value(serial_port, last_update_value, "ICMP Defender Protect", ID, Value, buffer);
                        usleep(100000);
                    }
                    // IPSEC IKE DEFENDER
                    else if (current_config_type == CONFIG_IPSEC_IKE_DEFENDER)
                    {
                        char key_mode = 'J';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);

                        Send_enable_value(serial_port, last_update_value, "IPSEC IKE Defender Protect", ID, Value, buffer);
                        usleep(100000);
                    }
                    // TCP FRAGMENT DEFENDER
                    else if (current_config_type == CONFIG_TCP_FRAG_DEFENDER)
                    {
                        char key_mode = 'N';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);
                        Send_enable_value(serial_port, last_update_value, "TCP Frag Defender Protect", ID, Value, buffer);
                        usleep(100000);
                    }
                    // UDP FRAGMENT DEFENDER
                    else if (current_config_type == CONFIG_UDP_FRAG_DEFENDER)
                    {
                        char key_mode = 'O';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);
                        Send_enable_value(serial_port, last_update_value, "UDP Frag Defender Protect", ID, Value, buffer);
                        usleep(100000);
                    }
                    // HTTP DEFENDER
                    else if (current_config_type == CONFIG_HTTP_DEFENDER)
                    {
                        char key_mode = 'P';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);
                        Send_enable_value(serial_port, last_update_value, "HTTP Defender Protect", ID, Value, buffer);
                        usleep(100000);
                    }

                    // HTTPS DEFENDER
                    else if (current_config_type == CONFIG_HTTPS_DEFENDER)
                    {
                        char key_mode = 0xFF;
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        write(serial_port, &port_char, sizeof(port_char));
                        usleep(100000);
                        Send_enable_value(serial_port, last_update_value, "HTTPS Defender Protect", ID, Value, buffer);
                        usleep(100000);
                    }

                    ///////////////////////////////////////////////////////////////////////////////////OTHER//////////////////////////////////////////////////////////////////////////////

                    // TIME WHITE LIST
                    else if (current_config_type == SETTING_TIME_WHITE_LIST)
                    {
                        char key_mode = '9';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);

                        char array[10];
                        snprintf(array, sizeof(array), "%d", last_update_value);
                        int n = strlen(array);
                        for (int i = 0; i < n; i++)
                        {
                            write(serial_port, &array[i], sizeof(char));
                            printf(C3 "%c" RE "", array[i]);
                            usleep(100000);
                        }
                        write(serial_port, &enter, sizeof(enter));
                        usleep(1000000);
                        // char *resp = receive_data(serial_port);
                        // if (resp && strchr(resp, 'Y'))
                        // {
                        //     printf("\n TIME_WHITE_LIST OK\n");
                        //     break;
                        // }
                        // else if (resp && strchr(resp, 'N'))
                        // {
                        //     printf("\n TIME_WHITE_LIST ERROR\n");
                        //     break;
                        // }
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message:" RE " " C3 "%s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }

                    // DEFENDER PORT
                    else if (current_config_type == CONFIG_PORT_DEFENDER)
                    {
                        char key_mode = 0x07;
                        printf(C3 "DEBUG: Sending key_mode:" RE " " C6 "0x%02X\n" RE "", key_mode);
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(1000000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(1000000);

                        char port_key;
                        switch (current_port)
                        {
                        case 1:
                            port_key = 'A';
                            break;
                        case 2:
                            port_key = 'B';
                            break;
                        case 3:
                            port_key = 'C';
                            break;
                        case 4:
                            port_key = 'D';
                            break;
                        case 5:
                            port_key = 'E';
                            break;
                        case 6:
                            port_key = 'F';
                            break;
                        case 7:
                            port_key = 'G';
                            break;
                        case 8:
                            port_key = 0xFF;
                            break;
                        default:
                            port_key = 'A';
                            break;
                        }
                        printf(C3 "DEBUG: Sending port_key:" RE " " C6 "0x%02X\n" RE "", port_key);
                        write(serial_port, &port_key, sizeof(port_key));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);

                        char mode_key = (last_update_value == 1) ? '1' : '0';
                        printf(C3 "DEBUG: Sending mode_key: %c\n" RE "", mode_key);
                        write(serial_port, &mode_key, sizeof(mode_key));
                        usleep(1000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);

                        // resp = receive_data(serial_port);
                        // printf("\nDEBUG: resp = %s\n", resp);
                        // if (resp && (strchr(resp, 'Y') || strstr(resp, "OK")))
                        //     printf("Defender Port OK\n");
                        // else
                        //     printf("Defender Port ERROR\n");
                        // char *resp = receive_data(serial_port);
                        // printf("\nDEBUG: resp = %s\n", resp);
                        // if (resp && (strchr(resp, 'Y') || strstr(resp, "OK")))
                        //     printf("DEFENDER Port OK\n");
                        // else
                        //     printf("DEFENDER Port ERROR\n");
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }

                    // ADD IPv4 Protection
                    else if (current_config_type == CONFIG_IPV4_PROTECT)
                    {
                        char key_07 = 0x07;
                        char enter = '\r';
                        write(serial_port, &key_07, sizeof(key_07));
                        usleep(1000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(10000);

                        char port_key;
                        switch (current_port)
                        {
                        case 1:
                            port_key = 'H';
                            break;
                        case 2:
                            port_key = 'I';
                            break;
                        case 3:
                            port_key = 'J';
                            break;
                        case 4:
                            port_key = 'K';
                            break;
                        default:
                            port_key = 'H';
                            break;
                        }
                        write(serial_port, &port_key, sizeof(port_key));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);

                        // G?i t?ng k  t? IP t? bi?n to n c?c
                        int n = strlen(temp_ipv4_address);
                        for (int i = 0; i < n; i++)
                        {
                            char data = temp_ipv4_address[i];
                            send_data(serial_port, &data, sizeof(data));
                            usleep(100000);
                        }
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        // char *resp = receive_data(serial_port);
                        // printf("\nDEBUG: resp = %s\n", resp);
                        // if (resp && (strchr(resp, 'Y') || strstr(resp, "OK")))
                        //     printf("ADD IPv4 Port OK\n");
                        // else
                        //     printf("ADD IPv4 Port ERROR\n");
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                    // REMOVE IPv4 Protection
                    else if (current_config_type == CONFIG_REMOVE_IPV4_PROTECT)
                    {
                        char key_07 = 0x07;
                        char enter = '\r';
                        write(serial_port, &key_07, sizeof(key_07));
                        usleep(1000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(10000);

                        char port_key;
                        switch (current_port)
                        {
                        case 1:
                            port_key = 'O';
                            break;
                        case 2:
                            port_key = 'P';
                            break;
                        case 3:
                            port_key = 'Q';
                            break;
                        case 4:
                            port_key = 'R';
                            break;
                        }
                        write(serial_port, &port_key, sizeof(port_key));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);

                        // G?i t?ng k  t? IP t? bi?n to n c?c
                        int n = strlen(temp_ipv4_address);
                        for (int i = 0; i < n; i++)
                        {
                            char data = temp_ipv4_address[i];
                            send_data(serial_port, &data, sizeof(data));
                            usleep(100000);
                        }
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        // char *resp = receive_data(serial_port);
                        // printf("\nDEBUG: resp = %s\n", resp);
                        // if (resp && (strchr(resp, 'Y') || strstr(resp, "OK")))
                        //     printf("REMOVE IPv4 Port OK\n");
                        // else
                        //     printf("REMOVE IPv4 Port ERROR\n");
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                    // ADD IPv6 Protect
                    else if (current_config_type == CONFIG_IPV6_PROTECT)
                    {
                        char key_07 = 0x07;
                        char enter = '\r';
                        write(serial_port, &key_07, sizeof(key_07));
                        usleep(1000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(10000);

                        char port_key;
                        switch (current_port)
                        {
                        case 1:
                            port_key = 'V';
                            break;
                        case 2:
                            port_key = 'W';
                            break;
                        case 3:
                            port_key = 'X';
                            break;
                        case 4:
                            port_key = 'Y';
                            break;
                        }
                        write(serial_port, &port_key, sizeof(port_key));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        // G?i IP t? bi?n to n c?c
                        int n = strlen(full_ipv6_address);
                        for (int i = 0; i < n; i++)
                        {
                            char data = full_ipv6_address[i];
                            send_data(serial_port, &data, sizeof(data));
                            usleep(100000); // gi? l?i delay n?u c?n thi?t cho UART
                        }
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        // char *resp = receive_data(serial_port);
                        // printf("\nDEBUG: resp = %s\n", resp);
                        // if (resp && (strchr(resp, 'Y') || strstr(resp, "OK")))
                        //     printf("ADD IPv6 Port OK\n");
                        // else
                        //     printf("ADD IPv6 Port ERROR\n");
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                    // REMOVE IPv6 Protect
                    else if (current_config_type == CONFIG_REMOVE_IPV6_PROTECT)
                    {
                        char key_07 = 0x07;
                        char enter = '\r';
                        write(serial_port, &key_07, sizeof(key_07));
                        usleep(1000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(10000);

                        char port_key;
                        switch (current_port)
                        {
                        case 1:
                            port_key = '-';
                            break;
                        case 2:
                            port_key = 0x02;
                            break;
                        case 3:
                            port_key = 0x0D;
                            break;
                        case 4:
                            port_key = 0x04;
                            break;
                        default:
                            printf(C3 "Invalid port!\n" RE);
                            return;
                        }
                        write(serial_port, &port_key, sizeof(port_key));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        int n = strlen(temp_ipv6_address);
                        for (int i = 0; i < n; i++)
                        {
                            char data = temp_ipv6_address[i];
                            send_data(serial_port, &data, sizeof(data));
                            usleep(100000);
                        }
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        // char *resp = receive_data(serial_port);
                        // printf("\nDEBUG: resp = %s\n", resp);
                        // if (resp && (strchr(resp, 'Y') || strstr(resp, "OK")))
                        //     printf("REMOVE IPV6 Port OK\n");
                        // else
                        //     printf("REMOVE IPV6 Port ERROR\n");
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                    // ADD IPv4 BLOCK
                    else if (current_config_type == CONFIG_IPV4_BLOCK)
                    {
                        char key_09 = 0x09;
                        char enter = '\r';
                        char key1 = '1';

                        write(serial_port, &key_09, sizeof(key_09));
                        usleep(10000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(10000);
                        write(serial_port, &key1, sizeof(key1));
                        usleep(100000);
                        char port_key;
                        switch (current_port)
                        {
                        case 1:
                            port_key = '1';
                            break;
                        case 2:
                            port_key = '2';
                            break;
                        case 3:
                            port_key = '3';
                            break;
                        case 4:
                            port_key = '4';
                            break;
                        default:
                            printf(C3 "Invalid port!\n" RE);
                            return;
                        }
                        write(serial_port, &port_key, sizeof(port_key));
                        usleep(100000);
                        // write(serial_port, &enter, sizeof(enter));
                        // usleep(100000);

                        int n = strlen(temp_ipv4_address);
                        for (int i = 0; i < n; i++)
                        {
                            char data = temp_ipv4_address[i];
                            send_data(serial_port, &data, sizeof(data));
                            usleep(100000);
                        }
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);
                        // char *resp = receive_data(serial_port);
                        // printf("\nDEBUG: resp = %s\n", resp);
                        // if (resp && (strchr(resp, 'Y') || strstr(resp, "OK")))
                        //     printf("ADD IPv4 Port OK\n");
                        // else
                        //     printf("ADD IPv4 Port ERROR\n");
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                    // REMOVE IPv4 BLOCK
                    else if (current_config_type == CONFIG_REMOVE_IPV4_BLOCK)
                    {
                        char key_09 = 0x09;
                        char enter = '\r';
                        char key2 = '2';

                        write(serial_port, &key_09, sizeof(key_09));
                        usleep(10000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(10000);
                        write(serial_port, &key2, sizeof(key2));
                        usleep(100000);
                        char port_key;
                        switch (current_port)
                        {
                        case 1:
                            port_key = '1';
                            break;
                        case 2:
                            port_key = '2';
                            break;
                        case 3:
                            port_key = '3';
                            break;
                        case 4:
                            port_key = '4';
                            break;
                        default:
                            printf(C3 "Invalid port!\n" RE);
                            return;
                        }
                        write(serial_port, &port_key, sizeof(port_key));
                        usleep(100000);
                        // write(serial_port, &enter, sizeof(enter));
                        // usleep(100000);

                        int n = strlen(temp_ipv4_address);
                        for (int i = 0; i < n; i++)
                        {
                            char data = temp_ipv4_address[i];
                            send_data(serial_port, &data, sizeof(data));
                            usleep(100000);
                        }
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);

                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                    // REMOVE IPV6 BLOCK
                    else if (current_config_type == CONFIG_REMOVE_IPV6_BLOCK)
                    {
                        char key_09 = 0x09;
                        char enter = '\r';
                        char key4 = '4';

                        write(serial_port, &key_09, sizeof(key_09));
                        usleep(10000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(10000);
                        write(serial_port, &key4, sizeof(key4));
                        usleep(100000);
                        char port_key;
                        switch (current_port)
                        {
                        case 1:
                            port_key = '1';
                            break;
                        case 2:
                            port_key = '2';
                            break;
                        case 3:
                            port_key = '3';
                            break;
                        case 4:
                            port_key = '4';
                            break;
                        default:
                            printf(C3 "Invalid port!\n" RE);
                            return;
                        }
                        write(serial_port, &port_key, sizeof(port_key));
                        usleep(100000);
                        // write(serial_port, &enter, sizeof(enter));
                        // usleep(100000);

                        int n = strlen(temp_ipv6_address);
                        for (int i = 0; i < n; i++)
                        {
                            char data = temp_ipv6_address[i];
                            send_data(serial_port, &data, sizeof(data));
                            usleep(100000);
                        }
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);

                        // char *resp = receive_data(serial_port);
                        // printf("\nDEBUG: resp = %s\n", resp);
                        // if (resp && (strchr(resp, 'Y') || strstr(resp, "OK")))
                        //     printf("REMOVE_BLOCK_IPv6 Port OK\n");
                        // else
                        //     printf("REMOVE_BLOCK_IPv6 Port ERROR\n");
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C2 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                    // ADD IPv6 BLOCK
                    else if (current_config_type == CONFIG_IPV6_BLOCK)
                    {
                        char key_09 = 0x09;
                        char enter = '\r';
                        char key3 = '3';

                        write(serial_port, &key_09, sizeof(key_09));
                        usleep(10000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(10000);
                        write(serial_port, &key3, sizeof(key3));
                        usleep(100000);
                        char port_key;
                        switch (current_port)
                        {
                        case 1:
                            port_key = '1';
                            break;
                        case 2:
                            port_key = '2';
                            break;
                        case 3:
                            port_key = '3';
                            break;
                        case 4:
                            port_key = '4';
                            break;
                        default:
                            printf(C3 "Invalid port!\n" RE);
                            return;
                        }
                        write(serial_port, &port_key, sizeof(port_key));
                        usleep(100000);
                        // write(serial_port, &enter, sizeof(enter));
                        // usleep(100000);

                        int n = strlen(temp_ipv6_address);
                        for (int i = 0; i < n; i++)
                        {
                            char data = temp_ipv6_address[i];
                            send_data(serial_port, &data, sizeof(data));
                            usleep(100000);
                        }
                        write(serial_port, &enter, sizeof(enter));
                        usleep(100000);

                        // char *resp = receive_data(serial_port);
                        // printf("\nDEBUG: resp = %s\n", resp);
                        // if (resp && (strchr(resp, 'Y') || strstr(resp, "OK")))
                        //     printf("ADD IPv6 Port OK\n");
                        // else
                        //     printf("ADD IPv6 Port ERROR\n");
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                    // HTTP
                    // ADD IPv4 HTTP
                    else if (current_config_type == CONFIG_IPV4_HTTP)
                    {
                        char key_mode = 'T';
                        char key_enter = '\r';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(1000000);
                        write(serial_port, &key_enter, sizeof(key_enter));
                        usleep(1000000);
                        send_ipv4_address_http_add(serial_port, temp_ipv4_address);
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                    // REMOVE IPv4 HTTP
                    else if (current_config_type == CONFIG_REMOVE_IPV4_HTTP)
                    {
                        char key_mode = 'U';
                        char key_enter = '\r';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &key_enter, sizeof(key_enter));
                        usleep(100000);
                        send_ipv4_address_http_remove(serial_port, temp_ipv4_address);
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                    // ADD IPv6 HTTP
                    else if (current_config_type == CONFIG_IPV6_HTTP)
                    {
                        char key_mode = '{';
                        char key_enter = '\r';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &key_enter, sizeof(key_enter));
                        usleep(100000);
                        send_ipv6_address_http_add(serial_port, temp_ipv6_address);
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                    // REMOVE IPV6 HTTP
                    else if (current_config_type == CONFIG_REMOVE_IPV6_HTTP)
                    {
                        char key_mode = '}';
                        char key_enter = '\r';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &key_enter, sizeof(key_enter));
                        usleep(100000);
                        send_ipv6_address_http_remove(serial_port, temp_ipv6_address);
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                    // VPN
                    // ADD IPV4 VPN
                    else if (current_config_type == CONFIG_IPV4_VPN)
                    {
                        char key_mode = 'L';
                        char key_enter = '\r';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &key_enter, sizeof(key_enter));
                        usleep(100000);
                        send_ipv4_address(serial_port, temp_ipv4_address);
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                    else if (current_config_type == CONFIG_REMOVE_IPV4_VPN)
                    {
                        char key_mode = 'M';
                        char key_enter = '\r';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &key_enter, sizeof(key_enter));
                        usleep(100000);
                        send_ipv4_address(serial_port, temp_ipv4_address);
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                    else if (current_config_type == CONFIG_IPV6_VPN)
                    {
                        char key_mode = '#';
                        char key_enter = '\r';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &key_enter, sizeof(key_enter));
                        usleep(100000);
                        send_ipv6_address(serial_port, temp_ipv6_address);
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                    else if (current_config_type == CONFIG_REMOVE_IPV6_VPN)
                    {
                        char key_mode = '^';
                        char key_enter = '\r';
                        write(serial_port, &key_mode, sizeof(key_mode));
                        usleep(100000);
                        write(serial_port, &key_enter, sizeof(key_enter));
                        usleep(100000);
                        send_ipv6_address(serial_port, temp_ipv6_address);
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message: %s\n" RE "", resp);
                            if ((strchr(resp, 'Y') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nSYN_EN_DIS done\n");
                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nSYN_EN_DIS error\n");
                                break;
                            }
                            t++;
                        }
                    }
                }
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // C?p nh?t database
                if (last_update_field[0] && last_update_value != -1)
                {
                    UpdateDefenseProfileField(current_port, last_update_field, last_update_value);
                    last_update_field[0] = '\0';
                    last_update_value = -1;
                }
                break;
            }
            else if (key == 'n' || key == 'N')
            {
                printf(C3 "\r\n    SETTING" RE "     " C6 "|" RE " " C3 "Configuration not saved.\n" RE);
                last_update_field[0] = '\0';
                last_update_value = -1;
                break;
            }
            else
            {
                printf("\r    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Please enter" RE " " C2 "Y" RE " or " C1 "N" RE ": ");
            }
        }

        if (current_config_type == CONFIG_IPV4_PROTECT || current_config_type == CONFIG_REMOVE_IPV4_PROTECT)
        {
            // system("clear");
            // display_logo1();
            Display_IPv4_Protected_Table();
        }
        else if (current_config_type == CONFIG_IPV6_PROTECT || current_config_type == CONFIG_REMOVE_IPV6_PROTECT)
        {
            // system("clear");
            // display_logo1();
            Display_IPv6_Protected_Table();
        }
        else if (current_config_type == CONFIG_IPV4_HTTP || current_config_type == CONFIG_REMOVE_IPV4_HTTP)
        {
            // system("clear");
            // display_logo1();
            set_HTTP_IP_Table(serial_port);
        }
        else if (current_config_type == CONFIG_IPV6_HTTP || current_config_type == CONFIG_REMOVE_IPV6_HTTP)
        {
            // system("clear");
            // display_logo1();
            set_HTTP_IP_Table(serial_port);
        }
        else if (current_config_type == CONFIG_IPV4_BLOCK || current_config_type == CONFIG_REMOVE_IPV6_BLOCK)
        {
            // system("clear");
            // display_logo1();
            Display_IPv4_block_table();
        }
        else if (current_config_type == CONFIG_IPV6_BLOCK || current_config_type == CONFIG_REMOVE_IPV6_BLOCK)
        {
            // system("clear");
            // display_logo1();
            Display_IPv6_block_table();
        }
        else if (current_config_type == CONFIG_IPV4_VPN || current_config_type == CONFIG_REMOVE_IPV4_VPN)
        {
            // system("clear");
            // display_logo1();
            Display_IPv4_vpn_table();
        }
        else if (current_config_type == CONFIG_IPV6_VPN || current_config_type == CONFIG_REMOVE_IPV6_VPN)
        {
            // system("clear");
            // display_logo1();
            Display_IPv6_vpn_table();
        }
        else if (current_config_type == CONFIG_IPV4_HTTP || current_config_type == CONFIG_REMOVE_IPV4_HTTP)
        {
            // system("clear");
            // display_logo1();
            set_HTTP_IP_Table(serial_port);
        }
        else if (current_config_type == CONFIG_IPV6_HTTP || current_config_type == CONFIG_REMOVE_IPV6_HTTP)
        {
            // system("clear");
            // display_logo1();
            set_HTTP_IP_Table(serial_port);
        }
        else
        {
            // system("clear");
            // display_logo1();
            Display_table_2(current_port);
        }
        current_config_type = CONFIG_NONE;
        ReturnMode2(serial_port);
    }

    void SetSynThresh(int serial_port)
    {
        current_config_type = CONFIG_SYN_THRESHOLD;

        printf(C6 "\r\n================+====================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter the value of incoming SYN packet threshold (PPS) : " RE);
        last_update_value = send_array(serial_port);
        strcpy(last_update_field, "SYNFloodSYNThreshold");
        ConfirmAndSaveConfig(serial_port);
    }

    void SetAckThresh(int serial_port)
    {
        current_config_type = CONFIG_ACK_THRESHOLD;
        printf(C6 "\r\n================+====================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter the value of incoming ACK packet threshold (PPS) : " RE);
        last_update_value = send_array(serial_port);
        strcpy(last_update_field, "SYNFloodACKThreshold");
        ConfirmAndSaveConfig(serial_port);
    }

    void SetDurationTime(int serial_port)
    {
        // char key_mode = '%';
        // write(serial_port, &key_mode, sizeof(key_mode));
        // usleep(100000);
        // write(serial_port, &key_enter, sizeof(key_enter));
        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter the value of duration time export" RE);
        send_duration_time(serial_port);
        ReturnMode2(serial_port);
    }
    void SetTimeDelete(int serial_port)
    {
        current_config_type = SETTING_TIME_WHITE_LIST;
        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter the time to delete the whitelist's information (s): " RE);
        last_update_value = send_array(serial_port);
        strcpy(last_update_field, "SYNFloodWhiteListTimeOut");
        ConfirmAndSaveConfig(serial_port);
    }
    void SetSynonymousDefender(int serial_port)
    {
        current_config_type = CONFIG_LAND_DEFENDER;
        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to enable LAND Attack protect for port %d now" RE " (" C2 "Y" RE "/" C1 "N" RE ")" C6 "?: " RE "", current_port);
        char key;
        int enable_value = -1;
        while (1)
        {
            scanf(" %c", &key);
            if (key == 'y' || key == 'Y')
            {
                enable_value = 1;
                break;
            }
            else if (key == 'n' || key == 'N')
            {
                enable_value = 0;
                break;
            }
            else
            {
                printf("    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Invalid input. Please enter " C2 "Y" RE " or " C1 "N" RE ": " RE);
            }
        }
        // gui cau hinh va update tru?ng database
        last_update_value = enable_value;
        strcpy(last_update_field, "LANDAttackEnable");

        usleep(100000);
        ConfirmAndSaveConfig(serial_port); // X c nh?n v  g?i UART
    }

    void SetUDPDefender(int serial_port)
    {
        current_config_type = CONFIG_UDP_DEFENDER;
        char key;
        int enable_value = -1;
        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to enable UDP flood protect for port %d now" RE " (" C2 "Y" RE "/" C1 "N" RE ")" C6 "?:" RE "", current_port);
        while (1)
        {
            scanf(" %c", &key);
            if (key == 'y' || key == 'Y')
            {
                enable_value = 1;
                break;
            }
            else if (key == 'n' || key == 'N')
            {
                enable_value = 0;
                break;
            }
            else
            {
                printf("    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Invalid input. Please enter " C2 "Y" RE " or " C1 "N" RE ": ");
            }
        }

        // G n gi  tr? c?u h nh v  tru?ng tuong ?ng
        last_update_value = enable_value;
        strcpy(last_update_field, "UDPFloodEnable");
        usleep(100000);
        ConfirmAndSaveConfig(serial_port);
    }

    void SetUDPThresh(int serial_port)
    {
        current_config_type = CONFIG_UDP_THRESHOLD;

        printf(C6 "\r\n================+====================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter the value of incoming SYN packet threshold (PPS) : " RE);
        last_update_value = send_array(serial_port);
        strcpy(last_update_field, "UDPFloodThreshold");

        ConfirmAndSaveConfig(serial_port);
    }

    void SetUDPThresh1s(int serial_port)
    {
        current_config_type = CONFIG_UDP_1S_THRESHOLD;

        printf(C6 "\r\n================+====================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter the value of incoming SYN packet threshold (PPS) : " RE);
        last_update_value = send_array(serial_port); // NH?P, CHUA G?I UART
        strcpy(last_update_field, "UDPFloodRate");

        ConfirmAndSaveConfig(serial_port);
    }

    void SetDNSDefender(int serial_port)
    {
        current_config_type = CONFIG_DNS_DEFENDER;

        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to enable DNS flood protect for port %d now" RE " (" C2 "Y" RE "/" C1 "N" RE ")" C6 "?: " RE "", current_port);
        char key;
        int enable_value = -1;

        while (1)
        {
            scanf(" %c", &key);
            if (key == 'y' || key == 'Y')
            {
                enable_value = 1;
                break;
            }
            else if (key == 'n' || key == 'N')
            {
                enable_value = 0;
                break;
            }
            else
            {
                printf("    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Invalid input. Please enter " C2 "Y" RE " or " C1 "N" RE ": " RE);
            }
        }

        // G n gi  tr? c?u h nh v  tru?ng tuong ?ng
        last_update_value = enable_value;
        strcpy(last_update_field, "DNSFloodEnable");

        usleep(100000);
        ConfirmAndSaveConfig(serial_port); // X c nh?n v  g?i UART
    }

    void SetDNSThresh(int serial_port)
    {
        current_config_type = CONFIG_DNS_THRESHOLD;

        printf(C6 "\r\n================+====================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter the value of incoming SYN packet threshold (PPS) : " RE);
        last_update_value = send_array(serial_port); // NH?P, CHUA G?I UART
        strcpy(last_update_field, "DNSFloodThreshold");

        ConfirmAndSaveConfig(serial_port);
    }

    void SetICMPDefender(int serial_port)
    {
        current_config_type = CONFIG_ICMP_DEFENDER;

        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to enable ICMP Attack protect for port %d now" RE " (" C2 "Y" RE "/" C1 "N" RE ")" C6 "?: " RE "", current_port);
        char key;
        int enable_value = -1;

        while (1)
        {
            scanf(" %c", &key);
            if (key == 'y' || key == 'Y')
            {
                enable_value = 1;
                break;
            }
            else if (key == 'n' || key == 'N')
            {
                enable_value = 0;
                break;
            }
            else
            {
                printf("    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Invalid input. Please enter " C2 "Y" RE " or " C1 "N" RE ": " RE);
            }
        }

        // G n gi  tr? c?u h nh v  tru?ng tuong ?ng
        last_update_value = enable_value;
        strcpy(last_update_field, "ICMPFloodEnable");

        usleep(100000);
        ConfirmAndSaveConfig(serial_port); // X c nh?n v  g?i UART
    }

    void SetICMPThresh(int serial_port)
    {
        current_config_type = CONFIG_ICMP_THRESHOLD;

        printf(C6 "\r\n================+====================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter the value of incoming SYN packet threshold (PPS) : " RE);
        last_update_value = send_array(serial_port);
        strcpy(last_update_field, "ICMPFloodThreshold");

        ConfirmAndSaveConfig(serial_port);
    }
    void SetICMPThresh1s(int serial_port)
    {
        current_config_type = CONFIG_ICMP_1S_THRESHOLD;

        printf(C6 "\r\n================+====================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter the value of incoming SYN packet threshold (PPS) : " RE);
        last_update_value = send_array(serial_port); // NH?P, CHUA G?I UART
        strcpy(last_update_field, "ICMPFloodRate");

        ConfirmAndSaveConfig(serial_port);
    }

    void SetIPSecDefender(int serial_port)
    {
        current_config_type = CONFIG_IPSEC_IKE_DEFENDER;

        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to enable IPSec IKE Attack protect for port %d now" RE " (" C2 "Y" RE "/" C1 "N" RE ")" C6 "?: " RE "", current_port);

        char key;
        int enable_value = -1;

        while (1)
        {
            scanf(" %c", &key);
            if (key == 'y' || key == 'Y')
            {
                enable_value = 1;
                break;
            }
            else if (key == 'n' || key == 'N')
            {
                enable_value = 0;
                break;
            }
            else
            {
                printf("    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Invalid input. Please enter " C2 "Y" RE " or " C1 "N" RE ": " RE);
            }
        }

        // G n gi  tr? c?u h nh v  tru?ng tuong ?ng
        last_update_value = enable_value;
        strcpy(last_update_field, "IPSecIKEEnable");

        usleep(100000);
        ConfirmAndSaveConfig(serial_port); // X c nh?n v  g?i UART
    }

    void SetIPSecThresh(int serial_port)
    {
        current_config_type = CONFIG_IPSEC_IKE_THRESHOLD;

        printf(C6 "\r\n================+====================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter the value of incoming SYN packet threshold (PPS) : " RE);
        last_update_value = send_array(serial_port);
        strcpy(last_update_field, "IPSecIKEThreshold");

        ConfirmAndSaveConfig(serial_port);
    }

    void AddIPv4VPN(int serial_port)
    {
        // char key_mode = 'L';
        // char key_enter = '\r';
        // write(serial_port, &key_mode, sizeof(key_mode));
        // usleep(100000);
        current_config_type = CONFIG_IPV4_VPN;
        printf("\r\n");
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv4 address want to block : " RE);
        while (1)
        {
            scanf("%15s", temp_ipv4_address);
            if (validate_ip_address(temp_ipv4_address))
            {
                update_vpn_ipv4(temp_ipv4_address); // C?p nh?t v o database (n?u c )
                break;
            }
            else
            {
                printf(" " C3 "Invalid!" RE " " C6 "Please input again: " RE);
            }
        }
        process_ip(LOGFILE_HTTP_IPv4, temp_ipv4_address); // ghi log n?u c?n
        ConfirmAndSaveConfig(serial_port);                // g?i h m x c nh?n
    }

    void RemoveIPv4VPN(int serial_port)
    {
        current_config_type = CONFIG_REMOVE_IPV4_VPN;
        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv4 VPN address want to add: " RE);

        scanf("%s", temp_ipv4_address);

        if (validate_ip_address(temp_ipv4_address))
        {
            delete_vpn_ipv4(temp_ipv4_address);                // C?p nh?t v o database (n?u c )
            send_ipv4_address(serial_port, temp_ipv4_address); // G?i xu?ng board
        }
        else
        {
            printf(C3 "Invalid IP format, please try again!\n" RE);
        }

        // ReturnMode2(serial_port);
        ConfirmAndSaveConfig(serial_port); // g?i h m x?c nh?n
    }

    void AddIPv6VPN(int serial_port)
    {
        current_config_type = CONFIG_IPV6_VPN;
        printf("\r\n");
        printf(C6 "================+============================================================================================================================================================================================+\n" RE);
        printf("    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv6 VPN address to add: " RE);

        scanf("%s", temp_ipv6_address);

        if (validate_ip_address(temp_ipv6_address))
        {
            update_vpn_ipv6(temp_ipv6_address);                // C?p nh?t v o database (n?u c )
            send_ipv6_address(serial_port, temp_ipv6_address); // G?i xu?ng board
        }
        else
        {
            printf(C3 "Invalid IP format, please try again!\n" RE);
        }

        // ReturnMode2(serial_port);
        ConfirmAndSaveConfig(serial_port); // g?i h m x?c nh?n
    }

    void RemoveIPv6VPN(int serial_port)
    {
        // char key_mode = '^';
        // char key_enter = '\r';
        // write(serial_port, &key_mode, sizeof(key_mode));
        // usleep(100000);
        // write(serial_port, &key_enter, sizeof(key_enter));
        current_config_type = CONFIG_REMOVE_IPV6_VPN;
        printf("\r\n");
        printf(C6 "================+============================================================================================================================================================================================+\n" RE);
        printf("    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv6 VPN address to add: " RE);

        scanf("%s", temp_ipv6_address);

        if (validate_ip_address(temp_ipv6_address))
        {
            delete_vpn_ipv6(temp_ipv6_address);                // C?p nh?t v o database (n?u c )
            send_ipv6_address(serial_port, temp_ipv6_address); // G?i xu?ng board
        }
        else
        {
            printf(C3 "Invalid IP format, please try again!\n" RE);
        }

        // ReturnMode2(serial_port);
        ConfirmAndSaveConfig(serial_port);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void SetTCPFragDefender(int serial_port)
    {
        current_config_type = CONFIG_TCP_FRAG_DEFENDER;

        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to enable TCP Fragmentation flood protect for port %d now (" C2 "Y" RE "/" C1 "N" RE ")" C6 "?: " RE "", current_port);
        char key;
        int enable_value = -1;

        while (1)
        {
            scanf(" %c", &key);
            if (key == 'y' || key == 'Y')
            {
                enable_value = 1;
                break;
            }
            else if (key == 'n' || key == 'N')
            {
                enable_value = 0;
                break;
            }
            else
            {
                printf("    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Invalid input. Please enter " C2 "Y" RE " or " C1 "N" RE ": " RE);
            }
        }

        // G n gi  tr? c?u h nh v  tru?ng
        last_update_value = enable_value;
        strcpy(last_update_field, "TCPFragmentEnable");

        usleep(100000);
        ConfirmAndSaveConfig(serial_port); // X c nh?n v  g?i UART
    }

    void set_HTTP_IP_Table(int serial_port)
    {
    start_http:
        system("clear");
        display_logo1();
        char key = 0;
        char enter = '\r';

        printf("\r\n");
        printf(C6 "\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n"
                "RE");
        printf(C6 "\t\t|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                              " C6 "|\r\n"
                "RE");
        printf(C6 "\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n"
                "RE");
        printf(C6 "\t\t|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "Show Attacker's IP Table" RE "                                                                                 	 " C6 "|\r\n"
                "RE");
        printf(C6 "\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n"
                "RE");
        printf(C6 "\t\t|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 "Add IPv4 HTTP/HTTPs Table" RE "                                                                               	 " C6 "|\r\n"
                "RE");
        printf(C6 "\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n"
                "RE");
        printf(C6 "\t\t|" RE "     " C3 "3." RE "    " C6 "|" RE " " C6 "Add IPv6 HTTP/HTTPs Table" RE "                                                                              	 " C6 "|\r\n"
                "RE");
        printf(C6 "\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n"
                "RE");
        printf(C6 "\t\t|" RE "     " C3 "4." RE "    " C6 "|" RE " " C6 "Remove IPv4 HTTP/HTTPs Table" RE "                                                                               " C6 "|\r\n"
                "RE");
        printf(C6 "\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n"
                "RE");
        printf(C6 "\t\t|" RE "     " C3 "5." RE "    " C6 "|" RE " " C6 "Remove IPv6 HTTP/HTTPs Table" RE "                                                                               " C6 "|\r\n"
                "RE");
        printf(C6 "\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n"
                "RE");
        printf(C6 "\t\t|" RE "     " C3 "6." RE "    " C6 "|" RE " " C17 "Exit" RE "                                                                                                       " C6 "|\r\n"
                "RE");
        printf(C6 "\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n"
                "RE");
        printf(C3 "\t\tPlease choose mode:  " RE);
        while (1)
        {
            scanf("%c", &key);

            if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6')
            {
                break;
            }
            else if (key != '1' || key != '2' || key != '3' || key != '4' || key != '5' || key != '6')
            {
                printf("\r     " C3 "SETTING" RE "     | " C6 "--> Please choose Mode: " RE);
            }
        }
        if (key == '1')
        {
            system("clear");
            display_logo1();
            Display_IP_http_table();
            getchar();
            // getchar();
            system("clear");
            display_logo1();
            goto start_http;
        }
        else if (key == '2')
        {
            system("clear");
            display_logo1();
            Display_http_ipv4_table();
            SetIPv4HTTPBlock(serial_port);
            system("clear");
            display_logo1();
            // SetIPv4Block(serial_port);
            goto start_http;
        }
        else if (key == '3')
        {
            system("clear");
            display_logo1();
            Display_http_ipv6_table();
            SetIPv6HTTPBlock(serial_port);
            // SetIPv6Block(serial_port);
            system("clear");
            display_logo1();
            goto start_http;
        }
        else if (key == '4')
        {
            system("clear");
            display_logo1();
            Display_http_ipv4_table();
            RemoveIPv4HTTPBlock(serial_port);
            // RemoveIPv4Block(serial_port);
            system("clear");
            display_logo1();
            goto start_http;
        }
        else if (key == '5')
        {
            system("clear");
            display_logo1();
            Display_http_ipv6_table();
            RemoveIPv6HTTPBlock(serial_port);
            // RemoveIPv6Block(serial_port);
            system("clear");
            display_logo1();
            goto start_http;
        }
        else if (key == 'Z' || key == 'z')
        {
            system("clear");
            display_logo1();
            reconfig(serial_port); // Quay l?i menu ch  nh
        }
        if (key == '2' || key == '3' || key == '4' || key == '5')
        {
            ConfirmAndSaveConfig(serial_port); // h?i x c nh?n tru?c khi g?i
        }
        goto start_http;
        // ReturnMode2(serial_port);
        // ConfirmAndSaveConfig(serial_port); // h?i x c nh?n tru?c khi g?i
    }
    void SetIPv4HTTPBlock(int serial_port)
    {
        current_config_type = CONFIG_IPV4_HTTP;

        printf("\r\n");
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv4 address want to block : " RE);
        while (1)
        {
            scanf("%15s", temp_ipv4_address);
            if (validate_ip_address(temp_ipv4_address))
            {
                update_http_ipv4(temp_ipv4_address); // C?p nh?t v o database (n?u c )
                break;
            }
            else
            {
                printf(C3 " Invalid! Please input again: " RE);
            }
        }
        process_ip(LOGFILE_HTTP_IPv4, temp_ipv4_address); // ghi log n?u c?n
        ConfirmAndSaveConfig(serial_port);                // g?i h m x c nh?n
    }

    void SetIPv6HTTPBlock(int serial_port)
    {
        current_config_type = CONFIG_IPV6_HTTP;

        printf("\r\n");
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv4 address want to block : " RE);
        while (1)
        {
            scanf("%15s", temp_ipv6_address);
            if (validate_ip_address(temp_ipv6_address))
            {
                update_http_ipv6(temp_ipv6_address); // C?p nh?t v o database (n?u c )
                break;
            }
            else
            {
                printf(C3 " Invalid! Please input again: " RE);
            }
        }
        process_ip(LOGFILE_HTTP_IPv6, temp_ipv6_address); // ghi log n?u c?n
        ConfirmAndSaveConfig(serial_port);                // g?i h m x c nh?n
    }

    void RemoveIPv4HTTPBlock(int serial_port)
    {
        current_config_type = CONFIG_REMOVE_IPV4_HTTP;
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv4 address you want to REMOVE from block list: " RE);

        while (1)
        {
            scanf("%15s", temp_ipv4_address);
            if (validate_ip_address(temp_ipv4_address))
            {
                delete_http_ipv4(temp_ipv4_address); // C?p nh?t v o database (n?u c )
                break;
            }
            else
            {
                printf(C3 " Invalid! Please input again: " RE);
            }
        }
        ConfirmAndSaveConfig(serial_port); // h?i x c nh?n tru?c khi g?i
    }

    void RemoveIPv6HTTPBlock(int serial_port)
    {
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv4 address you want to REMOVE from block list: " RE);

        while (1)
        {
            scanf("%15s", temp_ipv6_address);
            if (validate_ip_address(temp_ipv6_address))
            {

                delete_http_ipv6(temp_ipv6_address); // C?p nh?t v o database (n?u c )
                break;
            }
            else
            {
                printf(C3 " Invalid! Please input again: " RE);
            }
        }

        current_config_type = CONFIG_REMOVE_IPV6_HTTP;

        ConfirmAndSaveConfig(serial_port); // h?i x c nh?n tru?c khi g?i
    }

    void SetHTTPDefender(int serial_port)
    {
        current_config_type = CONFIG_HTTP_DEFENDER;

        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to enable HTTP GET flood protect for port %d now (" C2 "Y" RE "/" C1 "N" RE ")?: " RE "", current_port);
        char key;
        int enable_value = -1;

        while (1)
        {
            scanf(" %c", &key);
            if (key == 'y' || key == 'Y')
            {
                enable_value = 1;
                break;
            }
            else if (key == 'n' || key == 'N')
            {
                enable_value = 0;
                break;
            }
            else
            {
                printf("    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Invalid input. Please enter " C2 "Y" RE " or " C1 "N" RE ": " RE);
            }
        }

        // G n gi  tr? c?u h nh v  tru?ng tuong ?ng
        last_update_value = enable_value;
        strcpy(last_update_field, "HTTPFloodEnable");

        usleep(100000);
        ConfirmAndSaveConfig(serial_port); // X c nh?n v  g?i UART
    }

    void SetHTTPSDefender(int serial_port)
    {
        current_config_type = CONFIG_HTTPS_DEFENDER;

        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to enable HTTPS GET flood protect for port %d now (" C2 "Y" RE "/" C1 "N" RE ")?: " RE "", current_port);
        char key;
        int enable_value = -1;

        while (1)
        {
            scanf(" %c", &key);
            if (key == 'y' || key == 'Y')
            {
                enable_value = 1;
                break;
            }
            else if (key == 'n' || key == 'N')
            {
                enable_value = 0;
                break;
            }
            else
            {
                printf("    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Invalid input. Please enter " C2 "Y" RE " or " C1 "N" RE ": " RE);
            }
        }

        // G n gi  tr? c?u h nh v  tru?ng tuong ?ng
        last_update_value = enable_value;
        strcpy(last_update_field, "HTTPSFloodEnable");

        usleep(100000);
        ConfirmAndSaveConfig(serial_port); // X c nh?n v  g?i UART
    }

    void SetUDPFragDefender(int serial_port)
    {
        current_config_type = CONFIG_UDP_FRAG_DEFENDER;

        printf("\r\n");
        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to enable UDP Fragmentation flood protect for port %d now (" C2 "Y" RE "/" C1 "N" RE ")?: " RE "", current_port);
        char key;
        int enable_value = -1;

        while (1)
        {
            scanf(" %c", &key);
            if (key == 'y' || key == 'Y')
            {
                enable_value = 1;
                break;
            }
            else if (key == 'n' || key == 'N')
            {
                enable_value = 0;
                break;
            }
            else
            {
                printf("    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Invalid input. Please enter " C2 "Y" RE " or " C1 "N" RE ": " RE);
            }
        }

        // G n gi  tr? c?u h nh v  tru?ng
        last_update_value = enable_value;
        strcpy(last_update_field, "UDPFragmentEnable");

        usleep(100000);
        ConfirmAndSaveConfig(serial_port); // X c nh?n v  g?i UART
    }

    // void send_ipv4_address(int serial_port)
    // {

    //     // system("clear");
    //     char ip_address[16];
    //     char enter[] = {'\r'};
    //     while (1)
    //     {
    //         // printf("Nh?p d?a ch? IP: ");
    //         scanf("%15s", ip_address);
    //         if (validate_ip_address(ip_address))
    //         {
    //             break;
    //         }
    //         else
    //         {
    //             printf(" Invalid! Please input!!!");
    //         }
    //     }
    //     // process_ip(LOGFILE_HTTP_IPv4, ip_address);
    //     int n = strlen(ip_address);
    //     for (int i = 0; i < n; i++)
    //     {
    //         char data[2] = {ip_address[i], '\0'};
    //         send_data(serial_port, data, sizeof(data) - 1);
    //     }
    //     sleep(1);
    //     // write(serial_port, enter, sizeof(enter));
    // }
    void send_ipv4_address(int serial_port, const char *ip_address)
    {
        char enter[] = {'\r'};

        if (!validate_ip_address(ip_address))
        {
            printf(C3 "Invalid IP address format!\n" RE);
            return;
        }

        int n = strlen(ip_address);
        for (int i = 0; i < n; i++)
        {
            char data[2] = {ip_address[i], '\0'};
            send_data(serial_port, data, sizeof(data) - 1);
        }

        sleep(1);
        write(serial_port, enter, sizeof(enter));
    }

    // --- Fix send_ipv4_address_http_add ---
    void send_ipv4_address_http_add(int serial_port, const char *ip_address)
    {
        char enter[] = {'\r'};
        if (!validate_ip_address(ip_address))
        {
            printf(C3 "Invalid IP address format!\n" RE);
            return;
        }
        process_ip(LOGFILE_HTTP_IPv4, ip_address);
        int n = strlen(ip_address);
        for (int i = 0; i < n; i++)
        {
            char data[2] = {ip_address[i], '\0'};
            send_data(serial_port, data, sizeof(data) - 1);
        }
        sleep(1);
        write(serial_port, enter, sizeof(enter));
    }

    // --- Fix send_ipv6_address_http_add ---
    void send_ipv6_address_http_add(int serial_port, const char *ipv6_address)
    {
        char full_ipv6_address[40];
        char enter[] = {'\r'};
        struct in6_addr addr;
        unsigned short ipv6_segments[8];

        if (inet_pton(AF_INET6, ipv6_address, &addr) == 1)
        {
            for (int i = 0; i < 8; i++)
            {
                ipv6_segments[i] = (addr.s6_addr[i * 2] << 8) | addr.s6_addr[i * 2 + 1];
            }
            snprintf(full_ipv6_address, sizeof(full_ipv6_address),
                    "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
                    ipv6_segments[0], ipv6_segments[1], ipv6_segments[2], ipv6_segments[3],
                    ipv6_segments[4], ipv6_segments[5], ipv6_segments[6], ipv6_segments[7]);
        }
        else
        {
            printf(C3 "Invalid IPv6 address. Please re-enter:\n" RE);
            return;
        }
        process_ip(LOGFILE_HTTP_IPv6, full_ipv6_address);
        int n = strlen(full_ipv6_address);
        for (int i = 0; i < n; i++)
        {
            char data = full_ipv6_address[i];
            send_data(serial_port, &data, sizeof(data));
        }
        sleep(1);
        write(serial_port, enter, sizeof(enter));
    }

    // --- Fix send_ipv6_address_http_remove ---
    void send_ipv6_address_http_remove(int serial_port, const char *ipv6_address)
    {
        char full_ipv6_address[40];
        char enter[] = {'\r'};
        struct in6_addr addr;
        unsigned short ipv6_segments[8];

        if (inet_pton(AF_INET6, ipv6_address, &addr) == 1)
        {
            for (int i = 0; i < 8; i++)
            {
                ipv6_segments[i] = (addr.s6_addr[i * 2] << 8) | addr.s6_addr[i * 2 + 1];
            }
            snprintf(full_ipv6_address, sizeof(full_ipv6_address),
                    "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
                    ipv6_segments[0], ipv6_segments[1], ipv6_segments[2], ipv6_segments[3],
                    ipv6_segments[4], ipv6_segments[5], ipv6_segments[6], ipv6_segments[7]);
        }
        else
        {
            printf(C3 "Invalid IPv6 address. Please re-enter:\n" RE);
            return;
        }
        remove_ip_from_file(LOGFILE_HTTP_IPv6, full_ipv6_address);
        remove_ip_HTTP_from_hash(full_ipv6_address);
        int n = strlen(full_ipv6_address);
        for (int i = 0; i < n; i++)
        {
            char data = full_ipv6_address[i];
            send_data(serial_port, &data, sizeof(data));
        }
        sleep(1);
        write(serial_port, enter, sizeof(enter));
        new_menu(serial_port); // Pass serial_port as argument
    }

    // --- Remove duplicate handle_signal definition ---
    // Only keep one definition of handle_signal in your file!

    // void send_ipv4_address_http_remove(int serial_port)
    // {
    //     // system("clear");
    //     char ip_address[16];
    //     char enter[] = {'\r'};
    //     while (1)
    //     {
    //         //  printf("Nh?p d?a ch? IP: ");
    //         scanf("%15s", ip_address);
    //         if (validate_ip_address(ip_address))
    //         {
    //             break;
    //         }
    //         else
    //         {
    //             printf(" Invalid! Please input!!!");
    //         }
    //     }
    //     remove_ip_from_file(LOGFILE_HTTP_IPv4, ip_address);
    //     remove_ip_HTTP_from_hash(ip_address);
    //     int n = strlen(ip_address);
    //     for (int i = 0; i < n; i++)
    //     {
    //         char data[2] = {ip_address[i], '\0'};
    //         send_data(serial_port, data, sizeof(data) - 1);
    //     }
    //     sleep(1);
    //     write(serial_port, enter, sizeof(enter));
    // }
    void send_ipv4_address_http_remove(int serial_port, const char *ip_address)
    {
        char enter = '\r';

        // G? kh?i file + hash table
        remove_ip_from_file(LOGFILE_HTTP_IPv4, ip_address);
        remove_ip_HTTP_from_hash(ip_address);

        // G?i t?ng k  t? c?a IP
        int n = strlen(ip_address);
        for (int i = 0; i < n; i++)
        {
            char data = ip_address[i];
            write(serial_port, &data, 1);
        }

        // G?i enter sau IP
        usleep(100000);
        write(serial_port, &enter, 1);
    }

    int validate_ip_address(const char *ip_address)
    {
        regex_t regex;
        int reti;

        // Bi?u th?c ch nh quy d? ki?m tra d?nh d?ng IP
        const char *pattern = "^([0-9]{1,3}\\.){3}[0-9]{1,3}$";

        // Bi n d?ch bi?u th?c ch nh quy
        reti = regcomp(&regex, pattern, REG_EXTENDED);
        if (reti)
        {
            printf(C3 "Could not compile regex\n" RE);
            return 0; // Kh ng h?p l? n?u regex kh ng bi n d?ch du?c
        }

        // So kh?p d?a ch? IP v?i bi?u th?c ch nh quy
        reti = regexec(&regex, ip_address, 0, NULL, 0);

        // Gi?i ph ng b? nh? du?c s? d?ng b?i bi?u th?c ch nh quy
        regfree(&regex);

        if (reti)
        {
            return 0; // Kh ng h?p l? n?u d?nh d?ng kh ng kh?p
        }

        // N?u d?nh d?ng h?p l?, ki?m tra gi  tr? c?a t?ng ph?n
        int octet[4];
        sscanf(ip_address, "%d.%d.%d.%d", &octet[0], &octet[1], &octet[2], &octet[3]);

        for (int i = 0; i < 4; i++)
        {
            if (octet[i] < 0 || octet[i] > 255)
            {
                return 0;
            }
        }

        return 1;
    }

    int is_valid_mac_address(const char *mac)
    {
        if (strlen(mac) != 17)
            return 0;

        for (int i = 0; i < 17; i++)
        {
            if ((i + 1) % 3 == 0)
            {
                if (mac[i] != ':' && mac[i] != '-')
                    return 0; // cho ph p c? ":" v  "-"
            }
            else
            {
                if (!isxdigit(mac[i]))
                    return 0; // ph?i l  k  t? hex: 0-9, a-f, A-F
            }
        }
        return 1;
    }

    // void send_ipv6_address(int serial_port)
    // {
    //     char ipv6_address[40];
    //     char full_ipv6_address[40];
    //     char enter[] = {'\r'};
    //     struct in6_addr addr;
    //     unsigned short ipv6_segments[8];

    //     while (1)
    //     {
    //         scanf("%39s", ipv6_address);
    //         if (inet_pton(AF_INET6, ipv6_address, &addr) == 1)
    //         {
    //             for (int i = 0; i < 8; i++)
    //             {
    //                 ipv6_segments[i] = (addr.s6_addr[i * 2] << 8) | addr.s6_addr[i * 2 + 1];
    //             }
    //             snprintf(full_ipv6_address, sizeof(full_ipv6_address),
    //                      "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
    //                      ipv6_segments[0], ipv6_segments[1], ipv6_segments[2], ipv6_segments[3],
    //                      ipv6_segments[4], ipv6_segments[5], ipv6_segments[6], ipv6_segments[7]);
    //             break;
    //         }
    //         else
    //         {
    //             printf("Invalid IPv6 address. Please re-enter:");
    //         }
    //     }
    //     // int n = strlen(full_ipv6_address);
    //     // for (int i = 0; i < n; i++)
    //     // {
    //     //   char data = full_ipv6_address[i];
    //     //   send_data(serial_port, &data, sizeof(data));
    //     // }
    //     // sleep(1);
    //     // write(serial_port, enter, sizeof(enter));
    // }

    void send_ipv6_address(int serial_port, const char *ipv6_address)
    {
        char full_ipv6_address[40];
        char enter[] = {'\r'};
        struct in6_addr addr;
        unsigned short ipv6_segments[8];

        if (inet_pton(AF_INET6, ipv6_address, &addr) == 1)
        {
            for (int i = 0; i < 8; i++)
            {
                ipv6_segments[i] = (addr.s6_addr[i * 2] << 8) | addr.s6_addr[i * 2 + 1];
            }
            snprintf(full_ipv6_address, sizeof(full_ipv6_address),
                    "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
                    ipv6_segments[0], ipv6_segments[1], ipv6_segments[2], ipv6_segments[3],
                    ipv6_segments[4], ipv6_segments[5], ipv6_segments[6], ipv6_segments[7]);

            // G?i t?ng byte c?a d?a ch?
            for (int i = 0; i < strlen(full_ipv6_address); i++)
            {
                char data = full_ipv6_address[i];
                write(serial_port, &data, sizeof(data));
            }
            usleep(100000);
            write(serial_port, enter, sizeof(enter));
        }
        else
        {
            printf(C3 "Invalid IPv6 address format.\n" RE);
        }
    }
    // Remove IP from file
    void remove_ip_from_file(const char *filename, const char *ip)
    {
        FILE *file = fopen(filename, "r");
        if (file == NULL)
        {
            perror(C3 "Error opening file" RE);
            return;
        }

        char **lines = NULL;
        size_t count = 0;
        char buffer[256];

        while (fgets(buffer, sizeof(buffer), file))
        {
            lines = realloc(lines, (count + 1) * sizeof(char *));
            lines[count] = strdup(buffer);
            count++;
        }
        fclose(file);

        file = fopen(filename, "w");
        if (file == NULL)
        {
            perror(C3 "Error opening file for writing" RE);
            for (size_t i = 0; i < count; i++)
            {
                free(lines[i]);
            }
            free(lines);
            return;
        }

        for (size_t i = 0; i < count; i++)
        {
            if (strstr(lines[i], ip) == NULL)
            {
                fputs(lines[i], file);
            }
            free(lines[i]);
        }
        free(lines);

        fclose(file);
    }

    // Remove IP HTTP from hash
    void remove_ip_HTTP_from_hash(const char *ip)
    {
        if (g_hash_table_size(ip_table) == 0)
        {
            return;
        }
        // remove  ip hash
        g_hash_table_remove(ip_table, ip);
    }

    // VPN
    // Display IPv4 VPN
    void Display_IPv4_vpn_table()
    {
        sqlite3 *db;
        const char *db_path = DB_PATH; // d  d?nh nghia ? d?u file
        if (sqlite3_open(db_path, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        sqlite3_stmt *stmt;
        const char *sql =
            "SELECT AddressId, InterfaceId, Address, AddressVersion, AddressType, AddressAddedDate, Port "
            "FROM NetworkAddresses "
            "WHERE AddressType = 'vpn_white' AND AddressVersion = 'IPv4' AND Port IS NULL";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
        {
            printf(C3 "Failed to prepare statement: %s\n" RE "", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        // In b?ng
        printf(C6 "\n+----+------------+-----------------+---------+-----------+---------------------+--------+\n" RE);
        printf(C6 "|" RE " " C3 "ID" RE " " C6 "|" RE " " C3 "Interface" RE "  " C6 "|" RE "    " C3 "Address" RE "      " C6 "|" RE " " C3 "Version" RE " " C6 "|" RE "   " C3 "Type" RE "    " C6 "|" RE "    " C3 "Added Date" RE "       " C6 "|" RE "   " C3 "Port " C6 "|\n" RE);
        printf(C6 "+----+------------+-----------------+---------+-----------+---------------------+--------+\n" RE);
        int stt = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            int interface_id = sqlite3_column_int(stmt, 1);
            const unsigned char *addr = sqlite3_column_text(stmt, 2);
            const unsigned char *version = sqlite3_column_text(stmt, 3);
            const unsigned char *type = sqlite3_column_text(stmt, 4);
            const unsigned char *date = sqlite3_column_text(stmt, 5);
            const unsigned char *port = sqlite3_column_text(stmt, 6);

            printf(C6 "| %-2d | %-10d | %-15s | %-7s | %-9s | %-19s | %-4s |\n" RE "",
                stt++,
                interface_id,
                addr ? (const char *)addr : "",
                version ? (const char *)version : "",
                type ? (const char *)type : "",
                date ? (const char *)date : "",
                port ? (const char *)port : "(NULL)");
        }
        printf(C6 "+----+------------+-----------------+---------+-----------+---------------------+--------+\n" RE);

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    // Display IPv6 VPN
    void Display_IPv6_vpn_table()
    {
        sqlite3 *db;
        const char *db_path = DB_PATH;
        if (sqlite3_open(db_path, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        sqlite3_stmt *stmt;
        const char *sql =
            "SELECT AddressId, InterfaceId, Address, AddressVersion, AddressType, AddressAddedDate, Port "
            "FROM NetworkAddresses "
            "WHERE AddressType = 'vpn_white' AND AddressVersion = 'IPv6' AND Port IS NULL";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
        {
            printf(C3 "Failed to prepare statement:" RE " " C6 "%s" RE "\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        // In b?ng
        printf(C6 "\n+----+------------+-----------------+---------+-----------+---------------------+--------+\n" RE);
        printf(C6 "|" RE " " C3 "ID" RE " " C6 "|" RE " " C3 "Interface" RE "  " C6 "|" RE "    " C3 "Address" RE "      " C6 "|" RE " " C3 "Version" RE " " C6 "|" RE "   " C3 "Type" RE "    " C6 "|" RE "    " C3 "Added Date" RE "       " C6 "|" RE "   " C3 "Port " C6 "|\n" RE);
        printf(C6 "+----+------------+-----------------+---------+-----------+---------------------+--------+\n" RE);
        int stt = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            int interface_id = sqlite3_column_int(stmt, 1);
            const unsigned char *addr = sqlite3_column_text(stmt, 2);
            const unsigned char *version = sqlite3_column_text(stmt, 3);
            const unsigned char *type = sqlite3_column_text(stmt, 4);
            const unsigned char *date = sqlite3_column_text(stmt, 5);
            const unsigned char *port = sqlite3_column_text(stmt, 6);

            printf(C6 "| %-2d | %-10d | %-15s | %-7s | %-9s | %-19s | %-4s |\n" RE "",
                stt++,
                interface_id,
                addr ? (const char *)addr : "",
                version ? (const char *)version : "",
                type ? (const char *)type : "",
                date ? (const char *)date : "",
                port ? (const char *)port : "(NULL)");
        }
        printf(C6 "+----+------------+-----------------+---------+-----------+---------------------+--------+\n" RE);

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    // HTTP

    // HTTP display
    void Display_IP_http_table()
    {
        sqlite3 *db;
        const char *db_path = DB_PATH;

        if (sqlite3_open(db_path, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database:" RE " " C6 "%s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        sqlite3_stmt *stmt;
        const char *sql = "SELECT Address, AddressVersion, AddressType, AddressAddedDate, AddressTimeOut, Port "
                        "FROM NetworkAddresses "
                        "WHERE AddressType = 'http_black'";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
        {
            printf(C3 "Failed to prepare statement:" RE " " C6 "%s\n" RE "", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        // In b?ng ti u d?
        printf(C6 "\n+------+---------------------------------------------+---------+-----------+---------------------+---------------------+--------+\n" RE);
        printf(C6 "|" RE "  " C3 "#" RE "   " C6 "|" RE "                   " C3 "Address" RE "                    " C6 "|" RE " " C3 "Version" RE " " C6 "|" RE "   " C3 "Type" RE "    " C6 "|" RE "   " C3 "Added Date" RE "        " C6 "|" RE "    " C3 "Time Out" RE "         " C6 "|" RE "  " C3 "Port" RE "  " C6 "|\n" RE);
        printf(C6 "+------+---------------------------------------------+---------+-----------+---------------------+---------------------+--------+\n" RE);

        int stt = 1;
        int found = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            found = 1;
            const unsigned char *addr = sqlite3_column_text(stmt, 0);
            const unsigned char *version = sqlite3_column_text(stmt, 1);
            const unsigned char *type = sqlite3_column_text(stmt, 2);
            const unsigned char *date = sqlite3_column_text(stmt, 3);
            const unsigned char *timeout = sqlite3_column_text(stmt, 4);
            const unsigned char *port = sqlite3_column_text(stmt, 5);

            printf(C6 "| %-4d | %-43s | %-7s | %-9s | %-19s | %-19s | %-6s |\n" RE "",
                stt++,
                addr ? (const char *)addr : "",
                version ? (const char *)version : "",
                type ? (const char *)type : "",
                date ? (const char *)date : "",
                timeout ? (const char *)timeout : "",
                port ? (const char *)port : "(NULL)");
        }

        if (!found)
        {
            printf(C3 "| %-80s |\n", "No HTTP blocked addresses found." RE);
        }

        printf(C6 "+------+-----------------------------------------------+---------+-----------+---------------------+---------------------+--------+\n" RE);
        printf(C3 "\n Press any key to continue...\n" RE);
        getchar(); //  ?i ngu?i d ng nh?n ph m
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    //  Display IPv4 HTTP
    void Display_http_ipv4_table()
    {
        sqlite3 *db;
        const char *db_path = DB_PATH;

        if (sqlite3_open(db_path, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database:" RE " " C6 "%s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        sqlite3_stmt *stmt;
        const char *sql =
            "SELECT AddressId, InterfaceId, Address, AddressVersion, AddressType, AddressAddedDate, Port "
            "FROM NetworkAddresses "
            "WHERE AddressType = 'http_black' AND AddressVersion = 'IPV4'";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
        {
            printf(C3 "Failed to prepare statement:" RE " " C6 "%s\n" RE "", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }
        // In ti u d? b?ng
        printf(C6 "\n+----+-----------------+---------+-------------+---------------------+--------+\n" RE);
        printf(C6 "|" RE " " C3 "#" RE "  " C6 "|" RE "     " C3 "Address" RE "     " C6 "|" RE " " C3 "Version" RE " " C6 "|" RE "    " C3 "Type" RE "     " C6 "|" RE "   " C3 "Added Date" RE "        " C6 "|" RE "  " C3 "Port" RE "  " C6 "|\n" RE);
        printf(C6 "+----+-----------------+---------+-------------+---------------------+--------+\n" RE);
        // printf("\n+----+---------------------------------------------+---------+-----------+---------------------+---------------------+--------+\n");
        // printf("| #  |                   Address                    | Version |   Type    |   Added Date        |    Time Out         |  Port  |\n");
        // printf("+----+---------------------------------------------+---------+-----------+---------------------+---------------------+--------+\n");
        int stt = 1;
        int found = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            found = 1;
            const unsigned char *addr = sqlite3_column_text(stmt, 2);
            const unsigned char *version = sqlite3_column_text(stmt, 3);
            const unsigned char *type = sqlite3_column_text(stmt, 4);
            const unsigned char *date = sqlite3_column_text(stmt, 5);
            const unsigned char *port = sqlite3_column_text(stmt, 6);
            printf(C6 "|" RE " " C3 "%-2d" RE " " C6 "|" RE " " C3 "%-15s" RE " " C6 "|" RE " " C3 "%-7s" RE " " C6 "|" RE " " C3 "%-9s" RE " " C6 "|" RE " " C3 "%-19s" RE " " C6 "|" RE " " C3 "%-6s" RE " " C6 "|" RE "\n",
                stt++,
                addr ? (const char *)addr : "",
                version ? (const char *)version : "",
                type ? (const char *)type : "",
                date ? (const char *)date : "",
                port ? (const char *)port : "(NULL)");
        }
        if (!found)
        {
            printf(C3 "| %-76s |\n", "No HTTP IPv4 addresses found." RE);
        }
        printf(C6 "+----+-----------------+---------+-----------+---------------------+--------+\n" RE);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    // Display IPv6 HTTP
    void Display_http_ipv6_table()
    {
        sqlite3 *db;
        const char *db_path = DB_PATH;
        if (sqlite3_open(db_path, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database:" RE " " C6 "%s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        sqlite3_stmt *stmt;
        const char *sql =
            "SELECT AddressId, InterfaceId, Address, AddressVersion, AddressType, AddressAddedDate, Port "
            "FROM NetworkAddresses "
            "WHERE AddressType = 'http_black' AND AddressVersion = 'IPV6'";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
        {
            printf(C3 "Failed to prepare statement:" RE " " C6 "%s\n" RE "", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        // In b?ng
        // printf("\n+----+------------+---------------------------+---------+-----------+---------------------+--------+\n");
        // printf("| ID | Interface  |         Address           | Version |   Type    |    Added Date       |   Port |\n");
        // printf("+----+------------+---------------------------+---------+-----------+---------------------+--------+\n");
        printf(C6 "\n+------+---------------------------------------------+---------+-----------+---------------------+---------------------+--------+\n" RE);
        printf(C6 "|" RE "  " C3 "#" RE "   " C6 "|" RE "                   " C3 "Address" RE "                    " C6 "|" RE " " C3 "Version" RE " " C6 "|" RE "   " C3 "Type" RE "    " C6 "|" RE "   " C3 "Added Date" RE "        " C6 "|" RE "    " C3 "Time Out" RE "         " C6 "|" RE "  " C3 "Port" RE "  " C6 "|\n" RE);
        printf(C6 "+------+---------------------------------------------+---------+-----------+---------------------+---------------------+--------+\n" RE);

        int stt = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            int interface_id = sqlite3_column_int(stmt, 1);
            const unsigned char *addr = sqlite3_column_text(stmt, 2);
            const unsigned char *version = sqlite3_column_text(stmt, 3);
            const unsigned char *type = sqlite3_column_text(stmt, 4);
            const unsigned char *date = sqlite3_column_text(stmt, 5);
            const unsigned char *port = sqlite3_column_text(stmt, 6);

            printf(C6 "| %-2d | %-10d | %-43s | %-7s | %-9s | %-19s | %-4s |\n" RE "",
                stt++,
                interface_id,
                addr ? (const char *)addr : "",
                version ? (const char *)version : "",
                type ? (const char *)type : "",
                date ? (const char *)date : "",
                port ? (const char *)port : "");
        }
        printf(C6 "+----+------------+---------------------------+---------+-----------+---------------------+--------+\n" RE);

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    // BLOCK
    // Display IPv4 block

    void Display_IPv4_block_table()
    {
        sqlite3 *db;
        const char *db_path = DB_PATH;

        if (sqlite3_open(db_path, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database:" RE " " C6 "%s" RE "\n", sqlite3_errmsg(db));
            return;
        }

        sqlite3_stmt *stmt;
        const char *sql = "SELECT Address, AddressVersion, AddressType, AddressAddedDate, AddressTimeOut, Port "
                        "FROM NetworkAddresses "
                        "WHERE AddressType = 'blocked' AND AddressVersion = 'IPv4' AND Port = ?";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
        {
            printf(C3 "Failed to prepare statement: %s\n" RE "", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        // T?o t n port theo d?nh d?ng "eth1", "eth2",...
        char port_name[8];
        snprintf(port_name, sizeof(port_name), "" C3 "eth%d" RE "", current_port);

        // G n gi  tr? cho bi?n bind (tham s? ?)
        if (sqlite3_bind_text(stmt, 1, port_name, -1, SQLITE_STATIC) != SQLITE_OK)
        {
            printf(C3 "Failed to bind port name:" RE " " C6 "%s\n" RE "", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        // In b?ng ti u d?
        printf(C6 "\n+----+-----------------+---------+-----------+---------------------+---------------------+--------+\n" RE);
        printf(C3 "| #  |     Address     | Version |   Type    |   Added Date        |    Time Out         |  Port  |\n" RE);
        printf(C6 "+----+-----------------+---------+-----------+---------------------+---------------------+--------+\n" RE);

        int stt = 1;
        int found = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            found = 1;
            const unsigned char *addr = sqlite3_column_text(stmt, 0);
            const unsigned char *version = sqlite3_column_text(stmt, 1);
            const unsigned char *type = sqlite3_column_text(stmt, 2);
            const unsigned char *date = sqlite3_column_text(stmt, 3);
            const unsigned char *timeout = sqlite3_column_text(stmt, 4);
            const unsigned char *port = sqlite3_column_text(stmt, 5);

            printf(C6 "| %-2d | %-15s | %-7s | %-9s | %-19s | %-19s | %-6s |\n" RE "",
                stt++,
                addr ? (const char *)addr : "",
                version ? (const char *)version : "",
                type ? (const char *)type : "",
                date ? (const char *)date : "",
                timeout ? (const char *)timeout : "",
                port ? (const char *)port : "");
        }

        if (!found)
        {
            printf(C3 "| %-80s |\n", "No IPv4 blocked addresses found for this port." RE);
        }

        printf(C6 "+----+-----------------+---------+-----------+---------------------+---------------------+--------+\n" RE);

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    // Display IPv6 block

    void Display_IPv6_block_table()
    {
        sqlite3 *db;
        const char *db_path = DB_PATH;
        if (sqlite3_open(db_path, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database:" RE " " C6 "%s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        sqlite3_stmt *stmt;
        const char *sql = "SELECT AddressId, InterfaceId, Address, AddressVersion, AddressType, AddressAddedDate, Port "
                        "FROM NetworkAddresses "
                        "WHERE AddressType = 'blocked' AND AddressVersion = 'IPv6' AND Port = ?";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
        {
            printf(C3 "Failed to prepare statement:" RE " " C6 "%s\n" RE "", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        // T?o t n port t? current_port (eth1, eth2, ...)
        char port_name[8];
        snprintf(port_name, sizeof(port_name), "eth%d", current_port);

        // Truy?n port_name v o SQL (d?u ?)
        if (sqlite3_bind_text(stmt, 1, port_name, -1, SQLITE_STATIC) != SQLITE_OK)
        {
            printf(C3 "Failed to bind port name:" RE " " C6 "%s\n" RE "", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        // In b?ng

        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf(C6 "\n+----+------------+-----------------------------------------+---------+-----------+---------------------+------+\n" RE);
        printf(C3 "\r| ID | Interface  |                   Address               | Version |    Type   |      Added Date     | Port |\n" RE);
        printf(C6 "\r+----+------------+-----------------------------------------+---------+-----------+---------------------+------+\n" RE);
        int stt = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            int interface_id = sqlite3_column_int(stmt, 1);
            const unsigned char *addr = sqlite3_column_text(stmt, 2);
            const unsigned char *version = sqlite3_column_text(stmt, 3);
            const unsigned char *type = sqlite3_column_text(stmt, 4);
            const unsigned char *date = sqlite3_column_text(stmt, 5);
            const unsigned char *port = sqlite3_column_text(stmt, 6);

            printf(C6 "| %-2d | %-10d | %-15s | %-7s | %-9s | %-19s | %-4s |\n" RE "",
                stt++,
                interface_id,
                addr ? (const char *)addr : "",
                version ? (const char *)version : "",
                type ? (const char *)type : "",
                date ? (const char *)date : "",
                port ? (const char *)port : "");
        }
        printf(C6 "\r+-----+-----------+-----------------------------------------+---------+-----------+---------------------+------+\n" RE);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }
    // PROTECT
    // Display IPv4 protect
    void Display_IPv4_Protected_Table()
    {
        sqlite3 *db;
        const char *db_path = DB_PATH; // d  d?nh nghia ? d?u file
        if (sqlite3_open(db_path, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database:" RE " " C6 "%s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        sqlite3_stmt *stmt;
        const char *sql = "SELECT AddressId, InterfaceId, Address, AddressVersion, AddressType, AddressAddedDate, Port "
                        "FROM NetworkAddresses "
                        "WHERE AddressType = 'protected' AND AddressVersion = 'IPv4' AND Port = ?";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
        {
            printf(C3 "Failed to prepare statement:" RE " " C6 "%s\n" RE "", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        // (eth1, eth2, ...)
        char port_name[8];
        snprintf(port_name, sizeof(port_name), "eth%d", current_port);

        // Truy?n port_name v o SQL (d?u ?)
        if (sqlite3_bind_text(stmt, 1, port_name, -1, SQLITE_STATIC) != SQLITE_OK)
        {
            printf(C3 "Failed to bind port name: %s\n" RE "", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        // In b?ng
        printf(C6 "\n+----+------------+-----------------+---------+-----------+---------------------+------+\n" RE);
        printf(C3 "| ID | Interface  |    Address      | Version |   Type    |    Added Date       | Port |\n" RE);
        printf(C6 "+----+------------+-----------------+---------+-----------+---------------------+------+\n" RE);
        int stt = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            int interface_id = sqlite3_column_int(stmt, 1);
            const unsigned char *addr = sqlite3_column_text(stmt, 2);
            const unsigned char *version = sqlite3_column_text(stmt, 3);
            const unsigned char *type = sqlite3_column_text(stmt, 4);
            const unsigned char *date = sqlite3_column_text(stmt, 5);
            const unsigned char *port = sqlite3_column_text(stmt, 6);

            printf(C6 "| %-2d | %-10d | %-15s | %-7s | %-9s | %-19s | %-4s |\n" RE "",
                stt++,
                interface_id,
                addr ? (const char *)addr : "",
                version ? (const char *)version : "",
                type ? (const char *)type : "",
                date ? (const char *)date : "",
                port ? (const char *)port : "");
        }
        printf(C6 "+----+------------+-----------------+---------+-----------+---------------------+------+\n" RE);

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    // Display IPv6 Protect
    void Display_IPv6_Protected_Table()
    {
        sqlite3 *db;
        const char *db_path = DB_PATH; // d  d?nh nghia ? d?u file
        if (sqlite3_open(db_path, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE, sqlite3_errmsg(db));
            return;
        }

        sqlite3_stmt *stmt;
        const char *sql = "SELECT AddressId, InterfaceId, Address, AddressVersion, AddressType, AddressAddedDate, Port "
                        "FROM  NetworkAddresses "
                        "WHERE AddressType = 'protected' AND AddressVersion = 'IPv6' AND Port = ?";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
        {
            printf(C3 "Failed to prepare statement: %s\n" RE, sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        // T?o t n port t? current_port (eth1, eth2, ...)
        char port_name[8];
        snprintf(port_name, sizeof(port_name), "eth%d", current_port);

        // Truy?n port_name v o SQL (d?u ?)
        if (sqlite3_bind_text(stmt, 1, port_name, -1, SQLITE_STATIC) != SQLITE_OK)
        {
            printf(C3 "Failed to bind port name: %s\n" RE "", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        // In b?ng

        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf(C6 "\n+----+------------+-----------------------------------------+---------+-----------+---------------------+------+\n" RE);
        printf(C3 "\r| ID | Interface  |                   Address               | Version |    Type   |      Added Date     | Port |\n" RE);
        printf(C6 "\r+----+------------+-----------------------------------------+---------+-----------+---------------------+------+\n" RE);
        int stt = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            int interface_id = sqlite3_column_int(stmt, 1);
            const unsigned char *addr = sqlite3_column_text(stmt, 2);
            const unsigned char *version = sqlite3_column_text(stmt, 3);
            const unsigned char *type = sqlite3_column_text(stmt, 4);
            const unsigned char *date = sqlite3_column_text(stmt, 5);
            const unsigned char *port = sqlite3_column_text(stmt, 6);

            printf(C6 "| %-2d | %-10d | %-15s | %-7s | %-9s | %-19s | %-4s |\n" RE "",
                stt++,
                interface_id,
                addr ? (const char *)addr : "",
                version ? (const char *)version : "",
                type ? (const char *)type : "",
                date ? (const char *)date : "",
                port ? (const char *)port : "");
        }
        printf(C6 "\r+-----+-----------+-----------------------------------------+---------+-----------+---------------------+------+\n" RE);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    // VPN
    // update ipv4 vpn
    void update_vpn_ipv4(const char *ip)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        // Ki?m tra IP d  t?n t?i chua
        sqlite3_stmt *stmt_check;
        const char *sql_check =
            "SELECT AddressId FROM NetworkAddresses WHERE Address = ? AND AddressType = 'vpn_white' AND AddressVersion = 'IPv4'";
        if (sqlite3_prepare_v2(db, sql_check, -1, &stmt_check, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt_check, 1, ip, -1, SQLITE_STATIC);

            int exists = 0;
            if (sqlite3_step(stmt_check) == SQLITE_ROW)
            {
                exists = 1;
            }
            sqlite3_finalize(stmt_check);

            if (exists)
            {
                printf(C3 "IPv4 address %s already exists in VPN table.\n" RE "", ip);
            }
            else
            {
                // Ch n m?i n?u chua t?n t?i
                sqlite3_stmt *stmt_insert;
                const char *sql_insert =
                    "INSERT INTO NetworkAddresses (InterfaceId, Address, AddressType, AddressVersion, Port, AddressAddedDate, AddressTimeOut) "
                    "VALUES (1, ?, 'vpn_white', 'IPv4', NULL, ?, 0)";

                if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt_insert, 0) == SQLITE_OK)
                {
                    // Format ng y gi? th nh "YYYY/MM/DD HH:MM:SS"
                    char current_datetime[20];
                    time_t now = time(NULL);
                    strftime(current_datetime, sizeof(current_datetime), "%Y/%m/%d %H:%M:%S", localtime(&now));

                    sqlite3_bind_text(stmt_insert, 1, ip, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_insert, 2, current_datetime, -1, SQLITE_STATIC);

                    if (sqlite3_step(stmt_insert) == SQLITE_DONE)
                        printf(C2 "Inserted new VPN IPv4 address %s.\n" RE "", ip);
                    else
                        printf(C3 "Failed to insert VPN IPv4 address.\n" RE);

                    sqlite3_finalize(stmt_insert);
                }
            }
        }

        sqlite3_close(db);
    }

    // Update ipv6 vpn
    void update_vpn_ipv6(const char *ip)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        sqlite3_stmt *stmt_check;
        const char *sql_check =
            "SELECT AddressId FROM NetworkAddresses WHERE Address = ? AND AddressType = 'vpn_white' AND AddressVersion = 'IPv6'";
        if (sqlite3_prepare_v2(db, sql_check, -1, &stmt_check, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt_check, 1, ip, -1, SQLITE_STATIC);

            int exists = 0;
            if (sqlite3_step(stmt_check) == SQLITE_ROW)
            {
                exists = 1;
            }
            sqlite3_finalize(stmt_check);

            if (exists)
            {
                printf(C3 "IPv6 address %s already exists in VPN table.\n" RE "", ip);
            }
            else
            {
                sqlite3_stmt *stmt_insert;
                const char *sql_insert =
                    "INSERT INTO NetworkAddresses (InterfaceId, Address, AddressType, AddressVersion, Port, AddressAddedDate, AddressTimeOut) "
                    "VALUES (1, ?, 'vpn_white', 'IPv6', NULL, ?, 0)";

                if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt_insert, 0) == SQLITE_OK)
                {

                    char current_datetime[20];
                    time_t now = time(NULL);
                    strftime(current_datetime, sizeof(current_datetime), "%Y/%m/%d %H:%M:%S", localtime(&now));

                    sqlite3_bind_text(stmt_insert, 1, ip, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_insert, 2, current_datetime, -1, SQLITE_STATIC);

                    if (sqlite3_step(stmt_insert) == SQLITE_DONE)
                        printf(C2 "Inserted new VPN IPv6 address %s.\n" RE "", ip);
                    else
                        printf(C3 "Failed to insert VPN IPv6 address.\n" RE);

                    sqlite3_finalize(stmt_insert);
                }
            }
        }

        sqlite3_close(db);
    }

    // HTTP
    // update ipv4 HTTP
    void update_http_ipv4(const char *ip)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        // Ki?m tra IP d  t?n t?i chua
        sqlite3_stmt *stmt_check;
        const char *sql_check =
            "SELECT AddressId FROM NetworkAddresses WHERE Address = ? AND AddressType = 'http_black' AND AddressVersion = 'IPV4'";
        if (sqlite3_prepare_v2(db, sql_check, -1, &stmt_check, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt_check, 1, ip, -1, SQLITE_STATIC);

            int exists = 0;
            if (sqlite3_step(stmt_check) == SQLITE_ROW)
            {
                exists = 1;
            }
            sqlite3_finalize(stmt_check);

            if (exists)
            {
                printf(C3 "IPv4 address %s already exists in HTTP table.\n" RE "", ip);
            }
            else
            {
                // Ch n m?i n?u chua t?n t?i
                sqlite3_stmt *stmt_insert;
                const char *sql_insert =
                    "INSERT INTO NetworkAddresses (InterfaceId, Address, AddressType, AddressVersion, Port, AddressAddedDate, AddressTimeOut) "
                    "VALUES (1, ?, 'http_black', 'IPV4', NULL, ?, 0)";

                if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt_insert, 0) == SQLITE_OK)
                {
                    // Format ng y gi? th nh "YYYY/MM/DD HH:MM:SS"
                    char current_datetime[20];
                    time_t now = time(NULL);
                    strftime(current_datetime, sizeof(current_datetime), "%Y/%m/%d %H:%M:%S", localtime(&now));

                    sqlite3_bind_text(stmt_insert, 1, ip, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_insert, 2, current_datetime, -1, SQLITE_STATIC);

                    if (sqlite3_step(stmt_insert) == SQLITE_DONE)
                        printf(C2 "Inserted new HTTP IPv4 address %s.\n" RE "", ip);
                    else
                        printf(C3 "Failed to insert HTTP IPv4 address.\n" RE);

                    sqlite3_finalize(stmt_insert);
                }
            }
        }

        sqlite3_close(db);
    }

    // Update ipv6 HTTP
    void update_http_ipv6(const char *ip)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        sqlite3_stmt *stmt_check;
        const char *sql_check =
            "SELECT AddressId FROM NetworkAddresses WHERE Address = ? AND AddressType = 'http_black' AND AddressVersion = 'IPv6'";
        if (sqlite3_prepare_v2(db, sql_check, -1, &stmt_check, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt_check, 1, ip, -1, SQLITE_STATIC);

            int exists = 0;
            if (sqlite3_step(stmt_check) == SQLITE_ROW)
            {
                exists = 1;
            }
            sqlite3_finalize(stmt_check);

            if (exists)
            {
                printf(C3 "IPv6 address %s already exists in HTTP table.\n" RE "", ip);
            }
            else
            {
                sqlite3_stmt *stmt_insert;
                const char *sql_insert =
                    "INSERT INTO NetworkAddresses (InterfaceId, Address, AddressType, AddressVersion, Port, AddressAddedDate, AddressTimeOut) "
                    "VALUES (1, ?, 'http_black', 'IPv6', NULL, ?, 0)";

                if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt_insert, 0) == SQLITE_OK)
                {

                    char current_datetime[20];
                    time_t now = time(NULL);
                    strftime(current_datetime, sizeof(current_datetime), "%Y/%m/%d %H:%M:%S", localtime(&now));

                    sqlite3_bind_text(stmt_insert, 1, ip, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_insert, 2, current_datetime, -1, SQLITE_STATIC);

                    if (sqlite3_step(stmt_insert) == SQLITE_DONE)
                        printf(C2 "Inserted new HTTP IPv6 address %s.\n" RE "", ip);
                    else
                        printf(C3 "Failed to insert HTTP IPv6 address.\n" RE);

                    sqlite3_finalize(stmt_insert);
                }
            }
        }

        sqlite3_close(db);
    }

    // BLOCKED
    // Update Block ipv4
    void update_blocked_ipv4(const char *ip, const char *port)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        int interface_id = -1;
        sqlite3_stmt *stmt_get_id;
        const char *sql_get_id = "SELECT InterfaceId FROM DeviceInterfaces WHERE InterfaceName = ?";
        if (sqlite3_prepare_v2(db, sql_get_id, -1, &stmt_get_id, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt_get_id, 1, port, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt_get_id) == SQLITE_ROW)
            {
                interface_id = sqlite3_column_int(stmt_get_id, 0);
            }
            sqlite3_finalize(stmt_get_id);
        }

        if (interface_id == -1)
        {
            printf(C3 "Failed to find InterfaceId for port %s.\n" RE "", port);
            sqlite3_close(db);
            return;
        }

        sqlite3_stmt *stmt_check;
        const char *sql_check = "SELECT AddressId FROM NetworkAddresses WHERE Address = ? AND AddressType = 'blocked' AND AddressVersion = 'IPv4'";
        if (sqlite3_prepare_v2(db, sql_check, -1, &stmt_check, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt_check, 1, ip, -1, SQLITE_STATIC);

            int exists = 0;
            if (sqlite3_step(stmt_check) == SQLITE_ROW)
            {
                exists = 1;
            }
            sqlite3_finalize(stmt_check);

            if (exists)
            {

                sqlite3_stmt *stmt_update;
                const char *sql_update = "UPDATE NetworkAddresses SET Port = ? WHERE Address = ? AND AddressType = 'blocked' AND AddressVersion = 'IPv4'";
                if (sqlite3_prepare_v2(db, sql_update, -1, &stmt_update, 0) == SQLITE_OK)
                {
                    sqlite3_bind_text(stmt_update, 1, port, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_update, 2, ip, -1, SQLITE_STATIC);
                    if (sqlite3_step(stmt_update) == SQLITE_DONE)
                        printf(C2 " Updated port of blocked IPv6 address %s to %s.\n" RE "", ip, port);
                    else
                        printf(C3 " Failed to update port.\n" RE);
                    sqlite3_finalize(stmt_update);
                }
            }
            else
            {

                sqlite3_stmt *stmt_insert;
                const char *sql_insert =
                    "INSERT INTO NetworkAddresses (InterfaceId, Address, AddressType, AddressVersion, Port, AddressAddedDate, AddressTimeOut) "
                    "VALUES (?, ?, 'blocked', 'IPv4', ?, ?, 0)";

                if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt_insert, 0) == SQLITE_OK)
                {
                    char current_date[20];
                    time_t now = time(NULL);
                    strftime(current_date, sizeof(current_date), "%Y/%m/%d %H:%M:%S", localtime(&now));
                    sqlite3_bind_int(stmt_insert, 1, interface_id);
                    sqlite3_bind_text(stmt_insert, 2, ip, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_insert, 3, port, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_insert, 4, current_date, -1, SQLITE_STATIC);

                    if (sqlite3_step(stmt_insert) == SQLITE_DONE)
                        printf(C2 " Inserted new blocked IPv4 address %s on port %s (InterfaceId %d).\n" RE "", ip, port, interface_id);
                    else
                        printf(C3 " Failed to insert IPv4 address.\n" RE);

                    sqlite3_finalize(stmt_insert);
                }
            }
        }

        sqlite3_close(db);
    }

    // Update IPv6 BLOCK
    void update_blocked_ipv6(const char *ip, const char *port)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        int interface_id = -1;
        sqlite3_stmt *stmt_get_id;
        const char *sql_get_id = "SELECT InterfaceId FROM DeviceInterfaces WHERE InterfaceName = ?";
        if (sqlite3_prepare_v2(db, sql_get_id, -1, &stmt_get_id, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt_get_id, 1, port, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt_get_id) == SQLITE_ROW)
            {
                interface_id = sqlite3_column_int(stmt_get_id, 0);
            }
            sqlite3_finalize(stmt_get_id);
        }

        if (interface_id == -1)
        {
            printf(C3 "Failed to find InterfaceId for port %s.\n" RE "", port);
            sqlite3_close(db);
            return;
        }

        sqlite3_stmt *stmt_check;
        const char *sql_check = "SELECT AddressId FROM NetworkAddresses WHERE Address = ? AND AddressType = 'blocked' AND AddressVersion = 'IPv6'";
        if (sqlite3_prepare_v2(db, sql_check, -1, &stmt_check, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt_check, 1, ip, -1, SQLITE_STATIC);

            int exists = 0;
            if (sqlite3_step(stmt_check) == SQLITE_ROW)
            {
                exists = 1;
            }
            sqlite3_finalize(stmt_check);

            if (exists)
            {
                sqlite3_stmt *stmt_update;
                const char *sql_update = "UPDATE NetworkAddresses SET Port = ? WHERE Address = ? AND AddressType = 'blocked' AND AddressVersion = 'IPv6'";
                if (sqlite3_prepare_v2(db, sql_update, -1, &stmt_update, 0) == SQLITE_OK)
                {
                    sqlite3_bind_text(stmt_update, 1, port, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_update, 2, ip, -1, SQLITE_STATIC);
                    if (sqlite3_step(stmt_update) == SQLITE_DONE)
                        printf(C2 " Updated port of blocked IPv6 address %s to %s.\n" RE "", ip, port);
                    else
                        printf(C3 " Failed to update port.\n" RE);
                    sqlite3_finalize(stmt_update);
                }
            }
            else
            {
                sqlite3_stmt *stmt_insert;
                const char *sql_insert =
                    "INSERT INTO NetworkAddresses (InterfaceId, Address, AddressType, AddressVersion, Port, AddressAddedDate, AddressTimeOut) "
                    "VALUES (?, ?, 'blocked', 'IPv6', ?, ?, 0)";

                if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt_insert, 0) == SQLITE_OK)
                {
                    char current_date[20];
                    time_t now = time(NULL);
                    strftime(current_date, sizeof(current_date), "%Y/%m/%d %H:%M:%S", localtime(&now));
                    sqlite3_bind_int(stmt_insert, 1, interface_id);
                    sqlite3_bind_text(stmt_insert, 2, ip, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_insert, 3, port, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_insert, 4, current_date, -1, SQLITE_STATIC);
                    if (sqlite3_step(stmt_insert) == SQLITE_DONE)
                        printf(C2 " Inserted new blocked IPv6 address %s on port %s (InterfaceId %d).\n" RE "", ip, port, interface_id);
                    else
                        printf(C3 " Failed to insert IPv6 address.\n" RE);

                    sqlite3_finalize(stmt_insert);
                }
            }
        }

        sqlite3_close(db);
    }

    // PROTECT
    // Update protect ipv4
    void update_protected_ipv4_port(const char *ip, const char *port)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database:" RE " " C6 "%s" RE "\n", sqlite3_errmsg(db));
            return;
        }

        int interface_id = -1;
        sqlite3_stmt *stmt_get_id;
        const char *sql_get_id = "SELECT InterfaceId FROM DeviceInterfaces WHERE InterfaceName = ?";
        if (sqlite3_prepare_v2(db, sql_get_id, -1, &stmt_get_id, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt_get_id, 1, port, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt_get_id) == SQLITE_ROW)
            {
                interface_id = sqlite3_column_int(stmt_get_id, 0);
            }
            sqlite3_finalize(stmt_get_id);
        }

        if (interface_id == -1)
        {
            printf(C3 "Failed to find InterfaceId for port %s.\n" RE "", port);
            sqlite3_close(db);
            return;
        }

        sqlite3_stmt *stmt_check;
        const char *sql_check = "SELECT AddressId FROM NetworkAddresses WHERE Address = ? AND AddressType = 'protected' AND AddressVersion = 'IPv4'";
        if (sqlite3_prepare_v2(db, sql_check, -1, &stmt_check, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt_check, 1, ip, -1, SQLITE_STATIC);

            int exists = 0;
            if (sqlite3_step(stmt_check) == SQLITE_ROW)
            {
                exists = 1;
            }
            sqlite3_finalize(stmt_check);

            if (exists)
            {

                sqlite3_stmt *stmt_update;
                const char *sql_update = "UPDATE NetworkAddresses SET Port = ? WHERE Address = ? AND AddressType = 'protected' AND AddressVersion = 'IPv4'";
                if (sqlite3_prepare_v2(db, sql_update, -1, &stmt_update, 0) == SQLITE_OK)
                {
                    sqlite3_bind_text(stmt_update, 1, port, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_update, 2, ip, -1, SQLITE_STATIC);
                    if (sqlite3_step(stmt_update) == SQLITE_DONE)
                        printf(C2 " Updated port of protected IPv6 address %s to %s.\n" RE "", ip, port);
                    else
                        printf(C3 " Failed to update port.\n" RE);
                    sqlite3_finalize(stmt_update);
                }
            }
            else
            {

                sqlite3_stmt *stmt_insert;
                const char *sql_insert =
                    "INSERT INTO NetworkAddresses (InterfaceId, Address, AddressType, AddressVersion, Port, AddressAddedDate, AddressTimeOut) "
                    "VALUES (?, ?, 'protected', 'IPv4', ?, ?, 0)";

                if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt_insert, 0) == SQLITE_OK)
                {
                    char current_date[20];
                    time_t now = time(NULL);
                    strftime(current_date, sizeof(current_date), "%Y/%m/%d %H:%M:%S", localtime(&now));
                    sqlite3_bind_int(stmt_insert, 1, interface_id);
                    sqlite3_bind_text(stmt_insert, 2, ip, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_insert, 3, port, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_insert, 4, current_date, -1, SQLITE_STATIC);

                    if (sqlite3_step(stmt_insert) == SQLITE_DONE)
                        printf(C2 " Inserted new protected IPv4 address %s on port %s (InterfaceId %d).\n" RE "", ip, port, interface_id);
                    else
                        printf(C3 " Failed to insert IPv4 address.\n" RE);

                    sqlite3_finalize(stmt_insert);
                }
            }
        }

        sqlite3_close(db);
    }

    // Update IPv6 protect
    void update_protected_ipv6_port(const char *ip, const char *port)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        int interface_id = -1;
        sqlite3_stmt *stmt_get_id;
        const char *sql_get_id = "SELECT InterfaceId FROM DeviceInterfaces WHERE InterfaceName = ?";
        if (sqlite3_prepare_v2(db, sql_get_id, -1, &stmt_get_id, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt_get_id, 1, port, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt_get_id) == SQLITE_ROW)
            {
                interface_id = sqlite3_column_int(stmt_get_id, 0);
            }
            sqlite3_finalize(stmt_get_id);
        }

        if (interface_id == -1)
        {
            printf(C3 "Failed to find InterfaceId for port %s.\n" RE "", port);
            sqlite3_close(db);
            return;
        }

        sqlite3_stmt *stmt_check;
        const char *sql_check = "SELECT AddressId FROM NetworkAddresses WHERE Address = ? AND AddressType = 'protected' AND AddressVersion = 'IPv6'";
        if (sqlite3_prepare_v2(db, sql_check, -1, &stmt_check, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt_check, 1, ip, -1, SQLITE_STATIC);

            int exists = 0;
            if (sqlite3_step(stmt_check) == SQLITE_ROW)
            {
                exists = 1;
            }
            sqlite3_finalize(stmt_check);

            if (exists)
            {
                sqlite3_stmt *stmt_update;
                const char *sql_update = "UPDATE NetworkAddresses SET Port = ? WHERE Address = ? AND AddressType = 'protected' AND AddressVersion = 'IPv6'";
                if (sqlite3_prepare_v2(db, sql_update, -1, &stmt_update, 0) == SQLITE_OK)
                {
                    sqlite3_bind_text(stmt_update, 1, port, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_update, 2, ip, -1, SQLITE_STATIC);
                    if (sqlite3_step(stmt_update) == SQLITE_DONE)
                        printf(C2 " Updated port of protected IPv6 address %s to %s.\n" RE "", ip, port);
                    else
                        printf(C3 " Failed to update port.\n" RE);
                    sqlite3_finalize(stmt_update);
                }
            }
            else
            {
                sqlite3_stmt *stmt_insert;
                const char *sql_insert =
                    "INSERT INTO NetworkAddresses (InterfaceId, Address, AddressType, AddressVersion, Port, AddressAddedDate, AddressTimeOut) "
                    "VALUES (?, ?, 'protected', 'IPv6', ?, ?, 0)";

                if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt_insert, 0) == SQLITE_OK)
                {
                    char current_date[20];
                    time_t now = time(NULL);
                    strftime(current_date, sizeof(current_date), "%Y/%m/%d %H:%M:%S", localtime(&now));
                    sqlite3_bind_int(stmt_insert, 1, interface_id);
                    sqlite3_bind_text(stmt_insert, 2, ip, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_insert, 3, port, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt_insert, 4, current_date, -1, SQLITE_STATIC);
                    if (sqlite3_step(stmt_insert) == SQLITE_DONE)
                        printf(C2 " Inserted new protected IPv6 address %s on port %s (InterfaceId %d).\n" RE "", ip, port, interface_id);
                    else
                        printf(C3 " Failed to insert IPv6 address.\n" RE);

                    sqlite3_finalize(stmt_insert);
                }
            }
        }

        sqlite3_close(db);
    }

    // VPN
    // Delete IPv4 VPN
    void delete_vpn_ipv4(const char *ip)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        sqlite3_stmt *stmt;
        const char *sql = "DELETE FROM NetworkAddresses WHERE Address = ? AND AddressType = 'vpn_white' AND AddressVersion = 'IPv4'";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, ip, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                printf(C2 "Deleted IPv4 address %s from VPN table.\n" RE "", ip);
            }
            else
            {
                printf(C3 "Failed to delete IPv4 address from VPN table.\n" RE);
            }
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);
    }

    // Delete IPv6 VPN
    void delete_vpn_ipv6(const char *ip)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n", sqlite3_errmsg(db));
            return;
        }

        sqlite3_stmt *stmt;
        const char *sql = "DELETE FROM NetworkAddresses WHERE Address = ? AND AddressType = 'vpn_white' AND AddressVersion = 'IPv6'";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, ip, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                printf(C2 "Deleted IPv6 address %s from VPN table.\n" RE "", ip);
            }
            else
            {
                printf(C3 "Failed to delete IPv6 address from VPN table.\n" RE);
            }
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);
    }
    // HTTP
    // Delete IPv4 HTTP
    void delete_http_ipv4(const char *ip)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n", sqlite3_errmsg(db));
            return;
        }

        sqlite3_stmt *stmt;
        const char *sql = "DELETE FROM NetworkAddresses WHERE Address = ? AND AddressType = 'http_black' AND AddressVersion = 'IPv4'";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, ip, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                printf(C2 "Deleted IPv4 address %s from HTTP table.\n" RE "", ip);
            }
            else
            {
                printf(C3 "Failed to delete IPv4 address from HTTP table.\n" RE);
            }
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);
    }

    // Delete IPv6 HTTP
    void delete_http_ipv6(const char *ip)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        sqlite3_stmt *stmt;
        const char *sql = "DELETE FROM NetworkAddresses WHERE Address = ? AND AddressType = 'http_black' AND AddressVersion = 'IPv6'";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, ip, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                printf(C2 "Deleted IPv4 address %s from HTTP table.\n" RE "", ip);
            }
            else
            {
                printf(C3 "Failed to delete IPv4 address from HTTP table.\n" RE);
            }
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);
    }

    // BLOCK
    // Delete IPv4 Blocked
    void delete_blocked_ipv4(const char *ip)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }
        sqlite3_stmt *stmt;
        const char *sql = "DELETE FROM NetworkAddresses WHERE Address = ? AND AddressType = 'blocked' AND AddressVersion = 'IPv4'";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, ip, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                // printf("Deleted IPv4 address %s from blocked table.\n", ip);
            }
            else
            {
                // printf("Failed to delete IPv4 address.\n");
            }
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);
    }

    // Delete IPv6 Blocked
    void delete_blocked_ipv6(const char *ip)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }
        sqlite3_stmt *stmt;
        const char *sql = "DELETE FROM NetworkAddresses WHERE Address = ? AND AddressType = 'blocked' AND AddressVersion = 'IPv6'";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, ip, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                printf(C2 "Deleted IPv6 address %s from blocked table.\n" RE "", ip);
            }
            else
            {
                printf(C3 "Failed to delete IPv6 address.\n" RE);
            }
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);
    }

    // PROTECT
    // Delete IPv4 Protect
    void delete_protected_ipv4(const char *ip)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }
        sqlite3_stmt *stmt;
        const char *sql = "DELETE FROM NetworkAddresses WHERE Address = ? AND AddressType = 'protected' AND AddressVersion = 'IPv4'";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, ip, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                // printf("Deleted IPv4 address %s from protected table.\n", ip);
            }
            else
            {
                // printf("Failed to delete IPv4 address.\n");
            }
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);
    }

    // Delete IPv6 Protect
    void delete_protected_ipv6(const char *ip)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }
        sqlite3_stmt *stmt;
        const char *sql = "DELETE FROM NetworkAddresses WHERE Address = ? AND AddressType = 'protected' AND AddressVersion = 'IPv6'";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, ip, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                printf(C2 "Deleted IPv6 address %s from protected table.\n" RE "", ip);
            }
            else
            {
                printf(C3 "Failed to delete IPv6 address.\n" RE);
            }
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);
    }
    ///////////////////////////////////////////////////////////////////////////////////////
    void SetIPv4Target(int serial_port)
    {
        char choice;
        printf(C6 "\r\n ============================================================================================================================================================================================================+");
        printf(C6 "\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to (1) ADD, (2) REMOVE Server IPv4 address from protect table, or (Z)" RE " " C1 "EXIT?" RE " (" C2 "1" RE "/" C3 "2" RE "/" C1 "Z" RE "):" RE " " RE);

        while (1)
        {
            scanf(" %c", &choice);
            clear_input();
            if (choice == '1' || choice == '2' || choice == 'Z' || choice == 'z')
                break;
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C3 "Please enter 1 (ADD), 2 (REMOVE), or z" RE " (" C1 "EXIT?" RE ")" C6 ":" RE " " RE);
        }

        if (choice == 'Z' || choice == 'z')
        {
            // printf("\r\n    SETTING     | Exiting IPv4 target configuration.\n");
            return;
        }
        if (choice == '1')
        {
            current_config_type = CONFIG_IPV4_PROTECT;
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv4 address you want to ADD:" RE " " RE);
        }
        else
        {
            current_config_type = CONFIG_REMOVE_IPV4_PROTECT;
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv4 address you want to REMOVE from protect table:" RE " " RE);
        }

        while (1)
        {
            scanf("%15s", temp_ipv4_address);
            if (validate_ip_address(temp_ipv4_address))
                break;
            printf(C3 " Invalid! Please input again: " RE);
        }

        if (choice == '1')
        {
            char port_name[8];
            snprintf(port_name, sizeof(port_name), "eth%d", current_port);
            update_protected_ipv4_port(temp_ipv4_address, port_name);
        }
        else
        {
            delete_protected_ipv4(temp_ipv4_address);
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C2 "Successfully removed %s from protect table (if existed).\n" RE "", temp_ipv4_address);
            usleep(100000);
        }

        ConfirmAndSaveConfig(serial_port);
    }

    void SetIPv6Target(int serial_port)
    {
        char choice;
        printf(C6 "\r\n ============================================================================================================================================================================================================+");
        printf(C6 "\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to (1) Add or (2) Remove Server IPv6 address from protect table ,or (Z) Exit?" RE " (" C2 "1" RE "/" C3 "2" RE "/" C1 "Z" RE "): " RE);
        while (1)
        {
            scanf(" %c", &choice);
            if (choice == '1' || choice == '2' || choice == 'Z' || choice == 'z')
                break;
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Please enter 1 (ADD), 2 (REMOVE), or Z" RE " (" C1 "EXIT?" RE "):" RE " ");
        }

        if (choice == 'Z' || choice == 'z')
        {
            // printf("\r\n    SETTING     | Exiting IPv4 target configuration.\n");
            return;
        }

        if (choice == '1')
        {
            current_config_type = CONFIG_IPV6_PROTECT;
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv6 address want to protect :" RE " ");
        }
        else
        {
            current_config_type = CONFIG_REMOVE_IPV6_PROTECT;
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv6 address want to remove IP from protect table : " RE);
        }

        char ipv6_address[40];
        struct in6_addr addr;
        unsigned short ipv6_segments[8];
        while (1)
        {
            scanf("%39s", ipv6_address);
            if (inet_pton(AF_INET6, ipv6_address, &addr) == 1)
            {
                for (int i = 0; i < 8; i++)
                {
                    ipv6_segments[i] = (addr.s6_addr[i * 2] << 8) | addr.s6_addr[i * 2 + 1];
                }
                snprintf(full_ipv6_address, sizeof(full_ipv6_address),
                        "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
                        ipv6_segments[0], ipv6_segments[1], ipv6_segments[2], ipv6_segments[3],
                        ipv6_segments[4], ipv6_segments[5], ipv6_segments[6], ipv6_segments[7]);
                strcpy(temp_ipv6_address, full_ipv6_address);
                break;
            }
            else
            {
                printf(C3 " Invalid! Please input!!!" RE);
            }
        }

        if (choice == '1')
        {
            char port_name[8];
            snprintf(port_name, sizeof(port_name), "eth%d", current_port);
            update_protected_ipv6_port(temp_ipv6_address, port_name);
        }
        else
        {
            delete_protected_ipv6(temp_ipv6_address);
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C2 "Successfully removed %s from protect table (if existed).\n" RE "", temp_ipv6_address);
            usleep(100000);
        }
        ConfirmAndSaveConfig(serial_port);
    }

    int validate_ipv6_address(const char *ip_address)
    {
        struct in6_addr addr6;
        return inet_pton(AF_INET6, ip_address, &addr6) == 1;
    }

    // void SetIPv6Block(int serial_port)
    // {
    //     char key_mode = 'X';
    //     write(serial_port, &key_mode, sizeof(key_mode));
    //     usleep(100000);
    //     write(serial_port, &key_enter, sizeof(key_enter));
    //     printf("\r\n");
    //     printf("\r\n =================================================================================================+");
    //     printf("\r\n    SETTING     | Enter Server IPv4 address want to remove IP from block table : ");
    //     send_ipv4_address_http_remove(serial_port);
    // }

    void SetIPv6Block(int serial_port)
    {
        char choice;
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to (1) Add or (2) Remove Server IPv6 address from Block table ,or (Z) Exit? (" C2 "1" RE "/" C3 "2" RE "/" C1 "Z" RE "):" RE " ");

        while (1)
        {
            scanf(" %c", &choice);
            if (choice == '1' || choice == '2' || choice == 'Z' || choice == 'z')
                break;
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Please enter" RE " " C2 "1 (ADD)" RE ", " C5 "2 (REMOVE)" RE ", " C6 "or" RE " " C1 "Z" RE " (" C1 "EXIT?" RE "): ");
        }

        if (choice == 'Z' || choice == 'z')
        {
            // printf("\r\n    SETTING     | Exiting IPv4 target configuration.\n");
            return;
        }

        if (choice == '3')
        {
            // printf("\r\n    SETTING     | Exiting IPv6 target configuration.\n");
            return;
        }
        if (choice == '1')
        {
            current_config_type = CONFIG_IPV6_BLOCK;
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv6 address you want to ADD: " RE);
        }
        else
        {
            current_config_type = CONFIG_REMOVE_IPV6_BLOCK;
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv6 address you want to REMOVE from protect table: " RE);
        }

        char ipv6_address[40];
        struct in6_addr addr;
        unsigned short ipv6_segments[8];
        while (1)
        {
            scanf("%39s", ipv6_address);
            if (inet_pton(AF_INET6, ipv6_address, &addr) == 1)
            {
                for (int i = 0; i < 8; i++)
                {
                    ipv6_segments[i] = (addr.s6_addr[i * 2] << 8) | addr.s6_addr[i * 2 + 1];
                }
                snprintf(full_ipv6_address, sizeof(full_ipv6_address),
                        "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
                        ipv6_segments[0], ipv6_segments[1], ipv6_segments[2], ipv6_segments[3],
                        ipv6_segments[4], ipv6_segments[5], ipv6_segments[6], ipv6_segments[7]);
                strcpy(temp_ipv6_address, full_ipv6_address);
                break;
            }
            else
            {
                printf(C3 " Invalid! Please input!!!" RE);
                printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv6 address : " RE);
            }
        }

        if (choice == '1')
        {
            char port_name[8];
            snprintf(port_name, sizeof(port_name), "eth%d", current_port);
            update_blocked_ipv6(temp_ipv6_address, port_name);
        }
        else
        {
            delete_blocked_ipv6(temp_ipv6_address);
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C2 "Successfully removed %s from protect table (if existed).\n" RE "", temp_ipv6_address);
            usleep(100000);
        }

        ConfirmAndSaveConfig(serial_port);
    }
    // void SetIPv4Block(int serial_port)
    // {
    //   char key_mode = 'T';
    //   write(serial_port, &key_mode, sizeof(key_mode));
    //   usleep(100000);
    //   write(serial_port, &key_enter, sizeof(key_enter));
    //   printf("\r\n");
    //   printf("\r\n ============================================================================================================================================================================================================+");
    //   printf("\r\n    SETTING     | Enter Server IPv4 address want to block : ");
    //   send_ipv4_address_http_add(serial_port);
    // }

    void SetIPv4Block(int serial_port)
    {
        char choice;
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C3 "Do you want to (1) Add or (2) Remove Server IPv4 address from Block table ,or (Z) Exit? (" C2 "1" RE "/" C3 "2" RE "/" C1 "Z" RE "): ");

        while (1)
        {
            scanf(" %c", &choice);
            if (choice == '1' || choice == '2' || choice == 'Z' || choice == 'z')
                break;
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Please enter " C2 "1" RE " (ADD), " C5 "2" RE " (REMOVE), or " C1 "Z" RE " (" C1 "EXIT?" RE "): " RE);
        }

        if (choice == 'Z' || choice == 'z')
        {
            // printf("\r\n    SETTING     | Exiting IPv4 target configuration.\n");
            return;
        }

        if (choice == '1')
        {
            current_config_type = CONFIG_IPV4_BLOCK;
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv4 address you want to ADD: " RE);
        }
        else
        {
            current_config_type = CONFIG_REMOVE_IPV4_BLOCK;
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv4 address you want to REMOVE from Block table: " RE);
        }

        while (1)
        {
            scanf("%15s", temp_ipv4_address);
            if (validate_ip_address(temp_ipv4_address))
                break;
            printf(C3 " Invalid! Please input again: " RE);
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv6 address you want to ADD: " RE); // Hi?n th? l?i yêu c?u nh?p IPv6// Hi?n th? l?i yêu c?u nh?p IPv6
        }

        if (choice == '1')
        {
            char port_name[8];
            snprintf(port_name, sizeof(port_name), "eth%d", current_port);
            update_blocked_ipv4(temp_ipv4_address, port_name);
        }
        else
        {
            delete_blocked_ipv4(temp_ipv4_address);
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C2 "Successfully removed %s from protect table (if existed).\n" RE "", temp_ipv4_address);
            usleep(100000);
        }

        ConfirmAndSaveConfig(serial_port);
    }

    void RemoveIPv6Block(int serial_port)
    {
        char key_mode = '}';
        char key_enter = '\r';
        write(serial_port, &key_mode, sizeof(key_mode));
        usleep(100000);
        write(serial_port, &key_enter, sizeof(key_enter));
        printf("\r\n");
        printf(C6 "\r\n ============================================================================================================================================================================================================+");
        printf(C6 "\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Enter Server IPv6 address want to remove IP from block table : " RE);
        send_ipv6_address_http_remove(serial_port, temp_ipv6_address);
    }

    void ReturnMode2(int serial_port)
    {
        char key;
        char enter = '\r';
        // system("clear");
        // display_logo1();
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Return to the device configuration menu? (" C2 "Y" RE "):" RE);
        while (1)
        {
            scanf("%c", &key);
            if (key == 'y' || key == 'Y')
            {
                break;
            }
            if (key != 'y' || key != 'Y')
            {
                printf("\r    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Return to the device configuration menu? (" C2 "Y" RE "):" RE);
            }
        }
        new_menu(serial_port);
    }

    void ReturnMode2b(int serial_port)
    {
        system("clear");
        display_logo1();
        // display_table(serial_port);
        char key;
        char enter = '\r';
        // printf("\n");
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Return to the device configuration menu? (" C2 "Y" RE "):" RE);

        while (1)
        {
            scanf("%c", &key);
            if (key == 'y' || key == 'Y')
            {
                break;
            }
            if (key != 'y' || key != 'Y')
            {
                printf("\r    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Return to the device configuration menu?" RE " (" C2 "Y" RE "):" RE);
            }
        }
    }
    void printf_uart(int serial_port)
    {
        int a = 0;
        char read1[50000];
        while (1)
        {
            memset(&read1, 0, sizeof(read1));
            int num_bytes = read(serial_port, &read1, sizeof(read1));
            printf(C3 "%s" RE "", read1);
            a = a + 1;
            if (read1[num_bytes - 1] == '*')
            {
                break;
            }
        }
    }

    void printf_uart1(int serial_port)
    {

        struct winsize w;
        int max_lines, current_line = 0;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        max_lines = w.ws_row - 3;
        char key2, ch, key3;
        bool rs = false;
        struct statvfs stat1;
        full_sd1 = false;
        system("clear");
    start:
        printf(C3 "\033[1;36m \n\n" C6 "|" RE "" C3 "\tTime\t\t" RE "" C6 "|" RE "  " C3 "\t\tSource IP\t\t" RE "    " C6 "|" RE "  " C3 "\t\tDest IP\t\t\t" RE "       " C6 "|" RE " " C3 "Source Port\t" RE "" C6 "|" RE " " C3 "Dest Port\t" RE "" C6 "|" RE " " C3 "Protocol\t" RE "" C6 "|" RE " " C3 "\tType\t\t" RE "" C6 "|" RE "" C3 "\tBW\t" RE "" C6 "|" RE "" C3 "\tPKT/s\t" RE "" C6 "|" RE "\n\033[0m" RE);

        if (statvfs("/", &stat1) != 0)
        {
            perror(C3 "statvfs error" RE);
            pthread_exit(NULL);
        }

        unsigned long total_space = (stat1.f_blocks * stat1.f_frsize);
        unsigned long used_space = (stat1.f_blocks - stat1.f_bfree) * stat1.f_frsize;
        float memory_usage = (float)used_space / total_space * 100;
        float CAPACITY = (float)total_space / (1024 * 1024 * 1024);
        float used_space_gb = (float)used_space / (1024 * 1024 * 1024);

        char read1[50000];
        int ctrl_p_pressed = 0;
        time_t ctrl_p_time;

        char msg10[] = {'\r'};

        while (1)
        {

            if (print_buffer_pos > 0)
            {

                if (current_line >= max_lines - 3)
                {
                    system("clear");
                    printf(C3 "\033[1;36m \n\n" C6 "|" RE "" C3 "\tTime\t\t" RE "" C6 "|" RE "  " C3 "\t\tSource IP\t\t" RE "    " C6 "|" RE "  " C3 "\t\tDest IP\t\t\t" RE "       " C6 "|" RE " " C3 "Source Port\t" RE "" C6 "|" RE " " C3 "Dest Port\t" RE "" C6 "|" RE " " C3 "Protocol\t" RE "" C6 "|" RE " " C3 "\tType\t\t" RE "" C6 "|" RE "" C3 "\tBW\t" RE "" C6 "|" RE "" C3 "\tPKT/s\t" RE "" C6 "|" RE "\n\033[0m" RE);

                    current_line = 0;
                }

                printf(C3 "%s" RE "", print_buffer);
                current_line++;
                fflush(stdout);

                print_buffer_pos = 0;
            }

            if (kbhit())
            {
                ch = getchar();

                if (ch == 16)
                {

                    ctrl_p_pressed = 1;
                    ctrl_p_time = time(NULL);
                }
            }
            if (ctrl_p_pressed && (time(NULL) - ctrl_p_time >= 1))
            {
                break;
            }

            if ((full_sd == true && full_sd1 == false))
            {
                do
                {
                    printf("\r\n " C6 "|" RE " " C3 "ALLOWED THRESHOLD IS EXCEEDED, PLEASE CHECK LOGFILE !!! " RE);
                    printf("\r\n " C6 "|" RE "" C3 " CAPACITY:" RE " %.2f GB (100 %)" RE "   " C6 "|" RE "  " C3 "Used space: %.2f GB (%.2f%%)" RE "    " C6 "|" RE " " C3 "Free space: %.2f GB (%.2f%%)" RE "", CAPACITY, used_space_gb, 100 - memory_usage, CAPACITY - used_space_gb, memory_usage);
                    printf("\r\n " C6 "|" RE " " C3 "Do you want check log file (" C2 "Y" RE "/" C1 "N" RE ")? :  " RE);
                    scanf(" %c", &key2);
                    if ((key2 == 'Y') || (key2 == 'y'))
                    {
                        break;
                    }
                    else if ((key2 == 'N') || (key2 == 'n'))
                    {
                        break;
                    }

                } while (1);
                if ((key2 == 'Y') || (key2 == 'y'))
                {
                    Mode_Condition_SDCard(serial_port);
                }
                else if ((key2 == 'N') || (key2 == 'n'))
                {
                    full_sd1 = true;

                    goto start;
                }
            }
        }

        if (ch == 16)
        {
            system("clear");
            options_mode1(serial_port);
        }
        else if (ch == '\n')
        {
            system("clear");
            return;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    int send_array(int serial_port)
    {
        char array[10];
        int valid_input = 0;
        int num = 0;

        while (!valid_input)
        {
            scanf("%s", array);
            num = atoi(array);
            if (num > 65535)
            {
                printf(C3 "The value must be less than 65536, Please re-enter: " RE);
            }
            else
            {
                valid_input = 1;
            }
        }

        return num;
    }

    void send_duration_time(int serial_port)
    {
        char key_mode = '%';
        char array[10];
        char enter[] = {'\r'};
        int valid_input = 0;

        while (!valid_input)
        {
            scanf("%s", array);
            int num = atoi(array);
            if (num > 20000000)
            {
                printf(C3 "The value must be less than 2s, Please re-enter: " RE);
            }
            else
            {
                valid_input = 1;
            }
        }

        write(serial_port, &key_mode, sizeof(key_mode));
        usleep(100000);
        write(serial_port, &key_enter, sizeof(key_enter));
        usleep(300000);
        int n = strlen(array);
        for (int i = 0; i < n; i++)
        {
            char data[2] = {array[i], '\0'};
            send_data(serial_port, data, sizeof(data) - 1);
        }
        sleep(1);
        write(serial_port, enter, sizeof(enter));
    }

    int is_valid_date(int day, int month, int year)
    {
        int days_in_month[] = {31, 28 + (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (month < 1 || month > 12)
            return 0;
        return day >= 1 && day <= days_in_month[month - 1];
    }

    void send_user_time(int serial_port)
    {
        char msg10[] = {'\r'};
        int yy, mm, dd, hh, xx, cc;
        char time_array[11];
        char confirm;
        do
        {

            while (1)
            {
                printf(C3 "\r\n\t\t| Input year(yy, ex: 24 for 2024): " RE);
                scanf("%d", &yy);
                if (yy >= 0 && yy <= 99)
                    break;
                printf(C3 "\r\n\t\t| Invalid year. Please re-enter:" RE);
            }
            while (1)
            {
                printf(C3 "\r\n\t\t| Input month (mm,  01 --> 12): " RE);
                scanf("%d", &mm);
                if (mm >= 1 && mm <= 12)
                    break;
                printf(C3 "\r\n\t\t| Invalid month. Please re-enter:" RE);
            }
            while (1)
            {
                printf(C3 "\r\n\t\t| Input day (dd, 	01 --> 31): " RE);
                scanf("%d", &dd);
                //  break;
                if (is_valid_date(dd, mm, 2000 + yy))
                    break;
                printf(C3 "\r\n\t\t| Invalid day. Please re-enter:" RE);
            }

            while (1)
            {
                printf(C3 "\r\n\t\t| Input hour (hh,  01 --> 12): " RE);
                scanf("%d", &hh);
                //  break;
                if (hh >= 1 && hh <= 12)
                    break;
                printf(C3 "\r\n\t\t| Invalid hour. Please re-enter: " RE);
            }

            // Nh?p ph t (xx)
            while (1)
            {
                printf(C3 "\r\n\t\t| Input minutes (mm,  00 --> 59): " RE);
                scanf("%d", &xx);
                if (xx >= 0 && xx <= 59)
                    break;
                printf(C3 "\r\n\t\t| Invalid minutes. Please re-enter: " RE);
            }

            // Nh?p gi y (cc)
            while (1)
            {
                printf(C3 "\r\n\t\t| Input seconds (ss, 00 --> 59):  " RE);
                scanf("%d", &cc);
                if (cc >= 0 && cc <= 59)
                    break;
                printf(C3 "\r\n\t\t| Invalid seconds. Please re-enter:\n " RE);
            }
            printf(C3 "\n\t\t| You entered: %02d-%02d-%02d  %02d:%02d:%02d" RE "", yy, mm, dd, hh, xx, cc);
            printf(C3 "\n\t\t| Are you sure?" RE " (" C2 "Y" RE "/" C1 "N" RE "): ");
            scanf(" %c", &confirm);
        } while (confirm != 'Y' && confirm != 'y');

        time_array[0] = '0' + (yy / 10);
        time_array[1] = '0' + (yy % 10);
        time_array[2] = '0' + (mm / 10); // Ch? s? d?u ti n c?a mm
        time_array[3] = '0' + (mm % 10); // Ch? s? th? hai c?a mm
        time_array[4] = '0' + (dd / 10); // Ch? s? d?u ti n c?a dd

        time_array[5] = '0' + (dd % 10);  // Ch? s? th? hai c?a dd
        time_array[6] = '0' + (hh / 10);  // Ch? s? d?u ti n c?a hh
        time_array[7] = '0' + (hh % 10);  // Ch? s? th? hai c?a hh
        time_array[8] = '0' + (xx / 10);  // Ch? s? d?u ti n c?a xx
        time_array[9] = '0' + (xx % 10);  // Ch? s? th? hai c?a xx
        time_array[10] = '0' + (cc / 10); // Ch? s? d?u ti n c?a cc
        time_array[11] = '0' + (cc % 10); // Ch? s? th? hai c?a cc

        for (int i = 0; i < 12; i++)
        {
            usleep(100000);
            send_data(serial_port, &time_array[i], sizeof(char));
            // printf("   g?i : %s\n", time_array[i]);
            // printf("20%d - %d - %d  %d-%d-%d", yy,mm,dd,hh,xx,cc);
        }

        //  printf("\t\t=============================");
        //    printf("\t\t20%d - %d - %d  %d:%d:%d", yy,mm,dd,hh,xx,cc);

        sleep(1);
        write(serial_port, msg10, sizeof(msg10));
    }

    void change_info_acc_admin_mode(int serial_port)
    {
    start:
        system("clear");
        char key = 0;
        char enter = '\r';
        display_logo1();
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf(C6 "\r\n " C3 "==> Mode 3 is selected." RE "                                                                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n                                                                                                                                                                                                             |" RE);
        printf(C6 "\r\n================+===========+==========================================+=====================================================================================================================================+" RE);
        printf(C6 "\r\n    " C3 "DISPLAY" RE "     " C6 "|" RE "           " C6 "|" RE "                                          " C6 "|" RE "                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t|" RE " " C3 "Key Enter" RE " " C6 "|" RE "           " C3 "Choose 1 option below:" RE "         " C6 "|" RE "                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+------------------------------------------+                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "0." RE "    " C6 "|" RE " " C6 "Change password user account." RE "            " C6 "|" RE "                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+------------------------------------------+                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "Add new user account." RE "                    " C6 "|" RE "                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+------------------------------------------+                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 "Display saved user account." RE "              " C6 "|" RE "                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+------------------------------------------+                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "3." RE "    " C6 "|" RE " " C6 "Delete user account." RE "                     " C6 "|" RE "                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+------------------------------------------+                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "4." RE "    " C6 "|" RE " " C6 "Reset all user accounts." RE "                 " C6 "|" RE "                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+------------------------------------------+                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "5." RE "    " C6 "|" RE " " C6 "Change root password." RE "                    " C6 "|" RE "                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+------------------------------------------+                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "6." RE "    " C6 "|" RE " " C6 "Load default setting from manufacturer." RE "  " C6 "|" RE "                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+------------------------------------------+                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t|" RE "     " C3 "7." RE "    " C6 "|" RE " " C17 "==> Exit." RE "                                " C6 "|" RE "                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t+-----------+------------------------------------------+                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n================+============================================================================================================================================================================================+" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE "  " C3 "--> Your choice: " RE);

        while (1)
        {
            scanf("%c", &key);
            if (key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7')
            {
                break;
            }
            if (key != '0' || key != '1' || key != '2' || key != '3' || key != '4' || key != '5' || key != '6' || key != '7')
            {
                printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Your choice: " RE);
            }
        }

        if (key == '0')
        {
            system("clear");
            display_logo1();
            display_account(serial_port);
            check_username_change_pass(serial_port);
            goto start;
        }
        else if (key == '1')
        {
            system("clear");
            display_logo1();
            display_account(serial_port);
            add_acount(serial_port);
            goto start;
        }
        else if (key == '2')
        {
            system("clear");
            display_logo1();
            display_account(serial_port);
            ReturnMode3();
            goto start;
        }
        else if (key == '3')
        {
            system("clear");
            display_logo1();
            display_account(serial_port);
            delete_account(serial_port);
            goto start;
        }
        else if (key == '4')
        {
            system("clear");
            display_logo1();
            display_account(serial_port);
            reset_account(serial_port);
            goto start;
        }
        else if (key == '5')
        {
            system("clear");
            display_logo1();
            display_account(serial_port);
            change_root(serial_port);
            goto start;
        }

        else if (key == '6')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            Display_table_2(serial_port);
            setload_default(serial_port);
            goto start;
        }
        else if (key == '7')
        {
            // DisplayTable();
            // printf_uart(serial_port);
            // SaveEEPROM(serial_port);
        }
    }
    void ReturnMode3()
    {
        char key;
        char enter = '\r';
        // system("clear");
        // display_logo2();
        // display_account(serial_port);

        printf("\r\n================+============================================================================================================================================================================================+");
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Return to the device configuration menu?" RE " (" C2 "Y" RE "): ");

        while (1)
        {
            scanf("%c", &key);
            if (key == 'y' || key == 'Y')
            {
                system("clear");
                // write(serial_port, &key, sizeof(key));
                // usleep(100000);
                // write(serial_port, &enter, sizeof(enter));
                // usleep(1000000);
                break;
            }
            if (key != 'y' || key != 'Y')
            {
                printf("\r    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Return to the device configuration menu?" RE " (" C2 "Y" RE "): ");
            }
        }

        // char *data = receive_data(serial_port);
        // if (data == NULL)
        // {
        //   return;
        // }
        // if ((strchr(data, 'X') != NULL))
        // {
        //   change_info_acc_admin_mode(serial_port);
        // }
    }

    void add_acount(int serial_port)
    {
        // char key_mode = '+';
        // write(serial_port, &key_mode, sizeof(key_mode));
        // usleep(100000);
        // write(serial_port, &key_enter, sizeof(key_enter));
        // usleep(100000);
        input_and_send_account1(serial_port);
        char *data = receive_data(serial_port);
        if (data == NULL)
        {
            printf(C3 "Error receiving data\n" RE);
            return;
        }
        if ((strchr(data, 'Y') != NULL))
        {
            printf(C6 "\r\n\t\t+============================================================================================================================================================================================+" RE);
            printf(C6 "\r\n\t\t|" RE "                                               " C2 "Successfully !!!" RE "                                                                                                                             " C6 "|" RE);
            printf(C6 "\r\n\t\t+============================================================================================================================================================================================+" RE);
            usleep(2000000);
            system("clear");
            display_logo1();
            display_account(serial_port);
            ReturnMode3();
        }
        else if ((strchr(data, 'F') != NULL))
        {
            printf(C6 "\r\n\t\t+============================================================================================================================================================================================+" RE);
            printf(C6 "\r\n\t\t|" RE " " C3 "Cannot add new user. The number of user accounts is full!." RE "                                                                                                                                 " C6 "|" RE);
            printf(C6 "\r\n\t\t+============================================================================================================================================================================================+" RE);
            usleep(2000000);
            system("clear");
            display_logo1();
            display_account(serial_port);
            ReturnMode3();
        }
        else if ((strchr(data, 'V') != NULL))
        {
            printf(C6 "\r\n\t\t+============================================================================================================================================================================================+" RE);
            printf(C6 "\r\n\t\t|" RE " " C3 "Cannot add new user. Account already exists!." RE "                                                                                                                                              " C6 "|" RE);
            printf(C6 "\r\n\t\t+============================================================================================================================================================================================+" RE);
            printf(C6 "\r\n\t\t|                                                                                                                                                                                            |" RE);
            printf(C6 "\r\n\t\t+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
            usleep(2000000);
            system("clear");
            display_logo1();
            display_account(serial_port);
            ReturnMode3();
        }
        if ((strchr(data, 'G') != NULL))
        {
            printf(C6 "\r\n\t\t+============================================================================================================================================================================================+" RE);
            printf(C6 "\r\n\t\t|" RE " " C2 "Change Password Successfully!." RE "                                                                                                                                                             " C6 "|" RE);
            printf(C6 "\r\n\t\t+============================================================================================================================================================================================+" RE);
            usleep(2000000);
            system("clear");
            display_logo1();
            display_account(serial_port);
            ReturnMode3();
        }
    }
    void delete_account(int serial_port)
    {
        // char key_mode = ')';
        // write(serial_port, &key_mode, sizeof(key_mode));
        // usleep(100000);
        // write(serial_port, &key_enter, sizeof(key_enter));
        // usleep(100000);
        input_and_send_username(serial_port);
        char *data = receive_data(serial_port);
        if (data == NULL)
        {
            printf(C3 "Error receiving data\n" RE);
            return;
        }
        if ((strchr(data, 'Y') != NULL))
        {
            printf(C6 "\r\n\t\t+============================================================================================================================================================================================+" RE);
            printf(C6 "\r\n\t\t|" RE "                                               " C2 "Successfully !!!" RE "                                                                                                                             " C6 "|" RE);
            printf(C6 "\r\n\t\t+============================================================================================================================================================================================+" RE);
            sleep(2);
            system("clear");
            display_logo1();
            display_account(serial_port);
            ReturnMode3();
        }
        else if ((strchr(data, 'F')))
        {
            printf(C6 "\r\n\t\t+============================================================================================================================================================================================+" RE);
            printf(C6 "\r\n\t\t|" RE "                                               " C3 "Wrong user account." RE "                                                                                                                          " C6 "|" RE);
            printf(C6 "\r\n\t\t+============================================================================================================================================================================================+" RE);
            sleep(2);
            system("clear");
            display_logo1();
            display_account(serial_port);
            ReturnMode3();
        }
    }
    void DisplayAccount(int serial_port)
    {
        ReturnMode3(serial_port);
    }

    void reset_account(int serial_port)
    {
        char key;
        char enter = '\r';
        char key_mode = '[';
        // write(serial_port, &key_mode, sizeof(key_mode));
        // usleep(100000);
        // write(serial_port, &key_enter, sizeof(key_enter));
        // usleep(100000);
        printf(C6 "\r\n================+============================================================================================================================================================================================+" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to reset all user account?" RE " (" C2 "Y" RE "): ");

        while (1)
        {
            scanf("%c", &key);
            if (key == 'y' || key == 'Y')
            {
                write(serial_port, &key_mode, sizeof(key_mode));
                usleep(100000);
                write(serial_port, &key_enter, sizeof(key_enter));
                usleep(500000);
                write(serial_port, &key, sizeof(key));
                usleep(100000);
                // write(serial_port,&enter,sizeof(enter));
                // usleep(1000000);
                break;
            }
            if (key == 'n' || key == 'N')
            {
                break;
            }

            if (key != 'y' || key != 'Y' || key != 'n' || key != 'N')
            {
                printf("\r    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to reset all user account?" RE " (" C2 "Y" RE "): " RE);
            }
        }
        system("clear");
        display_logo1();
        display_account(serial_port);
        ReturnMode3();
        // char *data = receive_data(serial_port);
        // if (data == NULL)
        // {
        //   printf("Error receiving data\n");
        //   return;
        // }
        // if ((strchr(data, 'R') != NULL))
        // {
        //   system("clear");
        //   ReturnMode3(serial_port);
        // }
    }

    void change_root(int serial_port)
    {
        // char key_mode = ';';
        // write(serial_port, &key_mode, sizeof(key_mode));
        // usleep(100000);
        // write(serial_port, &key_enter, sizeof(key_enter));
        // usleep(100000);
        input_and_send_password(serial_port);
        // char *data = receive_data(serial_port);
        // if (data == NULL)
        // {
        //   printf("Error receiving data\n");
        //   return;
        // }
        // if ((strchr(data, 'N') != NULL) || (strchr(data, 'n')))
        // {
        //   system("clear");
        //   ReturnMode3(serial_port);
        // }
        system("clear");
        display_logo1();
        display_account(serial_port);
        ReturnMode3();
    }

    void setload_default(int serial_port)
    {
        char key;
        char enter = '\r';
        char key_mode = ']';

        printf(C6 "\r\n================+============================================================================================================================================================================================+\n" RE);
        printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to load default device configuration from manufacturer?" RE " (" C2 "Y" RE "/" C1 "N" RE "): ");

        while (1)
        {
            scanf("%c", &key);
            if (key == 'y' || key == 'Y' || key == 'n' || key == 'N')
            {

                write(serial_port, &key_mode, sizeof(key_mode));
                usleep(100000);
                write(serial_port, &key_enter, sizeof(key_enter));
                usleep(500000);

                write(serial_port, &key, sizeof(key));
                usleep(100000);

                break;
            }
            if (key != 'y' || key != 'Y' || key != 'n' || key != 'N')
            {

                printf("\r   " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Do you want to load default device configuration from manufacturer?" RE " (" C2 "Y" RE "/" C1 "N" RE "): ");
            }
        }
        system("clear");
        display_logo1();
        display_table(serial_port);
        ReturnMode3();
    }

    int kbhit(void)
    {
        struct termios oldt, newt;
        int ch;
        int oldf;

        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~ICANON;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

        ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);

        if (ch != EOF)
        {
            ungetc(ch, stdin);
            return 1;
        }

        return 0;
    }
    // Ham cau hinh port uart
    int configure_serial_port(const char *device, int baud_rate)
    {
        int serial_port = open(device, O_RDWR);
        struct termios tty;
        if (tcgetattr(serial_port, &tty) != 0)
        {
            printf(C3 "Error %i from tcgetattr: %s\n" RE "", errno, strerror(errno));
            return 1;
        }
        tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
        tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit used in communication (most common)
        tty.c_cflag &= ~CSIZE;         // Clear all bits that set the data size
        tty.c_cflag |= CS8;            // 8 bits per byte (most common)
        tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
        tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

        tty.c_lflag &= ~ICANON;
        tty.c_lflag &= ~ECHO;                                                        // Disable echo
        tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
        tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
        tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
        tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

        tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
        tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
        // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
        // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

        tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
        tty.c_cc[VMIN] = 0;

        // Set in/out baud rate
        cfsetispeed(&tty, baud_rate);
        cfsetospeed(&tty, baud_rate);

        // Save tty settings, also checking for error
        if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
        {
            printf(C3 "Error %i from tcsetattr: %s\n" RE "", errno, strerror(errno));
            return 1;
        }
        return serial_port;
    }
    // Ham nhap account
    void input_and_send_account(int serial_port)
    {
        char enter[] = {'\r'};
        char username[17];
        char password[17];
        int valid_input = 0;

        while (!valid_input)
        {
            printf(C6 "\r\n ===============+============================================================================================================================================================================================+" RE);
            printf("\r\n      " C3 "LOG IN" RE "    " C6 "|" RE " " C6 "Username:" RE " ");
            scanf("%16s", username);
            printf("\r\n\t\t| " C6 "Password:" RE " ");
            scanf("%16s", password);

            if (strlen(username) > 16 || strlen(password) > 16)
            {
                printf(C3 "The account or password exceeds 16 characters. Please re-enter.\n" RE);
            }
            else
            {
                valid_input = 1;
            }
        }

        // while (getchar() != '\n')
        //   ;
        // getchar();
        write(serial_port, &key_check_account, sizeof(key_check_account));
        usleep(1000);
        write(serial_port, &key_enter, sizeof(key_enter));
        usleep(200000);
        int n_username = strlen(username);
        for (int i = 0; i < n_username; i++)
        {
            usleep(50000);
            char data[2] = {username[i], '\0'};
            send_data(serial_port, data, sizeof(data) - 1);
        }
        usleep(500000);
        write(serial_port, enter, sizeof(enter));

        int n_password = strlen(password);
        for (int i = 0; i < n_password; i++)
        {
            usleep(50000);
            char data[2] = {password[i], '\0'};
            send_data(serial_port, data, sizeof(data) - 1);
        }
        usleep(500000);
        write(serial_port, enter, sizeof(enter));
    }
    void input_and_send_account1(int serial_port)
    {
        char key_mode = '+';
        char enter[] = {'\r'};
        char username[17];
        char password[17];
        int valid_input = 0;

        while (!valid_input)
        {
            printf(C6 "\r\n ===============+============================================================================================================================================================================================+" RE);
            printf("\r\n    " C3 "Account" RE "     " C6 "|" RE " " C6 "Enter Username:" RE "");
            scanf("%16s", username);
            printf(C6 "\r\n\t\t| Enter Password: " RE);
            scanf("%16s", password);

            if (strlen(username) > 16 || strlen(password) > 16)
            {
                printf(C3 "The account or password exceeds 16 characters. Please re-enter.\n" RE);
            }
            else
            {
                valid_input = 1;
            }
        }

        // while (getchar() != '\n')
        //   ;
        // getchar();
        write(serial_port, &key_mode, sizeof(key_mode));
        usleep(100000);
        write(serial_port, &key_enter, sizeof(key_enter));
        usleep(200000);
        int n_username = strlen(username);
        for (int i = 0; i < n_username; i++)
        {
            usleep(50000);
            char data[2] = {username[i], '\0'};
            send_data(serial_port, data, sizeof(data) - 1);
        }
        sleep(1);
        write(serial_port, enter, sizeof(enter));

        int n_password = strlen(password);
        for (int i = 0; i < n_password; i++)
        {
            usleep(50000);
            char data[2] = {password[i], '\0'};
            send_data(serial_port, data, sizeof(data) - 1);
        }
        sleep(1);
        write(serial_port, enter, sizeof(enter));
    }
    void input_and_send_account2(int serial_port)
    {
        char key_mode = '(';
        char enter[] = {'\r'};
        char username[17];
        char password[17];
        int valid_input = 0;

        while (!valid_input)
        {
            printf(C6 "\r\n ===============+============================================================================================================================================================================================+" RE);
            printf("\r\n    " C3 "Account" RE "     " C6 "|" RE " " C6 "Enter Username:" RE " ");
            scanf("%16s", username);
            printf(C6 "\r\n\t\t| Enter Password: " RE);
            scanf("%16s", password);

            if (strlen(username) > 16 || strlen(password) > 16)
            {
                printf(C3 "The account or password exceeds 16 characters. Please re-enter.\n" RE);
            }
            else
            {
                valid_input = 1;
            }
        }

        // while (getchar() != '\n')
        //   ;
        // getchar();
        write(serial_port, &key_mode, sizeof(key_mode));
        usleep(100000);
        write(serial_port, &key_enter, sizeof(key_enter));
        usleep(200000);
        int n_username = strlen(username);
        for (int i = 0; i < n_username; i++)
        {
            usleep(50000);
            char data[2] = {username[i], '\0'};
            send_data(serial_port, data, sizeof(data) - 1);
        }
        sleep(1);
        write(serial_port, enter, sizeof(enter));

        int n_password = strlen(password);
        for (int i = 0; i < n_password; i++)
        {
            usleep(50000);
            char data[2] = {password[i], '\0'};
            send_data(serial_port, data, sizeof(data) - 1);
        }
        sleep(1);
        write(serial_port, enter, sizeof(enter));
    }
    // delete acount
    void input_and_send_username(int serial_port)
    {
        char key_mode = ')';

        char enter[] = {'\r'};
        char username[17];
        int valid_input = 0;

        while (!valid_input)
        {
            printf(C6 "\r\n ===============+============================================================================================================================================================================================+" RE);
            printf("\r\n    " C3 "Account" RE "     " C6 "|" RE " " C6 "Enter an account username account need to be deleted:" RE " ");
            scanf("%16s", username);

            if (strlen(username) > 16)
            {
                printf(C3 "The username exceeds 16 characters. Please re-enter.\n" RE);
            }
            else
            {
                valid_input = 1;
            }
        }
        // while (getchar() != '\n')
        //   ;
        // getchar();
        write(serial_port, &key_mode, sizeof(key_mode));
        usleep(100000);
        write(serial_port, &key_enter, sizeof(key_enter));
        usleep(200000);
        int n_username = strlen(username);
        for (int i = 0; i < n_username; i++)
        {
            usleep(50000);
            char data[2] = {username[i], '\0'};
            send_data(serial_port, data, sizeof(data) - 1);
        }
        sleep(1);
        write(serial_port, enter, sizeof(enter));
    }

    void input_and_send_password(int serial_port)
    {

        char key_mode = ';';

        // usleep(100000);
        // write(serial_port, &key_enter, sizeof(key_enter));
        // usleep(100000);
        char enter[] = {'\r'};
        char password[17];
        int valid_input = 0;

        while (!valid_input)
        {
            printf(C6 "\r\n ===============+============================================================================================================================================================================================+" RE);
            printf("\r\n    " C3 "Account" RE "     " C6 "|" RE " " C6 "Enter new password for root:" RE " ");
            scanf("%16s", password);

            if (strlen(password) > 16)
            {
                printf(C3 "The username exceeds 16 characters. Please re-enter.\n" RE);
            }
            else
            {
                valid_input = 1;
            }
        }
        // while (getchar() != '\n')
        //   ;
        // getchar();
        write(serial_port, &key_mode, sizeof(key_mode));
        usleep(100000);
        write(serial_port, &key_enter, sizeof(key_enter));
        usleep(200000);
        int n_password = strlen(password);
        for (int i = 0; i < n_password; i++)
        {
            usleep(50000);
            char data[2] = {password[i], '\0'};
            send_data(serial_port, data, sizeof(data) - 1);
        }
        sleep(1);
        write(serial_port, enter, sizeof(enter));
    }
    // Ham gui du lieu tu uart
    int send_data(int serial_port, const char *data, size_t size)
    {
        int num_byte = write(serial_port, data, size);
        if (num_byte < 0)
        {
            printf(C3 "Error reading: %s" RE "", strerror(errno));
            return 1;
        }
        return num_byte;
    }
    /////////////////////////khong duoc xoa ////////////////////////////////
    // Ham nhan du lieu tu uart
    char *receive_data(int serial_port)
    {
        static char read_buf[256];
        memset(read_buf, '\0', sizeof(read_buf));

        int num_bytes = read(serial_port, &read_buf, sizeof(read_buf) - 1);
        if (num_bytes < 0)
        {
            // printf("Error reading: %s\n", strerror(errno));
            return NULL;
        }
        // read_buf[num_bytes] = '\0';
        return read_buf;
    }

    char *receive_data2(int serial_port)
    {
        static char read_buf[256];
        memset(read_buf, '\0', sizeof(read_buf));

        int num_bytes = read(serial_port, read_buf, sizeof(read_buf) - 1); // Ð?c t?i da 255 ký t?, ch?a 1 cho '\0'
        if (num_bytes < 0)
        {
            printf(C3 "Error reading: %s\n" RE "", strerror(errno));
            return NULL;
        }

        // read_buf[num_bytes] = '\0'; // Ð?m b?o k?t thúc chu?i
        return read_buf;
    }

    //
    void Mode_Condition_SDCard(int serial_port)
    {
        system("clear");
        display_logo1();
        char key, key1;
        char enter = '\r';
        printf("\r\n                                                                                                                                                                                                             |");
        display_setting_user1();
        printf(C6 "\r\n ================+===========+===============================================================================================================================================================================+" RE);
        printf(C6 "\r\n     " C3 "DISPLAY" RE "     " C6 "|" RE "           " C6 "|" RE "                                                                                                                                                                               " C6 "|" RE);
        printf(C6 "\r\n\t\t |" RE " " C3 "Key Enter" RE " " C6 "|" RE "                  " C3 "Mode" RE "                                                                                                                                                         " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "1:" RE "    " C6 "|" RE " " C6 "Display Flood LogFile" RE "                                                                                                                                                                " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "2:" RE "    " C6 "|" RE " " C6 "Display Normal LogFile" RE "                                                                                                                                                               " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "Z." RE "    " C6 "|" RE " " C6 "Exit" RE "                                                                                                                                                                          " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n ----------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n ================+===========================================================================================================================================================================================+\n" RE);

        while (1)
        {
            scanf("%c", &key);
            if (key == '1')
            {
                display_log_files(LOG_FLOOD_DIR);

                printf(C6 "\r\n ================+===========================================================================================================================================================================================+\n" RE);
                printf(C3 "\r\n    SETTING" RE "     " C6 "|" RE " " C6 "Press key" RE " " C2 "Y" RE " " C6 "to return!" RE);

                while (1)
                {
                    scanf("%c", &key1);
                    if (key1 == 'y' || key1 == 'Y')
                    {
                        system("clear");
                        display_logo1();
                        Mode_Condition_SDCard(serial_port);
                    }
                    if (key1 != 'y' || key1 != 'Y')
                    {
                        printf(C3 "\r\n    SETTING" RE "     " C6 "|" RE " " C6 "Press key" RE " " C2 "Y" RE " " C6 "to return!" RE);
                    }
                }
            }
            if (key == '2')
            {
                display_log_files(LOG_NORMAL_DIR);

                printf(C6 "\r\n ================+===========================================================================================================================================================================================+\n" RE);
                printf(C3 "\r\n    SETTING" RE "     " C6 "|" RE " " C6 "Press key" RE " " C2 "Y" RE " " C6 "to return!" RE);

                while (1)
                {
                    scanf("%c", &key1);
                    if (key1 == 'y' || key1 == 'Y')
                    {
                        system("clear");
                        display_logo1();
                        Mode_Condition_SDCard(serial_port);
                    }
                    if (key1 != 'y' || key1 != 'Y')
                    {
                        printf(C3 "\r\n    SETTING" RE "     " C6 "|" RE " " C6 "Press key" RE " " C2 "Y" RE " " C6 "to return!" RE);
                    }
                }
            }
            if (key == 'Z' || key == 'z')
            {
                options_mode1(serial_port);
                break;
            }

            if (key != '1' || key != '2' || key == 'Z' || key == 'z')
            {
                printf(C3 "\r     SETTING" RE "     " C6 "|" RE " " C6 "--> Please choose Mode:" RE " ");
            }
        }
    }

    void Mode_Condition_SDCard_User(int serial_port)
    {
    start:
        system("clear");
        display_logo1();
        char key, key1;
        char key3 = 'E';
        char enter = '\r';
        printf("\r\n                                                                                                                                                                                                             |");
        display_setting_user();
        printf(C6 "\r\n ================+===========+===============================================================================================================================================================================+" RE);
        printf(C6 "\r\n     " C3 "DISPLAY" RE "     " C6 "|" RE "           " C6 "|" RE "                                                                                                                                                                               " C6 "|" RE);
        printf(C6 "\r\n\t\t |" RE " " C3 "Key Enter" RE " " C6 "|" RE "                  " C3 "Mode" RE "                                                                                                                                                         " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "1:" RE "    " C6 "|" RE " " C6 "Display Flood logfile" RE "                                                                                                                                                              " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "2:" RE "    " C6 "|" RE " " C6 "Display Normal logfile" RE "                                                                                                                                                              " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "3:" RE "    " C6 "|" RE " " C6 "Change time counter" RE "                                                                                                                                                           " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "Z" RE "    " C6 "|" RE " " C6 "Exit" RE "                                                                                                                                                                          " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n ====+===========+===========================================================================================================================================================================================+" RE);
        printf(C6 "\r\n\t\t |" RE "  " C3 "SETTING" RE "  " C6 "|" RE " " C6 "--> Please choose Mode: " RE);
        while (1)
        {
            scanf("%c", &key);
            if (key == '1')
            {
                display_log_files(LOG_FLOOD_DIR);
                printf(C6 "\r\n ===============+============================================================================================================================================================================================+" RE);
                printf(C6 "\r\n    Press" RE " " C2 "Y" RE " " C6 "to continue... " RE);
                while (1)
                {
                    scanf("%c", &key1);
                    if (key1 == 'y' || key1 == 'Y')
                    {
                        break;
                    }
                    if (key1 != 'y' || key1 != 'Y')
                    {
                        // printf("\r    SETTING     | Return? (Y): ");
                    }
                }
                break;
            }
            else if (key == '2')
            {
                display_log_files(LOG_NORMAL_DIR);
                printf(C6 "\r\n ===============+============================================================================================================================================================================================+" RE);
                printf(C6 "\r\n    Press" RE " " C2 "Y" RE " " C6 "to continue... " RE);
                while (1)
                {
                    scanf("%c", &key1);
                    if (key1 == 'y' || key1 == 'Y')
                    {
                        break;
                    }
                    if (key1 != 'y' || key1 != 'Y')
                    {
                        // printf("\r    SETTING     | Return? (Y): ");
                    }
                }
                break;
            }
            else if (key == '3')
            {
                break;
            }
            else if (key == 'Z' || key == 'z')
            {
                break;
            }

            // if (key != '1' || key != '2' || key != '3')
            //{
            //   printf("\r\t\t |SETTING    | --> Please choose Mode: ");
            // }
        }
        if (key == '3')
        {
            update_threshold_time_counter();
            goto start;
        }
        if (key == '1' || key == '2')
        {
            goto start;
        }
    }
    void Mode_Condition_SDCard_Admin(int serial_port)
    {
    start:
        system("clear");
        display_logo1();
        char key, key2;
        char key3 = 'E';
        char enter = '\r';
        display_setting_admin();
        printf(C6 "\r\n ================+===========+===============================================================================================================================================================================+" RE);
        printf(C6 "\r\n     " C3 "DISPLAY" RE "     " C6 "|" RE "           " C6 "|" RE "                                                                                                                                                                               " C6 "|" RE);
        printf(C6 "\r\n\t\t |" RE " " C3 "Key Enter" RE " " C6 "|" RE "                  " C3 "Mode" RE "                                                                                                                                                         " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "1:" RE "    " C6 "|" RE " " C6 "Display Flood log file" RE "                                                                                                                                                              " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "2:" RE "    " C6 "|" RE " " C6 "Display Normal log file" RE "                                                                                                                                                              " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "3:" RE "    " C6 "|" RE " " C6 "Delete Flood log file" RE "                                                                                                                                                               " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "4:" RE "    " C6 "|" RE " " C6 "Delete Normal log file" RE "                                                                                                                                                               " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "5:" RE "    " C6 "|" RE " " C6 "Change log file saving mode" RE "                                                                                                                                                   " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "6:" RE "    " C6 "|" RE " " C6 "Change the threshold for saving log file" RE "                                                                                                                                      " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "7:" RE "    " C6 "|" RE " " C6 "Change time counter" RE "                                                                                                                                                           " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "8:" RE "    " C6 "|" RE " " C6 "Change duration time export" RE "                                                                                                                                                   " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t |" RE "     " C3 "Z" RE "    " C6 "|" RE " " C17 "Exit" RE "                                                                                                                                                                          " C6 "|" RE);
        printf(C6 "\r\n\t\t +-----------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n ================+===========================================================================================================================================================================================+\n" RE);
        printf(C6 "\r\n\t\t |" RE "" C3 "SETTING" RE "    " C6 "|" RE " " C6 "--> Please choose Mode: " RE);
        while (1)
        {
            scanf("%c", &key);
            if (key == '1')
            {
                display_log_files(LOG_FLOOD_DIR);
                printf(C6 "\r\n ================+===========================================================================================================================================================================================+\n" RE);
                printf("\r\n    " C3 "SETTING" RE "      " C6 "|" RE " " C6 "Press" RE " " C2 "Y" RE " " C6 "to return ! " RE);
                while (1)
                {
                    scanf("%c", &key2);
                    if (key2 == 'y' || key2 == 'Y')
                    {
                        system("clear");
                        display_logo1();
                        break;
                    }
                    if (key2 != 'y' || key2 != 'Y')
                    {
                        printf("\r    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Press " C2 "Y" RE " to return ! " RE);
                    }
                }
                goto start;
            }
            if (key == '2')
            {
                display_log_files(LOG_NORMAL_DIR);
                printf(C6 "\r\n ================+===========================================================================================================================================================================================+\n");
                printf("\r\n    " C3 "SETTING" RE "      " C6 "|" RE " " C6 "Press" RE " " C2 "Y" RE " " C6 "to return ! " RE);
                while (1)
                {
                    scanf("%c", &key2);
                    if (key2 == 'y' || key2 == 'Y')
                    {
                        system("clear");
                        display_logo1();
                        break;
                    }
                    if (key2 != 'y' || key2 != 'Y')
                    {
                        printf("\r\n    " C3 "SETTING" RE "      " C6 "|" RE " " C6 "Press" RE " " C2 "Y" RE " " C6 "to return ! " RE);
                    }
                }
                goto start;
            }
            if (key == '3')
            {
                display_log_files(LOG_FLOOD_DIR);
                delete_log_file(LOG_FLOOD_DIR);
                goto start;
            }
            if (key == '4')
            {
                display_log_files(LOG_NORMAL_DIR);
                delete_log_file(LOG_NORMAL_DIR);
                goto start;
            }
            if (key == '5')
            {
                update_mode_auto_manual();
                sleep(2);
                goto start;
            }
            if (key == '6')
            {
                update_threshold_SDCard();
                sleep(2);
                goto start;
            }
            if (key == '7')
            {
                update_threshold_time_counter();
                sleep(2);
                goto start;
            }
            if (key == '8')
            {
                SetDurationTime(serial_port);
                sleep(2);
                goto start;
            }
            if (key == 'Z' || key == 'z')
            {
                break;
            }
            if (key != '1' || key != '2' || key != '3' || key != '4' || key != '5' || key != '6' || key != '7' || key != '8' || key == 'Z' || key == 'z')
            {
                printf("\r\t\t " C6 "|" RE "" C3 "SETTING" RE "    " C6 "|" RE " " C6 "--> Please choose Mode: " RE);
            }
        }
    }
    void read_config_mode_save_logfile()
    {
        FILE *config_fp = fopen(AUTO_MANUAL_CONFIG_FILE, "r");
        if (config_fp == NULL)
        {
            config_fp = fopen(AUTO_MANUAL_CONFIG_FILE, "w");
            if (config_fp == NULL)
            {
                printf(C3 "Error creating config file: %s\n" RE "", AUTO_MANUAL_CONFIG_FILE);
                return;
            }
            fprintf(config_fp, "true");
            fclose(config_fp);
        }
        else
        {
            char value[10];
            fscanf(config_fp, "%s", value);
            fclose(config_fp);

            if (strcmp(value, "true") == 0)
            {
                auto_delete_logs = true;
            }
            else if (strcmp(value, "false") == 0)
            {
                auto_delete_logs = false;
            }
            else
            {
                printf(C3 "Invalid value in config file. Using default: true\n" RE);
            }
        }
    }

    void write_config_mode_save_logfile()
    {
        FILE *config_fp = fopen(AUTO_MANUAL_CONFIG_FILE, "w");
        if (config_fp == NULL)
        {
            printf(C3 "Error opening config file: %s\n" RE "", AUTO_MANUAL_CONFIG_FILE);
            return;
        }

        if (auto_delete_logs)
        {
            fprintf(config_fp, "true");
        }
        else
        {
            fprintf(config_fp, "false");
        }
        fclose(config_fp);
    }

    void update_mode_auto_manual()
    {
        char select;
        // bool new_mode;
        char save_choice;

        printf(C3 "\n\t\tSelect the log file saving mode (1/2) " RE);
        printf(C6 "\r\n ================+===========+===============================================================================================================================================================================+" RE);
        printf(C6 "\r\n\t\t+============================================================================================================================================================================================+" RE);
        printf(C6 "\r\n\t\t| |" RE " " C3 "1. Auto" RE " 							                                                                                                                             " C6 "|" RE);
        printf(C6 "\r\n\t\t| +--------------+-------------+-------------+--------------------+--------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t| |" RE " " C3 "2. Manual" RE " 		                                                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n\t\t| +--------------+-------------+-------------+--------------------+--------------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n ================+===========+===============================================================================================================================================================================+" RE);
        printf(C6 "\r\n\t\t|" RE "" C3 "--> Please choose Mode: " RE);
        while (1)
        {
            scanf("%c", &select);

            if (select == '1' || select == '2')
            {
                printf(C3 "\r\n\t\tDo you want to save? (" C2 "Y" RE "/" C1 "N" RE "): " RE);
                scanf(" %c", &save_choice);

                if (save_choice == 'y' || save_choice == 'Y')
                {
                    if (select == '1')
                    {
                        auto_delete_logs = true;
                    }
                    else if (select == '2')
                    {
                        auto_delete_logs = false;
                    }

                    printf(C2 "\r\n\t\tUpdated logging mode successfully" RE);
                    write_config_mode_save_logfile();
                    break;
                }
                else

                    printf(C3 "\r\n\t\tUpdate logging mode failed\n" RE);
                break;
            }

            else
            {
                printf(C3 "\r\t\t| --> Please choose Mode: " RE);
            }
        }
    }
    void display_setting_user()
    {
        struct statvfs stat1;
        if (statvfs("/", &stat1) != 0)
        {
            perror(C3 "statvfs error" RE);
            pthread_exit(NULL);
        }

        unsigned long total_space = (stat1.f_blocks * stat1.f_frsize);

        unsigned long used_space = (stat1.f_blocks - stat1.f_bfree) * stat1.f_frsize;
        float memory_usage = (float)used_space / total_space * 100;
        float CAPACITY = (float)total_space / (1024 * 1024 * 1024);
        float used_space_gb = (float)used_space / (1024 * 1024 * 1024);
        printf(C6 "\r\n ================+===========+===============================================================================================================================================================================+" RE);
        printf(C6 "\r\n\t\t |" RE " " C3 "CAPACITY: %.2f GB (100%)" RE "           " C6 "|" RE "  " C3 "Used space: %.2f GB (%.2f%%)" RE "        " C6 "|" RE " " C3 "Free space: %.2f GB (%.2f%%)" RE "             " C6 "|" RE "", CAPACITY, used_space_gb, 100 - memory_usage, CAPACITY - used_space_gb, memory_usage);
        printf(C6 "\r\n\t\t +-------------------------------------+--------------------------------------+--------------------------------------------------------------------------------------------------------------+");
        printf(C6 "\r\n\t\t |" RE " " C3 "Time counter: %d s" RE "                                                                                                                                                                         " C6 "|" RE "", Threshold_time_counter);
        printf(C6 "\r\n ================+===========+===============================================================================================================================================================================+" RE);
    }
    void display_setting_user1()
    {
        struct statvfs stat1;
        if (statvfs("/", &stat1) != 0)
        {
            perror(C3 "statvfs error" RE);
            pthread_exit(NULL);
        }

        unsigned long total_space = (stat1.f_blocks * stat1.f_frsize);

        unsigned long used_space = (stat1.f_blocks - stat1.f_bfree) * stat1.f_frsize;
        float memory_usage = (float)used_space / total_space * 100;
        float CAPACITY = (float)total_space / (1024 * 1024 * 1024);
        float used_space_gb = (float)used_space / (1024 * 1024 * 1024);

        printf(C6 "\r\n ================+===========+===============================================================================================================================================================================+" RE);
        printf(C6 "\r\n\t\t | |" RE " " C3 "CAPACITY: %.2f GB (100 %)" RE "         " C6 "|" RE "  " C3 "Used space: %.2f GB (%.2f%%)" RE "        " C6 "|" RE " " C3 "Free space: %.2f GB" RE " (%.2f%%)" RE "", CAPACITY, used_space_gb, 100 - memory_usage, CAPACITY - used_space_gb, memory_usage);
        printf(C6 "\r\n\t\t | +----------------------------------+-----------------------------------+------------------------------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n ================+===========+===============================================================================================================================================================================+" RE);
    }
    void display_setting_admin()
    {
        struct statvfs stat1;
        if (statvfs("/", &stat1) != 0)
        {
            perror(C3 "statvfs error" RE);
            pthread_exit(NULL);
        }
        char mode_save_logfile[7];

        unsigned long total_space = (stat1.f_blocks * stat1.f_frsize);

        unsigned long used_space = (stat1.f_blocks - stat1.f_bfree) * stat1.f_frsize;
        float memory_usage = (float)used_space / total_space * 100;
        float CAPACITY = (float)total_space / (1024 * 1024 * 1024);
        float used_space_gb = (float)used_space / (1024 * 1024 * 1024);

        if (auto_delete_logs == true)
        {
            strcpy(mode_save_logfile, "Auto");
        }
        else
        {
            strcpy(mode_save_logfile, "Manual");
        }
        printf(C6 "\r\n ================+===========+===============================================================================================================================================================================+" RE);
        printf(C6 "\r\n\t\t | |" RE " " C3 "CAPACITY: %.2f GB (100%)" RE "         " C6 "|" RE "  " C3 "Used space: %.2f GB (%.2f%%)" RE "        " C6 "|" RE " " C3 "Free space: %.2f GB (%.2f%%)" RE "                                                                                " C6 "|" RE "", CAPACITY, used_space_gb, 100 - memory_usage, CAPACITY - used_space_gb, memory_usage);
        printf(C6 "\r\n\t\t | +--------------+-------------+-------------+-------------+---------------------+--------------+-------------------------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t | |" RE " " C3 "Current log file saving threshold: %.2f %" RE "                                                                                                                                             " C6 "|" RE "", Threshold_SD);
        printf(C6 "\r\n\t\t | +--------------+-------------+-------------+-------------+---------------------+--------------+------------------+------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t | +--------------+-------------+-------------+-------------+---------------------+--------------+------------------+------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t | |" RE " " C3 "Current log file saving mode: %s" RE "                                                                                                                                                      " C6 "|" RE "", mode_save_logfile);
        printf(C6 "\r\n\t\t | +--------------+-------------+-------------+-------------+---------------------+--------------+------------------+------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t | +--------------+-------------+-------------+-------------+---------------------+--------------+------------------+------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n\t\t | |" RE " " C3 "Time counter: %d s" RE "                                                                                                                                                                      " C6 "|" RE "", Threshold_time_counter);
        printf(C6 "\r\n\t\t | +--------------+-------------+-------------+-------------+---------------------+--------------+------------------+------------------------------------------------------------------------+" RE);
        printf(C6 "\r\n ================+===========+===============================================================================================================================================================================+" RE);
    }
    // void display_log_files(const char *filename)
    // {
    //   char key;
    //   DIR *dir;
    //   struct dirent *entry;
    //   int file_count = 0;

    //   dir = opendir(filename);
    //   if (dir == NULL)
    //   {
    //     perror("opendir error");
    //     return;
    //   }

    //   printf("\r\n ================+===========================================================================================================================================================================================+");
    //   printf("\r\n\t\t | List of Log File                                                                                                     |");

    //   while ((entry = readdir(dir)) != NULL)
    //   {
    //     if (entry->d_type == DT_REG && strstr(entry->d_name, ".log") != NULL)
    //     {
    //       file_count++;
    //       printf("\r\n\t\t |%d. %s\n", file_count, entry->d_name);
    //       printf("\r\n -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------");
    //     }
    //   }
    //   closedir(dir);
    // }
    void display_log_files(const char *dir_path)
    {
        DIR *dir;
        struct dirent *entry;
        int file_count = 0;

        dir = opendir(dir_path);
        if (dir == NULL)
        {
            perror(C3 "opendir error" RE);
            return;
        }

        printf(C6 "\r\n ================+===========================================================================================================================================================================================+" RE);
        printf(C6 "\r\n\t\t |" RE " " C3 "List of Log Files" RE "                                                                                                    " C6 "|" RE);
        printf(C6 "\r\n -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" RE);

        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type == DT_REG && strstr(entry->d_name, ".log") != NULL)
            {
                file_count++;

                if (file_count % 2 == 1)
                {
                    printf(C3 "\r\n\t\t | %d. %-50s" RE "", file_count, entry->d_name);
                }
                else
                {
                    printf(C6 " |" RE " %d. %-50s |" RE "", file_count, entry->d_name);
                    printf(C6 "\r\n -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" RE);
                }
            }
        }

        if (file_count % 2 == 1)
        {
            printf(C6 " |\n -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" RE);
        }

        closedir(dir);
    }

    void delete_log_file(const char *dir_path)
    {

        char key;
        int file_index;
        char file_name[256] = "";
        char file_path[256];
        printf(C3 "\r\n\t\tEnter the number: " RE);
        scanf("%d", &file_index);

        if (file_index < 1)
        {
            printf(C3 "\r\n\t\tThe number not valid! " RE);
            return;
        }

        DIR *dir = opendir(dir_path);
        if (dir == NULL)
        {
            perror(C3 "opendir error" RE);
            return;
        }

        struct dirent *entry;
        int count = 0;
        char newest_file[256] = "";
        struct stat file_stat;
        time_t newest_time = 0;

        // find the newest file in the directory
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type == DT_REG && strstr(entry->d_name, ".log") != NULL)
            {
                sprintf(file_path, "%s/%s", dir_path, entry->d_name);
                if (stat(file_path, &file_stat) == 0)
                {
                    if (file_stat.st_mtime > newest_time)
                    {
                        newest_time = file_stat.st_mtime;
                        strcpy(newest_file, entry->d_name);
                    }
                }
            }
        }

        rewinddir(dir); // Reset the directory stream to read it again

        //  find the file corresponding to the user input
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type == DT_REG && strstr(entry->d_name, ".log") != NULL)
            {
                count++;
                if (count == file_index)
                {
                    strcpy(file_name, entry->d_name);
                    break;
                }
            }
        }

        closedir(dir);

        if (file_name[0] == '\0')
        {
            printf(C3 "\r\n\t\tFile not found! " RE);
            return;
        }

        // Check if the selected file is the newest one
        if (strcmp(file_name, newest_file) == 0)
        {

            printf("\n");
            printf("\r\n    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "File %s is currently being written, if you delete it a new file will automatically be created to replace it? (" C2 "Y" RE "/" C1 "N" RE "):" RE "", newest_file);
            while (1)
            {
                scanf("%c", &key);
                if (key == 'y' || key == 'Y')
                {
                    sprintf(file_path, "%s/%s", dir_path, file_name);
                    if (unlink(file_path) != 0)
                    {
                        perror(C3 "remove error" RE);
                        return;
                    }
                    printf(C2 "\r\n\t\tFile deleted successfully!!! %s\n" RE "", file_name);

                    create_new_log_file();
                    sleep(2);
                    break;
                }
                else if (key == 'N' || key == 'n')
                {
                    break;
                }
                if (key != 'y' || key != 'Y' || key != 'n' || key != 'N')
                {
                    printf("\r    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "File %s is currently being written, if you delete it a new file will automatically be created to replace it? (" C2 "Y" RE "/" C1 "N" RE "):" RE "", newest_file);
                }
            }
        }
        else
        {
            sprintf(file_path, "%s/%s", dir_path, file_name);
            if (unlink(file_path) != 0)
            {
                perror(C3 "remove error" RE);
                return;
            }
            printf(C2 "\r\n\t\tFile deleted successfully!!! %s\n" RE "", file_name);
            sleep(2);
        }
    }
    //=========================================================
    // attack
    // void open_attacker_log_file()
    // {
    //   // Create the directory if it doesn't exist
    //   struct stat st = {0};
    //   if (stat(ATTACKER_LOG_DIR, &st) == -1)
    //   {
    //     if (mkdir(ATTACKER_LOG_DIR, 0777) != 0)
    //     { // Create directory with permissions 0777 (read, write, execute for all)
    //       if (errno != EEXIST)
    //       {                        // Ignore error if directory already exists
    //         perror("mkdir error"); // Handle the error (e.g., exit or continue without logging)
    //         return;                // Or handle the error in a way that makes sense for your application
    //       }
    //     }
    //   }
    //   attacker_log_file = fopen(ATTACKER_LOG_FILE, "a");
    //   if (attacker_log_file == NULL)
    //   {
    //     perror("fopen attacker log file error"); // Handle error appropriately
    //   }
    // }
    // void close_attacker_log_file()
    // {
    //   if (attacker_log_file != NULL)
    //   {
    //     fclose(attacker_log_file);
    //     attacker_log_file = NULL;
    //   }
    // }
    // void log_attacker_ip(const char *src_ip)
    // {
    //   pthread_mutex_lock(&log_mutex); // Use the same mutex as the main log file

    //   if (attacker_log_file != NULL)
    //   {
    //     fprintf(attacker_log_file, "%s\n", src_ip);
    //     fflush(attacker_log_file); // Flush immediately for real-time logging
    //   }
    //   pthread_mutex_unlock(&log_mutex);
    // }
    //==========================================================
    void read_threshold_timecounter_from_file()
    {
        FILE *file = fopen(time_counter, "r");
        if (file == NULL)
        {

            file = fopen(time_counter, "w");
            if (file == NULL)
            {
                printf(C3 "Error creating config file: %s\n" RE "", time_counter);
                exit(1);
            }
            fprintf(file, "5"); //  ?t gi  tr? m?c d?nh l  true
            fclose(file);
        }
        else
        {
            if (fscanf(file, "%d", &Threshold_time_counter) != 1)
            {
                printf(C3 "\r\n\t\tCannot threshold from file\n" RE);
                fclose(file);
                exit(1);
            }

            fclose(file);
        }
    }

    //
    void write_threshold_time_counter_to_file()
    {
        FILE *file = fopen(time_counter, "w");
        if (file == NULL)
        {
            printf(C3 "\r\n\t\tCannot open file %s\n" RE "", time_counter);
            exit(1);
        }

        fprintf(file, "%d\n", Threshold_time_counter);
        fclose(file);
    }

    //
    void update_threshold_time_counter()
    {
        int new_threshold;
        char save_choice;

        do
        {
            printf(C3 "\r\n\t\t Enter the new time counter:  " RE);
            scanf("%d", &new_threshold);

            if (new_threshold >= 0)
            {

                printf(C3 "\r\n\t\t Do you want to save? (" C2 "Y" RE "/" C1 "N" RE "): " RE);
                scanf(" %c", &save_choice);

                if (save_choice == 'y' || save_choice == 'Y')
                {

                    Threshold_time_counter = new_threshold;

                    printf(C3 "\r\n\t\t Updated time counter:  %d\n" RE "", new_threshold);

                    write_threshold_time_counter_to_file();
                }
                else
                {

                    printf(C3 "\r\n\t\t Update time counter failed\n" RE);
                }
            }
            else
            {

                printf(C3 "Invalid value! Please re-enter.\n" RE);
            }
        } while (new_threshold < 0);
    }

    void read_threshold_from_file()
    {
        FILE *file = fopen(threshold_logfile, "r");
        if (file == NULL)
        {
            // T?o file n?u file chua t?n t?i
            file = fopen(threshold_logfile, "w");
            if (file == NULL)
            {
                printf(C3 "Error creating config file: %s\n" RE "", threshold_logfile);
                exit(1);
            }
            fprintf(file, "" C3 "80" RE "");
            fclose(file);
        }
        else
        {
            if (fscanf(file, "%f", &Threshold_SD) != 1)
            {
                printf(C3 "\r\n\t\tCannot open file \n" RE);
                fclose(file);
                exit(1);
            }

            fclose(file);
        }
    }

    //
    void write_threshold_to_file()
    {
        FILE *file = fopen(threshold_logfile, "w");
        if (file == NULL)
        {
            printf(C3 "Cannot open file %s\n" RE "", threshold_logfile);
            exit(1);
        }
        fprintf(file, "" C3 "%f\n" RE "", Threshold_SD);

        fclose(file);
    }

    //
    void update_threshold_SDCard()
    {
        float new_threshold;
        char save_choice;

        do
        {
            printf(C3 "\r\n\t\t Enter the new threshold value (0 -> 100): " RE);
            scanf("%f", &new_threshold);
            if (new_threshold >= 0 && new_threshold <= 100)
            {
                printf(C3 "\r\n\t\t Do you want to save? (" C2 "Y" RE "/" C1 "N" RE "): " RE);
                scanf(" %c", &save_choice);

                if (save_choice == 'y' || save_choice == 'Y')
                {

                    Threshold_SD = new_threshold;
                    printf(C3 "\r\n\t\t Threshold updated:  %f\n" RE "", new_threshold);
                    write_threshold_to_file();
                }
                else
                {
                    // Ngu?i d ng kh ng mu?n luu
                    printf(C3 "\r\n\t\t Update logging mode failed\n" RE);
                }
            }
            else
            {
                // N?u nh?p gi  tr? kh ng h?p l?, y u c?u nh?p l?i
                printf(C3 "Invalid value! Please re-enter.\n" RE);
            }
        } while (new_threshold < 0 || new_threshold > 100);
    }
    //**************************************************//
    void create_new_log_file()
    {
        // char new_log_file[32];
        char new_log_file_flood[64];
        char new_log_file_normal[64];
        char current_date[11];
        get_current_date(current_date);
        time_t now = time(NULL);
        struct tm *timeinfo = localtime(&now);
        char time_str[9];
        strftime(time_str, sizeof(time_str), "%H-%M-%S", timeinfo);
        sprintf(new_log_file_flood, "%s/%s_%s.log", LOG_FLOOD_DIR, current_date, time_str);
        sprintf(new_log_file_normal, "%s/%s_%s.log", LOG_NORMAL_DIR, current_date, time_str);

        strcpy(name_logfile_flood, new_log_file_flood);
        strcpy(name_logfile_normal, new_log_file_normal);

        if (current_log_file_flood != NULL)
        {
            fclose(current_log_file_flood);
        }
        if (current_log_file_normal != NULL)
        {
            fclose(current_log_file_normal);
        }

        current_log_file_flood = fopen(new_log_file_flood, "a");
        current_log_file_normal = fopen(new_log_file_normal, "a");

        if (current_log_file_flood == NULL || current_log_file_normal == NULL)
        {
            perror(C3 "fopen error" RE);

            return;
        }
        // open_attacker_log_file();
    }

    void process_packet(unsigned char *buffer, int size)
    {
        struct ethhdr *eth = (struct ethhdr *)buffer;
        int header_size = sizeof(struct ethhdr);
        int payload_size = size - header_size;
        const unsigned char *payload = buffer + header_size;

        if ((memcmp(eth->h_dest, target_mac_attack, 6) == 0))
        {
            unsigned char extracted_ID[2];
            unsigned char extracted_src_ip[16];
            unsigned char extracted_dst_ip[16];
            unsigned char extracted_src_port[2];
            unsigned char extracted_dst_port[2];
            unsigned char extracted_protocol[1];
            unsigned char extracted_time[4];
            unsigned char extracted_bw[4];
            unsigned char extracted_PKT_counter[4];
            unsigned char extracted_type[1];
            unsigned char extracted_check_header[1];
            unsigned char extracted_port_n[1];

            memcpy(extracted_ID, payload + 2, 2);
            memcpy(extracted_src_ip, payload + 6, 16);
            memcpy(extracted_dst_ip, payload + 22, 16);
            memcpy(extracted_src_port, payload + 38, 2);
            memcpy(extracted_dst_port, payload + 40, 2);
            memcpy(extracted_protocol, payload + 42, 1);
            memcpy(extracted_time, payload + 43, 4);
            memcpy(extracted_bw, payload + 47, 4);
            memcpy(extracted_PKT_counter, payload + 51, 4);
            memcpy(extracted_type, payload + 55, 1);
            memcpy(extracted_port_n, payload + 56, 1);
            memcpy(extracted_check_header, payload + 5, 1);

            unsigned short id_value = (extracted_ID[0] << 8) | extracted_ID[1];
            // //printf("id_value: %u\n", id_value);
            unsigned char protocol = extracted_protocol[0];
            //
            time_t rawtime = (time_t)((extracted_time[0] << 24) | (extracted_time[1] << 16) | (extracted_time[2] << 8) | (extracted_time[3]));
            struct tm *timeinfo = gmtime(&rawtime);
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
            //
            unsigned int bw = ntohl(*((unsigned int *)extracted_bw));
            unsigned int pkt_counter = ntohl(*((unsigned int *)extracted_PKT_counter));
            unsigned char type = extracted_type[0];
            unsigned char port_n = extracted_port_n[0];
            unsigned int src_port = ntohs(*((unsigned short *)extracted_src_port));
            unsigned int dst_port = ntohs(*((unsigned short *)extracted_dst_port));

            char src_ip[42];
            char dst_ip[42];
            if ((unsigned char)extracted_check_header[0] == 0x42)
            {
                unsigned int src_ip_int = ntohl(*((unsigned int *)extracted_src_ip));
                unsigned int dst_ip_int = ntohl(*((unsigned int *)extracted_dst_ip));

                sprintf(src_ip, "%d.%d.%d.%d", extracted_src_ip[12], extracted_src_ip[13], extracted_src_ip[14], extracted_src_ip[15]);
                sprintf(dst_ip, "%d.%d.%d.%d", extracted_dst_ip[12], extracted_dst_ip[13], extracted_dst_ip[14], extracted_dst_ip[15]);
            }
            else if ((unsigned char)extracted_check_header[0] == 0x62)
            {

                sprintf(src_ip, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", extracted_src_ip[0], extracted_src_ip[1], extracted_src_ip[2], extracted_src_ip[3],
                        extracted_src_ip[4], extracted_src_ip[5], extracted_src_ip[6], extracted_src_ip[7], extracted_src_ip[8], extracted_src_ip[9], extracted_src_ip[10], extracted_src_ip[11],
                        extracted_src_ip[12], extracted_src_ip[13], extracted_src_ip[14], extracted_src_ip[15]);

                sprintf(dst_ip, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", extracted_dst_ip[0], extracted_dst_ip[1], extracted_dst_ip[2], extracted_dst_ip[3],
                        extracted_dst_ip[4], extracted_dst_ip[5], extracted_dst_ip[6], extracted_dst_ip[7], extracted_dst_ip[8], extracted_dst_ip[9], extracted_dst_ip[10], extracted_dst_ip[11],
                        extracted_dst_ip[12], extracted_dst_ip[13], extracted_dst_ip[14], extracted_dst_ip[15]);
            }
            char type_str[32];
            char protocol_str[32];
            // char *type_str = "Normal";

            //
            switch (type)
            {
            case 1:
                strcpy(type_str, "SYN Flood");
                break;
            case 2:
                strcpy(type_str, "LAND Attack");
                break;
            case 3:
                strcpy(type_str, "UDP Flood");
                break;
            case 4:
                strcpy(type_str, "DNS Flood");
                break;
            case 5:
                strcpy(type_str, "IPSec IKE Flood");
                break;
            case 6:
                strcpy(type_str, "ICMP Flood");
                break;
            case 7:
                strcpy(type_str, "TCP Fragment");
                break;
            case 8:
                strcpy(type_str, "UDP Fragment");
                break;
            case 9:
                strcpy(type_str, "DNS AUTH PKT");
                break;
            case 10:
                strcpy(type_str, "HTTP Flood");
                break;
            default:
                strcpy(type_str, "Unknown");
                break;
            }
            // port
            //  port
            char name_port_str[32];
            //
            if (port_n == 4)
            {
                return;
            }
            switch (port_n)
            {
            case 1:
                strcpy(name_port_str, "1");
                break;
            case 2:
                strcpy(name_port_str, "2");
                break;
            case 4:
                strcpy(name_port_str, "0");
                break;
            case 8:
                strcpy(name_port_str, "3");
                break;
            case 16:
                strcpy(name_port_str, "4");
                break;
            case 32:
                strcpy(name_port_str, "6");
                break;
            case 64:
                strcpy(name_port_str, "7");
                break;
            case 128:
                strcpy(name_port_str, "8");
                break;
            default:
                strcpy(name_port_str, "0");
                break;
            }

            //
            switch (protocol)
            {
            case 6:
                strcpy(protocol_str, "TCP");
                break;
            case 17:
                strcpy(protocol_str, "UDP");
                break;
            case 1:
                strcpy(protocol_str, "ICMP");
                break;
            case 58:
                strcpy(protocol_str, "ICMP");
                break;
            default:
                strcpy(protocol_str, "Unknown");
                break;
            }

            char binary[10];
            for (int i = 0; i < 10; i++)
            {
                binary[i] = (id_value & (1 << (9 - i))) ? '1' : '0';
            }
            binary[11] = '\0';

            int result_str_size = 1;
            char *result_str = NULL;
            char *mapping[] = {" HTTP Flood ", " UDP Fragmentation attack ", " TCP Fragmentation attack ", " IPSec IKE Flood ", " ICMP Flood ", " DNS Flood ", " UDP Flood ", " LAND Attack ", " SYN Flood "};

            for (int i = 0; i < 8; i++)
            {
                if (binary[i] == '1')
                {
                    result_str_size += strlen(mapping[i]) + 3;
                }
            }
            result_str = (char *)malloc(result_str_size + 32);
            if (result_str == NULL)
            {
                perror(C3 "Failed to allocate memory" RE);
                return;
            }
            result_str[0] = '\0';
            int types_count = 0;
            for (int i = 0; i < 9; i++)
            {
                if (binary[i] == '1')
                {
                    if (types_count > 0)
                    {
                        strcat(result_str, "+");
                    }

                    strcat(result_str, mapping[i]);
                    types_count++;
                }
            }
            char lcd_message[result_str_size + 32];
            snprintf(lcd_message, result_str_size + 32, "%s", result_str);
            // printf("\n ss: %s", lcd_message);
            free(result_str);

            //
            pthread_mutex_lock(&lcd_queue.mutex);
            if (strcmp(lcd_message, current_attack) != 0)
            {
                strcpy(current_attack, lcd_message);
                //  pthread_mutex_lock(&lcd_queue.mutex);
                lcd_queue.front = lcd_queue.rear = 0;
                // pthread_mutex_unlock(&lcd_queue.mutex);
            }

            snprintf(lcd_queue.messages[lcd_queue.rear], result_str_size + 32, "%s", lcd_message);
            lcd_queue.rear = (lcd_queue.rear + 1) % QUEUE_SIZE;
            pthread_cond_signal(&lcd_queue.cond);
            pthread_mutex_unlock(&lcd_queue.mutex);

            if ((memcmp(extracted_time, prev_time, 4) == 0))
            {
                bw_accumulated += bw;
            }
            else
            {

                sprintf(bw1, "%d", bw_accumulated);

                bw_accumulated = bw;
                memcpy(prev_time, extracted_time, 4);
            }
            // Log to file

            // Send socket

            pthread_mutex_lock(&log_mutex);
            if (current_log_file_flood != NULL)
            {

                fprintf(current_log_file_flood, "%s  %s  %s  %u  %u  %s  %s  %u  %u\n",
                        time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, type_str, bw, pkt_counter);
                fflush(current_log_file_flood);
            }
            pthread_mutex_unlock(&log_mutex);
            // log_attacker_ip(src_ip);

            static int packet_count = 0;
            if (packet_count % SAMPLING_RATE == 0)
            {

                if ((unsigned char)extracted_check_header[0] == 0x42)
                {
                    // snprintf(print_buffer + print_buffer_pos, PRINT_BUFFER_SIZE - print_buffer_pos,
                    //          " \n|  %s  |  \t\t%s\t\t    |  \t\t%s\t\t       |  %6u\t|  %6u\t|  %6s\t|  %17s    |  %8u     |  %8u     |",
                    //          time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, type_str, bw, pkt_counter);

                    snprintf(print_buffer + print_buffer_pos, PRINT_BUFFER_SIZE - print_buffer_pos,
                            " \n" C6 "|" RE "  %s  " C6 "|" RE "  \t\t%s\t\t    " C6 "|" RE "  \t\t%s\t\t       " C6 "|" RE "  %6u\t" C6 "|" RE "  %6u\t" C6 "|" RE "  %6s\t" C6 "|" RE "  %s%17s%s    " C6 "|" RE "  %8u     " C6 "|" RE "  %10u     " C6 "|" RE "",
                            time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, KRED, type_str, RESET, bw, pkt_counter);
                }
                else if ((unsigned char)extracted_check_header[0] == 0x62)
                {
                    snprintf(print_buffer + print_buffer_pos, PRINT_BUFFER_SIZE - print_buffer_pos,
                            " \n" C6 "|" RE "  %s  " C6 "|" RE "  \t\t%s\t\t    " C6 "|" RE "  \t\t%s\t\t       " C6 "|" RE "  %6u\t" C6 "|" RE "  %6u\t" C6 "|" RE "  %6s\t" C6 "|" RE "  %s%17s%s    " C6 "|" RE "  %8u     " C6 "|" RE "  %10u     " C6 "|" RE "",
                            time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, KRED, type_str, RESET, bw, pkt_counter);
                }
                print_buffer_pos += strlen(print_buffer + print_buffer_pos);
            }
            packet_count++;

            if (strcmp(type_str, "HTTP Flood") == 0)
            {
                if (strlen(src_ip) > 20)
                {
                    process_ip(LOGFILE_HTTP_IPv6, src_ip);
                }
                else
                {
                    process_ip(LOGFILE_HTTP_IPv4, src_ip);
                }
            }
            // count++;
            count_tancong++;
        }

        else if ((memcmp(eth->h_dest, target_mac, 6) == 0))
        {

            static const unsigned char zero_array[16] = {0};

            if (memcmp(payload + 6, zero_array, 16) == 0 || memcmp(payload + 22, zero_array, 16) == 0)
            {
                return;
            }
            unsigned char extracted_ID[2];
            unsigned char extracted_src_ip[16];
            unsigned char extracted_dst_ip[16];
            unsigned char extracted_src_port[2];
            unsigned char extracted_dst_port[2];
            unsigned char extracted_protocol[1];
            unsigned char extracted_time[4];
            unsigned char extracted_bw[4];
            unsigned char extracted_PKT_counter[4];
            unsigned char extracted_type[1];
            unsigned char extracted_check_header[1];
            memcpy(extracted_ID, payload + 2, 2);
            memcpy(extracted_src_ip, payload + 6, 16);
            memcpy(extracted_dst_ip, payload + 22, 16);

            memcpy(extracted_src_port, payload + 38, 2);
            memcpy(extracted_dst_port, payload + 40, 2);
            memcpy(extracted_protocol, payload + 42, 1);
            memcpy(extracted_time, payload + 43, 4);
            memcpy(extracted_bw, payload + 47, 4);
            memcpy(extracted_PKT_counter, payload + 51, 4);
            memcpy(extracted_type, payload + 55, 1);
            memcpy(extracted_check_header, payload + 5, 1);

            unsigned char id_value = extracted_ID[1];
            unsigned char protocol = extracted_protocol[0];
            //
            time_t rawtime = (time_t)((extracted_time[0] << 24) | (extracted_time[1] << 16) | (extracted_time[2] << 8) | (extracted_time[3]));
            struct tm *timeinfo = gmtime(&rawtime);
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
            //
            unsigned int bw = ntohl(*((unsigned int *)extracted_bw));
            unsigned int pkt_counter = ntohl(*((unsigned int *)extracted_PKT_counter));
            unsigned char type = extracted_type[0];
            unsigned int src_port = ntohs(*((unsigned short *)extracted_src_port));
            unsigned int dst_port = ntohs(*((unsigned short *)extracted_dst_port));

            char src_ip[42];
            char dst_ip[42];

            if ((unsigned char)extracted_check_header[0] == 0x42)
            {
                unsigned int src_ip_int = ntohl(*((unsigned int *)extracted_src_ip));
                unsigned int dst_ip_int = ntohl(*((unsigned int *)extracted_dst_ip));

                sprintf(src_ip, "%d.%d.%d.%d", extracted_src_ip[12], extracted_src_ip[13], extracted_src_ip[14], extracted_src_ip[15]);
                sprintf(dst_ip, "%d.%d.%d.%d", extracted_dst_ip[12], extracted_dst_ip[13], extracted_dst_ip[14], extracted_dst_ip[15]);
            }
            else if ((unsigned char)extracted_check_header[0] == 0x62)
            {

                sprintf(src_ip, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", extracted_src_ip[0], extracted_src_ip[1], extracted_src_ip[2], extracted_src_ip[3],
                        extracted_src_ip[4], extracted_src_ip[5], extracted_src_ip[6], extracted_src_ip[7], extracted_src_ip[8], extracted_src_ip[9], extracted_src_ip[10], extracted_src_ip[11],
                        extracted_src_ip[12], extracted_src_ip[13], extracted_src_ip[14], extracted_src_ip[15]);
                sprintf(dst_ip, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", extracted_dst_ip[0], extracted_dst_ip[1], extracted_dst_ip[2], extracted_dst_ip[3],
                        extracted_dst_ip[4], extracted_dst_ip[5], extracted_dst_ip[6], extracted_dst_ip[7], extracted_dst_ip[8], extracted_dst_ip[9], extracted_dst_ip[10], extracted_dst_ip[11],
                        extracted_dst_ip[12], extracted_dst_ip[13], extracted_dst_ip[14], extracted_dst_ip[15]);
            }
            char type_str[32];
            char protocol_str[32];

            //
            switch (type)
            {
            case 0:
                strcpy(type_str, "Normal");
                break;

            default:
                strcpy(type_str, "Normal");
                break;
            }
            //
            switch (protocol)
            {
            case 6:
                strcpy(protocol_str, "TCP");
                break;
            case 17:
                strcpy(protocol_str, "UDP");
                break;
            case 1:
                strcpy(protocol_str, "ICMP");
                break;
            case 58:
                strcpy(protocol_str, "ICMP");
                break;
            default:
                strcpy(protocol_str, "Unknown");
                break;
            }

            pthread_mutex_lock(&log_mutex);
            if (current_log_file_normal != NULL)
            {

                fprintf(current_log_file_normal, "" C3 "%s" RE "  " C6 "%s" RE "  " C2 "%s" RE "  " C6 "%u" RE "  " C2 "%u" RE "  " C3 "%s" RE "  " C3 "%s" RE "  " C3 "%u" RE "  " C3 "%u" RE "\n",
                        time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, type_str, bw, pkt_counter);
                fflush(current_log_file_normal);
            }
            pthread_mutex_unlock(&log_mutex);

            // In ra terminal n?u d p ?ng di?u ki?n sampling
            static int packet_count = 0;
            if (packet_count % SAMPLING_RATE == 0)
            {
                // printf(" \n\n|\tTime\t\t\t|\tSource IP\t\t|\tDest IP\t\t|\tSource Port\t|\tDest Port\t|\tProtocol\t|\tType\t|\tBW\t|\tPKT\t|");
                // Th m th ng tin v o b? d?m in ra terminal

                // snprintf(print_buffer + print_buffer_pos, PRINT_BUFFER_SIZE - print_buffer_pos,
                //          " \n|\t%s\t|\t%s\t|\t%s\t|\t%u\t|\t%u\t|\t%s\t|\t%s\t\t|\t%u\t|\t%u\t|",
                //          time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, type_str, bw, pkt_counter);

                if ((unsigned char)extracted_check_header[0] == 0x42)
                {
                    // snprintf(print_buffer + print_buffer_pos, PRINT_BUFFER_SIZE - print_buffer_pos,
                    //          " \n|  %s  |  \t\t%s\t\t    |  \t\t%s\t\t       |  %6u\t|  %6u\t|  %6s\t|  %17s    |  %8u     |  %8u     |",
                    //          time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, type_str, bw, pkt_counter);
                    snprintf(print_buffer + print_buffer_pos, PRINT_BUFFER_SIZE - print_buffer_pos,
                            " \n" C6 "|" RE "  " C3 "%s" RE "  " C6 "|" RE "  " C6 "\t\t%s\t\t" RE "    " C6 "|" RE "  " C2 "\t\t%s\t\t" RE "       " C6 "|" RE "  " C6 "%6u\t" RE "" C6 "|" RE "  " C2 "%6u\t" RE "" C6 "|" RE "  " C3 "%6s\t" RE "" C6 "|" RE "  " C17 "%17s" RE "    " C6 "|" RE "  " C3 "%8u" RE "     " C6 "|" RE "  " C3 "%10u" RE "     " C6 "|" RE "",
                            time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, type_str, bw, pkt_counter);
                }
                else if ((unsigned char)extracted_check_header[0] == 0x62)
                {
                    // snprintf(print_buffer + print_buffer_pos, PRINT_BUFFER_SIZE - print_buffer_pos,
                    //          " \n|  %s  |  %s  | %s  |  %6u\t|  %6u\t|  %6s\t|  %17s    |  %8u     |  %8u     |",
                    //          time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, type_str, bw, pkt_counter);
                    snprintf(print_buffer + print_buffer_pos, PRINT_BUFFER_SIZE - print_buffer_pos,
                            " \n" C6 "|" RE "  " C3 "%s" RE "  " C6 "|" RE "  " C6 "%s" RE "  " C6 "|" RE " " C2 "%s" RE "  " C6 "|" RE "  " C6 "%6u\t" RE "" C6 "|" RE "  " C2 "%6u\t" C6 "|" RE "  " C3 "%6s\t" RE "" C6 "|" RE "  " C17 "%17s" RE "    " C6 "|" RE "  " C3 "%8u" RE "     " C6 "|" RE "  " C3 "%10u" RE "     " C6 "|" RE "",
                            time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, type_str, bw, pkt_counter);
                }

                print_buffer_pos += strlen(print_buffer + print_buffer_pos);
            }
            packet_count++;
            // count++;
        }
    }

    void enqueue_packet(unsigned char *packet, int size)
    {
        pthread_mutex_lock(&packet_queue.mutex);
        memcpy(packet_queue.packets[packet_queue.rear], packet, size);
        packet_queue.rear = (packet_queue.rear + 1) % PACKET_QUEUE_SIZE;
        pthread_cond_signal(&packet_queue.cond);
        pthread_mutex_unlock(&packet_queue.mutex);
    }

    //
    void *packet_queue_processing_thread(void *arg)
    {
        while (1)
        {
            pthread_mutex_lock(&packet_queue.mutex);
            while (packet_queue.front == packet_queue.rear)
            {
                pthread_cond_wait(&packet_queue.cond, &packet_queue.mutex);
            }
            unsigned char *packet = packet_queue.packets[packet_queue.front];
            int size = BUFFER_SIZE; // Assuming all packets have the same size
            packet_queue.front = (packet_queue.front + 1) % PACKET_QUEUE_SIZE;
            pthread_mutex_unlock(&packet_queue.mutex);

            // X? l  packet
            process_packet(packet, size);
        }
        return NULL;
    }

    void *lcd_thread_function(void *arg)
    {
        while (1)
        {

            if (!is_idle2)
            {
                pthread_mutex_lock(&lcd_queue.mutex);
                // snprintf(lcd_queue.messages[lcd_queue.rear], 255, "ACRONICS SOLUTIONS");
                lcd_queue.rear = (lcd_queue.rear + 1) % QUEUE_SIZE;
                pthread_cond_signal(&lcd_queue.cond);
                pthread_mutex_unlock(&lcd_queue.mutex);
            }

            pthread_mutex_lock(&lcd_queue.mutex);

            while (lcd_queue.front == lcd_queue.rear)
            {
                pthread_cond_wait(&lcd_queue.cond, &lcd_queue.mutex);
            }

            char message[255];
            snprintf(message, 255, "%s", lcd_queue.messages[lcd_queue.front]);
            lcd_queue.front = (lcd_queue.front + 1) % QUEUE_SIZE;
            pthread_mutex_unlock(&lcd_queue.mutex);
            pthread_mutex_lock(&lcd_mutex);
            // //printf("\n aa: %s", message);
            if (show_disconnected_message)
            {
                ClrLcd();
                lcdLoc(LINE1);
                typeString("  DISCONNECTED");
            }
            else
            {
                if (is_idle)
                {
                    pthread_mutex_lock(&lcd_queue.mutex);
                    lcd_queue.front = lcd_queue.rear = 0;
                    pthread_mutex_unlock(&lcd_queue.mutex);
                    scroll_text1("ACRONICS SOLUTIONS", "ACS Sysnet-Def v1.0", 150);
                }
                else
                {
                    char bw1_with_space[20];
                    snprintf(bw1_with_space, sizeof(bw1_with_space), "  %s", bw1);
                    scroll_text1(message, bw1_with_space, 100);
                }
            }
            pthread_mutex_unlock(&lcd_mutex);

            // memory_check_thread_function();
            usleep(200000);
        }

        return NULL;
    }

    void scroll_text(const char *text, int delay_ms)
    {
        int len = strlen(text);
        char buffer[255] = {0};
        while (1)
        {
            for (int pos = 0; pos <= len; pos++)
            {
                ClrLcd();
                for (int i = 0; i < 16; i++)
                {
                    int text_index = i + pos;
                    if (text_index >= 0 && text_index < len)
                    {
                        buffer[i] = text[text_index];
                    }
                    else
                    {
                        buffer[i] = ' ';
                    }
                }
                lcdLoc(LINE1);
                typeString(buffer);
                delay(delay_ms);
                ClrLcd();
            }
            if (stop_scrolling)
            {
                break;
            }
        }
    }

    void update_lcd(const char *message)
    {
        ClrLcd();
        lcdLoc(LINE1);
        // lcdLoc(LINE2);
        typeString(message);
    }

    void get_current_date(char *date_str)
    {
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(date_str, 11, "%Y-%m-%d", timeinfo);
    }

    void scroll_text1(const char *text1, const char *text2, int delay_ms)
    {
        int len1 = strlen(text1);
        int len2 = strlen(text2);
        char buffer1[255] = {0};
        char buffer2[255] = {0};

        while (1)
        {
            for (int pos = 0; pos <= len1; pos++)
            {
                ClrLcd();
                for (int i = 0; i < 16; i++)
                {
                    int text_index = i + pos;
                    if (text_index >= 0 && text_index < len1)
                    {
                        buffer1[i] = text1[text_index];
                    }
                    else
                    {
                        buffer1[i] = ' ';
                    }
                }
                lcdLoc(LINE1);
                typeString(buffer1);

                //
                for (int i = 0; i < 16; i++)
                {
                    int text_index = i + pos;
                    if (text_index >= 0 && text_index < len2)
                    {
                        buffer2[i] = text2[text_index];
                    }
                    else
                    {
                        buffer2[i] = ' ';
                    }
                }
                lcdLoc(LINE2);
                typeString(buffer2);

                delay(delay_ms);
                ClrLcd();
            }
            if (stop_scrolling)
            {
                break;
            }
        }
    }

    // void remove_old_logs(void)
    // {
    //   DIR *dir;
    //   struct dirent *entry;
    //   time_t now = time(NULL);
    //   char file_path[512];

    //   dir = opendir(LOG_FLOOD_DIR);
    //   if (dir == NULL)
    //   {
    //     perror("opendir error");
    //     return;
    //   }

    //   while ((entry = readdir(dir)) != NULL)
    //   {
    //     if (entry->d_type == DT_REG)
    //     {
    //       char *file_name = entry->d_name;

    //       if (strstr(file_name, ".log") != NULL)
    //       {
    //         char file_date[11];
    //         strncpy(file_date, file_name, 10);
    //         file_date[10] = '\0';

    //         struct tm tm = {0};
    //         if (sscanf(file_date, "%4d-%2d-%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday) == 3)
    //         {
    //           tm.tm_year -= 1900;
    //           tm.tm_mon -= 1;
    //           time_t file_time = mktime(&tm);
    //           double diff_days = difftime(now, file_time) / (60 * 60 * 24);

    //           if (diff_days > MAX_LOG_DAYS)
    //           {
    //             snprintf(file_path, sizeof(file_path), "%s/%s", LOG_FLOOD_DIR, file_name);
    //             if (remove(file_path) != 0)
    //             {
    //               perror("remove error");
    //             }
    //           }
    //         }
    //       }
    //     }
    //   }
    //   closedir(dir);
    // }
    int open_and_check_dir(const char *dir_path)
    {
        DIR *dir;
        struct dirent *entry;
        int is_empty = 1;

        dir = opendir(dir_path);
        if (dir == NULL)
        {
            perror(C3 "opendir error" RE);
            return -1;
        }
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_name[0] == '.' && (entry->d_name[1] == '\0' || (entry->d_name[1] == '.' && entry->d_name[2] == '\0')))
            {
                continue;
            }
            is_empty = 0;
            break;
        }
        closedir(dir);
        return is_empty;
    }
    //

    void *memory_check_thread_function(void *arg)
    {
        struct statvfs stat1;
        while (1)
        {
            check_connect_eth();
            if (statvfs("/", &stat1) != 0)
            {
                perror(C3 "statvfs error" RE);
                pthread_exit(NULL);
            }

            unsigned long total_space = stat1.f_blocks * stat1.f_frsize;
            unsigned long used_space = (stat1.f_blocks - stat1.f_bfree) * stat1.f_frsize;
            float memory_usage = (float)used_space / total_space * 100;

            // IF USER >= MEMORY
            if (memory_usage >= Threshold_SD)
            {

                int check_empty_dir = open_and_check_dir(LOG_NORMAL_DIR);
                if (check_empty_dir == 1)
                {
                    empty_log_normal = true;
                }
                else if (check_empty_dir == 0)
                {
                    empty_log_normal = false;
                }

                if (auto_delete_logs)
                {

                    pthread_mutex_lock(&log_mutex);
                    if (!empty_log_normal)
                    {
                        DIR *dir = opendir(LOG_NORMAL_DIR);
                        struct dirent *entry;
                        time_t oldest_time = time(NULL);
                        char oldest_file[256] = {0};
                        int file_count = 0;
                        while ((entry = readdir(dir)) != NULL)
                        {
                            if (entry->d_type == DT_REG)
                            {
                                file_count++;
                                char file_path[256];
                                sprintf(file_path, "" C6 "%s/%s" RE "", LOG_NORMAL_DIR, entry->d_name);
                                struct stat file_stat;
                                stat(file_path, &file_stat);
                                if (file_stat.st_mtime < oldest_time)
                                {
                                    oldest_time = file_stat.st_mtime;
                                    strcpy(oldest_file, file_path);
                                }
                            }
                        }
                        closedir(dir);

                        if (file_count > 0 && oldest_file[0] != '\0')
                        {
                            // printf("Deleted oldest file: %s\n", oldest_file);
                            unlink(oldest_file);
                        }
                        else if (file_count == 0)
                        {
                            empty_log_normal = true;
                        }
                    }

                    else
                    {
                        DIR *dir = opendir(LOG_FLOOD_DIR);
                        struct dirent *entry;
                        time_t oldest_time = time(NULL);
                        char oldest_file[256] = {0};
                        int file_count = 0;
                        while ((entry = readdir(dir)) != NULL)
                        {
                            if (entry->d_type == DT_REG)
                            {
                                file_count++;
                                char file_path[256];
                                sprintf(file_path, "" C6 "%s/%s" RE "", LOG_FLOOD_DIR, entry->d_name);
                                struct stat file_stat;
                                stat(file_path, &file_stat);
                                if (file_stat.st_mtime < oldest_time)
                                {
                                    oldest_time = file_stat.st_mtime;
                                    strcpy(oldest_file, file_path);
                                }
                            }
                        }
                        closedir(dir);

                        if (file_count > 1 && oldest_file[0] != '\0')
                        {
                            // printf("Deleted oldest file: %s\n", oldest_file);
                            unlink(oldest_file);
                        }
                        else if (file_count == 1)
                        {

                            if (current_log_file_flood != NULL)
                            {
                                fclose(current_log_file_flood);
                                current_log_file_flood = NULL;
                                close_flood_log = true;
                            }

                            full_sd = true;
                        }
                    }
                    pthread_mutex_unlock(&log_mutex);
                }
                else
                {

                    if (current_log_file_flood != NULL)
                    {
                        fclose(current_log_file_flood);
                        current_log_file_flood = NULL;
                    }
                    if (current_log_file_normal != NULL)
                    {
                        fclose(current_log_file_normal);
                        current_log_file_normal = NULL;
                    }
                    close_flood_log = true;
                    close_normal_log = true;
                    full_sd = true;
                }
                // stop_writing = true;
            }

            // IF USE < MEMORY
            if (memory_usage < Threshold_SD)
            {
                if (empty_log_normal)
                {
                    char new_log_file1[32];
                    char current_date1[11];
                    get_current_date(current_date1);
                    time_t now = time(NULL);
                    struct tm *timeinfo = localtime(&now);
                    char time_str[9];
                    strftime(time_str, sizeof(time_str), "%H-%M-%S", timeinfo);
                    sprintf(new_log_file1, "" C6 "%s/%s_%s.log" RE "", LOG_NORMAL_DIR, current_date1, time_str);
                    strcpy(name_logfile_normal, new_log_file1);
                    if (current_log_file_normal != NULL)
                    {
                        fclose(current_log_file_normal);
                    }
                    current_log_file_normal = fopen(new_log_file1, "a");
                    // if (current_log_file_normal == NULL)
                    // {
                    //   perror("fopen error");
                    //   return;
                    // }
                    empty_log_normal = false;
                    close_normal_log = false;
                }

                //     full_sd2 = false;
                if (close_flood_log)
                {
                    current_log_file_flood = fopen(name_logfile_flood, "a");
                    if (current_log_file_flood == NULL || current_log_file_normal == NULL)
                    {
                        perror(C3 "Error opening log file" RE);
                        pthread_exit(NULL);
                    }
                    close_flood_log = false;
                }
                if (close_normal_log)
                {
                    current_log_file_normal = fopen(name_logfile_normal, "a");
                    if (current_log_file_flood == NULL || current_log_file_normal == NULL)
                    {
                        perror(C3 "Error opening log file" RE);
                        pthread_exit(NULL);
                    }
                    close_normal_log = false;
                }
                full_sd = false;
                // stop_writing = false;
            }
            sleep(2);
        }
        pthread_exit(NULL);
    }

    // void *log_buffer_thread(void *arg)
    // {
    //   while (1)
    //   {

    //     if (log_buffer_pos >= 0)
    //     { // printf("\nbye\n");
    //       fwrite(log_buffer, 1, log_buffer_pos, current_log_file);
    //       log_buffer_pos = 0; // Reset buffer
    //     }
    //     // pthread_mutex_unlock(&log_mutex);

    //     sleep(1);
    //   }
    //   return NULL;
    // }

    void handle_signal(int sig)
    {
        if (sig == SIGTSTP) // Ctrl+Z
        {
            printf(C3 "\nRestarting...\n" RE);
            sleep(2);
            printf(C3 "\nRestarted!\n" RE);
            send_reset(serial_port);
            sleep(1);
            exit(1);
        }
        else if (sig == SIGINT) // Ctrl+C
        {
            printf(C3 "\n[INFO] Caught Ctrl+C, cleaning up...\n" RE);

            if (hash_table)
            {
                g_hash_table_destroy(hash_table);
                hash_table = NULL;
            }
            if (queue)
            {
                g_queue_free(queue);
                queue = NULL;
            }

            exit(0);
        }
    }

    void send_reset(int serial_port)
    {
        char key = 03;
        char enter = '\r';
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        write(serial_port, &enter, sizeof(enter));
        usleep(1000000);
    }

    void get_custom_datetime(char *date_str)
    {
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(date_str, 17, "%y%m%d%H%M%S", timeinfo);
    }

    void send_time(int serial_port)
    {
        char enter = '\r';
        char date_str[17];
        get_custom_datetime(date_str);

        int n = strlen(date_str);
        for (int i = 0; i < n; i++)
        {
            char data = date_str[i];
            send_data(serial_port, &data, sizeof(data));
            // printf("data :%c\n",data);
            usleep(500000);
        }
        write(serial_port, &enter, sizeof(enter));
    }

    void check_connect_eth()
    {
        int sockfd;
        struct ifreq ifr;

        // while (1)
        // {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd == -1)
        {
            perror(C3 "socket" RE);
            pthread_exit(NULL);
        }
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
        {
            perror(C3 "ioctl" RE);
            close(sockfd);
            pthread_exit(NULL);
        }
        bool connected = (ifr.ifr_flags & IFF_RUNNING) != 0;
        pthread_mutex_lock(&lcd_mutex);
        if (connected && show_disconnected_message)
        {

            show_disconnected_message = false;
        }
        else if (!connected)
        {
            show_disconnected_message = true;
        }
        pthread_mutex_unlock(&lcd_mutex);

        close(sockfd);
    }

    //
    void previous_mode_fc()
    {
        FILE *file = fopen(previous_mode, "w");
        if (file == NULL)
        {
            printf(C3 "Cannot open file %s\n" RE "", previous_mode);
            exit(1);
        }
        fprintf(file, "%c\n", '2');

        fclose(file);
    }

    // Creat file log save HTTP ip table
    void create_http_filelog(const char *filename)
    {

        struct stat buffer;
        if (stat(filename, &buffer) != 0)
        {
            FILE *file = fopen(filename, "w");
            if (file == NULL)
            {
                perror(C3 "Err create file http log" RE);
                exit(EXIT_FAILURE);
            }
            fclose(file);
        }
        else
        {
        }
    }

    // Load file log to hash table
    void load_ips_from_file(const char *filename)
    {
        FILE *file = fopen(filename, "r");
        if (!file)
            return;

        char ip[MAX_IP_LEN];
        while (fgets(ip, sizeof(ip), file))
        {
            ip[strcspn(ip, "\n")] = '\0';
            g_hash_table_add(ip_table, g_strdup(ip));
        }
        fclose(file);
    }

    // Add ip to Filelog
    void flush_batch_to_file(const char *filename)
    {
        if (g_queue_is_empty(batch_queue))
            return;

        FILE *file = fopen(filename, "a");
        if (!file)
        {
            perror(C3 "Open file error" RE);
            return;
        }

        while (!g_queue_is_empty(batch_queue))
        {
            char *ip = g_queue_pop_head(batch_queue);
            fprintf(file, "" C3 "%s\n" RE "", ip);
            g_free(ip);
        }

        fclose(file);
    }

    // Check ip http
    void process_ip(const char *filename, const char *ip)
    {
        if (g_hash_table_size(ip_table) > MAX_IPS)
        {
            // printf(" ?t gi?i h?n %d IP, kh ng luu th m.\n", MAX_IPS);
            return;
        }
        // Check trung ip
        if (g_hash_table_contains(ip_table, ip))
        {
            return;
        }

        // add ip hash
        g_hash_table_add(ip_table, g_strdup(ip));
        g_queue_push_tail(batch_queue, g_strdup(ip));

        if (g_queue_get_length(batch_queue) >= BATCH_SIZE)
        {
            flush_batch_to_file(filename);
        }
    }

    // Send time sync in Core
    void send_data_sync_time(int serial_port)
    {
        char keyphay = ',';
        char key_enter = '\r';

        while (1)
        {
            bool flag = false;
            int i = 0;
            write(serial_port, &keyphay, sizeof(keyphay));
            usleep(10000);
            write(serial_port, &key_enter, sizeof(key_enter));
            usleep(1000000);
            send_time(serial_port);
            while (1)
            {
                char *data2 = receive_data(serial_port);
                if ((strchr(data2, 'S') != NULL))
                {
                    flag = true;
                    break;
                }
                i++;
                if (i == 10)
                {
                    break;
                }
            }
            if (flag == true)
            {
                break;
            }
            sleep(3);
        }
    }

    // Send data http ip via core when start
    void send_http_ipv4_start(int serial_port, const char *filename)
    {
        FILE *file = fopen(filename, "r");
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);

        if (file_size == 0)
        {
            fclose(file);
            return;
        }

        char enter = '\r';
        char keycham = '.';

        write(serial_port, &keycham, sizeof(keycham));
        usleep(1000);
        write(serial_port, &enter, sizeof(enter));
        usleep(10000);
        send_ips_via_uart(LOGFILE_HTTP_IPv4);
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message: %s\n" RE "", data1);
            if ((strchr(data1, 'Y') != NULL))
            {
                printf(C3 "\nSend HTTP_TABLE_IPV4 done\n" RE);
                break;
            }
        }
    }

    // Send data http ip via core when start
    void send_http_ipv6_start(int serial_port, const char *filename)
    {
        FILE *file = fopen(filename, "r");
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);

        if (file_size == 0)
        {
            fclose(file);
            return;
        }

        char enter = '\r';
        char keynhaynguoc = '`';

        write(serial_port, &keynhaynguoc, sizeof(keynhaynguoc));
        usleep(1000);
        write(serial_port, &enter, sizeof(enter));
        usleep(10000);
        send_ips_via_uart(LOGFILE_HTTP_IPv6);
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message2: %s\n" RE "", data1);
            if ((strchr(data1, 'Y') != NULL))
            {
                printf(C3 "\nSend HTTP_TABLE_IPV6 done\n" RE);
                break;
            }
        }
    }
    // Process data in file, and add to buffer
    void uart_send(const char *data, int serial_port)
    {
        char enter = '\r';
        int n = strlen(data);
        for (int i = 0; i < n; i++)
        {
            char data1 = data[i];
            send_data(serial_port, &data1, sizeof(data1));
            usleep(100);
        }
        write(serial_port, &enter, sizeof(enter));
        printf(C3 "Sending via UART: %s\n" RE "", data);
    }

    // Send data http via uart
    void send_ips_via_uart(const char *filename)
    {
        char buffer[BUFFER_SIZE_SEND_IP_VIA_UART] = "";
        char ip[MAX_IP_LEN];

        FILE *file = fopen(filename, "r");
        if (!file)
        {
            perror(C3 "Open file error" RE);
            return;
        }
        while (fgets(ip, sizeof(ip), file))
        {
            ip[strcspn(ip, "\n")] = '\0';
            if (strlen(buffer) + strlen(ip) + 2 > BUFFER_SIZE_SEND_IP_VIA_UART)
            {
                fprintf(stderr, "Full buffer\n");
                fclose(file);
                return;
            }
            strcat(buffer, ip);
            strcat(buffer, "$");
        }
        fclose(file);
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '$')
        {
            buffer[len - 1] = '\0';
        }

        uart_send(buffer, serial_port);
    }

    // run function
    void *run(void *arg)
    {
        wiringPiSetup();
        lcd_init(LCD_ADDR);
        ClrLcd();
        pthread_mutex_t mutex1;

        pthread_mutex_init(&mutex1, NULL);
        pthread_mutex_init(&log_mutex, NULL);
        pthread_mutex_init(&lcd_mutex, NULL);
        pthread_mutex_init(&packet_queue.mutex, NULL);
        pthread_cond_init(&packet_queue.cond, NULL);
        packet_queue.front = packet_queue.rear = 0;
        lcd_queue.front = lcd_queue.rear = 0;
        pthread_mutex_init(&lcd_queue.mutex, NULL);
        pthread_cond_init(&lcd_queue.cond, NULL);

        // Create threads
        pthread_t lcd_thread;
        pthread_create(&lcd_thread, NULL, lcd_thread_function, NULL);

        pthread_t memory_check_thread;
        pthread_create(&memory_check_thread, NULL, memory_check_thread_function, NULL);

        pthread_t packet_queue_processing_thread_id;
        pthread_create(&packet_queue_processing_thread_id, NULL, packet_queue_processing_thread, NULL);

        // Create thread for log buffer
        // pthread_t log_buffer_thread_id;
        // pthread_create(&log_buffer_thread_id, NULL, log_buffer_thread, NULL);

        int sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (sock_raw < 0)
        {
            perror(C3 "Socket Error" RE);
            exit(1);
        }
        // Set socket to non-blocking
        int flags = fcntl(sock_raw, F_GETFL, 0);
        if (flags == -1)
        {
            perror(C3 "fcntl(F_GETFL)" RE);
            exit(1);
        }
        if (fcntl(sock_raw, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            perror(C3 "fcntl(F_SETFL)" RE);
            exit(1);
        }
        // Configure interface in promiscuous mode
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
        if (ioctl(sock_raw, SIOCGIFFLAGS, &ifr) == -1)
        {
            perror(C3 "ioctl error" RE);
            close(sock_raw);
            exit(1);
        }
        ifr.ifr_flags |= IFF_PROMISC;
        if (ioctl(sock_raw, SIOCSIFFLAGS, &ifr) == -1)
        {
            perror(C3 "ioctl error" RE);
            close(sock_raw);
            exit(1);
        }
        unsigned char *buffer = (unsigned char *)malloc(BUFFER_SIZE);
        if (buffer == NULL)
        {
            perror(C3 "Failed to allocate memory" RE);
            exit(1);
        }
        struct sockaddr saddr;
        int saddr_len = sizeof(saddr);

        create_new_log_file();
        while (1)
        {
            int data_size = recvfrom(sock_raw, buffer, BUFFER_SIZE, 0, &saddr, (socklen_t *)&saddr_len);
            if (data_size > 0)
            {
                stop_scrolling = 1;
                struct ethhdr *eth = (struct ethhdr *)buffer;
                if ((memcmp(eth->h_dest, target_mac_attack, 6) == 0))
                {
                    detect_attack = true;
                    is_idle2 = true;
                    last_packet_time = time(NULL);
                    enqueue_packet(buffer, data_size);
                    count_tong++;
                }
                else if ((memcmp(eth->h_dest, target_mac, 6) == 0))
                {
                    enqueue_packet(buffer, data_size);
                }
                else
                {
                    is_idle2 = false;
                }
            }
            else if (data_size == -1 && errno != EAGAIN)
            {
                perror(C3 "Recvfrom Error" RE);
                exit(1);
            }
            time_t current_time = time(NULL);
            if (current_time - last_packet_time > 2)
            {
                detect_attack = false;
                stop_scrolling = 1;
                is_idle = true;
            }
            else
            {
                is_idle = false;
            }
        }

        // Clean up resources
        close(sock_raw);
        free(buffer);
        // free(buffer_size);
        pthread_mutex_destroy(&log_mutex);
        pthread_mutex_destroy(&lcd_queue.mutex);
        pthread_cond_destroy(&lcd_queue.cond);
    }

    void Display_table_2(int port)
    {
        sqlite3 *db;
        sqlite3_stmt *stmt;
        int rc;

        char target_iface[8];
        sprintf(target_iface, "eth%d", port);

        rc = sqlite3_open(DB_PATH, &db);
        if (rc)
        {
            printf(C3 "Can't open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        const char *sql = "SELECT rowid, DefenseProfileUsingTime FROM DefenseProfiles WHERE DefenseProfileUsingTime IS NOT NULL";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            printf(C3 "Failed to prepare: %s\n" RE "", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        typedef struct
        {
            char iface[8];
            char start_time[20];
            int rowid;
        } ProfileInfo;

        ProfileInfo latest = {"", "", -1};
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int rowid = sqlite3_column_int(stmt, 0);
            const char *json_text = (const char *)sqlite3_column_text(stmt, 1);

            cJSON *root = cJSON_Parse(json_text);
            if (!root || !cJSON_IsArray(root))
            {
                if (root)
                    cJSON_Delete(root);
                continue;
            }

            int arr_size = cJSON_GetArraySize(root);
            for (int index = 0; index < arr_size; ++index)
            {
                cJSON *entry = cJSON_GetArrayItem(root, index);
                if (!entry)
                    continue;

                cJSON *iface_obj = cJSON_GetObjectItem(entry, "name");
                cJSON *start_obj = cJSON_GetObjectItem(entry, "date");
                const char *iface = iface_obj && cJSON_IsString(iface_obj) ? iface_obj->valuestring : NULL;
                const char *start = start_obj && cJSON_IsString(start_obj) ? start_obj->valuestring : NULL;

                if (iface && start && strcmp(iface, target_iface) == 0)
                {
                    if (strlen(latest.start_time) == 0 || strcmp(start, latest.start_time) > 0)
                    {
                        strcpy(latest.iface, iface);
                        strcpy(latest.start_time, start);
                        latest.rowid = rowid;
                    }
                }
            }
            cJSON_Delete(root);
        }

        sqlite3_finalize(stmt);

        if (latest.rowid != -1)
        {
            char query[128];
            sprintf(query, "SELECT * FROM DefenseProfiles WHERE rowid = %d", latest.rowid);
            rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
            if (rc == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW)
            {
                // printf("\nInterface: %s (start: %s)\n", latest.iface, latest.start_time);
                // printf("ID: %d, UserId: %d\n", sqlite3_column_int(stmt, 0), sqlite3_column_int(stmt, 1));
                printf(C3 "\n profile_name: %s\n" RE "", sqlite3_column_text(stmt, 2));

                time_t now = time(NULL);
                struct tm *timeinfo = localtime(&now);
                printf(C6 "\n ============================================================================================================================================================================================================+\n" RE);
                printf(C6 " ------------------------------------------------------------------------------------ " C3 "System Configuration" RE " " C6 "--------------------------------------------------------------------------------------------------+\n" RE);
                printf(C6 " ---------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
                printf(C6 "     " C3 "DISPLAY" RE "    " C6 "|" RE " " C3 "Company" RE "                : " C3 "Acronics Solutions" RE "                                                                                                                                                |\n" RE);
                printf(C6 "\t\t|" RE " " C3 "Device                 :" RE " " C6 "DDoS Defender" RE "                                                                                                                                                     " C6 "|\n" RE);
                printf(C6 "\t\t|" RE " " C3 "Model                  :" RE " " C6 "Bandwidth 1Gbps" RE "                                                                                                                                                   " C6 "|\n" RE);
                printf(C6 "\t\t|" RE " " C3 "Version                :" RE " " C6 "1.0" RE "                                                                                                                                                               " C6 "|\n" RE);
                printf(C6 "\t\t|" RE " " C3 "Current date and time  :" RE " " C6 " %02dh.%02dm.%02ds  %02d.%02d.%04d" RE "                                                                                                                                          " C6 "|\n" RE "",
                    timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
                    timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
                int DetectionTime = sqlite3_column_int(stmt, 9);
                int EnableDefender = sqlite3_column_int(stmt, 10);
                int ICMPFloodEnable = sqlite3_column_int(stmt, 11);
                int ICMPFloodThreshold = sqlite3_column_int(stmt, 12);
                int ICMPFloodRate = sqlite3_column_int(stmt, 13);
                int SYNFloodEnable = sqlite3_column_int(stmt, 14);
                int SYNFloodSYNThreshold = sqlite3_column_int(stmt, 15);
                int SYNFloodACKThreshold = sqlite3_column_int(stmt, 16);
                int SYNFloodWhiteListTimeOut = sqlite3_column_int(stmt, 17);
                int UDPFloodEnable = sqlite3_column_int(stmt, 18);
                int UDPFloodThreshold = sqlite3_column_int(stmt, 19);
                int UDPFloodRate = sqlite3_column_int(stmt, 20);
                int DNSFloodEnable = sqlite3_column_int(stmt, 21);
                int DNSFloodThreshold = sqlite3_column_int(stmt, 22);
                int LandAttackEnable = sqlite3_column_int(stmt, 23);
                int IPSecIKEEnable = sqlite3_column_int(stmt, 24);
                int IPSecIKEThreshold = sqlite3_column_int(stmt, 25);
                int TCPFragmentEnable = sqlite3_column_int(stmt, 26);
                int UDPFragmentEnable = sqlite3_column_int(stmt, 27);
                int HTTPFloodEnable = sqlite3_column_int(stmt, 28);
                int HTTPSFloodEnable = sqlite3_column_int(stmt, 29);

                printf(C6 "\t\t| +==========================================================================================================================================================================================+\n" RE);
                printf(C6 "\t\t| |" RE " " C3 "DDoS Defender status           :" RE "   " C6 "%s." RE "                                                                                                                                                  " C6 "|\n" RE "", EnableDefender ? "ON" : "OFF");
                printf(C6 "\t\t| +==========================================================================================================================================================================================+\n" RE);
                printf(C6 "\t\t| |" RE " " C3 "SYN Flood" RE " " C6 "|" RE " " C3 "LAND ATTACK" RE " " C6 "|" RE "  " C3 "UDP Flood" RE "  " C6 "|" RE "  " C3 "DNS Amplification" RE "  " C6 "|" RE "  " C3 "ICMP Flood" RE "  " C6 "|" RE " " C3 "IPSec IKE Flood" RE "  | " C3 "TCP Fragmentation Flood" RE "  " C6 "|" RE " " C3 "UDP Fragmentation Flood" RE "  " C6 "|" RE "  " C3 "HTTP GET Flood" RE "  " C6 "|" RE " " C3 "HTTPS GET Flood" RE " " C6 "|\n" RE);
                printf(C6 "\t\t| +------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
                printf(C6 "\t\t| | %8s  |   %8s  |   %8s  |      %8s       |  %8s    |   %8s       |         %8s         |         %8s         |     %8s     |     %8s    |\n" RE "",
                    SYNFloodEnable ? "Enable" : "Disable",
                    LandAttackEnable ? "Enable" : "Disable",
                    UDPFloodEnable ? "Enable" : "Disable",
                    DNSFloodEnable ? "Enable" : "Disable",
                    ICMPFloodEnable ? "Enable" : "Disable",
                    IPSecIKEEnable ? "Enable" : "Disable",
                    TCPFragmentEnable ? "Enable" : "Disable",
                    UDPFragmentEnable ? "Enable" : "Disable",
                    HTTPFloodEnable ? "Enable" : "Disable",
                    HTTPSFloodEnable ? "Enable" : "Disable");
                printf(C6 "\t\t| +------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
                printf(C6 "\t\t| +==============================================================+==============+\n" RE);
                printf(C6 "\t\t| |                    " C3 "Configured Parameters" RE "                    " C6 "|" RE "     " C3 "Valid" RE "     " C6 "|\n" RE);
                printf(C6 "\t\t| +==============================================================+==============+\n");
                printf(C6 "\t\t| | " C6 "Attack detection time" RE " " C3 "(seconds):" RE "                             " C6 " |  %-10d |\n" RE "", DetectionTime);
                printf(C6 "\t\t| +==============================================================+==============+\n");
                printf(C6 "\t\t| | " C6 "SYN Flood attack detection throughput" RE " " C3 "(PPS):" RE "                  " C6 "|  %-10d |\n" RE "", SYNFloodSYNThreshold);
                printf(C6 "\t\t| +--------------------------------------------------------------+--------------+\n");
                printf(C6 "\t\t| | " C6 "ACK Flood attack detection throughput" RE " " C3 "(PPS):" RE "                  " C6 "|  %-10d |\n" RE "", SYNFloodACKThreshold);
                printf(C6 "\t\t| +--------------------------------------------------------------+--------------+\n");
                printf(C6 "\t\t| | " C6 "Delete information white list time" RE " " C3 "(seconds):" RE "                 " C6 "|  %-10d |\n" RE "", SYNFloodWhiteListTimeOut);
                printf(C6 "\t\t| +==============================================================+==============+\n");
                printf(C6 "\t\t| | " C6 "UDP Flood attack detection throughput" RE " " C3 "(PPS):" RE "                  " C6 "|  %-10d |\n" RE "", UDPFloodThreshold);
                printf(C6 "\t\t| +--------------------------------------------------------------+--------------+\n");
                printf(C6 "\t\t| | " C6 "Valid UDP packet throughput allowed" RE " " C3 "(PPS):" RE "                    " C6 "|  %-10d |\n" RE "", UDPFloodRate);
                printf(C6 "\t\t| +==============================================================+==============+\n");
                printf(C6 "\t\t| | " C6 "DNS Amplification attack detection throughput" RE " " C3 "(PPS):" RE "          " C6 "|  %-10d |\n" RE "", DNSFloodThreshold);
                printf(C6 "\t\t| +==============================================================+==============+\n");
                printf(C6 "\t\t| | " C6 "ICMP Flood attack detection throughput" RE " " C3 "(PPS):" RE "                 " C6 "|  %-10d |\n" RE "", ICMPFloodThreshold);
                printf(C6 "\t\t| +--------------------------------------------------------------+--------------+\n");
                printf(C6 "\t\t| | " C6 "Valid ICMP packet throughput allowed" RE " " C3 "(PPS):" RE "                   " C6 "|  %-10d |\n" RE "", ICMPFloodRate);
                printf(C6 "\t\t| +==============================================================+==============+\n");
                printf(C6 "\t\t| | " C6 "IPSec Flood attack detection throughput" RE " " C3 "(PPS):" RE "                " C6 "|  %-10d |\n" RE "", IPSecIKEThreshold);
                printf(C6 "\t\t| +==============================================================+==============+\n" RE);
                printf(C6 "----------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n" RE);
            }
            else
            {
                printf(C3 "error profile rowid = %d\n" RE "", latest.rowid);
            }
            sqlite3_finalize(stmt);
        }
        else
        {
            printf(C3 "errror interface %s\n" RE "", target_iface);
        }
        sqlite3_close(db);
    }
    ///////////////////////////////UPDATE///////////////////////////////////////
    void UpdateDefenseProfileField(int port, const char *field, int value)
    {
        sqlite3 *db;
        int rc = sqlite3_open(DB_PATH, &db);
        if (rc)
        {
            printf(C3 "Can't open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        // T m rowid m?i nh?t cho port n y
        char target_iface[8];
        sprintf(target_iface, "eth%d", port);

        char sql_find[256];
        sprintf(sql_find, "SELECT rowid, DefenseProfileUsingTime FROM DefenseProfiles WHERE DefenseProfileUsingTime IS NOT NULL");
        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, sql_find, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            printf(C3 "Failed to prepare: %s\n" RE "", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        int latest_rowid = -1;
        char latest_time[32] = "";
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int rowid = sqlite3_column_int(stmt, 0);
            const char *json_text = (const char *)sqlite3_column_text(stmt, 1);
            cJSON *root = cJSON_Parse(json_text);
            if (!root || !cJSON_IsArray(root))
            {
                if (root)
                    cJSON_Delete(root);
                continue;
            }
            int arr_size = cJSON_GetArraySize(root);
            for (int i = 0; i < arr_size; ++i)
            {
                cJSON *entry = cJSON_GetArrayItem(root, i);
                cJSON *iface_obj = cJSON_GetObjectItem(entry, "name");
                cJSON *start_obj = cJSON_GetObjectItem(entry, "date");
                const char *iface = iface_obj && cJSON_IsString(iface_obj) ? iface_obj->valuestring : NULL;
                const char *start = start_obj && cJSON_IsString(start_obj) ? start_obj->valuestring : NULL;
                if (iface && start && strcmp(iface, target_iface) == 0)
                {
                    if (strlen(latest_time) == 0 || strcmp(start, latest_time) > 0)
                    {
                        strcpy(latest_time, start);
                        latest_rowid = rowid;
                    }
                }
            }
            cJSON_Delete(root);
        }
        sqlite3_finalize(stmt);

        if (latest_rowid == -1)
        {
            printf(C3 "No profile found for %s\n" RE "", target_iface);
            sqlite3_close(db);
            return;
        }

        // Update tru?ng d? li?u
        char sql_update[256];
        sprintf(sql_update, "UPDATE DefenseProfiles SET %s = %d WHERE rowid = %d", field, value, latest_rowid);
        rc = sqlite3_exec(db, sql_update, 0, 0, NULL);
        if (rc != SQLITE_OK)
        {
            printf(C3 "Failed to update: %s\n" RE "", sqlite3_errmsg(db));
        }
        else
        {
            printf(C3 "Updated %s for %s to %d\n" RE "", field, target_iface, value);
        }
        sqlite3_close(db);
    }

    // Hàm parse_using_time: chuy?n d?i chu?i th?i gian t? DB thành d?ng d? d?c
    void parse_using_time(const char *input, char *output, size_t output_size)
    {
        if (input == NULL || strlen(input) == 0)
        {
            snprintf(output, output_size, "(NULL)");
            return;
        }

        // ? dây gi? s? input trong DB có d?ng "2025-09-05T12:34:56"
        // b?n có th? format l?i cho d?p, ví d? "2025-09-05 12:34:56"
        for (size_t i = 0; i < strlen(input) && i < output_size - 1; i++)
        {
            if (input[i] == 'T')
                output[i] = ' ';
            else
                output[i] = input[i];
        }
        output[strlen(input)] = '\0';
    }
    // // Hàm hi?n th? toàn b? profile
    // void display_profiles(sqlite3 *db)
    // {
    //     sqlite3_stmt *stmt;
    //     const char *sql = "SELECT DefenseProfileName, DefenseProfileUsingTime, DefenseProfileStatus, DefenseProfileType FROM DefenseProfiles";

    //     if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    //     {
    //         printf("Failed to fetch data: %s\n", sqlite3_errmsg(db));
    //         return;
    //     }

    //     printf(C6 "\n============================================================================================================================\n" RE);
    //     printf(C3 "  ID  | %-25s | %-40s | %-12s | %-15s\n" RE,
    //         "Profile Name", "Using Time", "Status", "Type");
    //     printf(C6 "----------------------------------------------------------------------------------------------------------------------------\n" RE);

    //     int id = 1;
    //     while (sqlite3_step(stmt) == SQLITE_ROW)
    //     {
    //         const unsigned char *name      = sqlite3_column_text(stmt, 0);
    //         const unsigned char *usingTime = sqlite3_column_text(stmt, 1);
    //         const unsigned char *status    = sqlite3_column_text(stmt, 2);
    //         const unsigned char *type      = sqlite3_column_text(stmt, 3);

    //         char using_time_fmt[256];
    //         parse_using_time((const char *)usingTime, using_time_fmt, sizeof(using_time_fmt));

    //         // Gi?i h?n d? dài t?ng tru?ng
    //         char name_display[26], using_time_display[41], status_display[13], type_display[16];
    //         snprintf(name_display, sizeof(name_display), "%.25s", name ? (const char *)name : "(NULL)");
    //         snprintf(using_time_display, sizeof(using_time_display), "%.40s", using_time_fmt);
    //         snprintf(status_display, sizeof(status_display), "%.12s", status ? (const char *)status : "(NULL)");
    //         snprintf(type_display, sizeof(type_display), "%.15s", type ? (const char *)type : "(NULL)");

    //         printf("  %-3d | %-25s | %-40s | %-12s | %-15s\n",
    //             id, name_display, using_time_display, status_display, type_display);
    //         id++;
    //     }

    //     sqlite3_finalize(stmt);
    //     printf(C6 "============================================================================================================================\n" RE);
    // }

    // Hàm hi?n th? toàn b? profile (in dúng DefenseProfileId)
    // Hàm hi?n th? toàn b? profile (in dúng DefenseProfileId)
    // void display_profiles(sqlite3 *db)
    // {
    //     sqlite3_stmt *stmt;
    //     const char *sql =
    //         "SELECT DefenseProfileId, DefenseProfileName, DefenseProfileUsingTime, DefenseProfileStatus, DefenseProfileType "
    //         "FROM DefenseProfiles";

    //     if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    //     {
    //         printf("Failed to fetch data: %s\n", sqlite3_errmsg(db));
    //         return;
    //     }

    //     printf(C6 "\n============================================================================================================================\n" RE);
    //     printf(C3 "  ID   | %-25s | %-35s | %-12s | %-12s\n" RE,
    //            "Profile Name", "Using Time", "Status", "Type");
    //     printf(C6 "----------------------------------------------------------------------------------------------------------------------------\n" RE);

    //     while (sqlite3_step(stmt) == SQLITE_ROW)
    //     {
    //         int profile_id = sqlite3_column_int(stmt, 0);
    //         const unsigned char *name = sqlite3_column_text(stmt, 1);
    //         const unsigned char *usingTime = sqlite3_column_text(stmt, 2);
    //         const unsigned char *status = sqlite3_column_text(stmt, 3);
    //         const unsigned char *type = sqlite3_column_text(stmt, 4);

    //         char using_time_fmt[128];
    //         parse_using_time((const char *)usingTime, using_time_fmt, sizeof(using_time_fmt));

    //         // Gi?i h?n chi?u dài c?t
    //         char name_display[26], using_time_display[41], status_display[13], type_display[16];
    //         snprintf(name_display, sizeof(name_display), "%.25s", name ? (const char *)name : "(NULL)");
    //         snprintf(using_time_display, sizeof(using_time_display), "%.40s", using_time_fmt);
    //         snprintf(status_display, sizeof(status_display), "%.12s", status ? (const char *)status : "(NULL)");
    //         snprintf(type_display, sizeof(type_display), "%.15s", type ? (const char *)type : "(NULL)");

    //         printf("  %-4d | %-25s | %-35s | %-12s | %-12s\n",
    //                profile_id,
    //                name_display,
    //                using_time_display,
    //                status_display,
    //                type_display);
    //     }

    //     sqlite3_finalize(stmt);
    //     printf(C6 "============================================================================================================================\n" RE);
    // }

    // // Hàm hi?n th? toàn b? profile (không c?t d? li?u)
    // void display_profiles(sqlite3 *db)
    // {
    //     sqlite3_stmt *stmt;
    //     const char *sql =
    //         "SELECT DefenseProfileId, DefenseProfileName, DefenseProfileUsingTime, DefenseProfileStatus, DefenseProfileType "
    //         "FROM DefenseProfiles";

    //     if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    //     {
    //         printf("Failed to fetch data: %s\n", sqlite3_errmsg(db));
    //         return;
    //     }

    //     printf(C6 "\n============================================================================================================================\n" RE);
    //     printf(C3 "  ID   | Profile Name              | Using Time                               | Status       | Type\n" RE);
    //     printf(C6 "----------------------------------------------------------------------------------------------------------------------------\n" RE);

    //     while (sqlite3_step(stmt) == SQLITE_ROW)
    //     {
    //         int profile_id = sqlite3_column_int(stmt, 0);
    //         const unsigned char *name      = sqlite3_column_text(stmt, 1);
    //         const unsigned char *usingTime = sqlite3_column_text(stmt, 2);
    //         const unsigned char *status    = sqlite3_column_text(stmt, 3);
    //         const unsigned char *type      = sqlite3_column_text(stmt, 4);

    //         // N?u NULL thì in (NULL)
    //         printf("  %-4d | %s | %s | %s | %s\n",
    //             profile_id,
    //             name      ? (const char *)name      : "[]",
    //             usingTime ? (const char *)usingTime : "[]",
    //             status    ? (const char *)status    : "[]",
    //             type      ? (const char *)type      : "[]");
    //     }

    //     sqlite3_finalize(stmt);
    //     printf(C6 "============================================================================================================================\n" RE);
    // }

    // In c?t Using Time, l?c ch? eth và date, h? tr? xu?ng dòng
    // In c?t Using Time, ch? hi?n th? eth và date, xu?ng dòng n?u nhi?u c?ng
    void print_using_time(const char *usingTime, int width)
    {
        if (!usingTime || strlen(usingTime) == 0 || strcmp(usingTime, "[]") == 0)
        {
            printf("%-*s", width, "[]");
            return;
        }

        const char *p = usingTime;
        int first = 1;
        char line[128];

        while (*p)
        {
            // Tìm "eth"
            const char *eth = strstr(p, "\"name\":\"eth");
            if (!eth)
                break;

            eth += 8; // b? "name":" d? con tr? vào s? c?ng
            const char *eth_end = strchr(eth, '"');
            if (!eth_end)
                break;

            // Tìm "date"
            const char *date = strstr(eth_end, "\"date\":\"");
            if (!date)
                break;
            date += 8; // b? "date":"

            const char *date_end = strchr(date, '"');
            if (!date_end)
                break;

            // L?y eth và date
            int len = snprintf(line, sizeof(line), "%.*s %.*s",
                            (int)(eth_end - eth), eth,
                            (int)(date_end - date), date);

            if (first)
            {
                printf("%-*s", width, line);
                first = 0;
            }
            else
            {
                // Xu?ng dòng, can c?t Profile Name b?ng kho?ng tr?ng
                printf("\n       | %-25s | %-*s", "", width, line);
            }

            p = date_end + 1;
        }
    }

    void display_profiles(sqlite3 *db)
    {
        sqlite3_stmt *stmt;
        const char *sql =
            "SELECT DefenseProfileId, DefenseProfileName, DefenseProfileUsingTime, DefenseProfileStatus, DefenseProfileType "
            "FROM DefenseProfiles";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        {
            printf("Failed to fetch data: %s\n", sqlite3_errmsg(db));
            return;
        }

        printf(C6 "\n============================================================================================================================\n" RE);
        printf(C3 "  ID   | %-25s | %-40s | %-12s | %-12s\n" RE,
            "Profile Name", "Using Time", "Status", "Type");
        printf(C6 "----------------------------------------------------------------------------------------------------------------------------\n" RE);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int profile_id = sqlite3_column_int(stmt, 0);
            const unsigned char *name = sqlite3_column_text(stmt, 1);
            const unsigned char *usingTime = sqlite3_column_text(stmt, 2);
            const unsigned char *status = sqlite3_column_text(stmt, 3);
            const unsigned char *type = sqlite3_column_text(stmt, 4);

            const char *s_name = name ? (const char *)name : "[]";
            const char *s_usingTime = usingTime ? (const char *)usingTime : "[]";
            const char *s_status = status ? (const char *)status : "[]";
            const char *s_type = type ? (const char *)type : "[]";

            // L?y danh sách eth
            const char *p = s_usingTime;
            char lines[16][128];
            int n_lines = 0;

            while (p && *p)
            {
                const char *eth = strstr(p, "\"name\":\"eth");
                if (!eth)
                    break;
                eth += 8;
                const char *eth_end = strchr(eth, '"');
                if (!eth_end)
                    break;

                const char *date = strstr(eth_end, "\"date\":\"");
                if (!date)
                    break;
                date += 8;
                const char *date_end = strchr(date, '"');
                if (!date_end)
                    break;

                snprintf(lines[n_lines], sizeof(lines[n_lines]), "%.*s %.*s",
                        (int)(eth_end - eth), eth,
                        (int)(date_end - date), date);
                n_lines++;

                p = date_end + 1;
            }

            if (n_lines == 0) // không có eth
            {
                printf("  %-4d | %-25s | %-40s | %-12s | %-12s\n",
                    profile_id, s_name, "[]", s_status, s_type);
            }
            else
            {
                for (int i = 0; i < n_lines; i++)
                {
                    if (i == 0)
                        printf("  %-4d | %-25s | %-40s | %-12s | %-12s\n",
                            profile_id, s_name, lines[i], s_status, s_type);
                    else
                        printf("       | %-25s | %-40s | %-12s | %-12s\n",
                            "", lines[i], "", "");
                }
            }

            printf(C6 "----------------------------------------------------------------------------------------------------------------------------\n" RE);
        }

        sqlite3_finalize(stmt);
        printf(C6 "============================================================================================================================\n" RE);
    }

    // Hàm l?y tên profile theo ID
    void get_profile_name(sqlite3 *db, int profile_id, char *profile_name, size_t size)
    {
        sqlite3_stmt *stmt;
        const char *sql = "SELECT DefenseProfileName FROM DefenseProfiles WHERE DefenseProfileId = ?";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
        {
            snprintf(profile_name, size, "Unknown");
            return;
        }

        sqlite3_bind_int(stmt, 1, profile_id);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const unsigned char *name = sqlite3_column_text(stmt, 0);
            snprintf(profile_name, size, "%s", name ? (const char *)name : "Unknown");
        }
        else
        {
            snprintf(profile_name, size, "Unknown");
        }

        sqlite3_finalize(stmt);
    }

    // Menu ch?n profile r?i g?i select_multi_port()
    void select_multi_port_menu(int serial_port)
    {
        sqlite3 *db;
        if (sqlite3_open(DB_PATH, &db))
        {
            printf("Can't open database: %s\n", sqlite3_errmsg(db));
            return;
        }

        char input[64];
        while (1)
        {
            system("clear");
            display_logo1();

            printf(C3 "\n ==> Defense Profiles:\n" RE);
            display_profiles(db);

            printf(C3 "\nEnter Profile ID to select multi-port (Z to Exit): " RE);
            if (!fgets(input, sizeof(input), stdin))
                continue;
            input[strcspn(input, "\n")] = 0; // b? newline

            if (input[0] == 'Z' || input[0] == 'z')
            {
                system("clear");
                display_logo1();
                new_menu(serial_port);
                break;
            }

            int profile_id = atoi(input);
            if (profile_id > 0)
            {
                char profile_name[128];
                get_profile_name(db, profile_id, profile_name, sizeof(profile_name));

                printf(C3 "\nYou selected Profile ID %d - Name: %s\n" RE, profile_id, profile_name);

                // G?i hàm multi-port cho profile du?c ch?n
                select_multi_port(serial_port, db, profile_id);

                printf("\nPress Enter to continue...");
                fgets(input, sizeof(input), stdin);
            }
        }

        sqlite3_close(db);
    }

    // Hàm xóa port kh?i các profile khác
    void remove_eth_from_other_profiles(sqlite3 *db, int current_profile_id, const char *eth_name)
    {
        sqlite3_stmt *stmt;
        const char *sql = "SELECT DefenseProfileId, DefenseProfileUsingTime FROM DefenseProfiles";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        {
            printf("Failed to prepare select: %s\n", sqlite3_errmsg(db));
            return;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int profile_id = sqlite3_column_int(stmt, 0);
            const unsigned char *raw = sqlite3_column_text(stmt, 1);

            if (profile_id == current_profile_id)
                continue;
            if (!raw)
                continue;

            cJSON *arr = cJSON_Parse((const char *)raw);
            if (!arr || !cJSON_IsArray(arr))
            {
                if (arr)
                    cJSON_Delete(arr);
                continue;
            }

            int changed = 0;
            for (int i = 0; i < cJSON_GetArraySize(arr); i++)
            {
                cJSON *item = cJSON_GetArrayItem(arr, i);
                cJSON *name = cJSON_GetObjectItem(item, "name");
                if (name && strcmp(name->valuestring, eth_name) == 0)
                {
                    cJSON_DeleteItemFromArray(arr, i);
                    changed = 1;
                    i--;
                }
            }

            if (changed)
            {
                char *new_json = cJSON_PrintUnformatted(arr);

                sqlite3_stmt *update_stmt;
                const char *update_sql =
                    "UPDATE DefenseProfiles SET DefenseProfileUsingTime = ? WHERE DefenseProfileId = ?";
                if (sqlite3_prepare_v2(db, update_sql, -1, &update_stmt, NULL) == SQLITE_OK)
                {
                    sqlite3_bind_text(update_stmt, 1, new_json, -1, SQLITE_TRANSIENT);
                    sqlite3_bind_int(update_stmt, 2, profile_id);
                    sqlite3_step(update_stmt);
                    sqlite3_finalize(update_stmt);
                }
                free(new_json);
            }

            cJSON_Delete(arr);
        }

        sqlite3_finalize(stmt);
    }

    // Hàm select multi-port
    void select_multi_port(int serial_port, sqlite3 *db, int profile_id)
    {
        int eth_selected[4] = {0};
        char input[128];

        printf("\nEnter Ethernet ports to select (1-4), separated by space. Example: 1 3\n");
        printf("Type 'done' when finished.\n");

        while (1)
        {
            printf("> ");
            if (!fgets(input, sizeof(input), stdin))
                continue;
            input[strcspn(input, "\n")] = 0;

            if (strcmp(input, "done") == 0)
                break;

            char *token = strtok(input, " ");
            while (token)
            {
                int port = atoi(token);
                if (port >= 1 && port <= 4)
                {
                    if (!eth_selected[port - 1])
                    {
                        eth_selected[port - 1] = 1;
                        printf("Port %d selected.\n", port);
                    }
                    else
                    {
                        printf("Port %d already selected.\n", port);
                    }
                }
                else
                {
                    printf("Invalid port! Choose 1–4.\n");
                }

                token = strtok(NULL, " ");
            }
        }

        // L?y JSON cu c?a profile hi?n t?i
        cJSON *arr = NULL;
        sqlite3_stmt *stmt;
        const char *sql_get = "SELECT DefenseProfileUsingTime FROM DefenseProfiles WHERE DefenseProfileId = ?";
        if (sqlite3_prepare_v2(db, sql_get, -1, &stmt, NULL) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, profile_id);
            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                const unsigned char *raw = sqlite3_column_text(stmt, 0);
                if (raw)
                    arr = cJSON_Parse((const char *)raw);
            }
            sqlite3_finalize(stmt);
        }
        if (!arr)
            arr = cJSON_CreateArray();

        // Thêm các port m?i, gi? port cu
        time_t now = time(NULL);
        char datetime[32];
        strftime(datetime, sizeof(datetime), "%Y/%m/%d %H:%M:%S", localtime(&now));

        for (int i = 0; i < 4; i++)
        {
            if (eth_selected[i])
            {
                char eth_name[16];
                snprintf(eth_name, sizeof(eth_name), "eth%d", i + 1);

                // Xóa port này kh?i các profile khác
                remove_eth_from_other_profiles(db, profile_id, eth_name);

                // Ki?m tra xem dã t?n t?i trong profile chua
                int exists = 0;
                for (int j = 0; j < cJSON_GetArraySize(arr); j++)
                {
                    cJSON *item = cJSON_GetArrayItem(arr, j);
                    cJSON *name = cJSON_GetObjectItem(item, "name");
                    if (name && strcmp(name->valuestring, eth_name) == 0)
                    {
                        exists = 1;
                        break;
                    }
                }

                if (!exists)
                {
                    cJSON *item = cJSON_CreateObject();
                    cJSON_AddStringToObject(item, "name", eth_name);
                    cJSON_AddStringToObject(item, "date", datetime);
                    cJSON_AddItemToArray(arr, item);
                }
            }
        }

        //     // Luu l?i DB
        //     char *new_json = cJSON_PrintUnformatted(arr);
        //     char sql_update[1024];
        //     snprintf(sql_update, sizeof(sql_update),
        //              "UPDATE DefenseProfiles SET DefenseProfileUsingTime = ? WHERE DefenseProfileId = ?");
        //     sqlite3_stmt *update_stmt;
        //     if (sqlite3_prepare_v2(db, sql_update, -1, &update_stmt, NULL) == SQLITE_OK)
        //     {
        //         sqlite3_bind_text(update_stmt, 1, new_json, -1, SQLITE_TRANSIENT);
        //         sqlite3_bind_int(update_stmt, 2, profile_id);
        //         sqlite3_step(update_stmt);
        //         sqlite3_finalize(update_stmt);
        //     }

        //     free(new_json);
        //     cJSON_Delete(arr);

        //     printf("\nDatabase updated successfully for profile %d.\n", profile_id);
        // }
        // Luu l?i DB
        char *new_json = cJSON_PrintUnformatted(arr);
        char sql_update[1024];
        snprintf(sql_update, sizeof(sql_update),
                "UPDATE DefenseProfiles SET DefenseProfileUsingTime = ? WHERE DefenseProfileId = ?");
        sqlite3_stmt *update_stmt;
        if (sqlite3_prepare_v2(db, sql_update, -1, &update_stmt, NULL) == SQLITE_OK)
        {
            sqlite3_bind_text(update_stmt, 1, new_json, -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(update_stmt, 2, profile_id);
            sqlite3_step(update_stmt);
            sqlite3_finalize(update_stmt);
        }

        free(new_json);
        cJSON_Delete(arr);

        printf("\nDatabase updated successfully for profile %d.\n", profile_id);

        // G?i ngu?ng và enable/disable cho các port dã ch?n
        // Truy v?n co s? d? li?u d? l?y ngu?ng và enable/disable
        const char *sql_thresholds = "SELECT SYNFloodSYNThreshold, SYNFloodACKThreshold, UDPFloodThreshold, UDPFloodRate, "
                                    "DNSFloodThreshold, ICMPFloodThreshold, ICMPFloodRate, IPSecIKEThreshold, "
                                    "LANDAttackEnable, UDPFloodEnable, DNSFloodEnable, ICMPFloodEnable, IPSecIKEEnable, "
                                    "TCPFragmentEnable, UDPFragmentEnable, HTTPFloodEnable, HTTPSFloodEnable, "
                                    "SYNFloodWhiteListTimeOut, SYNFloodEnable "
                                    "FROM DefenseProfiles WHERE rowid = ?";

        if (sqlite3_prepare_v2(db, sql_thresholds, -1, &stmt, NULL) != SQLITE_OK)
        {
            printf("Failed to prepare SQL query for thresholds\n");
            return;
        }

        sqlite3_bind_int(stmt, 1, profile_id); // Gán ID profile

        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            printf("No data found for profile ID %d\n", profile_id);
            sqlite3_finalize(stmt);
            return;
        }

        // L?y các ngu?ng t? co s? d? li?u
        int syn_en = sqlite3_column_int(stmt, 0);
        int syn_syn = sqlite3_column_int(stmt, 1);
        int syn_ack = sqlite3_column_int(stmt, 2);
        int udp_thr = sqlite3_column_int(stmt, 3);
        int udp_rate = sqlite3_column_int(stmt, 4);
        int dns_thr = sqlite3_column_int(stmt, 5);
        int icmp_thr = sqlite3_column_int(stmt, 6);
        int icmp_rate = sqlite3_column_int(stmt, 7);
        int ike_thr = sqlite3_column_int(stmt, 8);
        int land_en = sqlite3_column_int(stmt, 9);
        int udp_en = sqlite3_column_int(stmt, 10);
        int dns_en = sqlite3_column_int(stmt, 11);
        int icmp_en = sqlite3_column_int(stmt, 12);
        int ike_en = sqlite3_column_int(stmt, 13);
        int tcpfrag = sqlite3_column_int(stmt, 14);
        int udpfrag = sqlite3_column_int(stmt, 15);
        int http_en = sqlite3_column_int(stmt, 16);
        int https_en = sqlite3_column_int(stmt, 17);
        int wl_timeout = sqlite3_column_int(stmt, 18);

        sqlite3_finalize(stmt); // K?t thúc truy v?n

        // L?p qua các port dã ch?n và g?i ngu?ng và enable/disable
        for (int i = 0; i < 4; i++)
        {
            if (eth_selected[i])
            {
                char profile_id_str[16];
                char buffer[256] = "";
                char ID[32] = "";
                char Value[64] = "";

                int port = i + 1; // Port s? 1 d?n 4

                // G?i các giá tr? ngu?ng xu?ng các port dã ch?n
                printf(C3 "Port %d: Syncing thresholds...\n" RE, port);

                printf(C3 "  - SYN Enable: %d\n" RE "", syn_en);
                send_syn_enable_disable(serial_port, port, syn_en, syn_en, ID, Value, buffer);

                printf(C3 "  - SYN Threshold: %d\n" RE "", syn_syn);
                send_syn_threshold(serial_port, port, syn_syn, syn_en, ID, Value, buffer);

                printf(C3 "  - ACK Threshold: %d\n" RE "", syn_ack);
                send_ack_threshold(serial_port, port, syn_ack, syn_en, ID, Value, buffer);

                printf(C3 "  - UDP Threshold: %d\n" RE "", udp_thr);
                send_udp_threshold(serial_port, port, udp_thr, udp_en, ID, Value, buffer);

                printf(C3 "  - UDP Rate: %d\n" RE "", udp_rate);
                send_udp_threshold_ps(serial_port, port, udp_rate, udp_en, ID, Value, buffer);

                printf(C3 "  - DNS Threshold: %d\n" RE "", dns_thr);
                send_dns_threshold(serial_port, port, dns_thr, dns_en, ID, Value, buffer);

                printf(C3 "  - ICMP Threshold: %d\n" RE "", icmp_thr);
                send_icmp_threshold(serial_port, port, icmp_thr, icmp_en, ID, Value, buffer);

                printf(C3 "  - ICMP Rate: %d\n" RE "", icmp_rate);
                send_icmp_threshold_ps(serial_port, port, icmp_rate, icmp_en, ID, Value, buffer);

                printf(C3 "  - IPSec IKE Threshold: %d\n" RE "", ike_thr);
                send_ike_threshold(serial_port, port, ike_thr, ike_en, ID, Value, buffer);

                // G?i enable/disable cho các giá tr?
                printf(C3 "  - LAND Enable: %d\n" RE "", land_en);
                send_land_enable_disable(serial_port, port, land_en, land_en, ID, Value, buffer);

                printf(C3 "  - UDP Enable: %d\n" RE "", udp_en);
                send_udp_enable_disable(serial_port, port, udp_en, udp_en, ID, Value, buffer);

                printf(C3 "  - DNS Enable: %d\n" RE "", dns_en);
                send_dns_enable_disable(serial_port, port, dns_en, dns_en, ID, Value, buffer);

                printf(C3 "  - ICMP Enable: %d\n" RE "", icmp_en);
                send_icmp_enable_disable(serial_port, port, icmp_en, icmp_en, ID, Value, buffer);

                printf(C3 "  - IPSec IKE Enable: %d\n" RE "", ike_en);
                send_ike_enable_disable(serial_port, port, ike_en, ike_en, ID, Value, buffer);

                printf(C3 "  - TCP Fragment Enable: %d\n" RE "", tcpfrag);
                send_tcpfrag_enable_disable(serial_port, port, tcpfrag, tcpfrag, ID, Value, buffer);

                printf(C3 "  - UDP Fragment Enable: %d\n" RE "", udpfrag);
                send_udpfrag_enable_disable(serial_port, port, udpfrag, udpfrag, ID, Value, buffer);

                printf(C3 "  - HTTP Enable: %d\n" RE "", http_en);
                send_http_enable_disable(serial_port, port, http_en, http_en, ID, Value, buffer);

                printf(C3 "  - HTTPS Enable: %d\n" RE "", https_en);
                send_https_enable_disable(serial_port, port, https_en, https_en, ID, Value, buffer);

                // G?i whitelist timeout
                printf(C3 "  - Whitelist Timeout: %d\n" RE "", wl_timeout);
                send_whitelist_timeout(serial_port, port, wl_timeout, wl_timeout, ID, Value, buffer);

                printf(C3 "Port %d: Sync done!\n" RE "", port);
            }
        }
    }

    void new_menu(int serial_port)
    {
    start:
        system("clear");
        display_logo1();
        char key = 0;
        char enter = '\r';
        printf(C6 "\r\n *************************************************************************************************************************************************************************************************************" RE);
        printf("\r\n");
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf(C3 "\r\n ==> New Menu Selected" RE "                                                                                                                                                                                       " C6 "|" RE);
        printf("\r\n");
        printf(C6 " ===============+===========+================================================================================================================================================================================+\r\n" RE);
        printf("    " C3 "DISPLAY" RE "     " C6 "|" RE "           " C6 "|" RE "                                                                                                                                                                                " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                                                                                                  " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "Setting RTC." RE "                                                                                                                                                                   " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 "Port 1 Settings" RE "                                                                                                                                                                " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "3." RE "    " C6 "|" RE " " C6 "Port 2 Settings" RE "                                                                                                                                                                " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "4." RE "    " C6 "|" RE " " C6 "Port 3 Settings" RE "                                                                                                                                                                " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "5." RE "    " C6 "|" RE " " C6 "Port 4 Settings" RE "                                                                                                                                                                " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "6." RE "    " C6 "|" RE " " C6 "Port Mirroring Settings." RE "                                                                                                                                                       " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "           " C6 "|" RE " 	" C3 "->(Info: Configure port mirroring for network monitoring)." RE "                                                                                                                   " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "7." RE "    " C6 "|" RE " " C6 "Setting attack detection time." RE "                                                                                                                                                 " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "           " C6 "|" RE " 	" C3 "->(Info: The default value is: 1 second)." RE "                                                                                                                                    " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "8." RE "    " C6 "|" RE " " C6 "Add VPN server name or address to legitimate VPN list." RE "                                                                                                                         " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "9." RE "    " C6 "|" RE " " C6 "Remove the VPN server name or address from the legal VPN list." RE "                                                                                                                 " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "A." RE "    " C6 "|" RE " " C6 "Add VPN server name or address IPv6 to legitimate VPN list." RE "                                                                                                                    " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "B." RE "    " C6 "|" RE " " C6 "Remove the VPN server name or address IPv6 from the legal VPN list." RE "                                                                                                            " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "C." RE "    " C6 "|" RE " " C6 "Select multi port for profile" RE "                                                                                                                                                                " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "Z." RE "    " C6 "|" RE " " C17 "Exit." RE "                                                                                                                                                                          " C6 "|\r\n" RE);
        printf(C6 "----------------+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf("    " C3 "SETTING" RE "     " C6 "|" RE " " C3 "Your choice: " RE);

        while (1)
        {
            scanf(" %c", &key);
            if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' ||
                key == '8' || key == '9' || key == 'A' || key == 'a' || key == 'B' || key == 'b' || key == 'C' || key == 'c' || key == 'Z' || key == 'z')
            {
                break;
            }
            if (key != '1' && key != '2' && key != '3' && key != '4' && key != '5' && key != '6' && key != '7' &&
                key != '8' && key != '9' && key != 'A' && key != 'a' && key != 'B' && key != 'b' && key != 'c' && key != 'C' && key != 'Z' && key != 'z')
            {
                printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Your choice: " RE);
            }
        }

        usleep(500000);
        if (key == '1')
        {
            system("clear");
            display_logo1();
            SetDateTime(serial_port);
            return;
        }
        else if (key == '2')
        {
            current_port = 1;
            system("clear");
            display_logo1();
            printf(C3 "\r\n ==> Port 1 Configuration\n" RE);
            reconfig(serial_port);
            return;
        }
        else if (key == '3')
        {
            current_port = 2;
            system("clear");
            printf(C3 "\r\n ==> Port 2 Configuration\n" RE);
            reconfig(serial_port);
            return;
        }
        else if (key == '4')
        {
            current_port = 3;
            system("clear");

            printf(C3 "\r\n ==> Port 3 Configuration\n" RE);
            reconfig(serial_port);
            return;
        }
        else if (key == '5')
        {
            current_port = 4;
            display_logo1();
            printf(C3 "\r\n ==> Port 4 Configuration\n" RE);
            reconfig(serial_port);
            return;
        }
        else if (key == '6')
        {
            system("clear");
            display_logo1();
            port_mirroring_menu(serial_port);
            return;
        }
        else if (key == '7')
        {
            system("clear");
            display_logo1();
            display_table(serial_port);
            SetTimeflood(serial_port);
            return;
        }
        else if (key == '8')
        {
            system("clear");
            display_logo1();
            Display_IPv4_vpn_table();
            AddIPv4VPN(serial_port);
            return;
        }
        else if (key == '9')
        {
            system("clear");
            display_logo1();
            Display_IPv4_vpn_table();
            RemoveIPv4VPN(serial_port);
            return;
        }
        else if (key == 'A' || key == 'a')
        {
            system("clear");
            display_logo1();
            // display_table(serial_port);
            Display_IPv6_vpn_table();
            AddIPv6VPN(serial_port);
            return;
        }
        else if (key == 'B' || key == 'b')
        {
            system("clear");
            display_logo1();
            Display_IPv6_vpn_table();
            RemoveIPv6VPN(serial_port);
            return;
        }
        else if (key == 'C' || key == 'c')
        {
            system("clear");
            display_logo1();
            select_multi_port_menu(serial_port);
            return;
        }
        else if (key == 'Z' || key == 'z')
        {
            system("clear");
            display_logo1();
            AdminConfigMenu(serial_port);
        }
    }
    void wrap_field(const char *prefix, const char *content, int width)
    {
        int len = strlen(content);
        int i = 0;
        while (i < len)
        {
            printf(C3 "%s%-*.*s\n" RE "", prefix, width, width, content + i);
            i += width;
        }
    }

    void extract_json_values(const char *json, char *out, int out_size)
    {
        int len = strlen(json);
        int out_idx = 0;
        int in_quotes = 0;
        int is_key = 1;

        for (int i = 0; i < len && out_idx < out_size - 1; i++)
        {
            if (json[i] == '"')
            {
                in_quotes = !in_quotes;
                continue;
            }

            if (in_quotes && !is_key)
            {
                out[out_idx++] = json[i];
            }

            if (!in_quotes && json[i] == ':')
            {
                is_key = 0;
            }

            if (!in_quotes && json[i] == ',')
            {
                out[out_idx++] = ',';
                out[out_idx++] = ' ';
                is_key = 1;
            }

            if (!in_quotes && json[i] == '}')
            {
                break;
            }
        }

        out[out_idx] = '\0';
    }

    void clean_json_array_string(const char *input, char *output, int out_size)
    {
        int out_idx = 0;
        for (int i = 0; input[i] != '\0' && out_idx < out_size - 1; i++)
        {
            char c = input[i];
            if (c == '[' || c == ']' || c == '"')
                continue;
            output[out_idx++] = c;
        }
        output[out_idx] = '\0';
    }

    void display_port_mirroring_config_from_db(int serial_port, int show_prompt)
    {
        int max_width_type = 50;
        int max_width_value = 62;
        int idx = 1;

        sqlite3 *db;
        sqlite3_stmt *stmt;
        int rc = sqlite3_open(DB_PATH, &db);
        sqlite3_busy_timeout(db, 2000);

        if (rc)
        {
            printf(C3 "\n cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        const char *sql =
            "SELECT di.InterfaceId, di.InterfaceIsMirroring, di.InterfaceName, "
            "mi.InterfaceName AS MonitorInterfaceName, "
            "di.InterfaceMirrorSetting, di.MirrorType, di.Value "
            "FROM DeviceInterfaces di "
            "LEFT JOIN DeviceInterfaces mi ON di.InterfaceToMonitorInterfaceId = mi.InterfaceId";

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            printf(C3 "\nError query: %s\n" RE "", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        printf(C6 "\n=============================================================================================================================================================================================================+\n" RE);
        printf(C6 "|" RE " " C3 "%-3s" RE " " C6 "|" RE " " C3 "%-15s" RE " " C6 "|" RE " " C3 "%-10s" RE " " C6 "|" RE " " C3 "%-22s" RE " " C6 "|" RE " " C3 "%-22s" RE " " C6 "|" RE " " C3 "%-50s" RE " " C6 "|" RE " " C3 "%-62s" RE " " C6 "|\n" RE "",
            "No", "InterfaceName", "Mirroring", "Monitor Interface ID", "MirrorSetting", "MirrorType", "Value");
        printf(C6 "|=====|=================|============|========================|========================|====================================================|================================================================|\n" RE);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int mirroring = sqlite3_column_int(stmt, 1);
            const unsigned char *interface_name = sqlite3_column_text(stmt, 2);
            const unsigned char *monitor_name = sqlite3_column_text(stmt, 3);
            const unsigned char *setting = sqlite3_column_text(stmt, 4);
            const unsigned char *type = sqlite3_column_text(stmt, 5);
            const unsigned char *value = sqlite3_column_text(stmt, 6);

            const char *mirroring_str = mirroring ? "Active" : "Inactive";
            const char *interface_name_str = interface_name ? (const char *)interface_name : "";
            const char *monitor_name_str = monitor_name ? (const char *)monitor_name : "N/A";
            const char *setting_str = setting ? (const char *)setting : "";

            char cleaned_type[512] = "";
            if (type)
                clean_json_array_string((const char *)type, cleaned_type, sizeof(cleaned_type));
            else
                strcpy(cleaned_type, "");

            const char *value_str = value ? (const char *)value : "";

            char extracted_values[512] = "";
            if (value_str && strlen(value_str) > 0)
                extract_json_values(value_str, extracted_values, sizeof(extracted_values));

            printf(C6 "|" RE " " C3 "%-3d" RE " " C6 "|" RE " " C3 "%-15s" RE " " C6 "|" RE " " C3 "%-10s" RE " " C6 "|" RE " " C3 "%-22s" RE " " C6 "|" RE " " C3 "%-22s" RE " " C6 "|" RE "",
                idx++, interface_name_str, mirroring_str, monitor_name_str, setting_str);

            // In dòng d?u tiên c?a MirrorType + Value
            printf(C3 " %-*.*s" RE " " C6 "|" RE "", max_width_type, max_width_type, cleaned_type);
            printf(C3 " %-*.*s" RE " " C6 "|\n" RE "", max_width_value, max_width_value, extracted_values);

            // In ph?n ti?p theo n?u MirrorType ho?c Value d i
            int type_len = strlen(cleaned_type);
            int value_len = strlen(extracted_values);
            int line = 1; // reset l?i cho m?i row

            while (line * max_width_type < type_len || line * max_width_value < value_len)
            {
                printf(C6 "|" RE " %-3s" RE " " C6 "|" RE " %-15s" RE " " C6 "|" RE " %-10s" RE " " C6 "|" RE " %-22s" RE " " C6 "|" RE " %-22s" RE " " C6 "|" RE "",
                    "", "", "", "", "");

                if (line * max_width_type < type_len)
                    printf(C3 " %-*.*s" RE " " C6 "|" RE "", max_width_type, max_width_type, cleaned_type + line * max_width_type);
                else
                    printf(C3 " %-*s" RE " " C6 "|" RE "", max_width_type, "");

                if (line * max_width_value < value_len)
                    printf(C3 " %-*.*s" RE " " C6 "|\n" RE "", max_width_value, max_width_value, extracted_values + line * max_width_value);
                else
                    printf(C3 " %-*s" RE " " C6 "|\n" RE "", max_width_value, "");

                line++;
            }

            printf(C6 "|-----+-----------------+------------+------------------------+------------------------+----------------------------------------------------+----------------------------------------------------------------|\n" RE);
        }

        if (show_prompt)
        {
            printf(C3 "Press Enter to return to menu..." RE);
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    void Update_port_mirroring(int serial_port)
    {
        char port_name[32];
        system("clear");
        display_logo1();
        printf(C3 "\nCurrent Port Mirroring Configurations:\n" RE);
        display_port_mirroring_config_from_db(serial_port, 0);

        printf(C3 "\nEnter InterfaceName to update (e.g. eth1) or press " C1 "'exit'" RE " return previous menu :" RE);
        scanf("%31s", port_name);
        if (strcmp(port_name, "eth5") == 0)
        {
            printf(C3 "Interface 'eth5' is not allowed to be selected for update.\n" RE);
            printf(C3 "Press Enter to return to the previous menu..." RE);
            getchar(); //  ? d?c d?u Enter c n l?i trong buffer
            getchar();
            system("clear");
            display_logo1();
            port_mirroring_menu(serial_port); // Quay l?i menu ch nh ho?c menu port mirroring
            return;
        }

        // N?u ngu?i d ng nh?p 'exit' th  quay v? menu ch clearnh
        if (strcmp(port_name, "" C17 "exit" RE) == 0)
        {
            printf(C3 "Returning to main menu...\n" RE);
            getchar(); // Clear buffer sau scanf
            system("clear");
            display_logo1();
            port_mirroring_menu(serial_port);
        }
        // Ki?m tra port c  t?n t?i kh ng
        sqlite3 *db;
        int rc = sqlite3_open(DB_PATH, &db);
        sqlite3_busy_timeout(db, 2000);
        if (rc)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }
        const char *sql = "SELECT InterfaceIsMirroring, InterfaceMirrorSetting, MirrorType, Value FROM DeviceInterfaces WHERE InterfaceName=? AND InterfaceIsMirroring=1";
        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            printf(C3 "SQL error: %s\n" RE "", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }
        sqlite3_bind_text(stmt, 1, port_name, -1, SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW)
        {
            system("clear");
            printf(C3 "No mirroring configuration found for interface '%s'.\n" RE "", port_name);
            printf(C3 "You must add a mirroring configuration before updating.\n" RE);
            printf(C3 "No configuration found for %s.\n" RE "", port_name);
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            printf(C3 "Press Enter to return to menu..." RE);
            getchar();
            getchar();
            system("clear");
            display_logo1();
            return;
        }

        // L?y d? li?u cu
        PortMirroringConfig cfg = {0};
        strcpy(cfg.interface_name, port_name);
        cfg.is_mirroring = sqlite3_column_int(stmt, 0);
        const unsigned char *mirror_setting = sqlite3_column_text(stmt, 1);
        const unsigned char *mirror_type = sqlite3_column_text(stmt, 2);
        const unsigned char *value = sqlite3_column_text(stmt, 3);
        if (mirror_setting)
            strcpy(cfg.mirror_setting, (const char *)mirror_setting);
        if (mirror_type)
            strcpy(cfg.mirror_type, (const char *)mirror_type);
        if (value)
            strcpy(cfg.value, (const char *)value);

        sqlite3_finalize(stmt);
        sqlite3_close(db);

        printf(C3 "\nUpdating configuration for %s...\n" RE "", port_name);

        // Parse value JSON cu th nh map key-value d? c?p nh?t l?i gi  tr? m?i nh?t
        cJSON *json_old = NULL;
        if (cfg.value[0])
        {
            json_old = cJSON_Parse(cfg.value);
        }
        // T?o struct luu gi  tr? cu?i c ng cho t?ng tru?ng
        char last_dest_mac[18] = "";
        char last_src_mac[18] = "";
        char last_dest_ip[40] = "";
        char last_src_ip[40] = "";
        char last_dest_port[6] = "";
        char last_src_port[6] = "";
        char last_protocol[16] = "";

        if (json_old)
        {
            cJSON *item;
            if ((item = cJSON_GetObjectItemCaseSensitive(json_old, "DestMac")) && cJSON_IsString(item))
                strncpy(last_dest_mac, item->valuestring, sizeof(last_dest_mac));
            if ((item = cJSON_GetObjectItemCaseSensitive(json_old, "SourceMac")) && cJSON_IsString(item))
                strncpy(last_src_mac, item->valuestring, sizeof(last_src_mac));
            if ((item = cJSON_GetObjectItemCaseSensitive(json_old, "DestIPv4")) && cJSON_IsString(item))
                strncpy(last_dest_ip, item->valuestring, sizeof(last_dest_ip));
            if ((item = cJSON_GetObjectItemCaseSensitive(json_old, "SourceIPv4")) && cJSON_IsString(item))
                strncpy(last_src_ip, item->valuestring, sizeof(last_src_ip));
            if ((item = cJSON_GetObjectItemCaseSensitive(json_old, "DestIPv6")) && cJSON_IsString(item) && strlen(item->valuestring) > 0)
                strncpy(last_dest_ip, item->valuestring, sizeof(last_dest_ip));
            if ((item = cJSON_GetObjectItemCaseSensitive(json_old, "SourceIPv6")) && cJSON_IsString(item) && strlen(item->valuestring) > 0)
                strncpy(last_src_ip, item->valuestring, sizeof(last_src_ip));
            if ((item = cJSON_GetObjectItemCaseSensitive(json_old, "DestPort")) && cJSON_IsString(item))
                strncpy(last_dest_port, item->valuestring, sizeof(last_dest_port));
            if ((item = cJSON_GetObjectItemCaseSensitive(json_old, "SourcePort")) && cJSON_IsString(item))
                strncpy(last_src_port, item->valuestring, sizeof(last_src_port));
            if ((item = cJSON_GetObjectItemCaseSensitive(json_old, "Protocol")) && cJSON_IsString(item))
                strncpy(last_protocol, item->valuestring, sizeof(last_protocol));
            cJSON_Delete(json_old);
        }

        PortMirroringConfig cfg_update = cfg;
        // G n l?i c c tru?ng d  c  v o bi?n t?m d? khi v o ConfigTypePacket s? hi?n th? d ng
        strcpy(cfg_update.value, cfg.value);
        strcpy(cfg_update.mirror_type, cfg.mirror_type);
        // ConfigTypePacket(serial_port, &cfg_update);
        system("clear");
        display_logo1();
        Select_traffic_mirroring_mode(serial_port, &cfg_update);

        // Sau khi c?u h nh xong, parse l?i value m?i d? l?y gi  tr? m?i nh?t
        cJSON *json_new = NULL;
        if (cfg_update.value[0])
        {
            json_new = cJSON_Parse(cfg_update.value);
        }
        // N?u tru?ng n o kh ng c  trong value m?i th  gi? l?i gi  tr? cu
        if (json_new)
        {
            cJSON *item;
            if (!(item = cJSON_GetObjectItemCaseSensitive(json_new, "DestMac")) && last_dest_mac[0])
                cJSON_AddStringToObject(json_new, "DestMac", last_dest_mac);
            if (!(item = cJSON_GetObjectItemCaseSensitive(json_new, "SourceMac")) && last_src_mac[0])
                cJSON_AddStringToObject(json_new, "SourceMac", last_src_mac);
            if (!(item = cJSON_GetObjectItemCaseSensitive(json_new, "DestIPv4")) && last_dest_ip[0])
                cJSON_AddStringToObject(json_new, "DestIPv4", last_dest_ip);
            if (!(item = cJSON_GetObjectItemCaseSensitive(json_new, "SourceIPv4")) && last_src_ip[0])
                cJSON_AddStringToObject(json_new, "SourceIPv4", last_src_ip);
            if (!(item = cJSON_GetObjectItemCaseSensitive(json_new, "DestPort")) && last_dest_port[0])
                cJSON_AddStringToObject(json_new, "DestPort", last_dest_port);
            if (!(item = cJSON_GetObjectItemCaseSensitive(json_new, "SourcePort")) && last_src_port[0])
                cJSON_AddStringToObject(json_new, "SourcePort", last_src_port);
            if (!(item = cJSON_GetObjectItemCaseSensitive(json_new, "Protocol")) && last_protocol[0])
                cJSON_AddStringToObject(json_new, "Protocol", last_protocol);

            // Ghi l?i value m?i d  merge d? c c tru?ng
            char *new_value_str = cJSON_PrintUnformatted(json_new);
            strncpy(cfg_update.value, new_value_str, sizeof(cfg_update.value) - 1);
            free(new_value_str);
            cJSON_Delete(json_new);
        }
        // Luu l?i v o DB
        save_port_mirroring_to_db(&cfg_update);
        getchar();
        getchar();
        system("clear");
        display_logo1();
    }

    void port_mirroring_menu(int serial_port)
    {
    start:
        char key = 0;
        printf(C6 "\r\n *************************************************************************************************************************************************************************************************************" RE);
        printf(C6 "\r\n");
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf("\r\n " C3 "==> Port Mirroring Settings" RE "                                                                                                                                                                                 " C6 "|" RE);
        printf(C6 "\r\n");
        printf(C6 " ===============+===========+================================================================================================================================================================================+\r\n" RE);
        printf(C6 "    " C3 "DISPLAY" RE "     |           |                                                                                                                                                                                " C6 "|\r\n" RE);
        printf(C6 "\t\t|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "Please choose 1 option below:" RE "                                                                                                                                                  " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "Display Port Mirroring Configuration." RE "                                                                                                                                          " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 "Add Port Mirroring Configuration." RE "                                                                                                                                              " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "3." RE "    " C6 "|" RE " " C6 "Update Port Mirroring." RE "                                                                                                                                                         " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "4." RE "    " C6 "|" RE " " C6 "Delete Port Mirroring Configuration." RE "                                                                                                                                           " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "5." RE "    " C6 "|" RE " " C6 "Back to Previous Menu." RE "                                                                                                                                                         " C6 "|\r\n" RE);
        printf(C6 "----------------+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf("    " C3 "SETTING" RE "     " C6 "|" RE " " C6 "Your choice: " RE);

        while (1)
        {
            scanf("%c", &key);
            if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5')
            {
                break;
            }
            if (key != '1' && key != '2' && key != '3' && key != '4' && key != '5')
            {
                printf(C3 "\r     SETTING" RE "    " C6 "|" RE " " C3 "--> Your choice: " RE);
            }
        }

        usleep(500000);
        if (key == '1')
        {
            system("clear");
            display_logo1();
            display_port_mirroring_config_from_db(serial_port, 1);
            getchar();
            getchar();
            system("clear");
            display_logo1();
            goto start;
        }
        else if (key == '2')
        {
            system("clear");
            display_logo1();
            Add_port_mirroring(serial_port);
            printf(C3 "\nAdding Port Mirroring Configuration...\n" RE);
            goto start;
        }
        else if (key == '3')
        {
            system("clear");
            display_logo1();
            Update_port_mirroring(serial_port);
            printf(C3 "\nDeleting Port Mirroring Configuration...\n" RE);
            goto start;
        }
        else if (key == '4')
        {
            system("clear");
            display_logo1();
            Delete_port_mirroring(serial_port);
            goto start;
        }
        else if (key == '5')
        {
            system("clear");
            // display_logo1();
            new_menu(serial_port);
        }
    }
    void Delete_port_mirroring(int serial_port)
    {
        while (1)
        {
            char port_name[32];
            system("clear");
            display_logo1();
            printf(C3 "\nCurrent Port Mirroring Configurations:\n" RE);
            display_port_mirroring_config_from_db(serial_port, 0);
            printf(C3 "\nEnter InterfaceName to delete mirroring config (e.g. eth1), or type " C1 "'exit'" RE " to return: " RE);
            scanf("%31s", port_name);
            getchar(); // Clear newline kh?i stdin

            if (strcmp(port_name, "" C17 "exit" RE) == 0)
            {
                printf(C3 "Returning to main menu...\n" RE);
                system("clear");
                display_logo1();
                return;
            }

            sqlite3 *db;
            int rc = sqlite3_open(DB_PATH, &db);
            sqlite3_busy_timeout(db, 2000);
            if (rc)
            {
                printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
                return;
            }

            const char *update_sql =
                "UPDATE DeviceInterfaces SET "
                "InterfaceIsMirroring=0, InterfaceToMonitorInterfaceId=NULL, InterfaceMirrorSetting=NULL, MirrorType=NULL, Value=NULL "
                "WHERE InterfaceName=?";

            sqlite3_stmt *stmt;
            rc = sqlite3_prepare_v2(db, update_sql, -1, &stmt, NULL);
            if (rc != SQLITE_OK)
            {
                printf(C3 "SQL error: %s\n" RE "", sqlite3_errmsg(db));
                sqlite3_close(db);
                return;
            }

            sqlite3_bind_text(stmt, 1, port_name, -1, SQLITE_STATIC);
            rc = sqlite3_step(stmt);

            if (rc == SQLITE_DONE && sqlite3_changes(db) > 0)
            {
                printf(C2 "Deleted mirroring configuration for %s successfully!\n" RE "", port_name);

                // G?i l?nh t?t port mirroring qua UART
                int port_num = 0;
                if (sscanf(port_name, "eth%d", &port_num) == 1 && port_num > 0)
                {
                    char key_08 = 0x08;
                    char enter = '\r';
                    char keyX = 'X';
                    char port_key = '0' + port_num;
                    char value_key = '0'; // '0' d? t?t mirroring

                    write(serial_port, &key_08, sizeof(key_08));
                    usleep(1000);
                    write(serial_port, &enter, sizeof(enter));
                    usleep(1000);
                    write(serial_port, &keyX, sizeof(keyX));
                    usleep(100000);
                    write(serial_port, &port_key, sizeof(port_key));
                    usleep(10000);
                    write(serial_port, &value_key, sizeof(value_key));
                    usleep(10000);
                }

                sqlite3_finalize(stmt);
                sqlite3_close(db);
                break;
            }
            else
            {
                printf(C3 "No mirroring configuration found for %s or failed to delete.\n" RE "", port_name);
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                printf(C3 "Please try again or type" RE " " C1 "'exit'" RE " " C3 "to return." RE "\n");
                getchar();
            }
        }
        printf(C3 "Press Enter to return to menu..." RE);
        getchar();
        system("clear");
        display_logo1();
        port_mirroring_menu(serial_port);
    }
    void Add_port_mirroring(int serial_port)
    {
        system("clear");
        display_logo1();

        PortMirroringConfig cfg = {0};
        cfg.is_mirroring = 1;

        int choice = 0;
        char valid_ports[] = {1, 2, 3, 4, 6, 7, 8};
        char num_ports = sizeof(valid_ports) / sizeof(valid_ports[0]);

        // Menu ch?n c?ng
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf(C6 "\r\n   " C3 "CONFIGURE" RE "    " C6 "|" RE "           " C6 "|" RE "                                                                                                                                                                                " C6 "|\r\n" RE);
        printf(C6 "=============================================================================================================================================================================================================+\r\n" RE);
        printf(C6 "\t\t|" RE " " C3 "Key Enter" RE " " C6 "|" RE " " C3 "MONITORED PORT SELECTION" RE "                                                                                                                                                       " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);

        for (int i = 0; i < num_ports; i++)
        {
            printf(C6 "\t\t|" RE "     " C3 "%d." RE "    " C6 "|" RE " " C6 "eth%d" RE "                                                                                                                                                                           " C6 "|\r\n" RE "", i + 1, valid_ports[i]);
            printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        }

        printf(C6 "\t\t|" RE "     " C17 "%d." RE "    " C6 "|" RE " " C17 "Exit." RE "                                                                                                                                                                          " C6 "|\r\n" RE "", num_ports + 1);
        printf(C6 " ===============+=============================================================================================================================================================================================+\r\n" RE);
        printf(C6 "\r\n" C3 "SETTING" RE "       " C6 "|" RE "  " C3 "Enter your choice [1-%d]: " RE "", num_ports + 1);

        if (scanf("%d", &choice) != 1)
        {
            printf(C3 "\nInvalid input! Please enter a number between 1 and %d.\n" RE "", num_ports + 1);
            while (getchar() != '\n')
                ;
            Add_port_mirroring(serial_port);
            return;
        }

        if (choice == num_ports + 1)
        {
            system("clear");
            display_logo1();
            port_mirroring_menu(serial_port);
            return;
        }

        if (choice < 1 || choice > num_ports)
        {
            printf(C3 "\nChoice out of valid range! Please try again.\n" RE);
            Add_port_mirroring(serial_port);
            return;
        }

        int selected_port = valid_ports[choice - 1];
        sprintf(cfg.interface_name, "eth%d", selected_port);
        cfg.monitor_target_id = -1;

        printf(C3 "\nYou selected interface: %s\n" RE "", cfg.interface_name);

        Select_traffic_mirroring_mode(serial_port, &cfg);
    }

    void Select_traffic_mirroring_mode(int serial_port, PortMirroringConfig *cfg)
    {
        system("clear");
        display_logo1();
        char key1 = '1';
        char key2 = '2';
        char key3 = '3';
        // char mode = 0;
        printf(C6 "\r\n ============================================================================================================================================================================================================+\r\n" RE);
        printf(C6 "\r\n " C3 "TRAFFIC MIRRORING MODE SETTINGS" RE "                                                                                                                                                           " C6 "|" RE);
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf(C6 "\r\n " C3 "TRAFFIC MODE" RE "   " C6 "|" RE "           " C6 "|" RE "                                                                                                                                                                                " C6 "|" RE);
        printf(C6 "\r\n ===============+===========+================================================================================================================================================================================+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "INGRESS." RE "                                                                                                                                                                       " C6 "|\r\n" RE);
        printf(C6 "\t\t ---------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 "EGRESS." RE "                                                                                                                                                                        " C6 "|\r\n" RE);
        printf(C6 "\t\t ---------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "3." RE "    " C6 "|" RE " " C6 "INGRESS & EGRESS." RE "                                                                                                                                                              " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "4." RE "    " C6 "|" RE " " C17 "Exit." RE "                                                                                                                                                                          " C6 "|\r\n" RE);
        printf(C6 "\t\t =====================+======================================================================================================================================================================+\r\n" RE);
        printf(C6 "\t\t|" RE "  " C3 "SETTING" RE "  " C6 "|" RE " " C3 "Enter your choice [1/2/3/4]: " RE);
        char mode = 0;
        scanf(" %c", &mode);

        char *value = NULL;
        switch (mode)
        {
        case '1':
            strcpy(cfg->mirror_setting, "Ingress");
            // printf("Sent OK: key1 = %c (Ingress)\n", key1);
            break;
        case '2':
            strcpy(cfg->mirror_setting, "Egress");
            // printf("Sent OK: key2 = %c (Egress)\n", key2);
            break;
        case '3':
            strcpy(cfg->mirror_setting, "Ingress and Egress");
            // printf("Sent OK: key3 = %c (Ingress and Egress)\n", key3);
            break;
        case '4':
            system("clear");
            // Add_port_mirroring(serial_port);
            return;
        default:
            printf(C3 "\nInvalid mode selected!\n" RE);
            return;
        }
        usleep(100000);
        ConfigTypePacket(serial_port, cfg);
    }
    // H m ti?n  ch: X a tru?ng kh?i m?ng n?u d  t?n t?i
    void remove_field(char type_fields[][32], char value_fields[][40], int *field_count, const char *field_name)
    {
        for (int i = 0; i < *field_count; ++i)
        {
            if (strcmp(type_fields[i], field_name) == 0)
            {
                // D?ch c c ph?n t? sau l n
                for (int j = i; j < *field_count - 1; ++j)
                {
                    strcpy(type_fields[j], type_fields[j + 1]);
                    strcpy(value_fields[j], value_fields[j + 1]);
                }
                (*field_count)--;
                break;
            }
        }
    }

    void ConfigTypePacket(int serial_port, PortMirroringConfig *cfg)
    {
        cfg->monitor_target_id = 5;

        char dest_mac[18] = "";
        char src_mac[18] = "";
        char dest_ip[40] = "";
        char src_ip[40] = "";
        char dest_port[6] = "";
        char src_port[6] = "";
        int protocol = -1;
        int flag_dest_mac = 0, flag_src_mac = 0, flag_dest_ip = 0;
        int flag_src_ip = 0, flag_dest_port = 0, flag_src_port = 0;
        int flag_protocol = 0;
        char choice;
        char type_fields[7][32] = {0};
        char value_fields[7][40] = {0};
        int field_count = 0;
        char buffer[512] = "";
        char ID[64] = "";
        char Value[128] = "";

        char mirror_type[128] = "";
        char value[256] = "";
        system("clear");
        display_logo1();
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf(C3 "\r\n ==> Packet Filtering Configuration Menu" RE "                                                                                                                                                                     " C6 "|" RE);
        printf(C6 "\r\n ============================================================================================================================================================================================================+" RE);
        printf(C6 "\r\n    " C3 "DISPLAY" RE "     " C6 "|" RE "           " C6 "|" RE "                                                                                                                                                                                " C6 "|" RE);
        printf(C6 "\r\n ===============+===========+================================================================================================================================================================================+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "1." RE "    " C6 "|" RE " " C6 "Destination MAC." RE "                                                                                                                                                                " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "2." RE "    " C6 "|" RE " " C6 "Source MAC." RE "                                                                                                                                                                     " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "3." RE "    " C6 "|" RE " " C6 "Destination IP." RE "                                                                                                                                                                 " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "4." RE "    " C6 "|" RE " " C6 "Source IP." RE "                                                                                                                                                                      " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "5." RE "    " C6 "|" RE " " C6 "Destination Port." RE "                                                                                                                                                               " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "6." RE "    " C6 "|" RE " " C6 "Source Port." RE "                                                                                                                                                                   " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "7." RE "    " C6 "|" RE " " C6 "Protocol." RE "                                                                                                                                                                       " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "8." RE "    " C6 "|" RE " " C6 "Save." RE "                                                                                                                                                                           " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);
        printf(C6 "\t\t|" RE "     " C3 "9." RE "    " C6 "|" RE " " C17 "Exit." RE "                                                                                                                                                                           " C6 "|\r\n" RE);
        printf(C6 "\t\t+-----------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\r\n" RE);

        if (cfg->mirror_type[0] && cfg->value[0])
        {
            char type_buf[128];
            strncpy(type_buf, cfg->mirror_type, sizeof(type_buf));
            char *p = type_buf;
            while (*p)
            {
                if (*p == '[' || *p == ']' || *p == '"')
                    *p = ' ';
                p++;
            }
            char *type_token = strtok(type_buf, ",");
            cJSON *json = cJSON_Parse(cfg->value);
            while (type_token && field_count < 7)
            {
                while (*type_token == ' ')
                    type_token++;
                char *end = type_token + strlen(type_token) - 1;
                while (end > type_token && *end == ' ')
                {
                    *end = 0;
                    end--;
                }
                strncpy(type_fields[field_count], type_token, sizeof(type_fields[field_count]));

                char key[32] = "";
                if (strcmp(type_token, "Dest Mac") == 0)
                    strcpy(key, "DestMac");
                else if (strcmp(type_token, "Source Mac") == 0)
                    strcpy(key, "SourceMac");
                else if (strcmp(type_token, "Dest IP") == 0)
                    strcpy(key, "DestIPv4");
                else if (strcmp(type_token, "Source IP") == 0)
                    strcpy(key, "SourceIPv4");
                else if (strcmp(type_token, "Dest Port") == 0)
                    strcpy(key, "DestPort");
                else if (strcmp(type_token, "Source Port") == 0)
                    strcpy(key, "SourcePort");
                else if (strcmp(type_token, "Protocol") == 0)
                    strcpy(key, "Protocol");
                else
                    strncpy(key, type_token, sizeof(key) - 1);

                cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
                if (item && cJSON_IsString(item))
                {
                    strncpy(value_fields[field_count], item->valuestring, sizeof(value_fields[field_count]));
                }
                else
                {
                    value_fields[field_count][0] = '\0';
                }
                field_count++;
                type_token = strtok(NULL, ",");
            }
            cJSON_Delete(json);
        }

        while (1)
        {
            printf(C6 "+-----------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);
            if (flag_dest_mac)
                printf("| " C3 "- Destination MAC" RE "      " C6 "|" RE "  " C6 "%s\n" RE "", dest_mac);
            if (flag_src_mac)
                printf("| " C3 "- Source MAC" RE "          " C6 "|" RE "  " C6 "%s\n" RE "", src_mac);
            if (flag_dest_ip)
                printf("| " C3 "- Destination IP" RE "      " C6 "|" RE "  " C6 "%s\n" RE "", dest_ip);
            if (flag_src_ip)
                printf("| " C3 "- Source IP" RE "           " C6 "|" RE "  " C6 "%s\n" RE "", src_ip);
            if (flag_dest_port)
                printf("| " C3 "- Destination Port" RE "    " C6 "|" RE "  " C6 "%s\n" RE "", dest_port);
            if (flag_src_port)
                printf("| " C3 "- Source Port" RE "         " C6 "|" RE "  " C6 "%s\n" RE "", src_port);
            if (flag_protocol)
                printf("| " C3 "- Protocol" RE "            " C6 "|" RE "  " C6 "%d\n" RE "", protocol);
            printf(C6 "+-----------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+\n" RE);

            printf(C3 "Your choice: " RE);
            scanf(" %c", &choice);

            if (choice == '1')
            {
                InputDestMAC(dest_mac);
                if (strlen(dest_mac) > 0)
                {
                    remove_field(type_fields, value_fields, &field_count, "Dest Mac");
                    strcpy(type_fields[field_count], "Dest Mac");
                    strcpy(value_fields[field_count], dest_mac);
                    field_count++;
                    flag_dest_mac = 1;
                }
                getchar();
            }
            else if (choice == '2')
            {
                InputSourceMAC(src_mac);
                if (strlen(src_mac) > 0)
                {
                    remove_field(type_fields, value_fields, &field_count, "Source Mac");
                    strcpy(type_fields[field_count], "Source Mac");
                    strcpy(value_fields[field_count], src_mac);
                    field_count++;
                    flag_src_mac = 1;
                }
                getchar();
            }
            else if (choice == '3')
            {
                char ip_type = InputDestIP(dest_ip);
                if (strlen(dest_ip) > 0)
                {
                    remove_field(type_fields, value_fields, &field_count, "Dest IP");
                    strcpy(type_fields[field_count], "Dest IP");
                    strcpy(value_fields[field_count], dest_ip);
                    field_count++;
                    flag_dest_ip = 1;
                }
                getchar();
            }
            else if (choice == '4')
            {
                char ip_type = InputSourceIP(src_ip);
                if (strlen(src_ip) > 0)
                {
                    remove_field(type_fields, value_fields, &field_count, "Source IP");
                    strcpy(type_fields[field_count], "Source IP");
                    strcpy(value_fields[field_count], src_ip);
                    field_count++;
                    flag_src_ip = 1;
                }
                getchar();
            }
            else if (choice == '5')
            {
                InputDestPort(dest_port);
                if (strlen(dest_port) > 0)
                {
                    remove_field(type_fields, value_fields, &field_count, "Dest Port");
                    strcpy(type_fields[field_count], "Dest Port");
                    strcpy(value_fields[field_count], dest_port);
                    field_count++;
                    flag_dest_port = 1;
                }
                getchar();
            }
            else if (choice == '6')
            {
                InputSourcePort(src_port);
                if (strlen(src_port) > 0)
                {
                    remove_field(type_fields, value_fields, &field_count, "Source Port");
                    strcpy(type_fields[field_count], "Source Port");
                    strcpy(value_fields[field_count], src_port);
                    field_count++;
                    flag_src_port = 1;
                }
                getchar();
            }
            else if (choice == '7')
            {
                char protocol_str[16] = "";
                InputProtocol(&protocol, protocol_str);
                remove_field(type_fields, value_fields, &field_count, "Protocol");
                strcpy(type_fields[field_count], "Protocol");
                strcpy(value_fields[field_count], protocol_str);
                field_count++;
                flag_protocol = 1;
                getchar();
            }
            else if (choice == '8')
            {
                char *resp = NULL;
                int port_num = 1;
                if (cfg->interface_name[3] >= '0' && cfg->interface_name[3] <= '9')
                    port_num = cfg->interface_name[3] - '0';
                char port_key = '0' + port_num;
                char key_08 = 0x08;
                char enter = '\r';
                char keyX = 'X';
                char value_key = '1';

                write(serial_port, &key_08, sizeof(key_08));
                usleep(500000);
                write(serial_port, &enter, sizeof(enter));
                usleep(500000);
                write(serial_port, &keyX, sizeof(keyX));
                usleep(500000);
                write(serial_port, &port_key, sizeof(port_key));
                usleep(500000);
                write(serial_port, &value_key, sizeof(value_key));
                usleep(500000);
                // CHECK
                int t = 0;
                while (1)
                {
                    char *resp = receive_data(serial_port);
                    printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", resp);
                    if ((strchr(resp, 'K') != NULL))
                    {
                        strcat(buffer, ID);
                        strcat(buffer, "$");
                        strcat(buffer, Value);
                        strcat(buffer, "$OK$");
                        // printf("\nBLOCK_IPv4 done\n");

                        break;
                    }
                    else if ((strchr(resp, 'N') != NULL) || (t == 10))
                    {
                        strcat(buffer, ID);
                        strcat(buffer, "$");
                        strcat(buffer, Value);
                        strcat(buffer, "$ERROR$");
                        // printf("\nBLOCK_IPv4 ERROR\n");
                        break;
                    }
                    t++;
                }
                // 2. Set mirroring mode (keyK)
                char keyK = 'K';
                write(serial_port, &key_08, sizeof(key_08));
                usleep(500000);
                write(serial_port, &enter, sizeof(enter));
                usleep(50000);
                write(serial_port, &keyK, sizeof(keyK));
                usleep(500000);
                write(serial_port, &port_key, sizeof(port_key));
                usleep(500000);
                char key_mode = 0;
                if (strcmp(cfg->mirror_setting, "Ingress") == 0)
                    key_mode = '1';
                else if (strcmp(cfg->mirror_setting, "Egress") == 0)
                    key_mode = '2';
                else if (strcmp(cfg->mirror_setting, "Ingress and Egress") == 0)
                    key_mode = '3';
                if (key_mode)
                {
                    write(serial_port, &key_mode, sizeof(key_mode));
                    usleep(100000);

                    // CHECK
                    int t = 0;
                    while (1)
                    {
                        char *resp = receive_data(serial_port);
                        printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", resp);
                        if ((strchr(resp, 'K') != NULL))
                        {
                            strcat(buffer, ID);
                            strcat(buffer, "$");
                            strcat(buffer, Value);
                            strcat(buffer, "$OK$");
                            // printf("\nBLOCK_IPv4 done\n");

                            break;
                        }
                        else if ((strchr(resp, 'N') != NULL) || (t == 10))
                        {
                            strcat(buffer, ID);
                            strcat(buffer, "$");
                            strcat(buffer, Value);
                            strcat(buffer, "$ERROR$");
                            // printf("\nBLOCK_IPv4 ERROR\n");
                            break;
                        }
                        t++;
                    }
                }

                for (int i = 0; i < field_count; ++i)
                {
                    if (strcmp(type_fields[i], "Dest Mac") == 0)
                    {
                        char keyB = 'B';
                        write(serial_port, &key_08, sizeof(key_08));
                        usleep(1000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(1000);
                        write(serial_port, &keyB, sizeof(keyB));
                        usleep(1000);
                        write(serial_port, &port_key, sizeof(port_key));
                        usleep(1000);
                        for (int j = 0; j < strlen(value_fields[i]); j++)
                        {
                            char data = value_fields[i][j];
                            send_data(serial_port, &data, sizeof(data));

                            usleep(100000);
                        }
                        write(serial_port, &enter, sizeof(enter));
                        // CHECK
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", resp);
                            if ((strchr(resp, 'K') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nBLOCK_IPv4 done\n");

                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nBLOCK_IPv4 ERROR\n");
                                break;
                            }
                            t++;
                        }
                    }
                    else if (strcmp(type_fields[i], "Source Mac") == 0)
                    {
                        char keyA = 'A';
                        write(serial_port, &key_08, sizeof(key_08));
                        usleep(1000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(1000);
                        write(serial_port, &keyA, sizeof(keyA));
                        usleep(1000);
                        write(serial_port, &port_key, sizeof(port_key));
                        usleep(1000);
                        for (int j = 0; j < strlen(value_fields[i]); j++)
                        {
                            char data = value_fields[i][j];
                            send_data(serial_port, &data, sizeof(data));
                            usleep(100000);
                        }
                        write(serial_port, &enter, 1);
                        // CHECK
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", resp);
                            if ((strchr(resp, 'K') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nBLOCK_IPv4 done\n");

                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nBLOCK_IPv4 ERROR\n");
                                break;
                            }
                            t++;
                        }
                    }
                    else if (strcmp(type_fields[i], "Dest IP") == 0)
                    {
                        char keyD = 'D', keyF = 'F';
                        char key_func = strchr(value_fields[i], ':') ? keyF : keyD;
                        write(serial_port, &key_08, sizeof(key_08));
                        usleep(50000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(50000);
                        write(serial_port, &key_func, sizeof(key_func));
                        usleep(50000);
                        write(serial_port, &port_key, sizeof(port_key));
                        usleep(50000);
                        for (int j = 0; j < strlen(value_fields[i]); j++)
                        {
                            char data = value_fields[i][j];
                            send_data(serial_port, &data, sizeof(data));
                            usleep(100000);
                        }
                        write(serial_port, &enter, sizeof(enter));
                        // CHECK
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", resp);
                            if ((strchr(resp, 'K') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nBLOCK_IPv4 done\n");

                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nBLOCK_IPv4 ERROR\n");
                                break;
                            }
                            t++;
                        }
                    }
                    else if (strcmp(type_fields[i], "Source IP") == 0)
                    {
                        char keyC = 'C', keyE = 'E';
                        char key_func = strchr(value_fields[i], ':') ? keyE : keyC;
                        write(serial_port, &key_08, sizeof(key_08));
                        usleep(50000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(50000);
                        write(serial_port, &key_func, sizeof(key_func));
                        usleep(50000);
                        write(serial_port, &port_key, sizeof(port_key));
                        usleep(50000);

                        // G?i gi  tr? Source IP
                        for (int j = 0; j < strlen(value_fields[i]); j++)
                        {
                            char data = value_fields[i][j];
                            send_data(serial_port, &data, sizeof(data));
                            usleep(100000);
                        }
                        write(serial_port, &enter, sizeof(enter));
                        usleep(200000); // Th m delay nh? d? thi?t b? tr? l?i

                        // CHECK
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", resp);
                            if ((strchr(resp, 'K') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nBLOCK_IPv4 done\n");

                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nBLOCK_IPv4 ERROR\n");
                                break;
                            }
                            t++;
                        }
                    }
                    else if (strcmp(type_fields[i], "Dest Port") == 0)
                    {
                        char keyG = 'G';
                        write(serial_port, &key_08, sizeof(key_08));
                        usleep(50000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(50000);
                        write(serial_port, &keyG, sizeof(keyG));
                        usleep(50000);
                        write(serial_port, &port_key, sizeof(port_key));
                        usleep(50000);
                        for (int j = 0; j < strlen(value_fields[i]); j++)
                        {
                            char data = value_fields[i][j];
                            send_data(serial_port, &data, sizeof(data));
                            usleep(100000);
                        }
                        write(serial_port, &enter, sizeof(enter));
                        // CHECK
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", resp);
                            if ((strchr(resp, 'K') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nBLOCK_IPv4 done\n");

                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nBLOCK_IPv4 ERROR\n");
                                break;
                            }
                            t++;
                        }
                    }
                    else if (strcmp(type_fields[i], "Protocol") == 0)
                    {
                        char keyI = 'I';
                        write(serial_port, &key_08, sizeof(key_08));
                        usleep(50000);
                        write(serial_port, &enter, sizeof(enter));
                        usleep(50000);
                        write(serial_port, &keyI, sizeof(keyI));
                        usleep(50000);
                        write(serial_port, &port_key, sizeof(port_key));
                        usleep(10000);
                        char proto_key = 0;
                        if (strcmp(value_fields[i], "TCP") == 0)
                            proto_key = '1';
                        else if (strcmp(value_fields[i], "UDP") == 0)
                            proto_key = '2';
                        else if (strcmp(value_fields[i], "ICMP") == 0)
                            proto_key = '3';
                        if (proto_key)
                            write(serial_port, &proto_key, sizeof(proto_key));
                        usleep(100000);
                        write(serial_port, &enter, sizeof(enter));
                        // CHECK
                        int t = 0;
                        while (1)
                        {
                            char *resp = receive_data(serial_port);
                            printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", resp);
                            if ((strchr(resp, 'K') != NULL))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$OK$");
                                // printf("\nBLOCK_IPv4 done\n");

                                break;
                            }
                            else if ((strchr(resp, 'N') != NULL) || (t == 10))
                            {
                                strcat(buffer, ID);
                                strcat(buffer, "$");
                                strcat(buffer, Value);
                                strcat(buffer, "$ERROR$");
                                // printf("\nBLOCK_IPv4 ERROR\n");
                                break;
                            }
                            t++;
                        }
                    }
                    // Source Port: n?u c?n g?i UART, b? sung t?i d y
                }

                // Luu c?u h nh v o DB nhu cu
                mirror_type[0] = '\0';
                value[0] = '\0';
                strcat(mirror_type, "[");
                strcat(value, "{");
                for (int i = 0; i < field_count; ++i)
                {
                    strcat(mirror_type, "\"");
                    strcat(mirror_type, type_fields[i]);
                    strcat(mirror_type, "\"");
                    if (i < field_count - 1)
                        strcat(mirror_type, ",");

                    char key[32] = {0};
                    if (strcmp(type_fields[i], "Dest Mac") == 0)
                        strcpy(key, "DestMac");
                    else if (strcmp(type_fields[i], "Source Mac") == 0)
                        strcpy(key, "SourceMac");
                    else if (strcmp(type_fields[i], "Dest IP") == 0)
                        strcpy(key, strchr(value_fields[i], ':') ? "DestIPv6" : "DestIPv4");
                    else if (strcmp(type_fields[i], "Source IP") == 0)
                        strcpy(key, strchr(value_fields[i], ':') ? "SourceIPv6" : "SourceIPv4");
                    else if (strcmp(type_fields[i], "Dest Port") == 0)
                        strcpy(key, "DestPort");
                    else if (strcmp(type_fields[i], "Source Port") == 0)
                        strcpy(key, "SourcePort");
                    else if (strcmp(type_fields[i], "Protocol") == 0)
                        strcpy(key, "Protocol");
                    else
                        strcpy(key, type_fields[i]);

                    strcat(value, "\"");
                    strcat(value, key);
                    strcat(value, "\":\"");
                    strcat(value, value_fields[i]);
                    strcat(value, "\"");
                    if (i < field_count - 1)
                        strcat(value, ",");
                }
                strcat(mirror_type, "]");
                strcat(value, "}");

                strcpy(cfg->mirror_type, mirror_type);
                strcpy(cfg->value, value);

                system("clear");
                display_logo1();
                printf(C6 "\n+--------------------------------------------------------------+\n" RE);
                printf("\n" C6 "|" RE "  " C3 "Saving Packet Filtering Configuration..." RE "                   " C6 "|\n" RE);
                printf(C6 "+--------------------------------------------------------------+\n" RE);
                save_port_mirroring_to_db(cfg);
                break;
            }
            else if (choice == '9')
            {
                system("clear");
                display_logo1();
                Select_traffic_mirroring_mode(serial_port, cfg);
                return;
            }
            else
            {
                printf(C3 "Invalid selection! Please try again.\n" RE);
            }
        }

        system("clear");
        new_menu(serial_port);
    }

    void InputDestMAC(char *mac)
    {
        printf(C3 "\nEnter Destination MAC (format XX:XX:XX:XX:XX:XX): " RE);
        scanf("\n%17s", mac);
        while (!is_valid_mac_address(mac))
        {
            printf(C3 "Invalid MAC address! Please re-enter: " RE);
            scanf("%17s", mac);
        }
    }

    char InputSourceIP(char *ip)
    {
        struct in6_addr addr6;
        int valid = 0;
        char ip_type = 0;
        while (!valid)
        {
            printf(C3 "Enter Source IP (IPv4 or IPv6): " RE);
            scanf("%39s", ip);
            if (validate_ip_address(ip)) // IPv4
            {
                valid = 1;
                ip_type = '4';
            }
            else if (inet_pton(AF_INET6, ip, &addr6) == 1) // IPv6
            {
                valid = 1;
                ip_type = '6';
            }
            else
            {
                printf(C3 "Invalid IP address. Please re-enter:\n" RE);
            }
        }
        return ip_type;
    }

    void InputSourceMAC(char *mac)
    {
        printf(C3 "Enter Source MAC (format XX:XX:XX:XX:XX:XX): " RE);
        scanf("%17s", mac);
        while (!is_valid_mac_address(mac))
        {
            // printf("Invalid MAC address! Please re-enter: ");
            scanf("%17s", mac);
        }
    }
    void InputDestPort(char *port)
    {
        printf(C3 "Enter Destination Port: " RE);
        scanf("%5s", port);
    }
    void InputSourcePort(char *port)
    {
        printf(C3 "Enter Destination Port: " RE);
        scanf("%5s", port);
    }

    char InputDestIP(char *ip)
    {
        struct in6_addr addr6;
        int valid = 0;
        char ip_type = 0;
        while (!valid)
        {
            printf(C3 "Enter Destination IP (IPv4 or IPv6): " RE);
            scanf("%39s", ip);
            if (validate_ip_address(ip)) // IPv4
            {
                valid = 1;
                ip_type = '4';
            }
            else if (inet_pton(AF_INET6, ip, &addr6) == 1) // IPv6
            {
                valid = 1;
                ip_type = '6';
            }
            else
            {
                printf(C3 "Invalid IP address. Please re-enter:\n" RE);
            }
        }
        return ip_type;
    }
    void InputProtocol(int *protocol, char *protocol_str)
    {
        char choice;
        printf(C3 "Select Protocol:\n" RE);
        printf(C3 "  [1]" RE " " C6 "Any\n" RE);
        printf(C3 "  [2]" RE " " C6 "TCP\n" RE);
        printf(C3 "  [3]" RE " " C6 "UDP\n" RE);
        printf(C3 "  [4]" RE " " C6 "ICMP\n" RE);
        printf(C3 "  [5]" RE " " C6 "SCTP\n" RE);
        printf(C3 "  [6]" RE " " C6 "GRE\n" RE);
        printf(C3 "  [7]" RE " " C6 "ESP\n" RE);
        printf(C3 "  [8]" RE " " C6 "AH\n" RE);
        printf(C3 "  [9]" RE " " C6 "IPIP\n" RE);
        printf(C3 "  [A]" RE " " C6 "ICMPv6\n" RE);
        printf(C3 "  [B]" RE " " C6 "IGMP\n" RE);
        printf(C3 "  [C]" RE " " C6 "IPSec (custom)\n" RE);
        printf(C3 "  [D]" RE " " C6 "L2TP (custom)\n" RE);
        printf(C3 "  [E]" RE " " C6 "PPTP (custom)\n" RE);
        scanf(" %c", &choice);

        switch (choice)
        {
        case '1':
            *protocol = 0;
            strcpy(protocol_str, "Any");
            break;
        case '2':
            *protocol = 6;
            strcpy(protocol_str, "TCP");
            break;
        case '3':
            *protocol = 17;
            strcpy(protocol_str, "UDP");
            break;
        case '4':
            *protocol = 1;
            strcpy(protocol_str, "ICMP");
            break;
        case '5':
            *protocol = 132;
            strcpy(protocol_str, "SCTP");
            break;
        case '6':
            *protocol = 47;
            strcpy(protocol_str, "GRE");
            break;
        case '7':
            *protocol = 50;
            strcpy(protocol_str, "ESP");
            break;
        case '8':
            *protocol = 51;
            strcpy(protocol_str, "AH");
            break;
        case '9':
            *protocol = 4;
            strcpy(protocol_str, "IPIP");
            break;
        case 'A':
        case 'a':
            *protocol = 58;
            strcpy(protocol_str, "ICMPv6");
            break;
        case 'B':
        case 'b':
            *protocol = 2;
            strcpy(protocol_str, "IGMP");
            break;
        case 'C':
        case 'c':
            printf(C3 "Enter custom protocol number for IPSec: " RE);
            scanf("%d", protocol);
            strcpy(protocol_str, "IPSec");
            break;
        case 'D':
        case 'd':
            printf(C3 "Enter custom protocol number for L2TP: " RE);
            scanf("%d", protocol);
            strcpy(protocol_str, "L2TP");
            break;
        case 'E':
        case 'e':
            printf(C3 "Enter custom protocol number for PPTP: " RE);
            scanf("%d", protocol);
            strcpy(protocol_str, "PPTP");
            break;
        default:
            printf(C3 "Invalid choice. Defaulting to TCP.\n" RE);
            *protocol = 6;
            strcpy(protocol_str, "TCP");
        }
    }

    void save_port_mirroring_to_db(const PortMirroringConfig *cfg)
    {
        sqlite3 *db;
        int rc = sqlite3_open(DB_PATH, &db);
        sqlite3_busy_timeout(db, 2000);
        if (rc)
        {
            printf(C3 "Cannot open database: %s\n" RE "", sqlite3_errmsg(db));
            return;
        }

        const char *update_sql =
            "UPDATE DeviceInterfaces SET "
            "InterfaceIsMirroring=?, InterfaceToMonitorInterfaceId=?, InterfaceMirrorSetting=?, MirrorType=?, Value=? "
            "WHERE InterfaceName=?";

        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, update_sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            printf(C3 "SQL error: %s\n" RE "", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        sqlite3_bind_int(stmt, 1, cfg->is_mirroring);
        sqlite3_bind_int(stmt, 2, cfg->monitor_target_id);
        sqlite3_bind_text(stmt, 3, cfg->mirror_setting, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, cfg->mirror_type, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, cfg->value, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 6, cfg->interface_name, -1, SQLITE_STATIC);

        rc = sqlite3_step(stmt);
        int rows_affected = sqlite3_changes(db);
        sqlite3_finalize(stmt);

        if (rows_affected == 0)
        {
            // N?u chua c , th  INSERT m?i
            FILE *file = fopen(CONFIG_FILE_PORT, "w");
            if (file != NULL)
            {
                fprintf(file, "%d", current_port);
                fclose(file);
            }
        }
    }

    //////////////////////////////////////////Ð?NG B? PORT////////////////////////////////////////////////////////////////////////////
    // HÃ m gá»­i lá»‡nh SYN enable/disable
    static void send_syn_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer)
    {
        printf(C3 " SYN Enable:" RE " " C6 "%d" RE " \n", enable);
        char key = key6_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);
        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);
        if (enable == 1)
        {
            char key_y = keyY_1;
            write(serial_port, &key_y, sizeof(key_y));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        else
        {
            char key_n = keyN_1;
            write(serial_port, &key_n, sizeof(key_n));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        // CHECK
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message: %s\n" RE "", data1);
            if ((strchr(data1, 'Y') != NULL))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nSYN_EN_DIS done\n");
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 10))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nSYN_EN_DIS error\n");
                break;
            }
            t++;
        }
    }

    // HÃ m gá»­i lá»‡nh SYN threshold
    static void send_syn_threshold(int serial_port, int port, int value, int enable,
                                char *ID, char *Value, char *buffer)

    {
        // printf("  SYN Threshold: %d cho port %d\n", value, port);
        printf(C3 "  SYN Threshold:" RE " " C6 "%d\n" RE "", value);

        char key = key7_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        char value_str[16];
        snprintf(value_str, sizeof(value_str), "%d", value);
        int n = strlen(value_str);
        for (int i = 0; i < n; i++)
        {
            char data = value_str[i];
            write(serial_port, &data, sizeof(data));
            usleep(100000);
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);

        // CHECK
        int t = 0; // dùng ki?u int n?u t nh?n giá tr? t? getchar() ho?c read()
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message: %s\n" RE "", data1);

            if (strchr(data1, 'Y') != NULL)
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nACK_THR done\n");
                usleep(100000);
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 9))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nACK_THR error\n");
                break;
            }
            t++;
            usleep(100000); // ch? 100ms r?i th? d?c l?i
        }
    }

    static void send_ack_threshold(int serial_port, int port, int value, int enable,
                                char *ID, char *Value, char *buffer)
    {
        printf(C3 "   ACK Threshold: %d \n" RE "", value);

        char key = key8_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        // G?i s? port
        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        // G?i giá tr? threshold (theo t?ng ký t?)
        char value_str[16];
        snprintf(value_str, sizeof(value_str), "%d", value);
        int n = strlen(value_str);
        for (int i = 0; i < n; i++)
        {
            char data = value_str[i];
            write(serial_port, &data, sizeof(data));
            usleep(100000);
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);

        // CHECK
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message: %s\n" RE "", data1);

            if (strchr(data1, 'Y') != NULL)
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nACK_THR done\n");
                usleep(100000);
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 9))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nACK_THR error\n");
                break;
            }
            t++;
            usleep(100000); // ch? 100ms r?i th? d?c l?i
        }
    }

    // HÃ m gá»­i lá»‡nh UDP threshold
    static void send_udp_threshold(int serial_port, int port, int value, int enable,
                                char *ID, char *Value, char *buffer)
    {
        printf(C3 "  UDP Threshold: %d \n" RE "", value);

        char key = keyC_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        char value_str[16];
        snprintf(value_str, sizeof(value_str), "%d", value);
        int n = strlen(value_str);
        for (int i = 0; i < n; i++)
        {
            char data = value_str[i];
            write(serial_port, &data, sizeof(data));
            usleep(100000);
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        // === Ki?m tra ph?n h?i ===
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message: %s\n" RE "", data1);

            if (strchr(data1, 'Y') != NULL)
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nACK_THR done\n");
                usleep(100000);
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 9))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                /// printf("\nACK_THR error\n");
                break;
            }
            t++;
            usleep(100000); // ch? 100ms r?i th? d?c l?i
        }
    }

    // HÃ m gá»­i lá»‡nh UDP threshold per second
    static void send_udp_threshold_ps(int serial_port, int port, int value, int enable,
                                    char *ID, char *Value, char *buffer)

    {
        printf(C3 "  UDP Rate: %d \n" RE "", value, port);

        char key = keyD_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        char value_str[16];
        snprintf(value_str, sizeof(value_str), "%d", value);
        int n = strlen(value_str);
        for (int i = 0; i < n; i++)
        {
            char data = value_str[i];
            write(serial_port, &data, sizeof(data));
            usleep(100000);
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        // CHECK
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message: %s\n" RE "", data1);

            if (strchr(data1, 'Y') != NULL)
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nACK_THR done\n");
                usleep(100000);
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 9))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nACK_THR error\n");
                break;
            }
            t++;
            usleep(100000); // ch? 100ms r?i th? d?c l?i
        }
    }

    // HÃ m gá»­i lá»‡nh DNS threshold
    static void send_dns_threshold(int serial_port, int port, int value, int enable,
                                char *ID, char *Value, char *buffer)
    {
        printf(C3 " DNS Threshold: %d\n" RE "", value, port);

        char key = keyF_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        char value_str[16];
        snprintf(value_str, sizeof(value_str), "%d", value);
        int n = strlen(value_str);
        for (int i = 0; i < n; i++)
        {
            char data = value_str[i];
            write(serial_port, &data, sizeof(data));
            usleep(100000);
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        // CHECK
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message: %s\n" RE "", data1);

            if (strchr(data1, 'Y') != NULL)
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nACK_THR done\n");
                usleep(100000);
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 9))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nACK_THR error\n");
                break;
            }
            t++;
            usleep(100000); // ch? 100ms r?i th? ??c l?i
        }
    }

    // HÃ m gá»­i lá»‡nh ICMP threshold
    static void send_icmp_threshold(int serial_port, int port, int value, int enable, char *ID, char *Value, char *buffer)

    {
        printf(C3 "ICMP Threshold: %d\n" RE "", value, port);

        char key = keyH_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        char value_str[16];
        snprintf(value_str, sizeof(value_str), "%d", value);
        int n = strlen(value_str);
        for (int i = 0; i < n; i++)
        {
            char data = value_str[i];
            write(serial_port, &data, sizeof(data));
            usleep(100000);
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        // CHECK
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message: %s\n" RE "", data1);

            if (strchr(data1, 'Y') != NULL)
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nACK_THR done\n");
                usleep(100000);
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 9))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nACK_THR error\n");
                break;
            }
            t++;
            usleep(100000); // ch? 100ms r?i th? ??c l?i
        }
    }

    // HÃ m gá»­i lá»‡nh ICMP threshold per second
    static void send_icmp_threshold_ps(int serial_port, int port, int value, int enable,
                                    char *ID, char *Value, char *buffer)
    {
        printf(C3 "ICMP Rate: %d\n" RE "", value, port);

        char key = keyI_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        char value_str[16];
        snprintf(value_str, sizeof(value_str), "%d", value);
        int n = strlen(value_str);
        for (int i = 0; i < n; i++)
        {
            char data = value_str[i];
            write(serial_port, &data, sizeof(data));
            usleep(100000);
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        // CHECK
        int t = 0; // dùng ki?u int n?u t nh?n giá tr? t? getchar() ho?c read()
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message: %s\n" RE "", data1);
            if ((strchr(data1, 'Y') != NULL))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nSYN_EN_DIS done\n");
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 10))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nSYN_EN_DIS error\n");
                break;
            }
            t++;
        }
    }

    // HÃ m gá»­i lá»‡nh IPSec IKE threshold
    static void send_ike_threshold(int serial_port, int port, int value, int enable,
                                char *ID, char *Value, char *buffer)
    {
        printf(C3 "  IPSec IKE Threshold: %d \n" RE "", value, port);

        char key = keyK_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        char value_str[16];
        snprintf(value_str, sizeof(value_str), "%d", value);
        int n = strlen(value_str);
        for (int i = 0; i < n; i++)
        {
            char data = value_str[i];
            write(serial_port, &data, sizeof(data));
            usleep(100000);
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        // CHECK
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message: %s\n" RE "", data1);
            if ((strchr(data1, 'Y') != NULL))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nSYN_EN_DIS done\n");
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 10))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nSYN_EN_DIS error\n");
                break;
            }
            t++;
        }
    }

    // HÃ m gá»­i lá»‡nh LAND enable/disable
    static void send_land_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer)
    {
        printf(C3 "  LAND Enable: %d \n" RE "", enable, port);

        char key = keyA_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        if (enable == 1)
        {
            char key_y = keyY_1;
            write(serial_port, &key_y, sizeof(key_y));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        else
        {
            char key_n = keyN_1;
            write(serial_port, &key_n, sizeof(key_n));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        // CHECK
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", data1);
            if ((strchr(data1, 'Y') != NULL))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nSYN_EN_DIS done\n");
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 10))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nSYN_EN_DIS error\n");
                break;
            }
            t++;
        }
    }

    // HÃ m gá»­i lá»‡nh UDP enable/disable
    static void send_udp_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer)
    {
        printf(C3 " UDP Enable:" RE " " C6 "%d \n" RE "", enable, port);
        char key = keyB_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        if (enable == 1)
        {
            char key_y = keyY_1;
            write(serial_port, &key_y, sizeof(key_y));
            usleep(10000);
            write(serial_port, &ent, sizeof(ent));
        }
        else
        {
            char key_n = keyN_1;
            write(serial_port, &key_n, sizeof(key_n));
            usleep(10000);
            write(serial_port, &ent, sizeof(ent));
        }
        usleep(1000000);
        // CHECK
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", data1);
            if ((strchr(data1, 'Y') != NULL))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nSYN_EN_DIS done\n");
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 10))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nSYN_EN_DIS error\n");
                break;
            }
            t++;
        }
    }

    // HÃ m gá»­i lá»‡nh DNS enable/disable
    static void send_dns_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer)
    {
        printf(C3 " DNS Enable:" RE " " C6 "%d\n" RE "", enable, port);

        char key = keyE_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        if (enable == 1)
        {
            char key_y = keyY_1;
            write(serial_port, &key_y, sizeof(key_y));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        else
        {
            char key_n = keyN_1;
            write(serial_port, &key_n, sizeof(key_n));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        // CHECK
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", data1);
            if ((strchr(data1, 'Y') != NULL))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nSYN_EN_DIS done\n");
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 10))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nSYN_EN_DIS error\n");
                break;
            }
            t++;
        }
    }

    // HÃ m gá»­i lá»‡nh ICMP enable/disable
    static void send_icmp_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer)
    {
        printf(C3 " ICMP Enable:" RE " " C6 "%d \n" RE "", enable, port);

        char key = keyG_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        if (enable == 1)
        {
            char key_y = keyY_1;
            write(serial_port, &key_y, sizeof(key_y));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        else
        {
            char key_n = keyN_1;
            write(serial_port, &key_n, sizeof(key_n));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        // CHECK
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", data1);
            if ((strchr(data1, 'Y') != NULL))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nSYN_EN_DIS done\n");
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 10))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nSYN_EN_DIS error\n");
                break;
            }
            t++;
        }
    }

    // HÃ m gá»­i lá»‡nh IPSec IKE enable/disable
    static void send_ike_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer)
    {
        printf(C3 " IPSec IKE Enable: %d\n" RE "", enable, port);

        char key = keyJ_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        if (enable == 1)
        {
            char key_y = keyY_1;
            write(serial_port, &key_y, sizeof(key_y));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        else
        {
            char key_n = keyN_1;
            write(serial_port, &key_n, sizeof(key_n));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        // CHECK
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", data1);
            if ((strchr(data1, 'Y') != NULL))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nSYN_EN_DIS done\n");
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 10))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nSYN_EN_DIS error\n");
                break;
            }
            t++;
        }
    }

    // HÃ m gá»­i lá»‡nh TCP Fragment enable/disable
    static void send_tcpfrag_enable_disable(int serial_port, int port, int value, int enable,
                                            char *ID, char *Value, char *buffer)
    {
        printf(C3 "TCP Fragment Enable:" RE " " C6 "%d \n" RE "", enable, port);

        char key = keyN_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        if (enable == 1)
        {
            char key_y = keyY_1;
            write(serial_port, &key_y, sizeof(key_y));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        else
        {
            char key_n = keyN_1;
            write(serial_port, &key_n, sizeof(key_n));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        // CHECK
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", data1);
            if ((strchr(data1, 'Y') != NULL))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nSYN_EN_DIS done\n");
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 10))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nSYN_EN_DIS error\n");
                break;
            }
            t++;
        }
    }

    // HÃ m gá»­i lá»‡nh UDP Fragment enable/disable
    static void send_udpfrag_enable_disable(int serial_port, int port, int value, int enable,
                                            char *ID, char *Value, char *buffer)
    {
        printf(C3 " UDP Fragment Enable:" RE " " C6 "%d \n" RE "", enable, port);

        char key = keyO_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        if (enable == 1)
        {
            char key_y = keyY_1;
            write(serial_port, &key_y, sizeof(key_y));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        else
        {
            char key_n = keyN_1;
            write(serial_port, &key_n, sizeof(key_n));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        // CHECK
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", data1);
            if ((strchr(data1, 'Y') != NULL))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nSYN_EN_DIS done\n");
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 10))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nSYN_EN_DIS error\n");
                break;
            }
            t++;
        }
    }

    // HÃ m gá»­i lá»‡nh HTTP enable/disable
    static void send_http_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer)
    {
        printf(C3 " HTTP Enable:" RE " " C6 "%d \n" RE "", enable, port);

        char key = keyP_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        if (enable == 1)
        {
            char key_y = keyY_1;
            write(serial_port, &key_y, sizeof(key_y));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        else
        {
            char key_n = keyN_1;
            write(serial_port, &key_n, sizeof(key_n));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        // CHECK
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", data1);
            if ((strchr(data1, 'Y') != NULL))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nSYN_EN_DIS done\n");
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 10))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nSYN_EN_DIS error\n");
                break;
            }
            t++;
        }
    }

    // HÃ m gá»­i lá»‡nh HTTPS enable/disable
    static void send_https_enable_disable(int serial_port, int port, int value, int enable,
                                        char *ID, char *Value, char *buffer)
    {
        printf(C3 " HTTPS Enable:" RE " " C6 "%d \n" RE "", enable, port);

        char key_mode = keyFF_1;
        write(serial_port, &key_mode, sizeof(key_mode));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char port_char = '1' + (port - 1);
        write(serial_port, &port_char, sizeof(port_char));
        usleep(100000);

        if (enable == 1)
        {
            char key_y = keyY_1;
            write(serial_port, &key_y, sizeof(key_y));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        else
        {
            char key_n = keyN_1;
            write(serial_port, &key_n, sizeof(key_n));
            usleep(1000000);
            write(serial_port, &ent, sizeof(ent));
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        // CHECK
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message:" RE " " C6 "%s\n" RE "", data1);
            if ((strchr(data1, 'Y') != NULL))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nSYN_EN_DIS done\n");
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 10))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nSYN_EN_DIS error\n");
                break;
            }
            t++;
        }
    }

    // HÃ m gá»­i lá»‡nh Whitelist timeout
    static void send_whitelist_timeout(int serial_port, int port, int value, int enable,
                                    char *ID, char *Value, char *buffer)
    {
        printf(C3 "  Whitelist Timeout:" RE " " C6 "%d \n" RE "", value, port);

        char key = key9_1;
        write(serial_port, &key, sizeof(key));
        usleep(100000);
        char ent = enter_1;
        write(serial_port, &ent, sizeof(ent));
        usleep(100000);

        char value_str[16];
        snprintf(value_str, sizeof(value_str), "%d", value);
        int n = strlen(value_str);
        for (int i = 0; i < n; i++)
        {
            char data = value_str[i];
            write(serial_port, &data, sizeof(data));
            usleep(100000);
        }
        write(serial_port, &ent, sizeof(ent));
        usleep(1000000);
        int t = 0;
        while (1)
        {
            char *data1 = receive_data(serial_port);
            printf(C3 "\nReceived message: %s\n" RE "", data1);
            if ((strchr(data1, 'Y') != NULL))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$OK$");
                // printf("\nSYN_EN_DIS done\n");
                break;
            }
            else if ((strchr(data1, 'N') != NULL) || (t == 10))
            {
                strcat(buffer, ID);
                strcat(buffer, "$");
                strcat(buffer, Value);
                strcat(buffer, "$ERROR$");
                // printf("\nSYN_EN_DIS error\n");
                break;
            }
            t++;
        }
    }

    // // HÃ m Ä‘á»“ng bá»™ 1 port tá»« database vÃ  gá»­i lá»‡nh write()
    // static void dong_bo_port_write(int serial_port, int port)
    // {
    //     sqlite3 *db = NULL;
    //     sqlite3_stmt *st = NULL;
    //     char latest_time[32] = "";
    //     char latest_profile_name[64] = "";
    //     int latest_rowid = -1;

    //     if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
    //     {
    //         fprintf(stderr, "" C3 "Cannot open DB\n" RE);
    //         return;
    //     }

    //     char target_iface[8];
    //     sprintf(target_iface, "eth%d", port);

    //     // LÃ¡ÂºÂ¥y tÃ¡ÂºÂ¥t cÃ¡ÂºÂ£ row cÃƒÂ³ DefenseProfileUsingTime != NULL
    //     const char *sql = "SELECT rowid, DefenseProfileUsingTime, DefenseProfileName FROM DefenseProfiles WHERE DefenseProfileUsingTime IS NOT NULL";

    //     if (sqlite3_prepare_v2(db, sql, -1, &st, NULL) == SQLITE_OK)
    //     {
    //         while (sqlite3_step(st) == SQLITE_ROW)
    //         {
    //             int rowid = sqlite3_column_int(st, 0);
    //             const char *json_text = (const char *)sqlite3_column_text(st, 1);
    //             const char *profile_name = (const char *)sqlite3_column_text(st, 2);

    //             if (!json_text || !profile_name)
    //                 continue;

    //             cJSON *root_json = cJSON_Parse(json_text);
    //             if (!root_json || !cJSON_IsArray(root_json))
    //             {
    //                 if (root_json)
    //                     cJSON_Delete(root_json);
    //                 continue;
    //             }

    //             int n = cJSON_GetArraySize(root_json);
    //             for (int i = 0; i < n; i++)
    //             {
    //                 cJSON *e = cJSON_GetArrayItem(root_json, i);
    //                 cJSON *iface_obj = cJSON_GetObjectItem(e, "name");
    //                 cJSON *start_obj = cJSON_GetObjectItem(e, "date");
    //                 const char *iface = (iface_obj && cJSON_IsString(iface_obj)) ? iface_obj->valuestring : NULL;
    //                 const char *start = (start_obj && cJSON_IsString(start_obj)) ? start_obj->valuestring : NULL;

    //                 if (iface && start && strcmp(iface, target_iface) == 0)
    //                 {
    //                     if (latest_time[0] == 0 || strcmp(start, latest_time) > 0)
    //                     {
    //                         strcpy(latest_time, start);
    //                         strcpy(latest_profile_name, profile_name);
    //                         latest_rowid = rowid;
    //                     }
    //                 }
    //             }
    //             cJSON_Delete(root_json);
    //         }
    //     }
    //     sqlite3_finalize(st);
    //     // TÃ¡ÂºÂ¡o JSON gÃ¡Â»Â­i UART
    //     cJSON *root = cJSON_CreateObject();
    //     cJSON_AddStringToObject(root, "port", target_iface);
    //     cJSON_AddStringToObject(root, "DefenseProfileName", latest_profile_name);
    //     cJSON_AddStringToObject(root, "LatestStartTime", latest_time); // <-- thÃƒÂªm mÃ¡Â»â€˜c thÃ¡Â»Âi gian mÃ¡Â»â€ºi nhÃ¡ÂºÂ¥t

    //     printf(C3 "Port %d: Final selection - Profile: %s, Time: %s\n" RE "", port, latest_profile_name, latest_time);

    //     // Náº¿u tÃ¬m tháº¥y profile, Ä‘á»c cáº¥u hÃ¬nh vÃ  gá»­i lá»‡nh
    //     if (latest_rowid != -1)
    //     {
    //         printf(C3 "Port %d (eth%d) is using profile: %s\n" RE "", port, port, latest_profile_name);
    //         printf(C3 "Port %d: Syncing configuration...\n" RE "", port);

    //         const char *query =
    //             "SELECT "
    //             "SYNFloodSYNThreshold, SYNFloodACKThreshold, "
    //             "UDPFloodThreshold, UDPFloodRate, "
    //             "DNSFloodThreshold, "
    //             "ICMPFloodThreshold, ICMPFloodRate, "
    //             "IPSecIKEThreshold, "
    //             "LANDAttackEnable, UDPFloodEnable, DNSFloodEnable, ICMPFloodEnable, IPSecIKEEnable, "
    //             "TCPFragmentEnable, UDPFragmentEnable, HTTPFloodEnable, HTTPSFloodEnable, "
    //             "SYNFloodWhiteListTimeOut, SYNFloodEnable "
    //             "FROM DefenseProfiles WHERE rowid = ?";

    //         if (sqlite3_prepare_v2(db, query, -1, &st, NULL) == SQLITE_OK)
    //         {
    //             sqlite3_bind_int(st, 1, latest_rowid);
    //             if (sqlite3_step(st) == SQLITE_ROW)
    //             {
    //                 int i = 0;
    //                 int syn_syn = sqlite3_column_int(st, i++);
    //                 int syn_ack = sqlite3_column_int(st, i++);
    //                 int udp_thr = sqlite3_column_int(st, i++);
    //                 int udp_rate = sqlite3_column_int(st, i++);
    //                 int dns_thr = sqlite3_column_int(st, i++);
    //                 int icmp_thr = sqlite3_column_int(st, i++);
    //                 int icmp_rate = sqlite3_column_int(st, i++);
    //                 int ike_thr = sqlite3_column_int(st, i++);
    //                 int land_en = sqlite3_column_int(st, i++);
    //                 int udp_en = sqlite3_column_int(st, i++);
    //                 int dns_en = sqlite3_column_int(st, i++);
    //                 int icmp_en = sqlite3_column_int(st, i++);
    //                 int ike_en = sqlite3_column_int(st, i++);
    //                 int tcpfrag = sqlite3_column_int(st, i++);
    //                 int udpfrag = sqlite3_column_int(st, i++);
    //                 int http_en = sqlite3_column_int(st, i++);
    //                 int https_en = sqlite3_column_int(st, i++);
    //                 int wl_timeout = sqlite3_column_int(st, i++);
    //                 int syn_en = sqlite3_column_int(st, i++); // Láº¥y giÃ¡ trá»‹ SYNFloodEnable

    //                 // Khai báo bi?n buffer, ID, Value tru?c khi s? d?ng trong dong_bo_port_write
    //                 char buffer[256] = "";
    //                 char ID[32] = "";
    //                 char Value[64] = "";
    //                 int t = 0;
    //                 // G?i SYN enable/disable
    //                 printf(C3 "  - SYN Enable: %d\n" RE "", syn_en);
    //                 send_syn_enable_disable(serial_port, port, syn_en, syn_en, ID, Value, buffer);

    //                 // G?i t?t c? threshold/rate
    //                 printf(C3 "  - SYN Threshold: %d\n" RE "", syn_syn);
    //                 send_syn_threshold(serial_port, port, syn_syn, syn_en, ID, Value, buffer);

    //                 printf(C3 "  - ACK Threshold: %d\n" RE "", syn_ack);
    //                 send_ack_threshold(serial_port, port, syn_ack, syn_en, ID, Value, buffer);

    //                 printf(C3 "  - UDP Threshold: %d\n" RE "", udp_thr);
    //                 send_udp_threshold(serial_port, port, udp_thr, udp_en, ID, Value, buffer);

    //                 printf(C3 "  - UDP Rate: %d\n" RE "", udp_rate);
    //                 send_udp_threshold_ps(serial_port, port, udp_rate, udp_en, ID, Value, buffer);

    //                 printf(C3 "  - DNS Threshold: %d\n" RE "", dns_thr);
    //                 send_dns_threshold(serial_port, port, dns_thr, dns_en, ID, Value, buffer);

    //                 printf(C3 "  - ICMP Threshold: %d\n" RE "", icmp_thr);
    //                 send_icmp_threshold(serial_port, port, icmp_thr, icmp_en, ID, Value, buffer);

    //                 printf(C3 "  - ICMP Rate: %d\n" RE "", icmp_rate);
    //                 send_icmp_threshold_ps(serial_port, port, icmp_rate, icmp_en, ID, Value, buffer);

    //                 printf(C3 "  - IPSec IKE Threshold: %d\n" RE "", ike_thr);
    //                 send_ike_threshold(serial_port, port, ike_thr, ike_en, ID, Value, buffer);

    //                 // G?i enable/disable luôn, b?t k? giá tr? 0 hay 1
    //                 printf(C3 "  - LAND Enable: %d\n" RE "", land_en);
    //                 send_land_enable_disable(serial_port, port, land_en, land_en, ID, Value, buffer);

    //                 printf(C3 "  - UDP Enable: %d\n" RE "", udp_en);
    //                 send_udp_enable_disable(serial_port, port, udp_en, udp_en, ID, Value, buffer);

    //                 printf(C3 "  - DNS Enable: %d\n" RE "", dns_en);
    //                 send_dns_enable_disable(serial_port, port, dns_en, dns_en, ID, Value, buffer);

    //                 printf(C3 "  - ICMP Enable: %d\n" RE "", icmp_en);
    //                 send_icmp_enable_disable(serial_port, port, icmp_en, icmp_en, ID, Value, buffer);

    //                 printf(C3 "  - IPSec IKE Enable: %d\n" RE "", ike_en);
    //                 send_ike_enable_disable(serial_port, port, ike_en, ike_en, ID, Value, buffer);

    //                 printf(C3 "  - TCP Fragment Enable: %d\n" RE "", tcpfrag);
    //                 send_tcpfrag_enable_disable(serial_port, port, tcpfrag, tcpfrag, ID, Value, buffer);

    //                 printf(C3 "  - UDP Fragment Enable: %d\n" RE "", udpfrag);
    //                 send_udpfrag_enable_disable(serial_port, port, udpfrag, udpfrag, ID, Value, buffer);

    //                 printf(C3 "  - HTTP Enable: %d\n" RE "", http_en);
    //                 send_http_enable_disable(serial_port, port, http_en, http_en, ID, Value, buffer);

    //                 printf(C3 "  - HTTPS Enable: %d\n" RE "", https_en);
    //                 send_https_enable_disable(serial_port, port, https_en, https_en, ID, Value, buffer);

    //                 // G?i whitelist timeout luôn
    //                 printf(C3 "  - Whitelist Timeout: %d\n" RE "", wl_timeout);
    //                 send_whitelist_timeout(serial_port, port, wl_timeout, wl_timeout, ID, Value, buffer);

    //                 printf(C3 "Port %d: Sync done!\n" RE "", port);
    //             }
    //               sqlite3_finalize(st);
    //         }
    //          cJSON_Delete(root);
    //     }

    //     else
    //     {
    //         printf(C3 "Port %d (eth%d): No configuration found for this port. Skipping.\n" RE "", port, port);
    //     }

    //     sqlite3_close(db);
    // }

    static void dong_bo_port_write(int serial_port, int port)
    {
        sqlite3 *db = NULL;
        sqlite3_stmt *st = NULL;
        char latest_time[32] = "";
        char latest_profile_name[64] = "";
        int latest_rowid = -1;

        if (sqlite3_open(DB_PATH, &db) != SQLITE_OK)
        {
            fprintf(stderr, "" C3 "Cannot open DB\n" RE);
            return;
        }

        char target_iface[8];
        sprintf(target_iface, "eth%d", port);

        const char *sql = "SELECT rowid, DefenseProfileUsingTime, DefenseProfileName FROM DefenseProfiles WHERE DefenseProfileUsingTime IS NOT NULL";

        if (sqlite3_prepare_v2(db, sql, -1, &st, NULL) == SQLITE_OK)
        {
            while (sqlite3_step(st) == SQLITE_ROW)
            {
                int rowid = sqlite3_column_int(st, 0);
                const char *json_text = (const char *)sqlite3_column_text(st, 1);
                const char *profile_name = (const char *)sqlite3_column_text(st, 2);

                if (!json_text || !profile_name)
                    continue;

                cJSON *root_json = cJSON_Parse(json_text);
                if (!root_json || !cJSON_IsArray(root_json))
                {
                    if (root_json)
                        cJSON_Delete(root_json);
                    continue;
                }

                int n = cJSON_GetArraySize(root_json);
                for (int i = 0; i < n; i++)
                {
                    cJSON *e = cJSON_GetArrayItem(root_json, i);
                    cJSON *iface_obj = cJSON_GetObjectItem(e, "name");
                    cJSON *start_obj = cJSON_GetObjectItem(e, "date");
                    const char *iface = (iface_obj && cJSON_IsString(iface_obj)) ? iface_obj->valuestring : NULL;
                    const char *start = (start_obj && cJSON_IsString(start_obj)) ? start_obj->valuestring : NULL;

                    if (iface && start && strcmp(iface, target_iface) == 0)
                    {
                        if (latest_time[0] == 0 || strcmp(start, latest_time) > 0)
                        {
                            strcpy(latest_time, start);
                            strcpy(latest_profile_name, profile_name);
                            latest_rowid = rowid;
                        }
                    }
                }
                cJSON_Delete(root_json);
            }
        }
        sqlite3_finalize(st);

        if (latest_rowid != -1)
        {
            // T?o JSON và g?i UART
            cJSON *root = cJSON_CreateObject();
            cJSON_AddStringToObject(root, "port", target_iface);
            cJSON_AddStringToObject(root, "DefenseProfileName", latest_profile_name);
            cJSON_AddStringToObject(root, "LatestStartTime", latest_time);

            printf(C3 "Port %d: Final selection - Profile: %s, Time: %s\n" RE "", port, latest_profile_name, latest_time);

            // Náº¿u tÃ¬m tháº¥y profile, Ä‘á»c cáº¥u hÃ¬nh vÃ  gá»­i lá»‡nh
            if (latest_rowid != -1)
            {
                printf(C3 "Port %d (eth%d) is using profile: %s\n" RE "", port, port, latest_profile_name);
                printf(C3 "Port %d: Syncing configuration...\n" RE "", port);

                const char *query =
                    "SELECT SYNFloodSYNThreshold, SYNFloodACKThreshold, UDPFloodThreshold, UDPFloodRate, "
                    "DNSFloodThreshold, ICMPFloodThreshold, ICMPFloodRate, IPSecIKEThreshold, "
                    "LANDAttackEnable, UDPFloodEnable, DNSFloodEnable, ICMPFloodEnable, IPSecIKEEnable, "
                    "TCPFragmentEnable, UDPFragmentEnable, HTTPFloodEnable, HTTPSFloodEnable, "
                    "SYNFloodWhiteListTimeOut, SYNFloodEnable "
                    "FROM DefenseProfiles WHERE rowid = ?";

                if (sqlite3_prepare_v2(db, query, -1, &st, NULL) == SQLITE_OK)
                {
                    sqlite3_bind_int(st, 1, latest_rowid);
                    if (sqlite3_step(st) == SQLITE_ROW)
                    {
                        int i = 0;
                        int syn_syn = sqlite3_column_int(st, i++);
                        int syn_ack = sqlite3_column_int(st, i++);
                        int udp_thr = sqlite3_column_int(st, i++);
                        int udp_rate = sqlite3_column_int(st, i++);
                        int dns_thr = sqlite3_column_int(st, i++);
                        int icmp_thr = sqlite3_column_int(st, i++);
                        int icmp_rate = sqlite3_column_int(st, i++);
                        int ike_thr = sqlite3_column_int(st, i++);
                        int land_en = sqlite3_column_int(st, i++);
                        int udp_en = sqlite3_column_int(st, i++);
                        int dns_en = sqlite3_column_int(st, i++);
                        int icmp_en = sqlite3_column_int(st, i++);
                        int ike_en = sqlite3_column_int(st, i++);
                        int tcpfrag = sqlite3_column_int(st, i++);
                        int udpfrag = sqlite3_column_int(st, i++);
                        int http_en = sqlite3_column_int(st, i++);
                        int https_en = sqlite3_column_int(st, i++);
                        int wl_timeout = sqlite3_column_int(st, i++);
                        int syn_en = sqlite3_column_int(st, i++);

                        // char buffer[256] = "";
                        // char ID[32] = "";
                        // char Value[64] = "";

                        // send_syn_enable_disable(serial_port, port, syn_en, syn_en, ID, Value, buffer);
                        // send_syn_threshold(serial_port, port, syn_syn, syn_en, ID, Value, buffer);
                        // send_ack_threshold(serial_port, port, syn_ack, syn_en, ID, Value, buffer);
                        // send_udp_threshold(serial_port, port, udp_thr, udp_en, ID, Value, buffer);
                        // send_udp_threshold_ps(serial_port, port, udp_rate, udp_en, ID, Value, buffer);
                        // send_dns_threshold(serial_port, port, dns_thr, dns_en, ID, Value, buffer);
                        // send_icmp_threshold(serial_port, port, icmp_thr, icmp_en, ID, Value, buffer);
                        // send_icmp_threshold_ps(serial_port, port, icmp_rate, icmp_en, ID, Value, buffer);
                        // send_ike_threshold(serial_port, port, ike_thr, ike_en, ID, Value, buffer);
                        // send_land_enable_disable(serial_port, port, land_en, land_en, ID, Value, buffer);
                        // send_udp_enable_disable(serial_port, port, udp_en, udp_en, ID, Value, buffer);
                        // send_dns_enable_disable(serial_port, port, dns_en, dns_en, ID, Value, buffer);
                        // send_icmp_enable_disable(serial_port, port, icmp_en, icmp_en, ID, Value, buffer);
                        // send_ike_enable_disable(serial_port, port, ike_en, ike_en, ID, Value, buffer);
                        // send_tcpfrag_enable_disable(serial_port, port, tcpfrag, tcpfrag, ID, Value, buffer);
                        // send_udpfrag_enable_disable(serial_port, port, udpfrag, udpfrag, ID, Value, buffer);
                        // send_http_enable_disable(serial_port, port, http_en, http_en, ID, Value, buffer);
                        // send_https_enable_disable(serial_port, port, https_en, https_en, ID, Value, buffer);
                        // send_whitelist_timeout(serial_port, port, wl_timeout, wl_timeout, ID, Value, buffer);
                        // Khai báo bi?n buffer, ID, Value tru?c khi s? d?ng trong dong_bo_port_write
                        char buffer[256] = "";
                        char ID[32] = "";
                        char Value[64] = "";
                        int t = 0;
                        // G?i SYN enable/disable
                        printf(C3 "  - SYN Enable: %d\n" RE "", syn_en);
                        send_syn_enable_disable(serial_port, port, syn_en, syn_en, ID, Value, buffer);

                        // G?i t?t c? threshold/rate
                        printf(C3 "  - SYN Threshold: %d\n" RE "", syn_syn);
                        send_syn_threshold(serial_port, port, syn_syn, syn_en, ID, Value, buffer);

                        printf(C3 "  - ACK Threshold: %d\n" RE "", syn_ack);
                        send_ack_threshold(serial_port, port, syn_ack, syn_en, ID, Value, buffer);

                        printf(C3 "  - UDP Threshold: %d\n" RE "", udp_thr);
                        send_udp_threshold(serial_port, port, udp_thr, udp_en, ID, Value, buffer);

                        printf(C3 "  - UDP Rate: %d\n" RE "", udp_rate);
                        send_udp_threshold_ps(serial_port, port, udp_rate, udp_en, ID, Value, buffer);

                        printf(C3 "  - DNS Threshold: %d\n" RE "", dns_thr);
                        send_dns_threshold(serial_port, port, dns_thr, dns_en, ID, Value, buffer);

                        printf(C3 "  - ICMP Threshold: %d\n" RE "", icmp_thr);
                        send_icmp_threshold(serial_port, port, icmp_thr, icmp_en, ID, Value, buffer);

                        printf(C3 "  - ICMP Rate: %d\n" RE "", icmp_rate);
                        send_icmp_threshold_ps(serial_port, port, icmp_rate, icmp_en, ID, Value, buffer);

                        printf(C3 "  - IPSec IKE Threshold: %d\n" RE "", ike_thr);
                        send_ike_threshold(serial_port, port, ike_thr, ike_en, ID, Value, buffer);

                        // G?i enable/disable luôn, b?t k? giá tr? 0 hay 1
                        printf(C3 "  - LAND Enable: %d\n" RE "", land_en);
                        send_land_enable_disable(serial_port, port, land_en, land_en, ID, Value, buffer);

                        printf(C3 "  - UDP Enable: %d\n" RE "", udp_en);
                        send_udp_enable_disable(serial_port, port, udp_en, udp_en, ID, Value, buffer);

                        printf(C3 "  - DNS Enable: %d\n" RE "", dns_en);
                        send_dns_enable_disable(serial_port, port, dns_en, dns_en, ID, Value, buffer);

                        printf(C3 "  - ICMP Enable: %d\n" RE "", icmp_en);
                        send_icmp_enable_disable(serial_port, port, icmp_en, icmp_en, ID, Value, buffer);

                        printf(C3 "  - IPSec IKE Enable: %d\n" RE "", ike_en);
                        send_ike_enable_disable(serial_port, port, ike_en, ike_en, ID, Value, buffer);

                        printf(C3 "  - TCP Fragment Enable: %d\n" RE "", tcpfrag);
                        send_tcpfrag_enable_disable(serial_port, port, tcpfrag, tcpfrag, ID, Value, buffer);

                        printf(C3 "  - UDP Fragment Enable: %d\n" RE "", udpfrag);
                        send_udpfrag_enable_disable(serial_port, port, udpfrag, udpfrag, ID, Value, buffer);

                        printf(C3 "  - HTTP Enable: %d\n" RE "", http_en);
                        send_http_enable_disable(serial_port, port, http_en, http_en, ID, Value, buffer);

                        printf(C3 "  - HTTPS Enable: %d\n" RE "", https_en);
                        send_https_enable_disable(serial_port, port, https_en, https_en, ID, Value, buffer);

                        // G?i whitelist timeout luôn
                        printf(C3 "  - Whitelist Timeout: %d\n" RE "", wl_timeout);
                        send_whitelist_timeout(serial_port, port, wl_timeout, wl_timeout, ID, Value, buffer);

                        printf(C3 "Port %d: Sync done!\n" RE "", port);

                        printf(C3 "Port %d: Sync done!\n" RE, port);
                    }
                    sqlite3_finalize(st);
                }
                cJSON_Delete(root);
            }
            else
            {
                printf(C3 "Port %d (eth%d): No configuration found for this port. Skipping.\n" RE "", port, port);
            }
        }

        sqlite3_close(db);
    }
    // HÃ m hiá»ƒn thá»‹ progress bar
    void progress_bar(int percent, int width)
    {
        int step = (percent * width) / 100;
        printf(C3 "\033[2K\r[" RE);
        for (int i = 0; i < width; i++)
        {
            if (i < step)
                printf(C3 "#" RE);
            else
                printf(C3 " " RE);
        }
        printf(C3 "]" RE " " C6 "%d%%\r" RE "", percent);
        fflush(stdout);
    }

    // HÃ m Ä‘á»“ng bá»™ táº¥t cáº£ 4 port
    void dong_bo_all_ports_write(int serial_port)
    {
        printf(C3 "=== STARTING CONFIGURATION SYNC FOR 4 PORTS ===\n" RE);
        int total_ports = 4;
        int progress_percent[4] = {30, 50, 80, 100};
        int width = 40;
        for (int port = 1; port <= total_ports; port++)
        {
            printf(C3 "\n--- SYNCING PORT %d ---\n" RE "", port);
            dong_bo_port_write(serial_port, port);
            sleep(1);
        }
        for (int i = 0; i < 4; i++)
        {
            progress_bar(progress_percent[i], width);
            sleep(1);
        }
        printf(C3 "\nDone!\n" RE);
        printf(C3 "\n=== CONFIGURATION SYNC COMPLETED FOR ALL 4 PORTS ===\n" RE);
    }

    void load_port_config()
    {
        FILE *file = fopen(CONFIG_FILE, "r");
        if (file != NULL)
        {
            fscanf(file, "%d", &current_port);
            fclose(file);
        }
    }
    void cleanup_and_exit(int code)
    {
        flush_batch_to_file(LOGFILE_HTTP_IPv4);
        flush_batch_to_file(LOGFILE_HTTP_IPv6);
        if (batch_queue)
            g_queue_free(batch_queue);
        if (ip_table)
            g_hash_table_destroy(ip_table);
        if (serial_port > 0)
            close(serial_port);
        exit(code);
    }
    void clear_input()
    {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
    }

    // main C

    int main()
    {

        serial_port = configure_serial_port("/dev/ttyUSB0", B115200);
        // serial_port = configure_serial_port("/dev/pts/4", B115200);
        //  filepath: f:\DDoS_github\cli_working.c
        // serial_port = configure_serial_port("/dev/pts/23", B115200);
        if (serial_port < 0)
        {
            printf(C3 "Failed to open serial port\n" RE);
            exit(1);
        }
        // ...existing code...
        signal(SIGINT, handle_signal);  // Ctrl+C
        signal(SIGTERM, handle_signal); // kill
        signal(SIGTSTP, handle_signal); // Ctrl+Z
        // signal(SIGINT, handle_signal);
        // SIGTSTP
        signal(SIGTSTP, handle_signal);
        /******************************************************************/
        read_config_mode_save_logfile();
        read_threshold_from_file();
        read_threshold_timecounter_from_file();
        // load_port_config();  // Load saved port configuration
        // create_http_filelog(LOGFILE_HTTP_IPv4);
        // create_http_filelog(LOGFILE_HTTP_IPv6);
        ip_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        batch_queue = g_queue_new();
        // load_ips_from_file(LOGFILE_HTTP_IPv4);
        // load_ips_from_file(LOGFILE_HTTP_IPv6);
        // send_http_ipv4_start(serial_port, LOGFILE_HTTP_IPv4);
        sleep(1);
        // send_http_ipv6_start(serial_port, LOGFILE_HTTP_IPv6);
        // dong_bo_all_ports_write(serial_port);
        previous_mode_fc();
        // bom 5 l?n m?i lo?i

        /******************************************************************/
        pthread_mutex_lock(&run_mutex);
        if (!is_run_running)
        {
            if (pthread_create(&run_thread, NULL, run, NULL) != 0)
            {
                perror(C3 "pthread_create" RE);
                is_run_running = true;
                exit(1);
            }
        }
        pthread_mutex_unlock(&run_mutex);
        /******************************************************************/
        printf(C3 "\n 10" RE);
        sleep(2);
        // Sync Time
        // send_data_sync_time(serial_port);
        // dong_bo_all_ports_write(serial_port);
        ModeStart_cnt(serial_port);
        // G?i h m test

        // start_packet_test(10);

        // dong_bo_all_ports_write(serial_port);
        // new_menu(serial_port);
        //  Mode_Condition_SDCard_Admin(serial_port);

        flush_batch_to_file(LOGFILE_HTTP_IPv4);
        flush_batch_to_file(LOGFILE_HTTP_IPv6);
        g_queue_free(batch_queue);
        g_hash_table_destroy(ip_table);
        if (pthread_join(run_thread, NULL) != 0)
        {
            perror(C3 "pthread_join" RE);
            exit(1);
        }
        close(serial_port);
        return 0;
    }
