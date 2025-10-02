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
#include <sys/un.h>
#include <curl/curl.h>
#include <glib.h>

/*Library wolfssl*/
#include <wolfssl/options.h>
// #include <config.h>
#include <wolfssl/ssl.h> /* name change portability layer */
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/ecc.h> /* ecc_fp_free */

// #include "cmsis_os.h"
//  #include "rl_fs.h"
//  #include "rl_net.h"
//  #include "wolfssl_MDK_ARM.h"

#include <wolfssl/ssl.h>
#include <wolfssl/test.h>

#ifndef NO_MAIN_DRIVER
#define ECHO_OUT
#endif

#include <wolfssl/options.h>
#include <math.h>

#define SVR_COMMAND_SIZE 256

/*===============================================*/
int luong = 0;
// #define NUM_THREADS 4
// char test[200];
struct timespec start, end;
double elapsed;
struct processing_table
{
  char initial_timestamp[10];
  char ip_client_addr[50];
  char uri_request_firt[200];
  char uri_redirect[200];
  char verify[10];
  struct flow *next;
};
struct processing_table *processing_table_0 = NULL;

struct whitelist_table
{
  char last_timestamp[10];
  char ip_legit_client_addr[50];
  char uri_legit[200];
  struct legit_connection *next;
};
struct whitelist_table *whitelist_table_0 = NULL;

struct blacklist_table
{
  char timestamp[10];
  char ip_block_client_addr[50];
  char uri_block[200];
  unsigned int packet_timer;
  struct block_connection *next;
};
struct blacklist_table *blacklist_table_0 = NULL;

/*=================== Khu vuc xu ly HTTP ===============================*/
unsigned int *Attacker_List;
int Num_of_IP = 300;
int Num_of_URL = 100;
int Num_in_PAL = 100;
time_t last_time;
#define IP_TIMEOUT 1
struct IP_Connection
{
  bool b_flag; // busy flag
  unsigned int src_ip;
  unsigned int timer;
};
struct IP_Connection *IP_Conn_Table;

struct Potential_IP_List
{
  bool b_flag; // busy flag
  unsigned int src_ip_1;
  unsigned int pkt_counter_1;
  // int src_ip_2;
  // unsigned int pkt_counter_2;
};

struct URL_Connection
{
  bool b_flag; // busy flag
  int index;
  unsigned long hash_url;
  unsigned int request_counter;
  struct Potential_IP_List *PAL;
};
struct URL_Connection *URL_Conn_Table;

struct target_URL
{
  unsigned long url;
  int index;
  int max_URL_cnt;
  int num_URLs;

} Atk_URL;

struct Layer7_Parameter
{
  unsigned int connected_IP;
  unsigned long attacked_URL;
  unsigned int just_detected_IP;
} Atk_HTTP;

/*===============================================================*/
static const byte cert_server_pem[] = "\
-----BEGIN CERTIFICATE-----\n\
MIIEnjCCA4agAwIBAgIBATANBgkqhkiG9w0BAQsFADCBlDELMAkGA1UEBhMCVVMx\n\
EDAOBgNVBAgMB01vbnRhbmExEDAOBgNVBAcMB0JvemVtYW4xETAPBgNVBAoMCFNh\n\
d3Rvb3RoMRMwEQYDVQQLDApDb25zdWx0aW5nMRgwFgYDVQQDDA93d3cud29sZnNz\n\
bC5jb20xHzAdBgkqhkiG9w0BCQEWEGluZm9Ad29sZnNzbC5jb20wHhcNMTgwNDEz\n\
MTUyMzEwWhcNMjEwMTA3MTUyMzEwWjCBkDELMAkGA1UEBhMCVVMxEDAOBgNVBAgM\n\
B01vbnRhbmExEDAOBgNVBAcMB0JvemVtYW4xEDAOBgNVBAoMB3dvbGZTU0wxEDAO\n\
BgNVBAsMB1N1cHBvcnQxGDAWBgNVBAMMD3d3dy53b2xmc3NsLmNvbTEfMB0GCSqG\n\
SIb3DQEJARYQaW5mb0B3b2xmc3NsLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEP\n\
ADCCAQoCggEBAMCVCOFXQfJxbbfSRUEnAWXGRa7yvCQwuJXOL07W9hyIvHyf+6hn\n\
f/5cnFF194rKB+c1L4/hvXvAL3yrZKgX/Mpde7rgIeVyLm8uhtiVc9qsG1O5Xz/X\n\
GQ0lT+FjY1GLC2Q/rUO4pRxcNLOuAKBjxfZ/C1loeHOmjBipAm2vwxkBLrgQ48bM\n\
QLRpo0YzaYduxLsXpvPo3a1zvHsvIbX9ZlEMvVSz4W1fHLwjc9EJA4kU0hC5ZMMq\n\
0KGWSrzh1Bpbx6DAwWN4D0Q3MDKWgDIjlaF3uhPSl3PiXSXJag3DOWCktLBpQkIJ\n\
6dgIvDMgs1gip6rrxOHmYYPF0pbf2dBPrdcCAwEAAaOB/DCB+TAdBgNVHQ4EFgQU\n\
sxEyyZKYhOLJ+NA7bgNCyh8OjjwwgckGA1UdIwSBwTCBvoAUJ45nEXTDJh0/7TNj\n\
s6TYHTDl6NWhgZqkgZcwgZQxCzAJBgNVBAYTAlVTMRAwDgYDVQQIDAdNb250YW5h\n\
MRAwDgYDVQQHDAdCb3plbWFuMREwDwYDVQQKDAhTYXd0b290aDETMBEGA1UECwwK\n\
Q29uc3VsdGluZzEYMBYGA1UEAwwPd3d3LndvbGZzc2wuY29tMR8wHQYJKoZIhvcN\n\
AQkBFhBpbmZvQHdvbGZzc2wuY29tggkAhv/1jhDeuPswDAYDVR0TBAUwAwEB/zAN\n\
BgkqhkiG9w0BAQsFAAOCAQEAtFRgraADMt4CfyFKgcbtzc3YEorAuoJbda1U43yA\n\
aqwubCBOvk2Cp0cTXPTGaisQmVjeq2t8IgXBg53L/zzkLVdqppbf08Fo49LGg0uX\n\
4sYyDr7EA7kHilu4hLrFOT8cWKdV1/Cb6NJFueODLu62cVa5Ou4/J9h36PtESGUn\n\
R0z7/nLDrAV7HcvrXmWaqwLkiFs7iwvHzKmmi+GHsBkaDChYb5lSfu2wOmg7jAoI\n\
dHKruQnF7QR+bwscCSHQzX/5xF4nIOSFc1IF0rr41Y9BzCMuEm28MZjnY6OOJs3o\n\
K4ju4v46dFI0Dv0S5V5pUCAxNOQx8efkWwMT2qxBbOfPKw==\n\
-----END CERTIFICATE-----\n";

/*
static const byte cert_2_server_pem[]= "\
-----BEGIN CERTIFICATE-----\n\
MIIE/zCCA+egAwIBAgIUM0QaqGwB7PZg8nBRCkzRFPq86UQwDQYJKoZIhvcNAQEL\n\
BQAwgZQxCzAJBgNVBAYTAlVTMRAwDgYDVQQIDAdNb250YW5hMRAwDgYDVQQHDAdC\n\
b3plbWFuMREwDwYDVQQKDAhTYXd0b290aDETMBEGA1UECwwKQ29uc3VsdGluZzEY\n\
MBYGA1UEAwwPd3d3LndvbGZzc2wuY29tMR8wHQYJKoZIhvcNAQkBFhBpbmZvQHdv\n\
bGZzc2wuY29tMB4XDTIzMTIxMzIyMTkyOFoXDTI2MDkwODIyMTkyOFowgZQxCzAJ\n\
BgNVBAYTAlVTMRAwDgYDVQQIDAdNb250YW5hMRAwDgYDVQQHDAdCb3plbWFuMREw\n\
DwYDVQQKDAhTYXd0b290aDETMBEGA1UECwwKQ29uc3VsdGluZzEYMBYGA1UEAwwP\n\
d3d3LndvbGZzc2wuY29tMR8wHQYJKoZIhvcNAQkBFhBpbmZvQHdvbGZzc2wuY29t\n\
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvwzKLRSyHoRCW804H0ry\n\
TXUQ8bY1n9/KfQOY06zeA2buKvHYsH1uB1QLEJghTYDLEiDnzE/eRX3Jcncy6sqQ\n\
u2lSEAMvqPOVxfGLYlYb72dvpBBBla0Km+OlwLDScHZQMFuo6AgsfO2nonqNOCkc\n\
rMft8nyVsJWCfUlcOM13Je+9gHVTlDw9ymNbnxW10x0TLxnRPNt2Osy4fcnlwtfa\n\
QG/YIdxzG0ItU5z+Gvx9q3o2P5jehHwFZ85qFDiHqfGMtWjLaH9xICv1oGP1Vi+j\n\
JtK3b7FaF9c4mQj+k1hv/sMTSQgWC6dNZwBSMWcjTpjtUUUduQTZC+zYKLNLve02\n\
eQIDAQABo4IBRTCCAUEwHQYDVR0OBBYEFCeOZxF0wyYdP+0zY7Ok2B0w5ejVMIHU\n\
BgNVHSMEgcwwgcmAFCeOZxF0wyYdP+0zY7Ok2B0w5ejVoYGapIGXMIGUMQswCQYD\n\
VQQGEwJVUzEQMA4GA1UECAwHTW9udGFuYTEQMA4GA1UEBwwHQm96ZW1hbjERMA8G\n\
A1UECgwIU2F3dG9vdGgxEzARBgNVBAsMCkNvbnN1bHRpbmcxGDAWBgNVBAMMD3d3\n\
dy53b2xmc3NsLmNvbTEfMB0GCSqGSIb3DQEJARYQaW5mb0B3b2xmc3NsLmNvbYIU\n\
M0QaqGwB7PZg8nBRCkzRFPq86UQwDAYDVR0TBAUwAwEB/zAcBgNVHREEFTATggtl\n\
eGFtcGxlLmNvbYcEfwAAATAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIw\n\
DQYJKoZIhvcNAQELBQADggEBAC38+TJavtadQouGTmciw1AtyxQnHZTzzYhC2kEc\n\
OSRnp5JNJ+pWghm/EbJDpI1dh7InZGaCgd/E/VtisMJNnSnyQTLMLrXaOAYb6H+M\n\
bj2AHgBWSb854NpoL8T9AObRgRrRSrt2Us5NJJ3Eo6fxZRQvH6gtxsvOsaeJdCYn\n\
w/OjhEw0ARQDfRY6yIslLnuQzEaxUjS6k27v/kOjrcZvUfu66jjjb9buY2I26l4I\n\
tOIqRonjrrO0Bu9jem5d3cnsAk/3ZMAnB7RvShhyWzR0fNCpBI9Ai2o50msaAfIB\n\
qIE0OuWwVdE8lcqwgtbtmCgVWX6Vp2nHtXvsAadN5rmi/jU=\n\
-----END CERTIFICATE-----\n";
*/

static const byte key_server_pem[] = "\
-----BEGIN RSA PRIVATE KEY-----\n\
MIIEpQIBAAKCAQEAwJUI4VdB8nFtt9JFQScBZcZFrvK8JDC4lc4vTtb2HIi8fJ/7\n\
qGd//lycUXX3isoH5zUvj+G9e8AvfKtkqBf8yl17uuAh5XIuby6G2JVz2qwbU7lf\n\
P9cZDSVP4WNjUYsLZD+tQ7ilHFw0s64AoGPF9n8LWWh4c6aMGKkCba/DGQEuuBDj\n\
xsxAtGmjRjNph27Euxem8+jdrXO8ey8htf1mUQy9VLPhbV8cvCNz0QkDiRTSELlk\n\
wyrQoZZKvOHUGlvHoMDBY3gPRDcwMpaAMiOVoXe6E9KXc+JdJclqDcM5YKS0sGlC\n\
Qgnp2Ai8MyCzWCKnquvE4eZhg8XSlt/Z0E+t1wIDAQABAoIBAQCa0DQPUmIFUAHv\n\
n+1kbsLE2hryhNeSEEiSxOlq64t1bMZ5OPLJckqGZFSVd8vDmp231B2kAMieTuTd\n\
x7pnFsF0vKnWlI8rMBr77d8hBSPZSjm9mGtlmrjcxH3upkMVLj2+HSJgKnMw1T7Y\n\
oqyGQy7E9WReP4l1DxHYUSVOn9iqo85gs+KK2X4b8GTKmlsFC1uqy+XjP24yIgXz\n\
0PrvdFKB4l90073/MYNFdfpjepcu1rYZxpIm5CgGUFAOeC6peA0Ul7QS2DFAq6EB\n\
QcIw+AdfFuRhd9Jg8p+N6PS662PeKpeB70xs5lU0USsoNPRTHMRYCj+7r7X3SoVD\n\
LTzxWFiBAoGBAPIsVHY5I2PJEDK3k62vvhl1loFk5rW4iUJB0W3QHBv4G6xpyzY8\n\
ZH3c9Bm4w2CxV0hfUk9ZOlV/MsAZQ1A/rs5vF/MOn0DKTq0VO8l56cBZOHNwnAp8\n\
yTpIMqfYSXUKhcLC/RVz2pkJKmmanwpxv7AEpox6Wm9IWlQ7xrFTF9/nAoGBAMuT\n\
3ncVXbdcXHzYkKmYLdZpDmOzo9ymzItqpKISjI57SCyySzfcBhh96v52odSh6T8N\n\
zRtfr1+elltbD6F8r7ObkNtXczrtsCNErkFPHwdCEyNMy/r0FKTV9542fFufqDzB\n\
hV900jkt/9CE3/uzIHoumxeu5roLrl9TpFLtG8SRAoGBAOyY2rvV/vlSSn0CVUlv\n\
VW5SL4SjK7OGYrNU0mNS2uOIdqDvixWl0xgUcndex6MEH54ZYrUbG57D8rUy+UzB\n\
qusMJn3UX0pRXKRFBnBEp1bA1CIUdp7YY1CJkNPiv4GVkjFBhzkaQwsYpVMfORpf\n\
H0O8h2rfbtMiAP4imHBOGhkpAoGBAIpBVihRnl/Ungs7mKNU8mxW1KrpaTOFJAza\n\
1AwtxL9PAmk4fNTm3Ezt1xYRwz4A58MmwFEC3rt1nG9WnHrzju/PisUr0toGakTJ\n\
c/5umYf4W77xfOZltU9s8MnF/xbKixsX4lg9ojerAby/QM5TjI7t7+5ZneBj5nxe\n\
9Y5L8TvBAoGATUX5QIzFW/QqGoq08hysa+kMVja3TnKW1eWK0uL/8fEYEz2GCbjY\n\
dqfJHHFSlDBD4PF4dP1hG0wJzOZoKnGtHN9DvFbbpaS+NXCkXs9P/ABVmTo9I89n\n\
WvUi+LUp0EQR6zUuRr79jhiyX6i/GTKh9dwD5nyaHwx8qbAOITc78bA=\n\
-----END RSA PRIVATE KEY-----\n";

const char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

/*============FUNCTION FOR HTTPS==================*/
static void display_whitelist(void);
static void display_processing(void);
unsigned long ipv4_to_decimal(const char *ip);
unsigned long hash_string_to_int(unsigned char *str);
int hash_time_to_int(void);
void init_http_resource(void);
int get_HTTP_Data(unsigned char *s_ip, unsigned char *uri, int packet_timer);
int IP_Management(struct IP_Connection *IP_Conn_Table_1, unsigned int ip, unsigned int timer, int *numIP);
unsigned long URL_Management(struct URL_Connection *URL_Conn_Table_1, unsigned long url, unsigned int IP_cnt, int *numURL);
int Attacker_Detection(struct URL_Connection *URL_Conn_Table_1, unsigned int ip, int *numPAL);
void reset_HTTP_Table(struct IP_Connection *IP_Conn_Table_1, struct URL_Connection *URL_Conn_Table, int *numURL, int *numIP, int *numPAL);
void add_blacklist(char *key_addr);
static void display_blacklist(void);
void get_data_BL_from_Thread_HTTPS(char *get_ip);
void Send_ipV4_HTTPS_to_BLackList(int serial_port, char *get_ip);
void get_data_WL_from_Thread_HTTPS(char *get_ip);
void Send_IPv4_HTTPS_to_WhiteList(int serial_port, char *get_ip);
THREAD_RETURN WOLFSSL_THREAD process_https(void *args);
/*==================================================*/
/*==============================================*/
// unsigned long long int test = 0;
// unsigned long long int test1 = 0;
void send_reset(int serial_port);
void *key_listener(void *arg);
int configure_serial_port(const char *device, int baud_rate);
int send_data(int serial_port, const char *data, size_t size);
char *receive_data(int serial_port);
int kbhit(void);
//
void display_memory();
void display_log_files();
void delete_log_file();
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
void previous_mode_fc();
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
void remove_old_logs(void);
// void memory_check_thread_function(void);
void *memory_check_thread_function(void *arg);
void *run(void *arg);
//
void *packet_queue_processing_thread(void *arg);
void enqueue_packet(unsigned char *packet, int size);
// void *log_buffer_thread(void *arg);
void check_connect_eth();
void create_new_log_file();

void load_ips_from_file(const char *filename);
void load_remove_hash_ip_http_from_file(const char *filename);
void flush_batch_to_file(const char *filename);
void create_http_filelog(const char *filename);
void process_ip(const char *filename, const char *ip);
void remove_ip_HTTP_from_hash(const char *ip);
void append_ips_to_file(const char *source_file, const char *dest_file);
void remove_matching_ips(const char *source_file, const char *dest_file);
void send_ips_via_uart(const char *filename);
void uart_send(const char *data, int serial_port);
void send_http_ipv4_start(int serial_port, const char *filename);
void send_http_ipv6_start(int serial_port, const char *filename);
void send_data_sync_gui(int serial_port);
void send_data_sync_time(int serial_port);
void remove_ip_from_file(const char *filename, const char *ip);
//
void process_key(int serial_port, char *ID, char *Value, int client_sock, char *buffer);
void process_buffer(char *client_message, int client_sock);
void printf_uart2(int serial_port);
//
static char current_attack[255] = "";
#define LCD_ADDR 0x27
#define BUFFER_SIZE 4096
const char *serverBasedUrl = "http://localhost:3000/";
#define TARGET_MAC {0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA}
#define TARGET_MAC_ATTACK {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
#define LOG_FLOOD_DIR "Log/Log_Flood"
#define LOG_NORMAL_DIR "Log/Log_Normal"
#define ATTACKER_LOG_DIR "HTTP_ip_table"
#define LOGFILE_HTTP_IPv4 "HTTP_ip_table/http_ipv4.log"
#define LOGFILE_HTTP_IPv6 "HTTP_ip_table/http_ipv6.log"
#define MAX_LOG_DAYS 100

#define MAX_IP_LEN 42
#define BUFFER_SIZE_SEND_IP_VIA_UART 8192
#define MAX_IPS 65536
#define BATCH_SIZE 1

int serial_port;
float Threshold_SD;
int Threshold_time_check_create_logfile;
int Threshold_time_counter;
const char *previous_mode = "/home/antiddos/DDoS_V1/Setting/mode.conf";
const char *time_check_create_log = "/home/antiddos/DDoS_V1/Setting/time_check_create_log.conf";
const char *threshold_logfile = "/home/antiddos/DDoS_V1/Setting/threshold_logfile.conf";
const char *time_counter = "/home/antiddos/DDoS_V1/Setting/time_counter.conf";
#define CONFIG_FILE "/home/antiddos/DDoS_V1/Setting/config_auto_manual.conf"

volatile bool auto_delete_logs;
volatile int stop_scrolling = 0;
bool reset_program = false;
bool create_new_log = false;
bool created_new_log = false;
char bw1[16];
static unsigned char prev_time[4] = {0};
static unsigned int bw_accumulated = 0;
////////////////////////////////////////////////////////////////////////////////
// Global Variables
I2C16x2 lcd;
unsigned char target_mac[6] = TARGET_MAC;
unsigned char target_mac_attack[6] = TARGET_MAC_ATTACK;
time_t last_packet_time;
pthread_mutex_t log_mutex;          // Mutex for synchronizing log access
pthread_mutex_t lcd_mutex;          // Mutex for synchronizing LCD updates
pthread_mutex_t packet_queue_mutex; // Mutex for packet queue
pthread_cond_t packet_queue_cond;   // Condition variable for packet queue
pthread_t run_thread;
pthread_t wolfssl_thread;
pthread_mutex_t run_mutex = PTHREAD_MUTEX_INITIALIZER;     // run
pthread_mutex_t wolfssl_mutex = PTHREAD_MUTEX_INITIALIZER; // http
pthread_mutex_t mutex_new_log = PTHREAD_MUTEX_INITIALIZER;

FILE *current_log_file_flood = NULL;  // Bien luu tru con tro logfile
FILE *current_log_file_normal = NULL; // Bien luu tru con tro logfile
// FILE *attacker_log_file = NULL;
GHashTable *ip_table;
GQueue *batch_queue;
// LCD update queue
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
int log_buffer_pos = 0;
char name_logfile_flood[64];
char name_logfile_normal[64];
// Buffer printf terminal
#define PRINT_BUFFER_SIZE 1024
char print_buffer[PRINT_BUFFER_SIZE];
int print_buffer_pos = 0;
//////////////////////
bool stop_writing;
bool full_sd = false;
bool full_sd1 = false;
bool full_sd2 = false;
bool is_run_running = false;
bool is_connected = true;
bool show_disconnected_message = false;
bool is_idle = false;
bool is_idle2 = false;

int count_tancong = 0;
int count_tong = 0;
int count_bth = 0;

char *socket_path = "/tmp/defender";
int fd;
char uds_msg[256];

/*Function WOLFSSL*/

/*==================================================*/

/*Ham them tu thu vien src*/

// int intN(int n);
// static char *randomString(int len);

/**
 * not a cryptographically secure number
 * return interger [0,n]
 */
static int intN(int n) { return rand() % n; }

/**
 * Input: Length of the random string [a-z A-Z 0-9] to be generated
 */
static char *randomString(int len)
{
  char *rstr = malloc((len + 1) * sizeof(char));
  int i;
  for (i = 0; i < len; i++)
  {
    rstr[i] = alphabet[intN(strlen(alphabet))];
  }
  rstr[len] = '\0';
  return rstr;
}

// static void concatenate(char p[], char q[]) {
//    int c, d;

//    c = 0;

//    while (p[c] != '\0') {
//       c++;
//    }

//    d = 0;

//    while (q[d] != '\0') {
//       p[c] = q[d];
//       d++;
//       c++;
//    }

//    p[c] = '\0';
// }
/*=====================HASH=====================================*/
int hash_time_to_int(void)
{
  time_t now = time(NULL);
  return now;
}

// Convert IP to DEC
unsigned long ipv4_to_decimal(const char *ip)
{
  unsigned int a, b, c, d;
  sscanf(ip, "%u.%u.%u.%u", &a, &b, &c, &d);
  return ((a << 24) | (b << 16) | (c << 8) | d);
}

// unsigned long hash_string_to_int(unsigned char *str)
// {
//   size_t len = strlen((char *)str);
//   //printf("\n\r\nString URI: %s, Length: %zu\n", str, len);
//   unsigned long hash = 5381;
//   int c;
//   while ((c = *str++))
//   {
//     hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
//   }
//   //printf("\n\r\n iNT URI hash:%lu\n", hash);
//   return hash;
// }
unsigned long hash_string_to_int(unsigned char *str)
{
  // printf("\n\r\nString URI hash: %s\n", str);
  long hash = 5381;
  int c;
  while ((c = *str++))
  {
    hash = ((hash << 5) + hash) + c;
  }
  unsigned long positive_hash = (unsigned long)(hash < 0 ? -hash : hash);
  // printf("\n\r\nINT URI hash: %lu\n", positive_hash);
  return positive_hash;
}

/*===============================================================*/

/*============== FUNCTIONS XU LY HTTP ==========================*/
void init_http_resource(void)
{
  Attacker_List = (unsigned int *)malloc(100 * sizeof(unsigned int));
  IP_Conn_Table = (struct IP_Connection *)malloc(Num_of_IP * sizeof(struct IP_Connection));
  URL_Conn_Table = (struct URL_Connection *)malloc(Num_of_URL * sizeof(struct URL_Connection));
  for (int i = 0; i < Num_of_URL; i++)
  {
    (URL_Conn_Table + i)->PAL = (struct Potential_IP_List *)malloc(Num_in_PAL * sizeof(struct Potential_IP_List));
    // if ((URL_Conn_Table+i)->PAL == NULL)
    //     xil_printf("PAL malloc failed\r\n");
    //  else xil_printf("PAL malloc successfully\r\n");
  }
}

int get_HTTP_Data(unsigned char *s_ip, unsigned char *uri, int packet_timer)
{
  time_t current_time = time(NULL);
  // unsigned int packet_timer;
  unsigned int src_ip;
  unsigned long hash_url;
  if (current_time - last_time > 3)
  {
    reset_HTTP_Table(IP_Conn_Table, URL_Conn_Table, &Num_of_URL, &Num_of_IP, &Num_in_PAL);
    last_time = current_time;
  }
  // packet_timer = hash_time_to_int();
  src_ip = ipv4_to_decimal((const char *)s_ip);
  hash_url = hash_string_to_int(uri);
  Atk_HTTP.connected_IP = IP_Management(IP_Conn_Table, src_ip, packet_timer, &Num_of_IP);
  ////  printf("\r\n              hash time: %u", packet_timer);
  // printf("\r\n             ip: %u", src_ip);
  //  printf("\r\n              urrl: %ld", hash_url);

  Atk_HTTP.attacked_URL = URL_Management(URL_Conn_Table, hash_url, Atk_HTTP.connected_IP, &Num_of_URL);
  if (Atk_HTTP.attacked_URL > 0)
  {
  }
  if (Atk_HTTP.attacked_URL > 0 && Atk_HTTP.attacked_URL == hash_url)
  {
    Attacker_Detection(URL_Conn_Table, src_ip, &Num_in_PAL);
  }
  // if (current_time - last_time > 50)
  // {
  //   reset_HTTP_Table(IP_Conn_Table, URL_Conn_Table, &Num_of_URL, &Num_of_IP, &Num_in_PAL);
  //   last_time = current_time;
  // }

  return Atk_HTTP.connected_IP;
}

int IP_Management(struct IP_Connection *IP_Conn_Table_1, unsigned int ip, unsigned int timer, int *numIP)
{
  int connected_IP;
  if (IP_Conn_Table_1 == NULL)
  {
    // printf("\r\n      Return 0");
    // printf("\r\n ");

    return 0;
  }
  // printf("\r\n      IP_mana");
  // printf("\r\n ");

  // int next_size;
  for (int i = 0; i < *numIP; i++)
  {

    if (!((IP_Conn_Table_1 + i)->b_flag))
    {
      (IP_Conn_Table_1 + i)->b_flag = true;
      (IP_Conn_Table_1 + i)->src_ip = ip;
      (IP_Conn_Table_1 + i)->timer = timer;

      break;
    }
    else if ((IP_Conn_Table_1 + i)->b_flag)
    {

      if ((IP_Conn_Table_1 + i)->src_ip == ip)
      {
        // if ((timer - (IP_Conn_Table + i)->timer) > (IP_TIMEOUT * 1000)) // vi timestamp_counter duoi core tinh bang ms nen TIMEMOUT*1000
        if ((timer - (IP_Conn_Table_1 + i)->timer) > (IP_TIMEOUT))
        {
          (IP_Conn_Table_1 + i)->b_flag = false;
          (IP_Conn_Table_1 + i)->src_ip = 0;
          (IP_Conn_Table_1 + i)->timer = 0;
          break;
        }
        else
        {

          (IP_Conn_Table_1 + i)->timer = timer;
          break;
        }
      }
      // else if (i == *numIP-1)
      // {
      //     *numIP += 100;
      //     next_size = *numIP;
      //     //xil_printf("| NSz:%d ", next_size);
      //     IP_Conn_Table = (struct IP_Connection *) realloc(IP_Conn_Table, next_size*sizeof(struct IP_Connection));
      //     if (IP_Conn_Table == NULL)
      //     {
      //         //xil_printf("IP malloc failed\r\n");
      //         // getchar();
      //     }
      // }
    }
  }

  connected_IP = 0;
  for (int i = 0; i < *numIP; i++)
  {
    if ((IP_Conn_Table_1 + i)->b_flag)
    {
      connected_IP++;
    }
  }
  // if (connected_IP == *numIP)
  // {
  //     *numIP += 100;
  //     IP_Conn_Table = (struct IP_Connection *) realloc(IP_Conn_Table, (*numIP)*sizeof(struct IP_Connection));
  //     if (IP_Conn_Table == NULL)
  //     {
  //         //xil_printf("IP malloc failed\r\n");
  //     }
  // }

  return connected_IP;
}

unsigned long URL_Management(struct URL_Connection *URL_Conn_Table_1, unsigned long url, unsigned int IP_cnt, int *numURL)
{

  int URL_Atk_Threshold;

  if (URL_Conn_Table_1 == NULL)
  {
    return 0;
  }
  // int attacked_URL;
  // int temp_counter;
  int index;
  // int numURLs = 0;
  // printf("\r\n      numURL: %d", *numURL);
  // printf("\r\n ");
  URL_Atk_Threshold = IP_cnt * 15 * IP_TIMEOUT;
  // printf("\r\n      URL_Atk_Threshold: %d", URL_Atk_Threshold);
  // printf("\r\n ");
  for (int i = 0; i < *numURL; i++)
  {
    if (!(URL_Conn_Table_1 + i)->b_flag)
    {
      (URL_Conn_Table_1 + i)->b_flag = true;
      (URL_Conn_Table_1 + i)->hash_url = url;
      (URL_Conn_Table_1 + i)->request_counter++;
      // printf("\r\n      Xem INDEX: %d", i);
      // printf("\r\n ");
      // printf("\r\n      Xem url: %lu", (URL_Conn_Table_1 + i)->hash_url);
      // printf("\r\n ");
      break;
    }
    else if ((URL_Conn_Table_1 + i)->b_flag)
    {
      // printf("\r\n      Xem INDEX2: %d", i);
      // printf("\r\n ");
      // printf("\r\n      Xem url2: %lu", (URL_Conn_Table_1 + i)->hash_url);
      // printf("\r\n ");
      if ((URL_Conn_Table_1 + i)->hash_url == url)
      {
        (URL_Conn_Table_1 + i)->request_counter++;
        // printf("\r\n     request_counter %u", (URL_Conn_Table_1 + i)->request_counter++);
        // printf("\r\n ");
        break;
      }
      // else
      // ; // neu khong cung URL va khong con vung nho trong trong khoi nho cua URL_Table thi cap phat them
    }
  }

  index = 0;
  // temp_counter = 0;
  for (int i = 0; i < *numURL; i++)
  {
    if ((URL_Conn_Table_1 + i)->b_flag && ((URL_Conn_Table_1 + i)->request_counter > (unsigned int)Atk_URL.max_URL_cnt))

    {
      Atk_URL.max_URL_cnt = (URL_Conn_Table_1 + i)->request_counter;
      index = i;
    }
    // if ((URL_Conn_Table_1 + i)->b_flag)
    // {

    //   numURLs++;
    // }
  }
  // printf("\r\n      index: %d", index);
  // printf("\r\n ");
  // printf("\r\n      (Atk_URL.max_URL_cnt: %d", Atk_URL.max_URL_cnt);
  // printf("\r\n ");
  if (Atk_URL.max_URL_cnt > URL_Atk_Threshold)
  {
    Atk_URL.index = index;
    Atk_URL.url = (URL_Conn_Table_1 + index)->hash_url;
    // printf("\r\n      Xem INDEXend: %d", index);
    // printf("\r\n ");
    // printf("\r\n      Xem urlend: %lu", (URL_Conn_Table_1 + index)->hash_url);
    // printf("\r\n ");
    // printf("\r\n      Xem Atk_URL.url: %lu", Atk_URL.url);
    // printf("\r\n ");
    return Atk_URL.url;
    // break;
  }

  // for (int i=0; i<*numURL; i++)
  // {
  //     if ((URL_Conn_Table+i)->b_flag && ((URL_Conn_Table+i)->request_counter > URL_Atk_Threshold))
  //     {
  //         //xil_printf("| URL %d Max: %d ", i, (URL_Conn_Table+i)->request_counter);
  //         // temp_counter = (URL_Conn_Table+i)->request_counter;
  //         Atk_URL.index = i;
  //         Atk_URL.url = (URL_Conn_Table + i)->hash_url;
  //     }
  //     if ((URL_Conn_Table+i)->b_flag)
  //     {
  //         //xil_printf("| URL %d: %d ", i, (URL_Conn_Table+i)->request_counter);
  //     }
  // }
  // return Atk_URL.url;

  return 0;
}

int Attacker_Detection(struct URL_Connection *URL_Conn_Table_1, unsigned int ip, int *numPAL)
{

  int PAL_Thres;
  PAL_Thres = 15 * IP_TIMEOUT;
  // unsigned int target_IP;

  for (int i = 0; i < *numPAL; i++)
  {
    if (!((URL_Conn_Table_1 + Atk_URL.index)->PAL + i)->b_flag)
    {
      ((URL_Conn_Table_1 + Atk_URL.index)->PAL + i)->b_flag = true;
      ((URL_Conn_Table_1 + Atk_URL.index)->PAL + i)->src_ip_1 = ip;
      ((URL_Conn_Table_1 + Atk_URL.index)->PAL + i)->pkt_counter_1++;
      break;
    }
    else if (((URL_Conn_Table_1 + Atk_URL.index)->PAL + i)->b_flag)
    {
      if (((URL_Conn_Table_1 + Atk_URL.index)->PAL + i)->src_ip_1 == ip)
      {
        ((URL_Conn_Table_1 + Atk_URL.index)->PAL + i)->pkt_counter_1++;
        break;
      }
      // else
      //   ;
    }
  }

  for (int i = 0; i < *numPAL; i++)
  {
    if (((URL_Conn_Table_1 + Atk_URL.index)->PAL + i)->b_flag)
    {
      if (((URL_Conn_Table_1 + Atk_URL.index)->PAL + i)->pkt_counter_1 > (unsigned int)PAL_Thres)

      {
        char ipv4_str[16];
        int src_ip = ((URL_Conn_Table + Atk_URL.index)->PAL + i)->src_ip_1;
        sprintf(ipv4_str, "%d.%d.%d.%d",
                (src_ip >> 24) & 0xFF,
                (src_ip >> 16) & 0xFF,
                (src_ip >> 8) & 0xFF,
                src_ip & 0xFF);
        // add_blacklist(ipv4_str);
        get_data_BL_from_Thread_HTTPS(ipv4_str);
        //   printf("\n\r ADD: %s\n", ipv4_str);
        Atk_HTTP.just_detected_IP = ((URL_Conn_Table + Atk_URL.index)->PAL + i)->src_ip_1;
      }
    }
  }
  return 0;
}

void reset_HTTP_Table(struct IP_Connection *IP_Conn_Table_1, struct URL_Connection *URL_Conn_Table_1, int *numURL, int *numIP, int *numPAL)
{
  // for (int i=0; i<*numURL; i++)
  // {
  //     free((URL_Conn_Table+i)->PAL);
  //     // (URL_Conn_Table+i)->PAL = NULL;
  // }
  // free(IP_Conn_Table);
  // // IP_Conn_Table = NULL;
  // free(URL_Conn_Table);
  // // URL_Conn_Table = NULL;
  if (IP_Conn_Table_1 == NULL || URL_Conn_Table_1 == NULL)
  {
    return;
  }

  for (int i = 0; i < *numURL; i++)
  {
    (URL_Conn_Table_1 + i)->b_flag = false;
    (URL_Conn_Table_1 + i)->index = 0;
    (URL_Conn_Table_1 + i)->hash_url = 0;
    (URL_Conn_Table_1 + i)->request_counter = 0;
    for (int j = 0; j < *numPAL; j++)
    {
      ((URL_Conn_Table_1 + i)->PAL + j)->b_flag = false;
      ((URL_Conn_Table_1 + i)->PAL + j)->src_ip_1 = 0;
      ((URL_Conn_Table_1 + i)->PAL + j)->pkt_counter_1 = 0;
    }
  }

  for (int i = 0; i < *numIP; i++)
  {
    (IP_Conn_Table_1 + i)->b_flag = false;
    (IP_Conn_Table_1 + i)->src_ip = 0;
    (IP_Conn_Table_1 + i)->timer = 0;
  }

  Atk_HTTP.connected_IP = 0;
  Atk_HTTP.attacked_URL = 0;

  Atk_URL.index = 0;
  Atk_URL.url = 0;
  Atk_URL.max_URL_cnt = 0;
}
/*===============================================================*/

static int search_block(char key_src[50], int *ctr)
{
  struct blacklist_table *ptr_block;
  char item_src_block[50];
  // char item_uri_block[50];
  strcpy(item_src_block, key_src);
  // strcpy(item_uri_block, key_uri);
  int i = 0, flag = 0, nxt = 0;
  ptr_block = (struct blacklist_table *)blacklist_table_0; /* Blaclist Table */
  if (ptr_block == NULL)
  {
    // printf("\r\n                                                                                     Alert: Empty list of Blacklist");
    nxt = 1;
  }
  else if (!nxt)
  {
    while (ptr_block != NULL)
    {
      if ((memcmp(ptr_block->ip_block_client_addr, item_src_block, strlen(item_src_block)) == 0))
      {
        // printf("\r\n                                                                                     Item found at location %d of BlackList", i + 1);
        flag = 1;
        *ctr = i;
        break;
      }
      i++;
      ptr_block = (struct blacklist_table *)ptr_block->next;
    }
  }
  return flag;
}

static int search_whitelist(char key_src[50], char key_uri[200], int *ctr)
{
  struct whitelist_table *ptr_legit;
  char item_src_legit[50];
  char item_uri_legit[200];
  strcpy(item_src_legit, key_src);
  strcpy(item_uri_legit, key_uri);
  int i = 0, flag = 0;
  ptr_legit = (struct whitelist_table *)whitelist_table_0; /* Whitelist Table */
  if (ptr_legit == NULL)
  {
    // printf("\r\n                                                                                     Alert: Empty list of WhiteList");
    return 0;
  }
  else
  {
    while (ptr_legit != NULL)
    {
      if ((memcmp(ptr_legit->ip_legit_client_addr, item_src_legit, strlen(item_src_legit)) == 0) && (memcmp(ptr_legit->uri_legit, item_uri_legit, strlen(item_uri_legit)) == 0))
      {
        // printf("\r\n                                                                                     Item found at location %d of WhiteList", i + 1);
        flag = 1;
        *ctr = i;
        break;
      }
      else
      {
        i++;
        ptr_legit = (struct whitelist_table *)ptr_legit->next;
      }
    }
    if (flag)
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }
}

static void display_whitelist(void)
{
  struct whitelist_table *ptr;
  ptr = (struct whitelist_table *)whitelist_table_0;
  int i = 0;
  if (ptr == NULL)
  {
    printf("\r\nWhitelist Table is empty");
    return;
  }
  else
  {
    printf("\r\nWhitelist Table:");
    while (ptr != NULL)
    {
      printf("\r\n%d: %s-%s", i + 1, ptr->ip_legit_client_addr, ptr->uri_legit);
      ptr = (struct whitelist_table *)ptr->next;
      i++;
    }
  }
}

static void display_blacklist(void)
{
  struct blacklist_table *ptr_block = blacklist_table_0;
  int i = 0;
  if (ptr_block == NULL)
  {
    printf("\r\nBlacklist is empty.");
    return;
  }
  printf("\r\nBlacklist Table:");
  while (ptr_block != NULL)
  {
    printf("\r\n%d: %s", i + 1, ptr_block->ip_block_client_addr);
    // printf("\r\n                                                                                     Timestamp: %s", ptr_block->timestamp);
    // printf("\r\n                                                                                     IP Address: %s", ptr_block->ip_block_client_addr);
    // printf("\r\n                                                                                     URI: %s", ptr_block->uri_block);

    ptr_block = (struct blacklist_table *)ptr_block->next;
    i++;
  }
}

static void display_processing(void)
{
  struct processing_table *ptr_tmp;
  ptr_tmp = (struct processing_table *)processing_table_0;
  int i = 0;
  if (ptr_tmp == NULL)
  {
    //  printf("\r\nAlert: Processing table is empty");
    return;
  }
  else
  {
    // printf("\r\n Processing Table:");
    while (ptr_tmp != NULL)
    {
      //  printf("\r\n%d: %s-%s-%s", i + 1, ptr_tmp->ip_client_addr, ptr_tmp->uri_request_firt, ptr_tmp->uri_redirect);
      ptr_tmp = (struct processing_table *)ptr_tmp->next;
      i++;
    }
  }
}

static int search_processing_table(char key_src[50], char key_uri[200], int *ctr)
{
  struct processing_table *ptr;
  char item_src[50];
  char item_uri[200];
  strcpy(item_src, key_src);
  strcpy(item_uri, key_uri);

  // printf("\r\nitem_src: %s", item_src);
  // printf("\r\nitem_uri: %s", item_uri);
  // printf("\r\n");
  int i = 0, flag = 0, nxt = 0;
  ptr = (struct processing_table *)processing_table_0; /* Processing table*/
  if (ptr == NULL)
  {
    // printf("\r\n                                                                                     Alert: Empty list of Processing Table");
    flag = 1;
    nxt = 1;
  }
  else if (!nxt)
  {
    while (ptr != NULL)
    {
      if ((memcmp(ptr->ip_client_addr, item_src, strlen(item_src)) == 0) && (memcmp(ptr->uri_request_firt, item_uri, strlen(item_uri)) == 0))
      {
        /* Chua xac minh -> chuyen huong sai */

        // printf("\r\n                                                                                     Item phase 1 found at location %d of Processing Table", i + 1);
        // printf("\r\nALOOOOOOOO-%s\n", test);
        // printf("\r\n");
        *ctr = i;
        break;
      }
      if ((memcmp(ptr->ip_client_addr, item_src, strlen(item_src)) == 0) && (memcmp(ptr->uri_redirect, item_uri, strlen(item_uri)) == 0))
      {
        /* Da xac minh -> chuyen huong dung */
        // printf("\r\n                                                                                     Item phase 2 found at location %d of Processing Table", i + 1);
        flag = 2;
        *ctr = i;

        break;
      }
      i++;
      ptr = (struct processing_table *)ptr->next;
    }
  }
  // printf("\r\n                                                                                     xxxxx");
  return flag;
  // printf("\r\n                                                                                     mmmmm");
  // printf("\r\n                                                                                     vvvv");
}

static void update_processing(char key_src[50], char key_uri_req_1[200], char uri_direct[200], int *ctr)
{
  struct processing_table *ptr_update;
  char item_update_src[50];
  char item_update_uri_1[200];
  strcpy(item_update_src, key_src);
  strcpy(item_update_uri_1, key_uri_req_1);
  int i = 0, nxt = 0;
  ptr_update = (struct processing_table *)processing_table_0; /* Processing table*/
  // printf("\r\n UPDATE IP: %s", item_update_src);
  // printf("\r\n UPDATE URI: %s", item_update_uri_1);
  // printf("\r\n");
  if (ptr_update == NULL)
  {
    // printf("\r\n                                                                                     Alert: Empty list of Processing Table");
    nxt = 1;
  }
  else if (!nxt)
  {
    while (ptr_update != NULL)
    {
      // printf("\r\n UPDATE IP_1: %s", ptr_update->ip_client_addr);
      // printf("\r\n UPDATE URI_1: %s", ptr_update->uri_request_firt);
      // printf("\r\n");
      if ((memcpy(ptr_update->ip_client_addr, item_update_src, strlen(item_update_src)) == 0) && (memcpy(ptr_update->uri_request_firt, item_update_uri_1, strlen(item_update_uri_1)) == 0))
      {
        // printf("\r\nItem found at location %d of Processing Table", i + 1);
        // printf("\r\n");
        *ctr = i;
        strcpy(ptr_update->uri_redirect, uri_direct);
        // printf("\r\n UPDATE URI DIRECT: %s", ptr_update->uri_redirect);
        // printf("\r\n");
        break;
      }
      i++;
      ptr_update = (struct processing_table *)ptr_update->next;
    }
  }
}

static void create_processing(char addr[50], char uri[200], char uri_direct[200])
{
  struct processing_table *temp_processing, *ptr_processing;
  temp_processing = (struct processing_table *)malloc(1024);
  if (temp_processing == NULL)
  {
    // printf("\r\nAlert: Out of Memory Space");
    return;
  }
  memset(temp_processing, 0, sizeof(*temp_processing));
  strcpy(temp_processing->ip_client_addr, addr);
  // strcpy(test, temp_processing->ip_client_addr);
  // printf("\r\nALOOOOOOOO-%s", temp_processing->ip_client_addr);
  //  //printf("\r\n-%s", temp_legit->addr_legit);
  strcpy(temp_processing->uri_request_firt, uri);
  // //printf("\r\n-%s", temp_legit->uri_legit);
  strcpy(temp_processing->uri_redirect, uri_direct);
  // //printf("\r\n-%s", temp_processing->uri_redirect);
  temp_processing->next = NULL;
  if (processing_table_0 == NULL)
  {
    processing_table_0 = (struct processing_table *)temp_processing;
  }
  else
  {
    ptr_processing = (struct processing_table *)processing_table_0;
    while (ptr_processing->next != NULL)
    {
      ptr_processing = (struct processing_table *)ptr_processing->next;
    }
    ptr_processing->next = (struct flow *)temp_processing;
  }
}

static void create_whitelist(char addr[50], char uri[200])
{
  struct whitelist_table *temp, *ptr;
  temp = (struct whitelist_table *)malloc(1024);
  if (temp == NULL)
  {
    // printf("\r\nAlert: Out of Memory Space");
    return;
  }
  strcpy(temp->ip_legit_client_addr, addr);
  strcpy(temp->uri_legit, uri);
  temp->next = NULL;
  if (whitelist_table_0 == NULL)
  {
    whitelist_table_0 = (struct whitelist_table *)temp;
  }
  else
  {
    ptr = (struct whitelist_table *)whitelist_table_0;
    while (ptr->next != NULL)
    {
      ptr = (struct whitelist_table *)ptr->next;
    }
    ptr->next = (struct legit_connection *)temp;
  }
}

void add_blacklist(char *key_addr)
{
  struct blacklist_table *temp_block, *ptr_block;

  // ptr_block = (struct blacklist_table *)blacklist_table_0;
  // while (ptr_block->next != NULL)

  // {
  //   if (strcmp(ptr_block->ip_block_client_addr, key_addr) == 0)
  //   {
  //     return;
  //   }
  //   ptr_block = (struct blacklist_table *)ptr_block->next;
  // }

  temp_block = (struct blacklist_table *)malloc(sizeof(struct blacklist_table));
  if (temp_block == NULL)
  {
    // printf("\r\nAlert: Out of Memory Space");
    return;
  }
  strcpy(temp_block->ip_block_client_addr, key_addr);
  // strcpy(temp_block->uri_block, key_uri);
  temp_block->next = NULL;

  if (blacklist_table_0 == NULL)
  {
    blacklist_table_0 = (struct blacklist_table *)temp_block;

    // printf("\r\nAdded to empty Blacklist");
  }
  else
  {

    ptr_block = (struct blacklist_table *)blacklist_table_0;
    while (ptr_block->next != NULL)
    {
      if (strcmp(ptr_block->ip_block_client_addr, key_addr) == 0)
      {
        // free(temp_block);
        return;
      }
      ptr_block = (struct blacklist_table *)ptr_block->next;
    }
    // Check node cuoi trong list
    if (strcmp(ptr_block->ip_block_client_addr, key_addr) == 0)
    {
      // free(temp_block);
      return;
    }
    ptr_block->next = (struct block_connection *)temp_block;

    // printf("\r\nAdded to Blacklist");
  }
}

static void delete_processing(int pos)
{
  int i;
  struct processing_table *temp_tmp, *ptr_tmp;
  temp_tmp = (struct processing_table *)processing_table_0;
  if (processing_table_0 == NULL)
  {
    // printf("\r\nAlert: Empty list of Processing Table");
    // printf("\r\n");
    return;
  }
  else
  {
    if (pos == 0)
    {
      ptr_tmp = (struct processing_table *)processing_table_0;
      processing_table_0 = (struct processing_table *)processing_table_0->next;
      // printf("\r\nThe deleted element is: %s-%s-%s", ptr_tmp->ip_client_addr, ptr_tmp->uri_request_firt, ptr_tmp->uri_redirect);

      free(ptr_tmp);
    }
    else
    {
      ptr_tmp = (struct processing_table *)processing_table_0;
      // printf("\r\nptr_tmp   ");
      // printf("\r\n");
      for (i = 0; i < pos; i++)
      {

        ptr_tmp = (struct processing_table *)ptr_tmp->next;
        if (ptr_tmp == NULL)
        {
          // printf("\r\n                                                                                     Alert: Position not Found in Processing Table");
          return;
        }
      }
      // printf("\r\n   temp_tmp->next  ");
      // printf("\r\n");
      temp_tmp->next = (struct flow *)ptr_tmp->next;
      temp_tmp->next = (struct flow *)ptr_tmp;
      // printf("\r\n                                                                                     The deleted element is: %s-%s-%s", ptr_tmp->ip_client_addr, ptr_tmp->uri_request_firt, ptr_tmp->uri_redirect);
      free(ptr_tmp);
    }
  }
}

static void SignalReady(void *args, word16 port)
{
  // printf("SIGNAL READY\n");
#if defined(NO_MAIN_DRIVER) && defined(WOLFSSL_COND)
  /* signal ready to tcp_accept */
  func_args *server_args = (func_args *)args;
  tcp_ready *ready = server_args->signal;
  THREAD_CHECK_RET(wolfSSL_CondStart(&ready->cond));
  ready->ready = 1;
  ready->port = port;
  THREAD_CHECK_RET(wolfSSL_CondSignal(&ready->cond));
  THREAD_CHECK_RET(wolfSSL_CondEnd(&ready->cond));
#endif /* NO_MAIN_DRIVER && WOLFSSL_COND */
  (void)args;
  (void)port;
}

// Ham lay du lieu IP tu thread HTTPS BL
void get_data_BL_from_Thread_HTTPS(char *get_ip)
{
  Send_ipV4_HTTPS_to_BLackList(serial_port, get_ip);
}

// Ham gui blacklist xuong core
void Send_ipV4_HTTPS_to_BLackList(int serial_port, char *get_ip)
{

  char keyT = 'T';
  char enter = '\r';
  int t = 0;
  write(serial_port, &keyT, sizeof(keyT));
  usleep(1000);
  write(serial_port, &enter, sizeof(enter));
  usleep(100000);
  int n = strlen(get_ip);
  for (int i = 0; i < n; i++)
  {
    char data = get_ip[i];
    send_data(serial_port, &data, sizeof(data));
    printf("\n BL :%c\n", data);
    usleep(1000);
  }
  write(serial_port, &enter, sizeof(enter));
  while (1)
  {
    char *data1 = receive_data(serial_port);
    // printf("\nReceived message: %s\n", data1);
    //  //printf("Received message$ %s\n", data);
    if ((strchr(data1, 'Y') != NULL))
    {
      printf("\n BLackList done\n");
      break;
    }
  }
}

// Ham lay du lieu IP tu thread HTTPS Wl
void get_data_WL_from_Thread_HTTPS(char *get_ip)
{
  Send_IPv4_HTTPS_to_WhiteList(serial_port, get_ip);
}

// Ham gui blacklist xuong core
void Send_IPv4_HTTPS_to_WhiteList(int serial_port, char *get_ip)
{

  char key04 = 04;
  char enter = '\r';
  int t = 0;

  write(serial_port, &key04, sizeof(key04));
  usleep(1000);
  write(serial_port, &enter, sizeof(enter));
  usleep(100000);
  int n = strlen(get_ip);
  for (int i = 0; i < n; i++)
  {
    char data = get_ip[i];
    send_data(serial_port, &data, sizeof(data));
    printf("\n Wl :%c\n", data);
    usleep(10000);
  }
  write(serial_port, &enter, sizeof(enter));
  while (1)
  {
    char *data1 = receive_data(serial_port);
    //   printf("\nReceived message: %s\n", data1);
    //  //printf("Received message$ %s\n", data);
    if ((strchr(data1, 'Y') != NULL))
    {
      printf("\nWhilist done\n");
      break;
    }
  }
}

// Ham xu ly Chung HTTP/HTTPS
void process_pkg_https(int ret, int shutDown, WOLFSSL *ssl, char *addr_client, int timer_pkg)
{
  /*Xu ly save white-black list*/
  char buff1[2048];
  char buff2[2048];
  char buff3[2048];
  char version_http[10];
  char version_http_1[10];
  char version_http_c[100];
  const char *header_down_responds = "Content-Length: 0\r\n"
                                     "Connection: close\r\n\r\n";
  const char *text_buff = " 302 Found\n"; // 301 Moved Permanently

  char URN[200];
  char URN1[200];
  char URN2[200];
  char URN_C[200];
  char URN1_C[200];
  char URL[200];
  char URL1[200];
  char URL2[200];
  char URL_C[200];
  char scheme[200];
  char new_uri[200];
  char save_uri[200];
  char save_uri1[200];
  int a;
  int b;
  int c = 0;
  int d;

  size_t len;

  /* Message chuyen huong dung */
  // const char *message_direct_2 =
  //     "HTTP/1.1 302 Found\n"
  //     "Location: https://thi.com/\n"
  //     "Content-Length: 0\r\n"
  //     "Connection: close\r\n\r\n";

  const char *message_1 =
      "HTTP/1.1 302 Found\n"
      "Location: ";
  const char *message_2 = "\nContent-Length: 0\r\n"
                          "Connection: close\r\n\r\n";

  int trusted_block;
  int trusted_legit;
  int trusted_proceesing;

  char text_local[200]; // = "Location: "
  char save_buffer[1024];
  memset(buff1, 0, sizeof(buff1));
  if ((ret = wolfSSL_read(ssl, buff1, sizeof(buff1) - 1)) == -1)
  {
  }
  if (buff1 == NULL || strlen(buff1) == 0)
  {
    printf("Dang le la bi loi nek!!!\n");
    return;
  }
  if (strncmp(buff1, "shutdown", strlen("shutdown")) == 0)
  {
    shutDown = 1;
  }
  // printf("\r\nClient Message: %s\n", buff1);

  memset(buff2, 0, sizeof(buff2));
  strcpy(buff2, buff1);
  // printf("buff2: %s\n", buff2);
  memset(buff3, 0, sizeof(buff3));
  strcpy(buff3, buff1);
  // printf("buff3: %s\n", buff3);
  /* Get version HTTP */
  memset(version_http, 0, sizeof(version_http));
  // printf("\r\n buff3: %s\n", buff3);
  memcpy(version_http, strstr(buff3, "HTTP"), 10);
  // printf("version_http: %s\n", version_http);
  memset(version_http_1, 0, sizeof(version_http_1));
  strcpy(version_http_1, strtok(version_http, "\n"));
  memset(version_http_c, 0, sizeof(version_http_c));
  memcpy(version_http_c, version_http_1, strlen(version_http_1));
  version_http_c[strlen(version_http_c) - 1] = '\0'; // \0 la ky tu thong bao chuoi ket thuc
  strcat(version_http_c, text_buff);
  memset(URN, 0, sizeof(URN));
  memcpy(URN, strstr(buff2, "GET"), 200);
  memset(URN1, 0, sizeof(URN1));
  strcpy(URN1, strtok(URN, "\n"));
  memset(URN2, 0, sizeof(URN2));
  strcpy(URN2, strstr(URN1, "/"));
  memset(URN_C, 0, sizeof(URN_C));
  memcpy(URN_C, strtok(URN2, " "), strlen(URN2));

  // printf("URN: %s\n", URN);
  // printf("URN1: %s\n", URN1);
  // printf("URN_C: %s\n", URN_C);
  memset(URL, 0, sizeof(URL));
  memcpy(URL, strstr(buff1, "Host"), 200);
  memset(URL1, 0, sizeof(URL1));
  strcpy(URL1, strtok(URL, "\n"));
  memset(URL2, 0, sizeof(URL2));
  strcpy(URL2, strstr(URL1, " "));
  memset(URL_C, 0, sizeof(URL_C));
  memcpy(URL_C, strtok(URL2, " "), strlen(URL2));
  URL_C[strlen(URL_C) - 1] = '\0'; // URL_C se la dia chi IP dich (IP Host)
  memset(scheme, 0, sizeof(scheme));
  memcpy(scheme, "https://", strlen("https://"));
  strcat(scheme, URL_C);
  strcat(scheme, URN_C);
  memset(new_uri, 0, sizeof(new_uri));
  strcpy(new_uri, scheme);

  if (strncmp(buff1, "shutdown", strlen("shutdown")) == 0)
  {
    shutDown = 1;
  }
  get_HTTP_Data((unsigned char *)addr_client, (unsigned char *)new_uri, timer_pkg);

  trusted_proceesing = 0;
  trusted_proceesing = search_processing_table(addr_client, new_uri, &c);
  if (trusted_proceesing == 0)
  {
    // printf("\r\nDang xu ly -> chuyen huong sai\n");

    memset(save_uri, 0, sizeof(save_uri));
    memcpy(save_uri, "https://", strlen("https://"));
    strcat(save_uri, URL_C);

    /*
     * Generator URN to direct verify client
     */
    char *URN_gen;
    URN_gen = randomString(10);
    strcat(save_uri, "/");
    strcat(save_uri, URN_gen);
    free(URN_gen);

    /* Update URI_direct to Processing Table
     *
     */
    d = 0;
    update_processing(addr_client, new_uri, save_uri, &d);

    memset(text_local, 0, sizeof(text_local));
    strcat(text_local, "Location: ");
    strcat(text_local, save_uri); /* //printf("\r\nText_local: %s", text_local); */
    text_local[strlen(text_local)] = '\0';
    strcat(version_http_c, text_local);
    strcat(version_http_c, "\n"); /* //printf("\r\nVersion_http_c: %s", version_http_c); */

    /*
     * Create Packet HTTPS failse direct to Client
     */
    memset(buff1, 0, sizeof(save_buffer));
    memcpy(buff1, version_http_c, strlen(version_http_c)); /* //printf("\r\nBuffer_Reply_Failse: %s", buff1); */
    strcat(buff1, header_down_responds);
    len = strnlen(buff1, sizeof(buff1));
    delete_processing(c);
    // printf("\r\nSend Client 0: %s\n", buff1);
    // printf("\r\n ");
    if ((ret = wolfSSL_write(ssl, buff1, len)) != (int)len)
    {
    }
  }
  else if (trusted_proceesing == 1)
  {
    // printf("\r\nChua xu ly -> Tao Processing Table -> Chuyen huong sai\n");
    // printf("\r\nPHASE I: CHUYEN HUONG SAI 1");
    memset(save_uri, 0, sizeof(save_uri));
    memcpy(save_uri, "https://", strlen("https://"));
    strcat(save_uri, URL_C);
    // printf("URLC_my: %s\n", URL_C);
    // printf("saveurri_my: %s\n", save_uri);
    // printf("saveurri_my: %s\n", save_uri);
    memset(save_uri1, 0, sizeof(save_uri1));
    strcat(save_uri1, save_uri);

    memset(URN1_C, 0, sizeof(URN1_C));
    strcpy(URN1_C, URN_C);
    char *URN_gen;
    URN_gen = randomString(10);
    strcat(save_uri, "/");
    strcat(save_uri, URN_gen);
    free(URN_gen);

    create_processing(addr_client, new_uri, save_uri);
    display_processing();

    memset(text_local, 0, sizeof(text_local));
    strcat(text_local, "Location: ");
    strcat(text_local, save_uri);

    text_local[strlen(text_local)] = '\0';
    strcat(version_http_c, text_local);
    strcat(version_http_c, "\n");
    // printf("\r\nVersion_http_c: %s", version_http_c);
    memset(buff1, 0, sizeof(save_buffer));
    memcpy(buff1, version_http_c, strlen(version_http_c));
    // printf("\r\nText_local: %s", text_local);
    // printf("\r\nBuffer_Reply_Failse: %s", buff1);
    strcat(buff1, header_down_responds);
    len = strnlen(buff1, sizeof(buff1));
    // printf("\r\nCHUYEN HUONG SAI 1. - buff1: %s", buff1);
    // printf("\r\nSend Client 1 : %s\n", buff1);
    if ((ret = wolfSSL_write(ssl, buff1, len)) != (int)len)
    {
    }
  }
  else if (trusted_proceesing == 2)
  {
    // char *t = "/hello";
    // URN1_C
    // printf("\r\nPHASE II: CHUYEN HUONG DUNG");
    // create_whitelist(addr_client, processing_table_0->uri_request_firt);
    get_data_WL_from_Thread_HTTPS(addr_client);

    display_processing();
    memset(text_local, 0, sizeof(text_local));
    strcat(text_local, "Location: ");
    strcat(text_local, processing_table_0->uri_request_firt);
    text_local[strlen(text_local)] = '\0';
    strcat(version_http_c, text_local);
    strcat(version_http_c, "\n"); /* //printf("\r\nMessage_True: %s", version_http_c); */
    memset(buff1, 0, sizeof(buff1));

    memcpy(buff1, message_1, strlen(message_1)); /* //printf("\r\nBuffer_Reply_True: %s", buff1); */
    strcat(buff1, save_uri1);
    // strcat(buff1, t);
    strcat(buff1, URN1_C);
    strcat(buff1, message_2);
    // printf("THIDT: %s\n", buff1);
    delete_processing(c);
    len = strnlen(buff1, sizeof(buff1));
    if ((ret = wolfSSL_write(ssl, buff1, len)) != (int)len)
    {
    }
  }
}

/*ham chinh WOLFSSL*/

// THREAD_RETURN WOLFSSL_THREAD process_https(void *args)
void *process_https(void *args)
{
  printf("Hello WSSL\n");
  SOCKET_T sockfd = 0;
  WOLFSSL_METHOD *method = 0;
  WOLFSSL_CTX *ctx = 0;
  int ret = 0;
  int doDTLS = 0;
  int doPSK;
  int outCreated = 0;
  int shutDown = 0;
  int useAnyAddr = 0;
  word16 port;
  int argc = ((func_args *)args)->argc;
  char **argv = ((func_args *)args)->argv;
  // char buffer_error[WOLFSSL_MAX_ERROR_SZ];
  last_time = time(NULL);

#ifdef HAVE_TEST_SESSION_TICKET
  // MyTicketCtx myTicketCtx;
#endif
#ifdef ECHO_OUT
  FILE *fout = stdout;
  if (argc >= 2)
  {
    fout = fopen(argv[1], "w");
    outCreated = 1;
  }
  if (!fout)
    err_sys("can't open output file");
#endif
  (void)outCreated;
  (void)argc;
  (void)argv;

  ((func_args *)args)->return_code = -1; /* error state */
#ifdef WOLFSSL_DTLS
  // doDTLS = 1;
#endif
#if (defined(NO_RSA) && !defined(HAVE_ECC) && !defined(HAVE_ED25519) && \
     !defined(HAVE_ED448)) ||                                           \
    defined(WOLFSSL_LEANPSK)
  doPSK = 1;
#else
  doPSK = 0;
#endif
  port = 443;
  useAnyAddr = 1;
  tcp_listen(&sockfd, &port, useAnyAddr, doDTLS, 0);
  method = wolfTLSv1_2_server_method();
  ctx = wolfSSL_CTX_new(method);
  if (ctx == NULL)
    printf("ERROR: failed to create WOLFSSL_CTX\n");
  else
    printf("Run wolfSSL_CTX_new successfully\n");

  // wolfSSL_CTX_set_session_cache_mode(ctx, WOLFSSL_SESS_CACHE_OFF);

  //   #ifdef WOLFSSL_ENCRYPTED_KEYS
  //   wolfSSL_CTX_set_default_passwd_cb(ctx, PasswordCallBack);
  // #endif

  // #ifdef HAVE_TEST_SESSION_TICKET
  //   if (TicketInit() != 0)
  //     err_sys("unable to setup Session Ticket Key context");
  //   wolfSSL_CTX_set_TicketEncCb(ctx, myTicketEncCb);
  //   XMEMSET(&myTicketCtx, 0, sizeof(myTicketCtx));
  //   wolfSSL_CTX_set_TicketEncCtx(ctx, &myTicketCtx);
  // #endif

  // #ifndef NO_FILESYSTEM
  //   if (doPSK == 0)
  //   {
  // #if defined(HAVE_ECC) && !defined(WOLFSSL_SNIFFER)
  //     /* ecc */
  //     if (wolfSSL_CTX_use_certificate_file(ctx, eccCertFile, WOLFSSL_FILETYPE_PEM)
  //             != WOLFSSL_SUCCESS)
  //         err_sys("can't load server cert file, "
  //                 "Please run from wolfSSL home dir");

  //     if (wolfSSL_CTX_use_PrivateKey_file(ctx, eccKeyFile, WOLFSSL_FILETYPE_PEM)
  //             != WOLFSSL_SUCCESS)
  //         err_sys("can't load server key file, "
  //                 "Please run from wolfSSL home dir");
  // #elif defined(HAVE_ED25519) && !defined(WOLFSSL_SNIFFER)
  //     /* ed25519 */
  //     if (wolfSSL_CTX_use_certificate_chain_file(ctx, edCertFile)
  //             != WOLFSSL_SUCCESS)
  //         err_sys("can't load server cert file, "
  //                 "Please run from wolfSSL home dir");

  //     if (wolfSSL_CTX_use_PrivateKey_file(ctx, edKeyFile, WOLFSSL_FILETYPE_PEM)
  //             != WOLFSSL_SUCCESS)
  //         err_sys("can't load server key file, "
  //                 "Please run from wolfSSL home dir");
  // #elif defined(HAVE_ED448) && !defined(WOLFSSL_SNIFFER)
  //     /* ed448 */
  //     if (wolfSSL_CTX_use_certificate_chain_file(ctx, ed448CertFile)
  //             != WOLFSSL_SUCCESS)
  //         err_sys("can't load server cert file, "
  //                 "Please run from wolfSSL home dir");

  //     if (wolfSSL_CTX_use_PrivateKey_file(ctx, ed448KeyFile,
  //             WOLFSSL_FILETYPE_PEM) != WOLFSSL_SUCCESS)
  //         err_sys("can't load server key file, "
  //                 "Please run from wolfSSL home dir");
  // #elif defined(NO_CERTS)
  //     /* do nothing, just don't load cert files */
  // #else
  /* normal */
  // if (wolfSSL_CTX_use_certificate_file(ctx, svrCertFile, WOLFSSL_FILETYPE_PEM)
  //         != WOLFSSL_SUCCESS)
  // {
  //     err_sys("can't load server cert file, "
  //             "Please run from wolfSSL home dir");
  // }
  // else
  // {
  //     //printf("\r\nLoad SERVER CERT file successfully");
  // }

  // //printf("\r\nBetween wolfSSL_CTX_use_certificate_file() and wolfSSL_CTX_use_PrivateKey_file()");

  // if (wolfSSL_CTX_use_PrivateKey_file(ctx, svrKeyFile, WOLFSSL_FILETYPE_PEM)
  //         != WOLFSSL_SUCCESS)
  // {
  //     err_sys("can't load server key file, "
  //             "Please run from wolfSSL home dir");
  // }
  // else
  // {
  //     //printf("\r\nLoad SERVER CERT file successfully");
  // }
  // #endif

  //   } /* doPSK */
  // #elif !defined(NO_CERTS)
  //   if (!doPSK)
  //   {
  //     if (wolfSSL_CTX_use_certificate_buffer(ctx, server_cert_der_2048,
  //                                            sizeof_server_cert_der_2048, WOLFSSL_FILETYPE_ASN1) != WOLFSSL_SUCCESS)
  //       err_sys("can't load server cert buffer");

  //     if (wolfSSL_CTX_use_PrivateKey_buffer(ctx, server_key_der_2048,
  //                                           sizeof_server_key_der_2048, WOLFSSL_FILETYPE_ASN1) != WOLFSSL_SUCCESS)
  //       err_sys("can't load server key buffer");
  //   }
  // #endif

  ret = wolfSSL_CTX_use_certificate_buffer(ctx, cert_server_pem, sizeof(cert_server_pem), WOLFSSL_FILETYPE_PEM);
  if (ret != WOLFSSL_SUCCESS)
    err_sys("ERROR: failed to load server certificate in buffer\n");
  else
    // printf("Load SERVER CERT successfully\n");

    ret = wolfSSL_CTX_use_PrivateKey_buffer(ctx, key_server_pem, sizeof(key_server_pem), SSL_FILETYPE_PEM);
  if (ret != WOLFSSL_SUCCESS)
    err_sys("ERROR: failed to load server certificate in buffer\n");
  else
    // printf("Load SERVER KEY successfully\n");

    // #if defined(WOLFSSL_SNIFFER)
    //     /* Only set if not running testsuite */
    //     if (XSTRSTR(argv[0], "testsuite") == NULL)
    //     {
    //       /* don't use EDH, can't sniff tmp keys */
    //       wolfSSL_CTX_set_cipher_list(ctx, "AES256-SHA");
    //     }
    // #endif

    if (doPSK)
    {
#ifndef NO_PSK
      const char *defaultCipherList;

      // printf("Run wolfSSL_CTX_set_psk_server_callback\n");
      wolfSSL_CTX_set_psk_server_callback(ctx, my_psk_server_cb);
      wolfSSL_CTX_use_psk_identity_hint(ctx, "cyassl server");
#ifdef HAVE_NULL_CIPHER
      defaultCipherList = "PSK-NULL-SHA256";
#elif defined(HAVE_AESGCM) && !defined(NO_DH)
#ifdef WOLFSSL_TLS13
      defaultCipherList = "TLS13-AES128-GCM-SHA256"
#ifndef WOLFSSL_NO_TLS12
                          ":DHE-PSK-AES128-GCM-SHA256"
#endif
          ;
#else
      defaultCipherList = "DHE-PSK-AES128-GCM-SHA256";
#endif
#elif defined(HAVE_AESGCM) && defined(WOLFSSL_TLS13)
      defaultCipherList = "TLS13-AES128-GCM-SHA256"
#ifndef WOLFSSL_NO_TLS12
                          ":PSK-AES128-GCM-SHA256"
#endif
          ;
#else
      defaultCipherList = "PSK-AES128-CBC-SHA256";
#endif
      if (wolfSSL_CTX_set_cipher_list(ctx, defaultCipherList) != WOLFSSL_SUCCESS)
        err_sys("server can't set cipher list 2");
      else
        // printf("\r\nServer set cipher list successfully");
        wolfSSL_CTX_set_psk_callback_ctx(ctx, (void *)defaultCipherList);
#endif
    }

#ifdef WOLFSSL_ASYNC_CRYPT
  ret = wolfAsync_DevOpen(&devId);
  if (ret < 0)
  {
    fprintf(stderr, "Async device open failed\nRunning without async\n");
  }
  wolfSSL_CTX_SetDevId(ctx, devId);
#endif /* WOLFSSL_ASYNC_CRYPT */

  SignalReady(args, port);

  int soluong = 0;
  int tsoluong = 0;
  time_t start_time = time(NULL);
  while (1)
  {
    clock_gettime(CLOCK_MONOTONIC, &start);
    if (shutDown)
    {
      break;
    }

    time_t current_time = time(NULL);
    if (difftime(current_time, start_time) >= 1)
    {
      // printf("Connection/s: %d\n", soluong);
      //  printf("\n\r\n");
      //  printf("time pr: %ld\n", current_time);
      //  printf("time pr: %ld\n", start_time);
      //  In ra "hello"
      start_time = current_time; // C?p nh?t l?i th?i gian b?t d?u
      soluong = 0;
    }

    /*WOLF SSL begin*/
    WOLFSSL *ssl = NULL;
    WOLFSSL *write_ssl = NULL; /* may have separate w/ HAVE_WRITE_DUP */
    char command[SVR_COMMAND_SIZE + 1];
    int clientfd;
    int firstRead = 1;
    int gotFirstG = 0;
    int err = 0;
    char *addr_client;
    int timer_pkg;
    SOCKADDR_IN_T client;
    socklen_t client_len = sizeof(client);

#ifndef WOLFSSL_DTLS
    /*Accept client connections*/
    clientfd = accept(sockfd, (struct sockaddr *)&client,
                      (ACCEPT_THIRD_T)&client_len);
    addr_client = inet_ntoa(client.sin_addr);
    timer_pkg = hash_time_to_int();
    if (clientfd == -1)
      // {
      //   printf("\n\r2\n");
      //   // printf("\r\nERROR: failed to accept the connection");
      //   ret = -1;
      //   // goto server_cleannup;
      // }
      if (clientfd == -1)
      {
        // printf("\n\r2\n");
        ret = -1;
      }
      else
      {
        /* Tắt TCP Keep-Alive trên socket */
        int optval = 0;
        setsockopt(clientfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));

        //  printf("Accepted connection from %s\n", addr_client);
      }
#else
    // clientfd = sockfd;
    // {
    //   //  printf("\n\r3\n");
    //   /* For DTLS, peek at the next datagram so we can get the client's
    //    * address and set it into the ssl object later to generate the
    //    * cookie. */
    //   int n;
    //   byte b[1500];
    //   n = (int)recvfrom(clientfd, (char *)b, sizeof(b), MSG_PEEK,
    //                     (struct sockaddr *)&client, &client_len);
    //   if (n <= 0)
    //     err_sys("recvfrom failed");
    // }
#endif
    if (WOLFSSL_SOCKET_IS_INVALID(clientfd))
      err_sys("tcp accept failed");
    // printf("\n\r3\n");
    // Create a WOLFSSL object
    ssl = wolfSSL_new(ctx);
    if (ssl == NULL)
      err_sys("SSL_new failed");
    else
      // printf("\r\nwolfSSL_new ctx successfully");
      //  printf("\n\r5\n");
      // Attach wolfSSL to the socket
      if (wolfSSL_set_fd(ssl, clientfd) == WOLFSSL_SUCCESS)
      {
      }
      // printf("\r\nwolfSSL_set_fd SUCCESSED");
      // else
      //  printf("\r\nwolfSSL_set_fd FAILED");

#ifdef WOLFSSL_DTLS
    wolfSSL_dtls_set_peer(ssl, &client, client_len);
#endif
#if !defined(NO_FILESYSTEM) && !defined(NO_DH) && !defined(NO_ASN)
    wolfSSL_SetTmpDH_file(ssl, dhParamFile, WOLFSSL_FILETYPE_PEM);
#elif !defined(NO_DH)
    SetDH(ssl); /* will repick suites with DHE, higher than PSK */
#endif

    do
    {
      err = 0; /* Reset error */
      // Establish TLS connection
      ret = wolfSSL_accept(ssl);
      if (ret != WOLFSSL_SUCCESS)
      {
        //  printf("\n\r6\n");
        err = wolfSSL_get_error(ssl, 0);
#ifdef WOLFSSL_ASYNC_CRYPT
        if (err == WC_PENDING_E)
        {
          ret = wolfSSL_AsyncPoll(ssl, WOLF_POLL_FLAG_CHECK_HW);
          if (ret < 0)
            printf("\n\r7\n");
          break;
        }
#endif
      }
    } while (err == WC_PENDING_E);
    if (ret != WOLFSSL_SUCCESS)
    {
      // printf("\n\r8\n");
      //  fprintf(stderr, "SSL_accept error = %d, %s\n", err,
      //   wolfSSL_ERR_error_string((unsigned long)err, buffer_error));
      // fprintf(stderr, "SSL_accept failed\n");
      wolfSSL_free(ssl);
      CloseSocket(clientfd);

      continue;
    }
    process_pkg_https(ret, shutDown, ssl, addr_client, timer_pkg);
    // shutDown = 1;
    // test_start:
    soluong = soluong + 1;
    tsoluong = tsoluong + 1;

    //  printf("\r\n\t\t\t            { Client connected successfully }              ");
    // printf("Tong Connection/s: %d\n", tsoluong);
    // printf("\r\n");
    goto Skip_Sample;
#if defined(PEER_INFO)
    showPeer(ssl);
#endif

#ifdef HAVE_WRITE_DUP
    write_ssl = wolfSSL_write_dup(ssl);
    if (write_ssl == NULL)
    {
      fprintf(stderr, "wolfSSL_write_dup failed\n");
      wolfSSL_free(ssl);
      CloseSocket(clientfd);
      continue;
    }
#else
    write_ssl = ssl;
#endif

    while (1)
    {
      int echoSz;
      do
      {
        err = 0; /* reset error */
        ret = wolfSSL_read(ssl, command, sizeof(command) - 1);
        if (ret <= 0)
        {
          err = wolfSSL_get_error(ssl, 0);
#ifdef WOLFSSL_ASYNC_CRYPT
          if (err == WC_PENDING_E)
          {
            ret = wolfSSL_AsyncPoll(ssl, WOLF_POLL_FLAG_CHECK_HW);
            if (ret < 0)
              break;
          }
#endif
        }
      } while (err == WC_PENDING_E);
      if (ret <= 0)
      {
        if (err != WOLFSSL_ERROR_WANT_READ && err != WOLFSSL_ERROR_ZERO_RETURN)
        {
          // fprintf(stderr, "SSL_read echo error %d, %s!\n", err,
          //     wolfSSL_ERR_error_string((unsigned long)err, buffer_error));
        }
        break;
      }

      echoSz = ret;

      if (firstRead == 1)
      {
        firstRead = 0; /* browser may send 1 byte 'G' to start */
        if (echoSz == 1 && command[0] == 'G')
        {
          gotFirstG = 1;
          continue;
        }
      }
      else if (gotFirstG == 1 && strncmp(command, "ET /", 4) == 0)
      {
        memcpy(command, "GET", 4);
        /* fall through to normal GET */
      }

      if (strncmp(command, "quit", 4) == 0)
      {
        // printf("client sent quit command: shutting down!\n");
        shutDown = 1;
        break;
      }
      if (strncmp(command, "break", 5) == 0)
      {
        // printf("client sent break command: closing session!\n");
        break;
      }
#ifdef PRINT_SESSION_STATS
      if (strncmp(command, "printstats", 10) == 0)
      {
        wolfSSL_PrintSessionStats();
        break;
      }
#endif
      if (strncmp(command, "GET", 3) == 0)
      {
        const char resp[] =
            "HTTP/1.0 200 ok\r\nContent-type: text/html\r\n\r\n"
            "<html><body BGCOLOR=\"#ffffff\"><pre>\r\n"
            "greetings from wolfSSL\r\n</pre></body></html>\r\n\r\n";

        echoSz = (int)strlen(resp) + 1;
        if (echoSz > (int)sizeof(command))
        {
          /* Internal error. */
          err_sys("HTTP response greater than buffer.");
        }
        memcpy(command, resp, sizeof(command));

        do
        {
          err = 0; /* reset error */
          ret = wolfSSL_write(write_ssl, command, echoSz);
          if (ret <= 0)
          {
            err = wolfSSL_get_error(write_ssl, 0);
#ifdef WOLFSSL_ASYNC_CRYPT
            if (err == WC_PENDING_E)
            {
              ret = wolfSSL_AsyncPoll(write_ssl, WOLF_POLL_FLAG_CHECK_HW);
              if (ret < 0)
                break;
            }
#endif
          }
        } while (err == WC_PENDING_E);
        if (ret != echoSz)
        {
          // fprintf(stderr, "SSL_write get error = %d, %s\n", err,
          // wolfSSL_ERR_error_string((unsigned long)err, buffer_error));
          // err_sys("SSL_write get failed");
        }
        break;
      }
      command[echoSz] = 0;

#ifdef ECHO_OUT
      LIBCALL_CHECK_RET(fputs(command, fout));
#endif

      do
      {
        err = 0; /* reset error */
        ret = wolfSSL_write(write_ssl, command, echoSz);
        if (ret <= 0)
        {
          err = wolfSSL_get_error(write_ssl, 0);
#ifdef WOLFSSL_ASYNC_CRYPT
          if (err == WC_PENDING_E)
          {
            ret = wolfSSL_AsyncPoll(write_ssl, WOLF_POLL_FLAG_CHECK_HW);
            if (ret < 0)
              break;
          }
#endif
        }
      } while (err == WC_PENDING_E);

      if (ret != echoSz)
      {
        // fprintf(stderr, "SSL_write echo error = %d, %s\n", err,
        //       wolfSSL_ERR_error_string((unsigned long)err, buffer_error));
        // err_sys("SSL_write echo failed");
      }
    }

  Skip_Sample:

    clock_gettime(CLOCK_MONOTONIC, &end);

    elapsed = (end.tv_sec - start.tv_sec) +
              (end.tv_nsec - start.tv_nsec) / 1e9;

    // printf("Thoi gian: %.9f giay\n", elapsed);
#ifndef WOLFSSL_DTLS
    /* Notify the client that the connection is ending */
    wolfSSL_shutdown(ssl);
    // printf("\r\n\t\t\t <----------------- Shutdown complete ------------------>");
#endif
#ifdef HAVE_WRITE_DUP
    wolfSSL_free(write_ssl);
#endif
    wolfSSL_free(ssl);
    CloseSocket(clientfd);
#ifdef WOLFSSL_DTLS
    tcp_listen(&sockfd, &port, useAnyAddr, doDTLS, 0);
    SignalReady(args, port);
#endif
  }

  CloseSocket(sockfd);
  wolfSSL_CTX_free(ctx);

#ifdef ECHO_OUT
  if (outCreated)
    fclose(fout);
#endif

  ((func_args *)args)->return_code = 0;

#if defined(NO_MAIN_DRIVER) && defined(HAVE_ECC) && defined(FP_ECC) && defined(HAVE_THREAD_LS)
  wc_ecc_fp_free(); /* free per thread cache */
#endif

#ifdef WOLFSSL_TIRTOS
  fdCloseSession(Task_self());
#endif

#ifdef HAVE_TEST_SESSION_TICKET
  TicketCleanup();
#endif

#ifdef WOLFSSL_ASYNC_CRYPT
  wolfAsync_DevClose(&devId);
#endif

  WOLFSSL_RETURN_FROM_THREAD(0);
}

// by thi :v
//========================================================================================
//  Ham tao socket va ket noi den server
void connect_to_server()
{
  struct sockaddr_un addr;

  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
  {
    perror("\nsocket error\n");

    return;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
  {
    perror("\nConnect error\n");
    close(fd);
    return;
  }
  signal(SIGPIPE, SIG_IGN);
  printf("\nConnected to server!\n");
}

// Send msg
void send_data_socket_uds(const char *message_uds)
{

  if (write(fd, message_uds, strlen(message_uds)) == -1)
  {
    //  printf("\nFD: %d\n", fd);

    // perror("\nwrite error\n");
    close(fd);
    connect_to_server();
  }
}

void close_connection()
{
  close(fd);
}

void tachChuoi(const char *message, char *chuoiKyTu, char *chuoiSo)
{
  char *delimiter = strchr(message, '$');
  if (delimiter != NULL)
  {
    strncpy(chuoiKyTu, message, delimiter - message);
    chuoiKyTu[delimiter - message] = '\0';
    strcpy(chuoiSo, delimiter + 1);
  }
  else
  {
    strcpy(chuoiKyTu, message);
    chuoiSo[0] = '\0';
  }
}

void process_buffer(char *client_message, int client_sock)
{
  // //printf("\n process_buffer $ %s\n",client_message);
  char buffer[2000] = "";
  char *token;
  char ID[50];
  char Value[50];

  token = strtok(client_message, "$");
  // //printf("\ntoken$ %s\n",token);
  while (token != NULL)
  {
    strcpy(ID, token);
    token = strtok(NULL, "$");
    if (token != NULL)
    {
      strcpy(Value, token);
      process_key(serial_port, ID, Value, client_sock, buffer);
      sleep(1);
    }

    token = strtok(NULL, "$");
    // //printf("\ntoken$ %s\n",token);
  }
}
void ModeGui(int serial_port)
{
  printf("\nHelloGUI!\n");
  int socket_desc, client_sock, c;
  struct sockaddr_in server, client;
  char client_message[2000];
  char response_message[2000] = "OK";
  char ID[50], Value[50];
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_desc == -1)
  {
    printf("Could not create socket");
  }
  puts("Socket created");

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(3367);

  if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    perror("bind failed. Error");
    exit(1);
  }
  // puts("Bind done");
  memset(client_message, 0, sizeof(client_message));
  listen(socket_desc, 3);
  puts("Waiting for incoming connections...");
  puts("GUI ready to use...");
  c = sizeof(struct sockaddr_in);
  while (1)
  {

    memset(client_message, 0, sizeof(client_message));
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);

    if (client_sock < 0)
    {
      perror("accept failed");
      exit(1);
    }
    puts("Connection accepted");
    (recv(client_sock, client_message, 2000, 0));
    if (strlen(client_message) > 8)
    {
      printf("Received character$ %s\n", client_message);
      process_buffer(client_message, client_sock);
    }

    if (client_sock < 0)
    {
      perror("recv failed");
      exit(1);
    }
    close(client_sock);
  }
}

void process_key(int serial_port, char *ID, char *Value, int client_sock, char *buffer)
{
  char buffer1[24];
  char buffer2[24];
  char buffer3[24];
  char enter = '\r';
  char keyY = 'Y';
  char keyN = 'N';
  char keyA = 'A';
  char keyB = 'B';
  char keyC = 'C';
  char keyD = 'D';
  char keyE = 'E';
  char keyF = 'F';
  char keyG = 'G';
  char keyH = 'H';
  char keyI = 'I';
  char keyJ = 'J';
  char keyK = 'K';
  char keyL = 'L';
  char keyM = 'M';
  char keyO = 'O';
  char keyQ = 'Q';
  char keyP = 'P';
  char keyZ = 'Z';
  char keyV = 'V';
  char keyR = 'R';
  char keyX = 'X';
  char keyS = 'S';
  char keyU = 'U';
  char keyW = 'W';
  char keyT = 'T';
  char key0 = '0';
  char key1 = '1';
  char key2 = '2';
  char key3 = '3';
  char key4 = '4';
  char key5 = '5';
  char key6 = '6';
  char key7 = '7';
  char key8 = '8';
  char key9 = '9';
  char keyva = '&';
  char keytru = '-';
  char keycong = '+';
  char keybang = '=';
  char keyhoi = '?';
  char keythang = '#';
  char keymu = '^';
  char keychamphay = ';';
  char keyngoacnhontrai = '{';
  char keyngoacnhonphai = '}';
  char keycham = '.';
  char keynhaynguoc = '`';
  char key_01 = 01;
  char key_02 = 02;
  char key_03 = 03;
  char key_04 = 04;

  char key_05 = 05;
  char key_06 = 06;
  char key_0A = 0x0A;
  char key_0D = 0x0D;
  char key_08 = 0x08;
  char key_09 = 0x09;
  char key_FF = 0xFF;
  char key_07 = 0x07;

  int t = 0;
  //=============================================================================================================
  // Set SYN_Threshould
  if (strcmp(ID, "PORT1_SYN_THR") == 0 || strcmp(ID, "PORT2_SYN_THR") == 0 ||
      strcmp(ID, "PORT3_SYN_THR") == 0 || strcmp(ID, "PORT4_SYN_THR") == 0 ||
      strcmp(ID, "PORT5_SYN_THR") == 0 || strcmp(ID, "PORT6_SYN_THR") == 0 ||
      strcmp(ID, "PORT7_SYN_THR") == 0 || strcmp(ID, "PORT8_SYN_THR") == 0)
  {
    t = 0;
    write(serial_port, &key7, sizeof(key7));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);

    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }

    usleep(1000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(100000);
    }
    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\n SYN Thr done \n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n SYN Thr error \n");
        break;
      }
      t++;
    }
  }

  // Set ACK_Threashold
  else if (strcmp(ID, "PORT1_ACK_THR") == 0 || strcmp(ID, "PORT2_ACK_THR") == 0 ||
           strcmp(ID, "PORT3_ACK_THR") == 0 || strcmp(ID, "PORT4_ACK_THR") == 0 ||
           strcmp(ID, "PORT5_ACK_THR") == 0 || strcmp(ID, "PORT6_ACK_THR") == 0 ||
           strcmp(ID, "PORT7_ACK_THR") == 0 || strcmp(ID, "PORT8_ACK_THR") == 0)
  {
    t = 0;
    write(serial_port, &key8, sizeof(key8));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);

    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(10000);

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {

      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(100000);
    }
    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nACK_THR done \n");
        usleep(100000);
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nACK_THR error \n");
        break;
      }
      t++;
    }
  }

  // Set en/dis SYN
  else if (strcmp(ID, "PORT1_SYN_EN_DIS") == 0 || strcmp(ID, "PORT2_SYN_EN_DIS") == 0 ||
           strcmp(ID, "PORT3_SYN_EN_DIS") == 0 || strcmp(ID, "PORT4_SYN_EN_DIS") == 0 ||
           strcmp(ID, "PORT5_SYN_EN_DIS") == 0 || strcmp(ID, "PORT6_SYN_EN_DIS") == 0 ||
           strcmp(ID, "PORT7_SYN_EN_DIS") == 0 || strcmp(ID, "PORT8_SYN_EN_DIS") == 0)
  {
    t = 0;
    write(serial_port, &key6, sizeof(key6));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(10000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &keyY, sizeof(keyY));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &keyN, sizeof(keyN));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nSYN_EN_DIS done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nSYN_EN_DIS error\n");
        break;
      }
      t++;
    }
  }
  //================================================================================================================
  // Set UDP_Threshould
  else if (strcmp(ID, "PORT1_UDP_THR") == 0 || strcmp(ID, "PORT2_UDP_THR") == 0 ||
           strcmp(ID, "PORT3_UDP_THR") == 0 || strcmp(ID, "PORT4_UDP_THR") == 0 ||
           strcmp(ID, "PORT5_UDP_THR") == 0 || strcmp(ID, "PORT6_UDP_THR") == 0 ||
           strcmp(ID, "PORT7_UDP_THR") == 0 || strcmp(ID, "PORT8_UDP_THR") == 0)
  {
    t = 0;
    write(serial_port, &keyC, sizeof(keyC));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(1000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {

      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(100000);
    }
    write(serial_port, &enter, sizeof(enter));

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nUDP_THR done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nUDP_THR error\n");
        break;
      }
      t++;
    }
  }

  // Set UDP_Threashold_perSecond
  else if (strcmp(ID, "PORT1_UDP_THR_PS") == 0 || strcmp(ID, "PORT2_UDP_THR_PS") == 0 ||
           strcmp(ID, "PORT3_UDP_THR_PS") == 0 || strcmp(ID, "PORT4_UDP_THR_PS") == 0 ||
           strcmp(ID, "PORT5_UDP_THR_PS") == 0 || strcmp(ID, "PORT6_UDP_THR_PS") == 0 ||
           strcmp(ID, "PORT7_UDP_THR_PS") == 0 || strcmp(ID, "PORT8_UDP_THR_PS") == 0)
  {
    t = 0;
    write(serial_port, &keyD, sizeof(keyD));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(1000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {

      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(100000);
    }
    write(serial_port, &enter, sizeof(enter));

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nUDP_THR_PS done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nUDP_THR_PS error\n");
        break;
        // }
      }
      t++;
    }
  }
  // Set en/dis UDP
  else if (strcmp(ID, "PORT1_UDP_EN_DIS") == 0 || strcmp(ID, "PORT2_UDP_EN_DIS") == 0 ||
           strcmp(ID, "PORT3_UDP_EN_DIS") == 0 || strcmp(ID, "PORT4_UDP_EN_DIS") == 0 ||
           strcmp(ID, "PORT5_UDP_EN_DIS") == 0 || strcmp(ID, "PORT6_UDP_EN_DIS") == 0 ||
           strcmp(ID, "PORT7_UDP_EN_DIS") == 0 || strcmp(ID, "PORT8_UDP_EN_DIS") == 0)
  {
    t = 0;
    write(serial_port, &keyB, sizeof(keyB));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(1000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &keyY, sizeof(keyY));
      usleep(10000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &keyN, sizeof(keyN));
      usleep(10000);
      write(serial_port, &enter, sizeof(enter));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nUDP_EN_DIS\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nUDP_EN_DIS ERROR\n");
        break;
      }
      t++;
    }
  }
  //=================================================================================================================
  // Set ICMP_Threshould
  else if (strcmp(ID, "PORT1_ICMP_THR") == 0 || strcmp(ID, "PORT2_ICMP_THR") == 0 ||
           strcmp(ID, "PORT3_ICMP_THR") == 0 || strcmp(ID, "PORT4_ICMP_THR") == 0 ||
           strcmp(ID, "PORT5_ICMP_THR") == 0 || strcmp(ID, "PORT6_ICMP_THR") == 0 ||
           strcmp(ID, "PORT7_ICMP_THR") == 0 || strcmp(ID, "PORT8_ICMP_THR") == 0)
  {
    t = 0;
    write(serial_port, &keyH, sizeof(keyH));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(1000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(10000);
    }
    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nICMP_THR done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nICMP_THR error\n");
        break;
      }
      t++;
    }
  }

  // Set ICMP_Threashold_perSecond
  else if (strcmp(ID, "PORT1_ICMP_THR_PS") == 0 || strcmp(ID, "PORT2_ICMP_THR_PS") == 0 ||
           strcmp(ID, "PORT3_ICMP_THR_PS") == 0 || strcmp(ID, "PORT4_ICMP_THR_PS") == 0 ||
           strcmp(ID, "PORT5_ICMP_THR_PS") == 0 || strcmp(ID, "PORT6_ICMP_THR_PS") == 0 ||
           strcmp(ID, "PORT7_ICMP_THR_PS") == 0 || strcmp(ID, "PORT8_ICMP_THR_PS") == 0)
  {
    t = 0;
    write(serial_port, &keyI, sizeof(keyI));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(1000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(10000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nICMP_THR_PS done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nICMP_THR_PS error\n");
        break;
      }
      t++;
    }
  }

  // Set en/dis ICMP
  else if (strcmp(ID, "PORT1_ICMP_EN_DIS") == 0 || strcmp(ID, "PORT2_ICMP_EN_DIS") == 0 ||
           strcmp(ID, "PORT3_ICMP_EN_DIS") == 0 || strcmp(ID, "PORT4_ICMP_EN_DIS") == 0 ||
           strcmp(ID, "PORT5_ICMP_EN_DIS") == 0 || strcmp(ID, "PORT6_ICMP_EN_DIS") == 0 ||
           strcmp(ID, "PORT7_ICMP_EN_DIS") == 0 || strcmp(ID, "PORT8_ICMP_EN_DIS") == 0)
  {
    t = 0;
    write(serial_port, &keyG, sizeof(keyG));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(1000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &keyY, sizeof(keyY));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &keyN, sizeof(keyN));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nICMP_EN_DIS done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nICMP_EN_DIS error\n");
        break;
      }
      t++;
    }
  }
  //=================================================================================================================
  // Set en/dis LAND
  else if (strcmp(ID, "PORT1_LAND_EN_DIS") == 0 || strcmp(ID, "PORT2_LAND_EN_DIS") == 0 ||
           strcmp(ID, "PORT3_LAND_EN_DIS") == 0 || strcmp(ID, "PORT4_LAND_EN_DIS") == 0 ||
           strcmp(ID, "PORT5_LAND_EN_DIS") == 0 || strcmp(ID, "PORT6_LAND_EN_DIS") == 0 ||
           strcmp(ID, "PORT7_LAND_EN_DIS") == 0 || strcmp(ID, "PORT8_LAND_EN_DIS") == 0)
  {
    t = 0;
    write(serial_port, &keyA, sizeof(keyA));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(1000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &keyY, sizeof(keyY));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &keyN, sizeof(keyN));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nLAND_EN_DIS done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nLAND_EN_DIS error\n");
        break;
      }
      t++;
    }
  }
  //=================================================================================================================
  // Set en/dis DNS
  else if (strcmp(ID, "PORT1_DNS_EN_DIS") == 0 || strcmp(ID, "PORT2_DNS_EN_DIS") == 0 ||
           strcmp(ID, "PORT3_DNS_EN_DIS") == 0 || strcmp(ID, "PORT4_DNS_EN_DIS") == 0 ||
           strcmp(ID, "PORT5_DNS_EN_DIS") == 0 || strcmp(ID, "PORT6_DNS_EN_DIS") == 0 ||
           strcmp(ID, "PORT7_DNS_EN_DIS") == 0 || strcmp(ID, "PORT8_DNS_EN_DIS") == 0)
  {
    t = 0;
    write(serial_port, &keyE, sizeof(keyE));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(1000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &keyY, sizeof(keyY));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &keyN, sizeof(keyN));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nDNS_EN_DIS dones\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        printf("\nDNS_EN_DIS error\n");
        break;
      }
      t++;
    }
  }

  // Set DNS_Threshould
  else if (strcmp(ID, "PORT1_DNS_THR") == 0 || strcmp(ID, "PORT2_DNS_THR") == 0 ||
           strcmp(ID, "PORT3_DNS_THR") == 0 || strcmp(ID, "PORT4_DNS_THR") == 0 ||
           strcmp(ID, "PORT5_DNS_THR") == 0 || strcmp(ID, "PORT6_DNS_THR") == 0 ||
           strcmp(ID, "PORT7_DNS_THR") == 0 || strcmp(ID, "PORT8_DNS_THR") == 0)
  {
    t = 0;
    write(serial_port, &keyF, sizeof(keyF));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(1000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {

      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(100000);
    }
    write(serial_port, &enter, sizeof(enter));

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nDNS_THR dones\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nDNS_THR error\n");
        break;
      }
      t++;
    }
  }

  //=================================================================================================================

  // Set en/dis IPSEC
  else if (strcmp(ID, "PORT1_IPSEC_IKE_EN_DIS") == 0 || strcmp(ID, "PORT2_IPSEC_IKE_EN_DIS") == 0 ||
           strcmp(ID, "PORT3_IPSEC_IKE_EN_DIS") == 0 || strcmp(ID, "PORT4_IPSEC_IKE_EN_DIS") == 0 ||
           strcmp(ID, "PORT5_IPSEC_IKE_EN_DIS") == 0 || strcmp(ID, "PORT6_IPSEC_IKE_EN_DIS") == 0 ||
           strcmp(ID, "PORT7_IPSEC_IKE_EN_DIS") == 0 || strcmp(ID, "PORT8_IPSEC_IKE_EN_DIS") == 0)
  {
    t = 0;
    write(serial_port, &keyJ, sizeof(keyJ));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &keyY, sizeof(keyY));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &keyN, sizeof(keyN));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nIPSEC_IKE_EN_DIS done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nIPSEC_IKE_EN_DIS ERROR\n");
        break;
      }
      t++;
    }
  }

  // Set IPSEC_IKE_THR
  else if (strcmp(ID, "PORT1_IPSEC_IKE_THR") == 0 || strcmp(ID, "PORT2_IPSEC_IKE_THR") == 0 ||
           strcmp(ID, "PORT3_IPSEC_IKE_THR") == 0 || strcmp(ID, "PORT4_IPSEC_IKE_THR") == 0 ||
           strcmp(ID, "PORT5_IPSEC_IKE_THR") == 0 || strcmp(ID, "PORT6_IPSEC_IKE_THR") == 0 ||
           strcmp(ID, "PORT7_IPSEC_IKE_THR") == 0 || strcmp(ID, "PORT8_IPSEC_IKE_THR") == 0)
  {
    t = 0;
    write(serial_port, &keyK, sizeof(keyK));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(1000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {

      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(100000);
    }
    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nIPSEC_IKE_THR done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nIPSEC_IKE_THR error\n");
        break;
      }
      t++;
    }
  }

  //=================================================================================================================
  // Set en/dis UDP Fragmentation
  else if (strcmp(ID, "PORT1_UDP_FRA_EN_DIS") == 0 || strcmp(ID, "PORT2_UDP_FRA_EN_DIS") == 0 ||
           strcmp(ID, "PORT3_UDP_FRA_EN_DIS") == 0 || strcmp(ID, "PORT4_UDP_FRA_EN_DIS") == 0 ||
           strcmp(ID, "PORT5_UDP_FRA_EN_DIS") == 0 || strcmp(ID, "PORT6_UDP_FRA_EN_DIS") == 0 ||
           strcmp(ID, "PORT7_UDP_FRA_EN_DIS") == 0 || strcmp(ID, "PORT8_UDP_FRA_EN_DIS") == 0)
  {
    t = 0;
    write(serial_port, &keyO, sizeof(keyO));
    usleep(1000);

    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(1000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &keyY, sizeof(keyY));
      usleep(1000);

      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &keyN, sizeof(keyN));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nUDP_FRA_EN_DIS done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nUDP_FRA_EN_DIS ERROR\n");
        break;
      }
      t++;
    }
  }

  //=============================================================================================================
  //=====================================================================================
  // Set en/dis TCP Fragmentation
  else if (strcmp(ID, "PORT1_TCP_FRA_EN_DIS") == 0 || strcmp(ID, "PORT2_TCP_FRA_EN_DIS") == 0 ||
           strcmp(ID, "PORT3_TCP_FRA_EN_DIS") == 0 || strcmp(ID, "PORT4_TCP_FRA_EN_DIS") == 0 ||
           strcmp(ID, "PORT5_TCP_FRA_EN_DIS") == 0 || strcmp(ID, "PORT6_TCP_FRA_EN_DIS") == 0 ||
           strcmp(ID, "PORT7_TCP_FRA_EN_DIS") == 0 || strcmp(ID, "PORT8_TCP_FRA_EN_DIS") == 0)
  {
    t = 0;
    write(serial_port, &keyN, sizeof(keyN));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(1000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &keyY, sizeof(keyY));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &keyN, sizeof(keyN));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nTCP_FRA_EN_DIS done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nTCP_FRA_EN_DIS error\n");
        break;
      }
      t++;
    }
  }

  //=============================================================================================================

  //=================================================================================================================
  // Set Time_Delete_White list
  else if (strcmp(ID, "TIME_WHITE_LIST") == 0)
  {
    t = 0;
    write(serial_port, &key9, sizeof(key9));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {

      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(100000);
    }
    write(serial_port, &enter, sizeof(enter));

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\n TIME_WHITE_LIST done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n TIME_WHITE_LIST ERROR\n");
        break;
      }
      t++;
    }
  }

  //=================================================================================================================
  // Set Time_Detect_Actack
  else if (strcmp(ID, "PORT1_TIME_DETECT_ATTACK") == 0 || strcmp(ID, "PORT2_TIME_DETECT_ATTACK") == 0 ||
           strcmp(ID, "PORT3_TIME_DETECT_ATTACK") == 0 || strcmp(ID, "PORT4_TIME_DETECT_ATTACK") == 0 ||
           strcmp(ID, "PORT5_TIME_DETECT_ATTACK") == 0 || strcmp(ID, "PORT6_TIME_DETECT_ATTACK") == 0 ||
           strcmp(ID, "PORT7_TIME_DETECT_ATTACK") == 0 || strcmp(ID, "PORT8_TIME_DETECT_ATTACK") == 0)
  {
    t = 0;
    write(serial_port, &key5, sizeof(key5));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(1000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {

      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(100000);
    }
    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      //  //printf("\nReceived messagebb: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nTIME_DETECT_ATTACK done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nTIME_DETECT_ATTACK ERROR\n");
        break;
      }
      t++;
    }
  }
  //=================================================================================================================
  // Seting anti by PORT or IP
  else if (strcmp(ID, "PROTECT") == 0)
  {
    t = 0;
    write(serial_port, &key2, sizeof(key2));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (strcmp(Value, "PORT") == 0)
    {
      write(serial_port, &keyY, sizeof(keyY));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "IP") == 0)
    {
      write(serial_port, &keyN, sizeof(keyN));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nPROTECT done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nPROTECT ERROR\n");
        break;
      }
      t++;
    }
  }
  //=================================================================================================================
  // Seting interface PORT
  else if (strcmp(ID, "PROTECT_PORT") == 0)
  {
    t = 0;
    write(serial_port, &key3, sizeof(key3));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &key1, sizeof(key1));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &key0, sizeof(key0));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nPROTECT_PORT done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nPROTECT_PORT error\n");
        break;
      }
      t++;
    }
  }
  //=================================================================================================================
  // Setting IP server to Protect
  else if (strcmp(ID, "PROTECT_SERVER_IPv4") == 0)
  {
    t = 0;
    write(serial_port, &key4, sizeof(key4));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(10000);
    }
    write(serial_port, &enter, sizeof(enter));

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nPROTECT_IP done\n");

        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nPROTECT_IP ERROR\n");
        break;
      }
      t++;
    }
  }

  //=================================================================================================================
  // Setting IP V6 server to Protect
  else if (strcmp(ID, "PROTECT_SERVER_IPv6") == 0)
  {
    t = 0;
    write(serial_port, &keyR, sizeof(keyR));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);

    int n = strlen(Value);
    //  //printf("\n strlen :%d", strlen);
    // //printf("\n Value :%s", Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // //printf("\n Char :%c", data);
      usleep(1000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nPROTECT_IPv6 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nPROTECT_IPv6 ERROR\n");
        break;
      }
      t++;
    }
    // //printf("\nT NE: %d\n", t);
  }
  // Delete IPv4/ipv6 protect server
  //=================================================================================================================
  // Setting IPv4 REMOVE server to Protect
  else if (strcmp(ID, "REMOVE_SERVER_IPv4") == 0)
  {
    t = 0;
    write(serial_port, &key_05, sizeof(key_05));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(10000);
    }
    write(serial_port, &enter, sizeof(enter));

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nPROTECT_IP done\n");

        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nPROTECT_IP ERROR\n");
        break;
      }
      t++;
    }
  }

  //=================================================================================================================
  // Setting REMOVE IP V6 server to Protect
  else if (strcmp(ID, "REMOVE_SERVER_IPv6") == 0)
  {
    t = 0;
    write(serial_port, &key_06, sizeof(key_06));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);

    int n = strlen(Value);
    //  //printf("\n strlen :%d", strlen);
    // //printf("\n Value :%s", Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // //printf("\n Char :%c", data);
      usleep(1000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nPROTECT_IPv6 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nPROTECT_IPv6 ERROR\n");
        break;
      }
      t++;
    }
    // //printf("\nT NE: %d\n", t);
  }
  //=================================================================================================================
  // BLOCK IP CLIENT ATTACK
  // Setting IP server to Protect
  else if (strcmp(ID, "BLOCK_IPv4") == 0)
  {
    t = 0;
    write(serial_port, &key_08, sizeof(key_08));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(10000);
    }
    write(serial_port, &enter, sizeof(enter));

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'K') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nBLOCK_IPv4 done\n");

        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nBLOCK_IPv4 ERROR\n");
        break;
      }
      t++;
    }
  }

  //=================================================================================================================
  // Setting IP V6 server to BLOCK
  else if (strcmp(ID, "BLOCK_IPv6") == 0)
  {
    t = 0;
    write(serial_port, &key_09, sizeof(key_09));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);

    int n = strlen(Value);
    //  //printf("\n strlen :%d", strlen);
    // //printf("\n Value :%s", Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // //printf("\n Char :%c", data);
      usleep(1000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'K') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nBLOCK_IPv6 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nBLOCK_IPv6 ERROR\n");
        break;
      }
      t++;
    }
    // //printf("\nT NE: %d\n", t);
  }
  // Delete IPv4/ipv6 BLOCK CLIENT
  //=================================================================================================================
  // Setting IPv4 REMOVE BLOCK IPv4
  else if (strcmp(ID, "REMOVE_BLOCK_IPv4") == 0)
  {
    t = 0;
    write(serial_port, &key_0A, sizeof(key_0A));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(10000);
    }
    write(serial_port, &enter, sizeof(enter));

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'K') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE_BLOCK_IPv4 done\n");

        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nREMOVE_BLOCK_IPv4 ERROR\n");
        break;
      }
      t++;
    }
  }

  //=================================================================================================================
  // Setting REMOVE IP V6 BLOCK
  else if (strcmp(ID, "REMOVE_BLOCK_IPv6") == 0)
  {
    t = 0;
    write(serial_port, &key_09, sizeof(key_09));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);

    int n = strlen(Value);
    //  //printf("\n strlen :%d", strlen);
    // //printf("\n Value :%s", Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // //printf("\n Char :%c", data);
      usleep(1000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'K') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE_BLOCK_IPv6 DONE\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nREMOVE_BLOCK_IPv6 ERROR\n");
        break;
      }
      t++;
    }
    // //printf("\nT NE: %d\n", t);
  }
  //=================================================================================================================
  // Add VPN in VPN List IP4
  else if (strcmp(ID, "ADD_IPv4_VPN") == 0)
  {
    t = 0;
    write(serial_port, &keyL, sizeof(keyL));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(10000);
    }

    write(serial_port, &enter, sizeof(enter));

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD_IP done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nADD_IP ERROR\n");
        break;
      }
      t++;
    }
  }

  //=================================================================================================================
  // Remove VPN in VPN List
  else if (strcmp(ID, "REMOVE_IPv4_VPN") == 0)
  {
    t = 0;
    write(serial_port, &keyM, sizeof(keyM));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(10000);
    }

    write(serial_port, &enter, sizeof(enter));

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE_IP done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nREMOVE_IP ERROR\n");
        break;
      }
      t++;
    }
  }

  //=================================================================================================================
  // Add VPN in VPN List IP6
  else if (strcmp(ID, "ADD_IPv6_VPN") == 0)
  {
    t = 0;
    write(serial_port, &keythang, sizeof(keythang));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(10000);
    }

    write(serial_port, &enter, sizeof(enter));

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD_IPv6_VPN done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nADD_IPv6 VPN ERROR\n");
        break;
      }
      t++;
    }
  }

  //=================================================================================================================
  // Remove VPN in VPN List
  else if (strcmp(ID, "REMOVE_IPv6_VPN") == 0)
  {
    t = 0;
    write(serial_port, &keymu, sizeof(keymu));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE_IPv6 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nREMOVE_IPv6 ERROR\n");
        break;
      }
      t++;
    }
  }
  // Set en/dis HTTPS

  else if (strcmp(ID, "PORT1_HTTPS_EN_DIS") == 0 || strcmp(ID, "PORT2_HTTPS_EN_DIS") == 0 ||
           strcmp(ID, "PORT3_HTTPS_EN_DIS") == 0 || strcmp(ID, "PORT4_HTTPS_EN_DIS") == 0 ||
           strcmp(ID, "PORT5_HTTPS_EN_DIS") == 0 || strcmp(ID, "PORT6_HTTPS_EN_DIS") == 0 ||
           strcmp(ID, "PORT7_HTTPS_EN_DIS") == 0 || strcmp(ID, "PORT8_HTTPS_EN_DIS") == 0)
  {
    t = 0;
    write(serial_port, &key_FF, sizeof(key_FF));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(1000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &keyY, sizeof(keyY));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &keyN, sizeof(keyN));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nHTTP_EN_DIS done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nHTTP_EN_DIS ERROR\n");
        break;
      }
      t++;
    }
  }
  //=================================================================================================================

  // Set en/dis HTTP

  else if (strcmp(ID, "PORT1_HTTP_EN_DIS") == 0 || strcmp(ID, "PORT2_HTTP_EN_DIS") == 0 ||
           strcmp(ID, "PORT3_HTTP_EN_DIS") == 0 || strcmp(ID, "PORT4_HTTP_EN_DIS") == 0 ||
           strcmp(ID, "PORT5_HTTP_EN_DIS") == 0 || strcmp(ID, "PORT6_HTTP_EN_DIS") == 0 ||
           strcmp(ID, "PORT7_HTTP_EN_DIS") == 0 || strcmp(ID, "PORT8_HTTP_EN_DIS") == 0)
  {
    t = 0;
    write(serial_port, &keyP, sizeof(keyP));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);

    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(1000);

    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &keyY, sizeof(keyY));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &keyN, sizeof(keyN));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nHTTP_EN_DIS done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nHTTP_EN_DIS ERROR\n");
        break;
      }
      t++;
    }
  }
  //================================================================================================================
  // Add HTTP IP4
  else if (strcmp(ID, "ADD_IPv4_HTTP") == 0)
  {
    t = 0;
    write(serial_port, &keyT, sizeof(keyT));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      printf("\n Char :%c\n", data);
      usleep(10000);
    }
    write(serial_port, &enter, sizeof(enter));
    // usleep(100);
    while (1)
    {

      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      ////printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        process_ip(LOGFILE_HTTP_IPv4, Value);
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD_IPv4_HTTP done\n");

        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 15))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nADD_IPv4_HTTP error\n");
        break;
      }
      t++;
    }
  }
  //=================================================================================================================
  // REmove http ip4
  else if (strcmp(ID, "REMOVE_IPv4_HTTP") == 0)
  {
    t = 0;
    write(serial_port, &keyX, sizeof(keyX));
    usleep(100000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(10000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        remove_ip_from_file(LOGFILE_HTTP_IPv4, Value);
        remove_ip_HTTP_from_hash(Value);
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE_IPv4_HTTP done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nREMOVE_IPv4_HTTP ERROR\n");
        break;
      }
      t++;
    }
  }
  //================================================================================================================
  // Add HTTP IP6
  else if (strcmp(ID, "ADD_IPv6_HTTP") == 0)
  {
    t = 0;
    write(serial_port, &keyngoacnhontrai, sizeof(keyngoacnhontrai));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      printf("\n Char :%c\n", data);
      usleep(10000);
    }
    write(serial_port, &enter, sizeof(enter));
    // usleep(100);
    while (1)
    {

      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      //  //printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        process_ip(LOGFILE_HTTP_IPv6, Value);
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD_IPv6_HTTP done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nADD_IPv6_HTTP error\n");
        break;
      }
      t++;
    }
  }
  //=================================================================================================================
  // REmove http ip6
  else if (strcmp(ID, "REMOVE_IPv6_HTTP") == 0)
  {
    t = 0;
    write(serial_port, &keyngoacnhonphai, sizeof(keyngoacnhonphai));
    usleep(100000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        remove_ip_from_file(LOGFILE_HTTP_IPv6, Value);
        remove_ip_HTTP_from_hash(Value);
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE_IPv6_HTTP done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nREMOVE_IPv6_HTTP ERROR\n");
        break;
      }
      t++;
    }
  }
  //=================================================================================================================
  // CHECK LOGIN USER
  else if (strcmp(ID, "CHECK_US") == 0)
  {
    t = 0;
    strcpy(buffer1, Value);
    write(serial_port, &key4, sizeof(key4));
    usleep(10000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
  }

  else if (strcmp(ID, "CHECK_PW") == 0)
  {
    t = 0;
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    usleep(100);
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'U') != NULL))
      {

        strcat(buffer, "CHECK_US");
        strcat(buffer, "$");
        strcat(buffer, buffer1);
        strcat(buffer, "$OK_USER$");

        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK_USER$");

        memset(buffer1, 0, sizeof(buffer1));
        break;
      }
      else if ((strchr(data1, 'A') != NULL))
      {
        strcat(buffer, "CHECK_US");
        strcat(buffer, "$");
        strcat(buffer, buffer1);
        strcat(buffer, "$OK_ADMIN$");

        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK_ADMIN$");

        memset(buffer1, 0, sizeof(buffer1));
        break;
      }
      else if ((strchr(data1, 'F') != NULL))
      {
        strcat(buffer, "CHECK_US");
        strcat(buffer, "$");
        strcat(buffer, buffer1);
        strcat(buffer, "$INVALID$");

        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$INVALID$");
        memset(buffer1, 0, sizeof(buffer1));
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
      }
      t++;
    }
  }
  //=================================================================================================================
  //=================================================================================================================

  // CHANGE USER
  else if (strcmp(ID, "CHANGE_USER") == 0)
  {
    t = 0;
    if (strcmp(Value, "admin") == 0)
    {
      strcpy(buffer3, Value);
      write(serial_port, &keychamphay, sizeof(keychamphay));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
      usleep(10000);
    }
    else
    {

      strcpy(buffer3, Value);
      write(serial_port, &keybang, sizeof(keybang));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
      usleep(10000);

      int n = strlen(Value);
      for (int i = 0; i < n; i++)
      {
        char data = Value[i];
        send_data(serial_port, &data, sizeof(data));
        usleep(100000);
      }
      write(serial_port, &enter, sizeof(enter));
    }
  }
  else if (strcmp(ID, "CHANGE_PASS") == 0)
  {
    t = 0;
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(100000);
    }
    write(serial_port, &enter, sizeof(enter));
    // usleep(100);
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      //  //printf("Received message$ %s\n", data);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, "CHANGE_USER");
        strcat(buffer, "$");
        strcat(buffer, buffer3);
        strcat(buffer, "$OK$");

        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");

        memset(buffer3, 0, sizeof(buffer3));
        printf("\nCHANGE_USER done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, "CHANGE_USER");
        strcat(buffer, "$");
        strcat(buffer, buffer3);
        strcat(buffer, "$ERROR$");

        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");

        memset(buffer3, 0, sizeof(buffer3));
        printf("\nCHANGE_USER error\n");
        break;
      }
      t++;
    }
  }
  //=================================================================================================================
  else if (strcmp(ID, "LOAD") == 0)
  {
    t = 0;
    write(serial_port, &keyva, sizeof(keyva));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    while (1)
    {
      char *data1 = receive_data(serial_port);
      // //printf("Received message$ %s\n", data);
      if ((strchr(data1, 'Y') != NULL))
      {
        // printf_uart2(serial_port, client_sock);
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        break;
      }
      t++;
    }
  }
  //=================================================================================================================
  // ADD new user
  else if (strcmp(ID, "ADD_USER") == 0)
  {
    t = 0;
    strcpy(buffer2, Value);

    write(serial_port, &keycong, sizeof(keycong));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(100000);
    }
    write(serial_port, &enter, sizeof(enter));
  }
  else if (strcmp(ID, "ADD_PW") == 0)
  {
    t = 0;
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      //  //printf("Received message$ %s\n", data);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, "ADD_USER");
        strcat(buffer, "$");
        strcat(buffer, buffer2);
        strcat(buffer, "$OK$");

        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");

        memset(buffer2, 0, sizeof(buffer2));
        printf("\nADD_USER done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, "ADD_USER");
        strcat(buffer, "$");
        strcat(buffer, buffer2);
        strcat(buffer, "$ERROR$");

        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERRO$");
        memset(buffer2, 0, sizeof(buffer2));
        printf("\nADD_USER error\n");
        break;
      }
      t++;
    }
  }

  //=================================================================================================================
  // REMOVE user
  else if (strcmp(ID, "REMOVE_USER") == 0)
  {
    t = 0;
    write(serial_port, &keytru, sizeof(keytru));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nRM_USER done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))

      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nRM_USER ERROR\n");
        break;
      }
      t++;
    }
  }
  //=================================================================================================================
  // RESET
  else if (strcmp(ID, "RS_TEMP") == 0)
  {

    send_reset(serial_port);
    usleep(100000);
    while (1)
    {
      bool flag2 = false;
      int i2 = 0;
      char keyphay1 = ',';
      char key_enter1 = '\r';
      write(serial_port, &keyphay1, sizeof(keyphay1));
      usleep(10000);
      write(serial_port, &key_enter1, sizeof(key_enter1));
      usleep(1000000);
      send_time(serial_port);
      while (1)
      {
        char *data2 = receive_data(serial_port);
        if ((strchr(data2, 'S') != NULL))
        {
          flag2 = true;
          break;
        }
        i2++;
        if (i2 == 10)
        {
          break;
        }
      }
      if (flag2 == true)
      {
        printf("\nRESET DONE\n");
        strcat(buffer, "RS_TEMP$1$OK$");
        break;
      }
      sleep(2);
    }
  }

  // Load default
  else if (strcmp(ID, "LOAD_DEFAULT") == 0)
  {
    t = 0;
    write(serial_port, &keyhoi, sizeof(keyhoi));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nLOAD_DEFAULT done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nLOAD_DEFAULT error\n");

        break;
      }
      t++;
    }
  }
  //
  // Create new log file
  else if (strcmp(ID, "CREATE_NEW_LOG") == 0)
  {
    create_new_log = true;
    while (1)
    {
      if (created_new_log == true)
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        created_new_log == false;
        break;
      }
    }
  }
  //=================================================================================================================
  // Check save HTTP table IPv4 - ADD
  else if (strcmp(ID, "HTTP_TABLE_IPv4_ADD") == 0)
  {
    write(serial_port, &keycham, sizeof(keycham));

    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    // b0: xu ly file and send qua core
    send_ips_via_uart(Value);

    // b1: dua vào hash
    load_ips_from_file(Value);
    // b2: luu vào file http table
    append_ips_to_file(Value, LOGFILE_HTTP_IPv4);
    // b4: phan hoi len GUI
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nHTTP_TABLE_IPv4 done\n");
        break;
      }
    }
  }

  // Check save HTTP table IPv6 - ADD
  else if (strcmp(ID, "HTTP_TABLE_IPv6_ADD") == 0)
  {
    write(serial_port, &keynhaynguoc, sizeof(keynhaynguoc));

    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    // b0: xu ly file and send qua core
    send_ips_via_uart(Value);

    // b1: dua vào hash
    load_ips_from_file(Value);
    // b2: luu vào file http table
    append_ips_to_file(Value, LOGFILE_HTTP_IPv6);
    // b4: phan hoi len GUI
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nHTTP_TABLE_IPv6 done\n");
        break;
      }
    }
  }

  // Clear
  else if (strcmp(ID, "HTTP_TABLE_IPv4_CLEAR") == 0)
  {
    write(serial_port, &key_01, sizeof(key_01));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    // b0: xu ly file and send qua core
    send_ips_via_uart(Value);

    // b1: xoa khoi vào hash
    load_remove_hash_ip_http_from_file(Value);
    // b2: xoa khoi file http table
    remove_matching_ips(Value, LOGFILE_HTTP_IPv4);
    // b4: phan hoi len GUI
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nCLear HTTP_TABLE_IPv4 done\n");
        break;
      }
    }
  }

  // Check save HTTP table IPv6 - ADD
  else if (strcmp(ID, "HTTP_TABLE_IPv6_CLEAR") == 0)
  {
    write(serial_port, &key_02, sizeof(key_02));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    // b0: xu ly file and send qua core
    send_ips_via_uart(Value);

    // b1: xoa khoi  vào hash
    load_remove_hash_ip_http_from_file(Value);
    // b2: xoa khoi file http table
    remove_matching_ips(Value, LOGFILE_HTTP_IPv6);
    // b4: phan hoi len GUI
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nClear HTTP_TABLE_IPv6 done\n");
        break;
      }
    }
  }
  //================================================================================================================
  // Nhiều port ( Cho vào 1 hàm ký tự khác)

  // else if (strcmp(ID, "PROTECT_PORT1") == 0)
  // {
  //   t = 0;
  //   write(serial_port, &key_07, sizeof(key_07));
  //   usleep(1000);
  //   write(serial_port, &key1, sizeof(key1));
  //   usleep(1000);
  //   write(serial_port, &enter, sizeof(enter));
  //   usleep(100000);
  //   if (strcmp(Value, "1") == 0)
  //   {
  //     write(serial_port, &key1, sizeof(key1));
  //     usleep(1000);
  //     write(serial_port, &enter, sizeof(enter));
  //   }
  //   else if (strcmp(Value, "0") == 0)
  //   {
  //     write(serial_port, &key0, sizeof(key0));
  //     usleep(1000);
  //     write(serial_port, &enter, sizeof(enter));
  //   }
  //   while (1)
  //   {
  //     char *data1 = receive_data(serial_port);
  //     printf("\nReceived message: %s\n", data1);
  //     if ((strchr(data1, 'Y') != NULL))
  //     {
  //       strcat(buffer, ID);
  //       strcat(buffer, "$");
  //       strcat(buffer, Value);
  //       strcat(buffer, "$OK$");
  //       printf("\nPROTECT_PORT1 done\n");
  //       break;
  //     }
  //     else if ((strchr(data1, 'N') != NULL) || (t == 10))
  //     {
  //       strcat(buffer, ID);
  //       strcat(buffer, "$");
  //       strcat(buffer, Value);
  //       strcat(buffer, "$ERROR$");
  //       printf("\nPROTECT_PORT1 error\n");
  //       break;
  //     }
  //     t++;
  //   }
  // }
  // else if (strcmp(ID, "PROTECT_PORT2") == 0)
  // {
  //   t = 0;
  //   write(serial_port, &key_07, sizeof(key_07));
  //   usleep(1000);
  //   write(serial_port, &key2, sizeof(key2));
  //   usleep(1000);
  //   write(serial_port, &enter, sizeof(enter));
  //   usleep(100000);
  //   if (strcmp(Value, "1") == 0)
  //   {
  //     write(serial_port, &key1, sizeof(key1));
  //     usleep(1000);
  //     write(serial_port, &enter, sizeof(enter));
  //   }
  //   else if (strcmp(Value, "0") == 0)
  //   {
  //     write(serial_port, &key0, sizeof(key0));
  //     usleep(1000);
  //     write(serial_port, &enter, sizeof(enter));
  //   }
  //   while (1)
  //   {
  //     char *data1 = receive_data(serial_port);
  //     printf("\nReceived message: %s\n", data1);
  //     if ((strchr(data1, 'Y') != NULL))
  //     {
  //       strcat(buffer, ID);
  //       strcat(buffer, "$");
  //       strcat(buffer, Value);
  //       strcat(buffer, "$OK$");
  //       printf("\nPROTECT_PORT2 done\n");
  //       break;
  //     }
  //     else if ((strchr(data1, 'N') != NULL) || (t == 10))
  //     {
  //       strcat(buffer, ID);
  //       strcat(buffer, "$");
  //       strcat(buffer, Value);
  //       strcat(buffer, "$ERROR$");
  //       printf("\nPROTECT_PORT2 error\n");
  //       break;
  //     }
  //     t++;
  //   }
  // }
  // else if (strcmp(ID, "PROTECT_PORT3") == 0)
  // {
  //   t = 0;
  //   write(serial_port, &key_07, sizeof(key_07));
  //   usleep(1000);
  //   write(serial_port, &key3, sizeof(key3));
  //   usleep(1000);
  //   write(serial_port, &enter, sizeof(enter));
  //   usleep(100000);
  //   if (strcmp(Value, "1") == 0)
  //   {
  //     write(serial_port, &key1, sizeof(key1));
  //     usleep(1000);
  //     write(serial_port, &enter, sizeof(enter));
  //   }
  //   else if (strcmp(Value, "0") == 0)
  //   {
  //     write(serial_port, &key0, sizeof(key0));
  //     usleep(1000);
  //     write(serial_port, &enter, sizeof(enter));
  //   }
  //   while (1)
  //   {
  //     char *data1 = receive_data(serial_port);
  //     printf("\nReceived message: %s\n", data1);
  //     if ((strchr(data1, 'Y') != NULL))
  //     {
  //       strcat(buffer, ID);
  //       strcat(buffer, "$");
  //       strcat(buffer, Value);
  //       strcat(buffer, "$OK$");
  //       printf("\nPROTECT_PORT3 done\n");
  //       break;
  //     }
  //     else if ((strchr(data1, 'N') != NULL) || (t == 10))
  //     {
  //       strcat(buffer, ID);
  //       strcat(buffer, "$");
  //       strcat(buffer, Value);
  //       strcat(buffer, "$ERROR$");
  //       printf("\nPROTECT_PORT3 error\n");
  //       break;
  //     }
  //     t++;
  //   }
  // }
  // else if (strcmp(ID, "PROTECT_PORT4") == 0)
  // {
  //   t = 0;
  //   write(serial_port, &key_07, sizeof(key_07));
  //   usleep(1000);
  //   write(serial_port, &key4 sizeof(key4));
  //   usleep(1000);
  //   write(serial_port, &enter, sizeof(enter));
  //   usleep(100000);
  //   if (strcmp(Value, "1") == 0)
  //   {
  //     write(serial_port, &key1, sizeof(key1));
  //     usleep(1000);
  //     write(serial_port, &enter, sizeof(enter));
  //   }
  //   else if (strcmp(Value, "0") == 0)
  //   {
  //     write(serial_port, &key0, sizeof(key0));
  //     usleep(1000);
  //     write(serial_port, &enter, sizeof(enter));
  //   }
  //   while (1)
  //   {
  //     char *data1 = receive_data(serial_port);
  //     printf("\nReceived message: %s\n", data1);
  //     if ((strchr(data1, 'Y') != NULL))
  //     {
  //       strcat(buffer, ID);
  //       strcat(buffer, "$");
  //       strcat(buffer, Value);
  //       strcat(buffer, "$OK$");
  //       printf("\nPROTECT_PORT4 done\n");
  //       break;
  //     }
  //     else if ((strchr(data1, 'N') != NULL) || (t == 10))
  //     {
  //       strcat(buffer, ID);
  //       strcat(buffer, "$");
  //       strcat(buffer, Value);
  //       strcat(buffer, "$ERROR$");
  //       printf("\nPROTECT_PORT4 error\n");
  //       break;
  //     }
  //     t++;
  //   }
  // }
  // else if (strcmp(ID, "PROTECT_PORT5") == 0)
  // {
  //   t = 0;
  //   write(serial_port, &key_07, sizeof(key_07));
  //   usleep(1000);
  //   write(serial_port, &key5, sizeof(key5));
  //   usleep(1000);
  //   write(serial_port, &enter, sizeof(enter));
  //   usleep(100000);
  //   if (strcmp(Value, "1") == 0)
  //   {
  //     write(serial_port, &key1, sizeof(key1));
  //     usleep(1000);
  //     write(serial_port, &enter, sizeof(enter));
  //   }
  //   else if (strcmp(Value, "0") == 0)
  //   {
  //     write(serial_port, &key0, sizeof(key0));
  //     usleep(1000);
  //     write(serial_port, &enter, sizeof(enter));
  //   }
  //   while (1)
  //   {
  //     char *data1 = receive_data(serial_port);
  //     printf("\nReceived message: %s\n", data1);
  //     if ((strchr(data1, 'Y') != NULL))
  //     {
  //       strcat(buffer, ID);
  //       strcat(buffer, "$");
  //       strcat(buffer, Value);
  //       strcat(buffer, "$OK$");
  //       printf("\nPROTECT_PORT5 done\n");
  //       break;
  //     }
  //     else if ((strchr(data1, 'N') != NULL) || (t == 10))
  //     {
  //       strcat(buffer, ID);
  //       strcat(buffer, "$");
  //       strcat(buffer, Value);
  //       strcat(buffer, "$ERROR$");
  //       printf("\nPROTECT_PORT5 error\n");
  //       break;
  //     }
  //     t++;
  //   }
  // }
  // else if (strcmp(ID, "PROTECT_PORT6") == 0)
  // {
  //   t = 0;
  //   write(serial_port, &key_07, sizeof(key_07));
  //   usleep(1000);
  //   write(serial_port, &key6, sizeof(key6));
  //   usleep(1000);
  //   write(serial_port, &enter, sizeof(enter));
  //   usleep(100000);
  //   if (strcmp(Value, "1") == 0)
  //   {
  //     write(serial_port, &key1, sizeof(key1));
  //     usleep(1000);
  //     write(serial_port, &enter, sizeof(enter));
  //   }
  //   else if (strcmp(Value, "0") == 0)
  //   {
  //     write(serial_port, &key0, sizeof(key0));
  //     usleep(1000);
  //     write(serial_port, &enter, sizeof(enter));
  //   }
  //   while (1)
  //   {
  //     char *data1 = receive_data(serial_port);
  //     printf("\nReceived message: %s\n", data1);
  //     if ((strchr(data1, 'Y') != NULL))
  //     {
  //       strcat(buffer, ID);
  //       strcat(buffer, "$");
  //       strcat(buffer, Value);
  //       strcat(buffer, "$OK$");
  //       printf("\nPROTECT_PORT6 done\n");
  //       break;
  //     }
  //     else if ((strchr(data1, 'N') != NULL) || (t == 10))
  //     {
  //       strcat(buffer, ID);
  //       strcat(buffer, "$");
  //       strcat(buffer, Value);
  //       strcat(buffer, "$ERROR$");
  //       printf("\nPROTECT_PORT6 error\n");
  //       break;
  //     }
  //     t++;
  //   }
  // }
  // else if (strcmp(ID, "PROTECT_PORT7") == 0)
  // {
  //   t = 0;
  //   write(serial_port, &key_07, sizeof(key_07));
  //   usleep(1000);
  //   write(serial_port, &key7, sizeof(key7));
  //   usleep(1000);
  //   write(serial_port, &enter, sizeof(enter));
  //   usleep(100000);
  //   if (strcmp(Value, "1") == 0)
  //   {
  //     write(serial_port, &key1, sizeof(key1));
  //     usleep(1000);
  //     write(serial_port, &enter, sizeof(enter));
  //   }
  //   else if (strcmp(Value, "0") == 0)
  //   {
  //     write(serial_port, &key0, sizeof(key0));
  //     usleep(1000);
  //     write(serial_port, &enter, sizeof(enter));
  //   }
  //   while (1)
  //   {
  //     char *data1 = receive_data(serial_port);
  //     printf("\nReceived message: %s\n", data1);
  //     if ((strchr(data1, 'Y') != NULL))
  //     {
  //       strcat(buffer, ID);
  //       strcat(buffer, "$");
  //       strcat(buffer, Value);
  //       strcat(buffer, "$OK$");
  //       printf("\nPROTECT_PORT7 done\n");
  //       break;
  //     }
  //     else if ((strchr(data1, 'N') != NULL) || (t == 10))
  //     {
  //       strcat(buffer, ID);
  //       strcat(buffer, "$");
  //       strcat(buffer, Value);
  //       strcat(buffer, "$ERROR$");
  //       printf("\nPROTECT_PORT7 error\n");
  //       break;
  //     }
  //     t++;
  //   }
  // }

  //=================================================================================================================
  // Bao ve PORT hoac IP
  else if (strcmp(ID, "PROTECT_PORT1_MODE") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyA, sizeof(keyA));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &key1, sizeof(key1));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &key0, sizeof(key0));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nPROTECT_PORT1 MODEdone\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nPROTECT_PORT1 MODEerror\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PROTECT_PORT2_MODE") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyB, sizeof(keyB));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &key1, sizeof(key1));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &key0, sizeof(key0));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nPROTECT_PORT2 MODE done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nPROTECT_PORT2 MODE error\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PROTECT_PORT3_MODE") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyC, sizeof(keyC));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &key1, sizeof(key1));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &key0, sizeof(key0));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nPROTECT_PORT3 MODE done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nPROTECT_PORT3 MODE error\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PROTECT_PORT4_MODE") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyD, sizeof(keyD));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &key1, sizeof(key1));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &key0, sizeof(key0));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nPROTECT_PORT4 MODE done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nPROTECT_PORT4 MODE error\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PROTECT_PORT5_MODE") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyE, sizeof(keyE));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &key1, sizeof(key1));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &key0, sizeof(key0));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nPROTECT_PORT5 MODE done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nPROTECT_PORT5 MODE error\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PROTECT_PORT6_MODE") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyF, sizeof(keyF));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &key1, sizeof(key1));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &key0, sizeof(key0));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nPROTECT_PORT6 MODE done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nPROTECT_PORT6  MODE error\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PROTECT_PORT7_MODE") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyG, sizeof(keyG));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &key1, sizeof(key1));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &key0, sizeof(key0));
      usleep(1000);
      write(serial_port, &enter, sizeof(enter));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nPROTECT_PORT7 MODE done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nPROTECT_PORT7 MODE error\n");
        break;
      }
      t++;
    }
  }

  // =================================================================================================================
  // Add IPv4 vào port

  else if (strcmp(ID, "PORT1_ADD_IPv4") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyH, sizeof(keyH));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD IPv4 port 1 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n ADD IPv4 port 1 ERROR\n");
        break;
      }
      t++;
    }
  }

  else if (strcmp(ID, "PORT2_ADD_IPv4") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyI, sizeof(keyI));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD IPv4 port 2 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n ADD IPv4 port 2 ERROR\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PORT3_ADD_IPv4") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyJ, sizeof(keyJ));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD IPv4 port 3 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n ADD IPv4 port 3 ERROR\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PORT4_ADD_IPv4") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyK, sizeof(keyK));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD IPv4 port 4 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n ADD IPv4 port 4 ERROR\n");
        break;
      }
      t++;
    }
  }

  // else if (strcmp(ID, "PORT5_ADD_IPv4") == 0)
  else if (strcmp(ID, "PORT1_ADD_IPv4_SRC") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyL, sizeof(keyL));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD IPv4 port 5 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n ADD IPv4 port 5 ERROR\n");
        break;
      }
      t++;
    }
  }

  // else if (strcmp(ID, "PORT6_ADD_IPv4") == 0)
  else if (strcmp(ID, "PORT3_ADD_IPv4_SRC") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyM, sizeof(keyM));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD IPv4 port 6 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n ADD IPv4 port 6 ERROR\n");
        break;
      }
      t++;
    }
  }

  // else if (strcmp(ID, "PORT7_ADD_IPv4") == 0)
  else if (strcmp(ID, "PORT4_ADD_IPv4_SRC") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyN, sizeof(keyN));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD IPv4 port 7 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n ADD IPv4 port 7 ERROR\n");
        break;
      }
      t++;
    }
  }
  // =================================================================================================================
  // XOA IPv4 TRONG PORT
  else if (strcmp(ID, "PORT1_REMOVE_IPv4") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyO, sizeof(keyO));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv4 port 1 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv4 port 1 ERROR\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PORT2_REMOVE_IPv4") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyP, sizeof(keyP));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv4 port 2 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv4 port 2 ERROR\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PORT3_REMOVE_IPv4") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyQ, sizeof(keyQ));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv4 port 3 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv4 port 3 ERROR\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PORT4_REMOVE_IPv4") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyR, sizeof(keyR));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data = receive_data(serial_port);
      printf("\nReceived message: %s\n", data);
      if ((strchr(data, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv4 port 4 done\n");
        break;
      }
      else if ((strchr(data, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv4 port 4 ERROR\n");
        break;
      }
      t++;
    }
  }
  // else if (strcmp(ID, "PORT5_REMOVE_IPv4") == 0)
  else if (strcmp(ID, "PORT1_REMOVE_IPv4_SRC") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyS, sizeof(keyS));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv4 port 5 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv4 port 5 ERROR\n");
        break;
      }
      t++;
    }
  }
  // else if (strcmp(ID, "PORT6_REMOVE_IPv4") == 0)
  else if (strcmp(ID, "PORT3_REMOVE_IPv4_SRC") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyT, sizeof(keyT));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv4 port 6 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv4 port 6 ERROR\n");
        break;
      }
      t++;
    }
  }
  // else if (strcmp(ID, "PORT7_REMOVE_IPv4") == 0)
  else if (strcmp(ID, "PORT4_REMOVE_IPv4_SRC") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyU, sizeof(keyU));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv4 port 7 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv4 port 7 ERROR\n");
        break;
      }
      t++;
    }
  }
  // =================================================================================================================
  // Thêm IPv6 trong PORT
  else if (strcmp(ID, "PORT1_ADD_IPv6") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyV, sizeof(keyV));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD IPv6 port 1 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n ADD IPv6 port 1 ERROR\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PORT2_ADD_IPv6") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyW, sizeof(keyW));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD IPv6 port 2 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n ADD IPv6 port 2 ERROR\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PORT3_ADD_IPv6") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyX, sizeof(keyX));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD IPv6 port 3 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n ADD IPv6 port 3 ERROR\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PORT4_ADD_IPv6") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyY, sizeof(keyY));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD IPv6 port 4 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n ADD IPv6 port 4 ERROR\n");
        break;
      }
      t++;
    }
  }
  // else if (strcmp(ID, "PORT5_ADD_IPv6") == 0)
  else if (strcmp(ID, "PORT1_ADD_IPv6_SRC") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keyZ, sizeof(keyZ));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceivedmessage: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD IPv6 port 5 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n ADD IPv6 port 5 ERROR\n");
        break;
      }
      t++;
    }
  }

  // else if (strcmp(ID, "PORT6_ADD_IPv6") == 0)
  else if (strcmp(ID, "PORT3_ADD_IPv6_SRC") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keychamphay, sizeof(keychamphay));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD IPv6 port 6 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n ADD IPv6 port 6 ERROR\n");
        break;
      }
      t++;
    }
  }
  // else if (strcmp(ID, "PORT7_ADD_IPv6") == 0)
  else if (strcmp(ID, "PORT4_ADD_IPv6_SRC") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keycong, sizeof(keycong));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nADD IPv6 port 7 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n ADD IPv6 port 7 ERROR\n");
        break;
      }
      t++;
    }
  }
  // ==================================================================================================================
  // XOA IPv6 TRONG PORT
  else if (strcmp(ID, "PORT1_REMOVE_IPv6") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &keytru, sizeof(keytru));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv6 port 1 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv6 port 1 ERROR\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PORT2_REMOVE_IPv6") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &key_02, sizeof(key_02));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv6 port 2 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv6 port 2 ERROR\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PORT3_REMOVE_IPv6") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &key_0D, sizeof(key_0D));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv6 port 3 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv6 port 3 ERROR\n");
        break;
      }
      t++;
    }
  }
  else if (strcmp(ID, "PORT4_REMOVE_IPv6") == 0)

  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);
    write(serial_port, &key_04, sizeof(key_04));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv6 port 4 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv6 port 4 ERROR\n");
        break;
      }
      t++;
    }
  }
  // else if (strcmp(ID, "PORT5_REMOVE_IPv6") == 0)
  else if (strcmp(ID, "PORT1_REMOVE_IPv6_SRC") == 0)
  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(10000);

    write(serial_port, &key_05, sizeof(key_05));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv6 port 5 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv6 port 5 ERROR\n");
        break;
      }
      t++;
    }
  }
  // else if (strcmp(ID, "PORT6_REMOVE_IPv6") == 0)
  else if (strcmp(ID, "PORT3_REMOVE_IPv6_SRC") == 0)

  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000);
    write(serial_port, &key_06, sizeof(key_06));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv6 port 6 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv6 port 6 ERROR\n");
        break;
      }
      t++;
    }
  }
  //  else if (strcmp(ID, "PORT7_REMOVE_IPv6") == 0)
  else if (strcmp(ID, "PORT4_REMOVE_IPv6_SRC") == 0)

  {
    t = 0;
    write(serial_port, &key_07, sizeof(key_07));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000);
    write(serial_port, &key_08, sizeof(key_08));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(100000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv6 port 7 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv6 port 7 ERROR\n");
        break;
      }
      t++;
    }
  }

  //=================================================================================================================
  // PORT MIRRORING
  //=================================================================================================================

  // PORT MONITORED 1
  else if (strcmp(ID, "PORT1_MONITORED") == 0 || strcmp(ID, "PORT2_MONITORED") == 0 || strcmp(ID, "PORT3_MONITORED") == 0 || strcmp(ID, "PORT4_MONITORED") == 0 || strcmp(ID, "PORT5_MONITORED") == 0 || strcmp(ID, "PORT6_MONITORED") == 0 || strcmp(ID, "PORT7_MONITORED") == 0 || strcmp(ID, "PORT8_MONITORED") == 0)
  {
    printf("helooo1");

    t = 0;
    write(serial_port, &key_08, sizeof(key_08));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000);
    write(serial_port, &keyX, sizeof(keyX));
    usleep(100000);
    printf("\nID: %c\n", ID[4]);
    printf("\nID: %c\n", ID[4]);
    printf("\nID: %c\n", ID[4]);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(10000);
    if (strcmp(Value, "1") == 0)
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (strcmp(Value, "0") == 0)
    {
      write(serial_port, &key0, sizeof(key0));
    }
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nENABLE PORT%c_MONITORED done\n", ID[4]);
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nENABLE PORT%c_MONITORED ERROR\n", ID[4]);
        break;
      }
      t++;
    }
  }

  // PORT MONITORED SRC MAC
  else if (strcmp(ID, "PORT1_MONITORED_SRC_MAC") == 0 || strcmp(ID, "PORT2_MONITORED_SRC_MAC") == 0 || strcmp(ID, "PORT3_MONITORED_SRC_MAC") == 0 || strcmp(ID, "PORT4_MONITORED_SRC_MAC") == 0 || strcmp(ID, "PORT5_MONITORED_SRC_MAC") == 0 || strcmp(ID, "PORT6_MONITORED_SRC_MAC") == 0 || strcmp(ID, "PORT7_MONITORED_SRC_MAC") == 0 || strcmp(ID, "PORT8_MONITORED_SRC_MAC") == 0)
  {
    t = 0;
    write(serial_port, &key_08, sizeof(key_08));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000);

    write(serial_port, &keyA, sizeof(keyA));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(10000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv6 port 7 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv6 port 7 ERROR\n");
        break;
      }
      t++;
    }
  }
  // PORT MONITORED DST MAC
  else if (strcmp(ID, "PORT1_MONITORED_DST_MAC") == 0 || strcmp(ID, "PORT2_MONITORED_DST_MAC") == 0 || strcmp(ID, "PORT3_MONITORED_DST_MAC") == 0 || strcmp(ID, "PORT4_MONITORED_DST_MAC") == 0 || strcmp(ID, "PORT5_MONITORED_DST_MAC") == 0 || strcmp(ID, "PORT6_MONITORED_DST_MAC") == 0 || strcmp(ID, "PORT7_MONITORED_DST_MAC") == 0 || strcmp(ID, "PORT8_MONITORED_DST_MAC") == 0)
  {
    t = 0;
    write(serial_port, &key_08, sizeof(key_08));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000);
    write(serial_port, &keyB, sizeof(keyB));

    usleep(1000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv6 port 7 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv6 port 7 ERROR\n");
        break;
      }
      t++;
    }
  }

  // PORT MONITORED SRC IP4
  else if (strcmp(ID, "PORT1_MONITORED_SRC_IPV4") == 0 || strcmp(ID, "PORT2_MONITORED_SRC_IPV4") == 0 || strcmp(ID, "PORT3_MONITORED_SRC_IPV4") == 0 || strcmp(ID, "PORT4_MONITORED_SRC_IPV4") == 0 || strcmp(ID, "PORT5_MONITORED_SRC_IPV4") == 0 || strcmp(ID, "PORT6_MONITORED_SRC_IPV4") == 0 || strcmp(ID, "PORT7_MONITORED_SRC_IPV4") == 0 || strcmp(ID, "PORT8_MONITORED_SRC_IPV4") == 0)
  {
    t = 0;
    write(serial_port, &key_08, sizeof(key_08));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000);

    write(serial_port, &keyC, sizeof(keyC));
    usleep(10000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv6 port 7 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv6 port 7 ERROR\n");
        break;
      }
      t++;
    }
  }
  // PORT MONITORED DST IPV4
  else if (strcmp(ID, "PORT1_MONITORED_DST_IPV4") == 0 || strcmp(ID, "PORT2_MONITORED_DST_IPV4") == 0 || strcmp(ID, "PORT3_MONITORED_DST_IPV4") == 0 || strcmp(ID, "PORT4_MONITORED_DST_IPV4") == 0 || strcmp(ID, "PORT5_MONITORED_DST_IPV4") == 0 || strcmp(ID, "PORT6_MONITORED_DST_IPV4") == 0 || strcmp(ID, "PORT7_MONITORED_DST_IPV4") == 0 || strcmp(ID, "PORT8_MONITORED_DST_IPV4") == 0)
  {
    t = 0;
    write(serial_port, &key_08, sizeof(key_08));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000);

    write(serial_port, &keyD, sizeof(keyD));
    usleep(10000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv6 port 7 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv6 port 7 ERROR\n");
        break;
      }
      t++;
    }
  }
  // PORT MONITORED SRC IPv6
  else if (strcmp(ID, "PORT1_MONITORED_SRC_IPV6") == 0 || strcmp(ID, "PORT2_MONITORED_SRC_IPV6") == 0 || strcmp(ID, "PORT3_MONITORED_SRC_IPV6") == 0 || strcmp(ID, "PORT4_MONITORED_SRC_IPV6") == 0 || strcmp(ID, "PORT5_MONITORED_SRC_IPV6") == 0 || strcmp(ID, "PORT6_MONITORED_SRC_IPV6") == 0 || strcmp(ID, "PORT7_MONITORED_SRC_IPV6") == 0 || strcmp(ID, "PORT8_MONITORED_SRC_IPV6") == 0)
  {
    t = 0;
    write(serial_port, &key_08, sizeof(key_08));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000);

    write(serial_port, &keyE, sizeof(keyE));
    usleep(100000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(10000);
    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv6 port 7 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv6 port 7 ERROR\n");
        break;
      }
      t++;
    }
  }
  // PORT MONITORED DST IPV6
  else if (strcmp(ID, "PORT1_MONITORED_DST_IPV6") == 0 || strcmp(ID, "PORT2_MONITORED_DST_IPV6") == 0 || strcmp(ID, "PORT3_MONITORED_DST_IPV6") == 0 || strcmp(ID, "PORT4_MONITORED_DST_IPV6") == 0 || strcmp(ID, "PORT5_MONITORED_DST_IPV6") == 0 || strcmp(ID, "PORT6_MONITORED_DST_IPV6") == 0 || strcmp(ID, "PORT7_MONITORED_DST_IPV6") == 0 || strcmp(ID, "PORT8_MONITORED_DST_IPV6") == 0)
  {
    t = 0;
    write(serial_port, &key_08, sizeof(key_08));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000);

    write(serial_port, &keyF, sizeof(keyF));
    usleep(10000);

    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      // printf("IP:%c\n", data);
      usleep(100000);
    }

    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE IPv6 port 7 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE IPv6 port 7 ERROR\n");
        break;
      }
      t++;
    }
  }

  // PORT MONITORED  PORT
  else if (strcmp(ID, "PORT1_MONITORED_PORT") == 0 || strcmp(ID, "PORT2_MONITORED_PORT") == 0 || strcmp(ID, "PORT3_MONITORED_PORT") == 0 || strcmp(ID, "PORT4_MONITORED_PORT") == 0 || strcmp(ID, "PORT5_MONITORED_PORT") == 0 || strcmp(ID, "PORT6_MONITORED_PORT") == 0 || strcmp(ID, "PORT7_MONITORED_PORT") == 0 || strcmp(ID, "PORT8_MONITORED_PORT") == 0)
  {
    t = 0;
    write(serial_port, &key_08, sizeof(key_08));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000);

    write(serial_port, &keyG, sizeof(keyG));

    usleep(10000);
    printf("ID NE:%S", ID[4]);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }

    int n = strlen(Value);
    for (int i = 0; i < n; i++)
    {
      char data = Value[i];
      send_data(serial_port, &data, sizeof(data));
      printf("IP:%c\n", data);
      usleep(100000);
    }
    write(serial_port, &enter, sizeof(enter));
    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nREMOVE PORT port 7 done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\n REMOVE PORT port 7 ERROR\n");
        break;
      }
      t++;
    }
  }

  // PORT MONITORED PROTOCOL
  else if (strcmp(ID, "PORT1_MONITORED_PROTOCOL") == 0 || strcmp(ID, "PORT2_MONITORED_PROTOCOL") == 0 || strcmp(ID, "PORT3_MONITORED_PROTOCOL") == 0 || strcmp(ID, "PORT4_MONITORED_PROTOCOL") == 0 || strcmp(ID, "PORT5_MONITORED_PROTOCOL") == 0 || strcmp(ID, "PORT6_MONITORED_PROTOCOL") == 0 || strcmp(ID, "PORT7_MONITORED_PROTOCOL") == 0 || strcmp(ID, "PORT8_MONITORED_PROTOCOL") == 0)
  {
    t = 0;
    write(serial_port, &key_08, sizeof(key_08));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000);

    write(serial_port, &keyI, sizeof(keyI));

    usleep(10000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(10000);
    if (strcmp(Value, "TCP") == 0)
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (strcmp(Value, "UDP") == 0)
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (strcmp(Value, "ICMP") == 0)
    {
      write(serial_port, &key3, sizeof(key3));
    }

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nmiroring protocol  done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nmiroring protocol  ERROR\n");
        break;
      }
      t++;
    }
  }

  // Mode mirroring
  //  PORT MONITORED MODE
  else if (strcmp(ID, "PORT1_MIRRORING") == 0 || strcmp(ID, "PORT2_MIRRORING") == 0 || strcmp(ID, "PORT3_MIRRORING") == 0 || strcmp(ID, "PORT4_MIRRORING") == 0 || strcmp(ID, "PORT5_MIRRORING") == 0 || strcmp(ID, "PORT6_MIRRORING") == 0 || strcmp(ID, "PORT7_MIRRORING") == 0 || strcmp(ID, "PORT8_MIRRORING") == 0)
  {
    t = 0;
    write(serial_port, &key_08, sizeof(key_08));
    usleep(1000);
    write(serial_port, &enter, sizeof(enter));
    usleep(1000);

    write(serial_port, &keyK, sizeof(keyK));
    usleep(10000);
    if (ID[4] == '1')
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (ID[4] == '2')
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (ID[4] == '3')
    {
      write(serial_port, &key3, sizeof(key3));
    }
    else if (ID[4] == '4')
    {
      write(serial_port, &key4, sizeof(key4));
    }
    else if (ID[4] == '5')
    {
      write(serial_port, &key5, sizeof(key5));
    }
    else if (ID[4] == '6')
    {
      write(serial_port, &key6, sizeof(key6));
    }
    else if (ID[4] == '7')
    {
      write(serial_port, &key7, sizeof(key7));
    }
    else if (ID[4] == '8')
    {
      write(serial_port, &key8, sizeof(key8));
    }
    usleep(10000);
    if (strcmp(Value, "I") == 0)
    {
      write(serial_port, &key1, sizeof(key1));
    }
    else if (strcmp(Value, "E") == 0)
    {
      write(serial_port, &key2, sizeof(key2));
    }
    else if (strcmp(Value, "IE") == 0)
    {
      write(serial_port, &key3, sizeof(key3));
    }

    while (1)
    {
      char *data1 = receive_data(serial_port);
      printf("\nReceived message: %s\n", data1);
      if ((strchr(data1, 'Y') != NULL))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$OK$");
        printf("\nmiroring mode  done\n");
        break;
      }
      else if ((strchr(data1, 'N') != NULL) || (t == 10))
      {
        strcat(buffer, ID);
        strcat(buffer, "$");
        strcat(buffer, Value);
        strcat(buffer, "$ERROR$");
        printf("\nmiroring mode  ERROR\n");
        break;
      }
      t++;
    }
  }

  //==================================================================================================================
  // HEADER CUOI CUA GOI TIN
  else if (strcmp(ID, "DONE") == 0)
  {
    printf("\nokkkk");
    printf("\n\rBUFFER SEND GUI \n:%s", buffer);
    printf("\nokkkk");
    send(client_sock, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
  }
  //=================================================================================================================
}
//===================================================================================================================
// Copy data file 1 to file 2
void append_ips_to_file(const char *source_file, const char *dest_file)
{
  FILE *log = fopen(dest_file, "a+");
  if (!log)
  {
    perror("Open error");
    return;
  }
  fseek(log, -1, SEEK_END);
  char last_char;
  if (fread(&last_char, 1, 1, log) == 1 && last_char != '\n')
  {
    fputc('\n', log);
  }
  FILE *src = fopen(source_file, "r");
  if (!src)
  {
    perror("Open error");
    fclose(log);
    return;
  }
  char ip[64];
  while (fgets(ip, sizeof(ip), src))
  {
    fputs(ip, log);
  }
  fclose(src);
  fclose(log);
}

// Delete data file http table
void remove_matching_ips(const char *source_file, const char *dest_file)
{
  GHashTable *source_ips = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

  FILE *src = fopen(source_file, "r");
  if (!src)
  {
    perror("Error opening source file");
    return;
  }
  char ip[MAX_IP_LEN];
  while (fgets(ip, sizeof(ip), src))
  {
    ip[strcspn(ip, "\n")] = '\0';
    if (strlen(ip) > 0)
    {
      g_hash_table_add(source_ips, g_strdup(ip));
    }
  }
  fclose(src);

  FILE *dest = fopen(dest_file, "r");
  if (!dest)
  {
    perror("Error opening destination file");
    g_hash_table_destroy(source_ips);
    return;
  }

  FILE *temp = fopen("/home/antiddos/DDoS_V1/HTTP_ip_table/temp_file.txt", "w");
  if (!temp)
  {
    perror("Error creating temporary file");
    fclose(dest);
    g_hash_table_destroy(source_ips);
    return;
  }

  while (fgets(ip, sizeof(ip), dest))
  {
    ip[strcspn(ip, "\n")] = '\0';
    if (!g_hash_table_contains(source_ips, ip))
    {
      fprintf(temp, "%s\n", ip);
    }
  }

  fclose(dest);
  fclose(temp);

  remove(dest_file);
  rename("/home/antiddos/DDoS_V1/HTTP_ip_table/temp_file.txt", dest_file);

  g_hash_table_destroy(source_ips);
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
  printf("Sending via UART: %s\n", data);
}

// Send ip http via core by uart
void send_ips_via_uart(const char *filename)
{
  char buffer[BUFFER_SIZE_SEND_IP_VIA_UART] = "";
  char ip[MAX_IP_LEN];

  FILE *file = fopen(filename, "r");
  if (!file)
  {
    perror("Open file error");
    return;
  }
  while (fgets(ip, sizeof(ip), file))
  {
    ip[strcspn(ip, "\n")] = '\0';
    if (strlen(buffer) + strlen(ip) + 2 > BUFFER_SIZE)
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

// Remove IP from file
void remove_ip_from_file(const char *filename, const char *ip)
{
  FILE *file = fopen(filename, "r");
  if (file == NULL)
  {
    perror("Error opening file");
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
    perror("Error opening file for writing");
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

// Send dong bo GUI
size_t response_handler(void *ptr, size_t size, size_t nmemb, void *userdata)
{
  // print server response
  fwrite(ptr, size, nmemb, stdout);
  return size * nmemb;
}

int post_request(const char *url, const char *json_payload)
{
  CURL *curl;
  CURLcode res;
  struct curl_slist *headers = NULL;

  curl = curl_easy_init();
  if (!curl)
  {
    fprintf(stderr, "Failed to initialize libcurl\n");
    return 1;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);

  headers = curl_slist_append(headers, "Content-Type: application/json");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_handler);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);

  res = curl_easy_perform(curl);

  if (res != CURLE_OK)
  {
    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return 1;
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);

  return 0;
}

int gui_init_sync(const char *data)
{
  const char *apiPath = "api/initsync";
  char *url = malloc(strlen(serverBasedUrl) + strlen(apiPath) + 1);
  strcpy(url, serverBasedUrl);
  strcat(url, apiPath);

  char json_payload[1024];

  snprintf(json_payload, sizeof(json_payload), "{\"data\":\"%s\"}", data);

  int result = post_request(url, json_payload);

  free(url);
  return result;
}
//==================================================================================
void printf_uart2(int serial_port)
{
  char data_buffer[1024] = "";
  memset(data_buffer, 0, sizeof(data_buffer)); // X a buffer
  // int a = 0;
  char read1[1024];
  while (1)
  {
    memset(&read1, 0, sizeof(read1));
    int num_bytes = read(serial_port, &read1, sizeof(read1));

    // Ki?m tra n?u vi?c d?c th?t b?i
    if (num_bytes < 0)
    {
      perror("Error reading from serial port");
      break;
    }

    strncat(data_buffer, read1, num_bytes);

    if (read1[num_bytes - 1] == '^')
    {
      data_buffer[strlen(data_buffer) - 1] = '\0';
      int y = strlen(data_buffer);
      printf("\n\r%s", data_buffer);
      printf("\n");
      gui_init_sync(data_buffer);
      //  send(client_sock, data_buffer, strlen(data_buffer), 0);
      memset(data_buffer, 0, sizeof(data_buffer));
      break;
    }
  }
}

// Ham cau hinh port uart
int configure_serial_port(const char *device, int baud_rate)
{
  int serial_port = open(device, O_RDWR);
  struct termios tty;
  if (tcgetattr(serial_port, &tty) != 0)
  {
    // printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
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
    // printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    return 1;
  }
  return serial_port;
}

// Ham gui du lieu tu uart
int send_data(int serial_port, const char *data, size_t size)
{
  int num_byte = write(serial_port, data, size);
  if (num_byte < 0)
  {
    // vError reading: %s", strerror(errno));
    return 1;
  }
  return num_byte;
}
//===================================================================================
// Creat file log save HTTP ip table
void create_http_filelog(const char *filename)
{
  struct stat buffer;
  if (stat(filename, &buffer) != 0)
  {
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
      perror("Err create file http log");
      exit(EXIT_FAILURE);
    }
    fclose(file);
  }
  else
  {
  }
}

// Send data http ip via core when start
void send_http_ipv4_start(int serial_port, const char *filename)
{
  FILE *file = fopen(filename, "r");
  // Kiem tra kich thuoc file
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
    printf("\nReceived message: %s\n", data1);
    if ((strchr(data1, 'Y') != NULL))
    {
      printf("\nSend HTTP_TABLE_IPv4 done\n");
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
    // printf()
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
    printf("\nReceived message: %s\n", data1);
    if ((strchr(data1, 'Y') != NULL))
    {
      printf("\nSend HTTP_TABLE_IPv6 done\n");
      break;
    }
  }
}

//  Load file log to hash table
void load_ips_from_file(const char *filename)
{
  FILE *file = fopen(filename, "r");
  if (!file)
    return;

  char ip[MAX_IP_LEN];
  while (fgets(ip, sizeof(ip), file))
  {
    ip[strcspn(ip, "\n")] = '\0';
    if (strlen(ip) > 0)
    {
      g_hash_table_add(ip_table, g_strdup(ip));
    }
  }
  fclose(file);
}

// Load file log to remove hash table
void load_remove_hash_ip_http_from_file(const char *filename)
{
  FILE *file = fopen(filename, "r");
  if (!file)
  {
    perror("Error opening file");
    return;
  }

  char ip[MAX_IP_LEN];
  while (fgets(ip, sizeof(ip), file))
  {
    ip[strcspn(ip, "\n")] = '\0';

    if (g_hash_table_remove(ip_table, ip))
    {
      // printf("Removed IP: %s\n", ip);
    }
    else
    {
      // printf("IP not found in hash table: %s\n", ip);
    }
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
    perror("Open file error");
    return;
  }

  while (!g_queue_is_empty(batch_queue))
  {
    char *ip = g_queue_pop_head(batch_queue);
    fprintf(file, "%s\n", ip);
    g_free(ip);
  }

  fclose(file);
}

// Check ip http
void process_ip(const char *filename, const char *ip)
{
  if (g_hash_table_size(ip_table) > MAX_IPS)
  {
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

// Send data sync CLI/GUI
void send_data_sync_gui(int serial_port)
{
  char keyva = '&';
  char key_enter = '\r';

  write(serial_port, &keyva, sizeof(keyva));
  usleep(1000);
  write(serial_port, &key_enter, sizeof(key_enter));
  usleep(10000);
  while (1)
  {
    char *data1 = receive_data(serial_port);
    if ((strchr(data1, 'Y') != NULL))
    {
      printf_uart2(serial_port);
      break;
    }
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
      printf("Syn Time completed");
      break;
    }
    sleep(3);
  }
}

//==========================================================
// Ham nhan du lieu tu uart
char *receive_data(int serial_port)
{
  static char read_buf[256];
  memset(read_buf, '\0', sizeof(read_buf));

  int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
  if (num_bytes < 0)
  {
    // printf("Error reading: %s\n", strerror(errno));
    return NULL;
  }

  // read_buf[num_bytes] = '\0';
  return read_buf;
}

//
void read_config_mode_save_logfile()
{
  FILE *config_fp = fopen(CONFIG_FILE, "r");
  if (config_fp == NULL)
  {
    config_fp = fopen(CONFIG_FILE, "w");
    if (config_fp == NULL)
    {
      exit(1);
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
    }
  }
}

void read_threshold_timecounter_from_file()
{
  FILE *file = fopen(time_counter, "r");
  if (file == NULL)
  {

    file = fopen(time_counter, "w");
    if (file == NULL)
    {

      exit(1);
    }
    fprintf(file, "5");
    fclose(file);
  }
  else
  {
    if (fscanf(file, "%d", &Threshold_time_counter) != 1)
    {
      fclose(file);
      exit(1);
    }

    fclose(file);
  }
}

void read_threshold_from_file()
{
  FILE *file = fopen(threshold_logfile, "r");
  if (file == NULL)
  {
    file = fopen(threshold_logfile, "w");
    if (file == NULL)
    {
      exit(1);
    }
    fprintf(file, "80");
    fclose(file);
  }
  else
  {
    if (fscanf(file, "%f", &Threshold_SD) != 1)
    {
      fclose(file);
      exit(1);
    }

    fclose(file);
  }
}

void read_time_check_create_logfile()
{
  FILE *file = fopen(time_check_create_log, "r");
  if (file == NULL)
  {

    file = fopen(time_check_create_log, "w");
    if (file == NULL)
    {
      exit(1);
    }
    fprintf(file, "1512000");
    fclose(file);
  }
  else
  {
    if (fscanf(file, "%d", &Threshold_time_check_create_logfile) != 1)
    {
      fclose(file);
      exit(1);
    }
    fclose(file);
  }
}

/**************************************************/
void create_new_log_file()
{
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
    perror("fopen error");

    return;
  }
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
      if (dst_port == 80)
      {
        strcpy(type_str, "HTTP Flood");
      }
      else if (dst_port == 443)
      {
        strcpy(type_str, "HTTPS Flood");
      }
      break;
    default:
      strcpy(type_str, "Unknown");
      break;
    }
    // port
    char name_port_str[32];
    //
    if (port_n == 2 || port_n == 4)
    {
      return;
    }
    switch (port_n)
    {
    case 1:
      strcpy(name_port_str, "1");
      break;
    case 2:
      strcpy(name_port_str, "0");
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
    case 50:
      strcpy(protocol_str, "ESP");
      break;
    default:

      strcpy(protocol_str, "Unknown");
      break;
    }

    char binary[11];
    for (int i = 0; i < 10; i++)
    {
      binary[i] = (id_value & (1 << (8 - i))) ? '1' : '0';
    }
    binary[10] = '\0';

    int result_str_size = 1;
    char *result_str = NULL;
    char *mapping[] = {" HTTPS Flood ", " HTTP Flood ", " UDP Fragmentation attack ", " TCP Fragmentation attack ", " IPSec IKE Flood ", " ICMP Flood ", " DNS Flood ", " UDP Flood ", " LAND Attack ", " SYN Flood "};

    for (int i = 0; i < 9; i++)
    {
      if (binary[i] == '1')
      {
        result_str_size += strlen(mapping[i]) + 3;
      }
    }
    result_str = (char *)malloc(result_str_size + 48);
    if (result_str == NULL)
    {
      perror("Failed to allocate memory");
      return;
    }
    result_str[0] = '\0';
    int types_count = 0;
    for (int i = 0; i < 10; i++)
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
    char lcd_message[result_str_size + 48];
    snprintf(lcd_message, result_str_size + 48, "%s", result_str);

    free(result_str);

    //
    pthread_mutex_lock(&lcd_queue.mutex);
    if (strcmp(lcd_message, current_attack) != 0)
    {
      strcpy(current_attack, lcd_message);
      //  pthread_mutex_lock(&lcd_queue.mutex);
      lcd_queue.front = lcd_queue.rear = 0;
      memset(lcd_queue.messages, 0, sizeof(lcd_queue.messages)); // Clear all messages
      // pthread_mutex_unlock(&lcd_queue.mutex);
    }

    snprintf(lcd_queue.messages[lcd_queue.rear], result_str_size + 48, "%s", lcd_message);
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

    // Send socket
    snprintf(uds_msg, sizeof(uds_msg), "%s  %s  %s  %u  %u  %s  %s  %u  %u  %s\n",
             time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, type_str, bw, pkt_counter, name_port_str);
    send_data_socket_uds(uds_msg);

    printf("%s  %s  %s  %u  %u  %s  %s  %u  %u  %s\n",
           time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, type_str, bw, pkt_counter, name_port_str);
    // test = test + pkt_counter;
    //  Log to file
    pthread_mutex_lock(&log_mutex);
    if (current_log_file_flood != NULL)
    {
      fprintf(current_log_file_flood, "%s  %s  %s  %u  %u  %s  %s  %u  %u  %s\n",
              time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, type_str, bw, pkt_counter, name_port_str);
      fflush(current_log_file_flood);
    }
    pthread_mutex_unlock(&log_mutex);

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
    unsigned char extracted_port_n[1];
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
    memcpy(extracted_port_n, payload + 55, 1);
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
    unsigned char port_name = extracted_port_n[0];
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
    char *type_str = "Normal";
    char name_port_str[8];
    char protocol_str[32];

    if (port_name == 2 || port_name == 4)
    {
      return;
    }
    //
    switch (port_name)
    {
    case 1:
      strcpy(name_port_str, "1");
      break;
    case 2:
      strcpy(name_port_str, "0");
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
    // Send socket
    // printf("1\n");

    snprintf(uds_msg, sizeof(uds_msg), "%s  %s  %s  %u  %u  %s  %s  %u  %u  %s\n",
             time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, type_str, bw, pkt_counter, name_port_str);
    send_data_socket_uds(uds_msg);

    // printf("%s  %s  %s  %u  %u  %s  %s  %u  %u  %s\n",
    //        time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, type_str, bw, pkt_counter, name_port_str);
    // printf("2\n");
    // test1 = test1 + pkt_counter;
    // Log to file
    pthread_mutex_lock(&log_mutex);
    if (current_log_file_normal != NULL)
    {
      fprintf(current_log_file_normal, "%s  %s  %s  %u  %u  %s  %s  %u  %u  %s\n",
              time_str, src_ip, dst_ip, src_port, dst_port, protocol_str, type_str, bw, pkt_counter, name_port_str);
      fflush(current_log_file_normal);
    }
    pthread_mutex_unlock(&log_mutex);
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

    if (create_new_log)
    {
      create_new_log_file();
      create_new_log = false;
      created_new_log = true;
    }
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
        if (strlen(message) == 0)
        {
          //
        }
        else
        {
          char bw1_with_space[20];
          snprintf(bw1_with_space, sizeof(bw1_with_space), "  %s", bw1);
          scroll_text1(message, bw1_with_space, 100);
        }
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
time_t save_current_time()
{
  return time(NULL);
}

bool check_time_create_log(time_t saved_time)
{
  time_t current_time = time(NULL);
  double seconds_passed = difftime(current_time, saved_time);
  return (seconds_passed >= Threshold_time_check_create_logfile);
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

void remove_old_logs(void)
{
  DIR *dir;
  struct dirent *entry;
  time_t now = time(NULL);
  char file_path[512];

  dir = opendir(LOG_FLOOD_DIR);

  if (dir == NULL)
  {
    perror("opendir error");
    return;
  }

  while ((entry = readdir(dir)) != NULL)
  {
    if (entry->d_type == DT_REG)
    {
      char *file_name = entry->d_name;

      if (strstr(file_name, ".log") != NULL)
      // if (strstr(file_name, ".txt") != NULL)
      {
        char file_date[11];
        strncpy(file_date, file_name, 10);
        file_date[10] = '\0';

        struct tm tm = {0};
        if (sscanf(file_date, "%4d-%2d-%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday) == 3)
        {
          tm.tm_year -= 1900;
          tm.tm_mon -= 1;
          time_t file_time = mktime(&tm);
          double diff_days = difftime(now, file_time) / (60 * 60 * 24);

          if (diff_days > MAX_LOG_DAYS)
          {
            snprintf(file_path, sizeof(file_path), "%s/%s", LOG_FLOOD_DIR, file_name);
            if (remove(file_path) != 0)
            {
              perror("remove error");
            }
          }
        }
      }
    }
  }
  closedir(dir);
}
void *memory_check_thread_function(void *arg)
{
  time_t saved_time = save_current_time();

  struct statvfs stat1;
  while (1)
  {
    read_threshold_from_file();
    read_time_check_create_logfile();
    check_connect_eth();
    if (check_time_create_log(saved_time))
    {
      create_new_log_file();
      saved_time = save_current_time();
    }

    if (statvfs("/", &stat1) != 0)
    {
      perror("statvfs error");
      pthread_exit(NULL);
    }

    unsigned long total_space = stat1.f_blocks * stat1.f_frsize;
    unsigned long used_space = (stat1.f_blocks - stat1.f_bfree) * stat1.f_frsize;
    float memory_usage = (float)used_space / total_space * 100;
    // //printf("\n Memory:%f\n", memory_usage);

    if (memory_usage > Threshold_SD)
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

      stop_writing = true;
    }
    else if (memory_usage < Threshold_SD && stop_writing)
    {

      current_log_file_flood = fopen(name_logfile_flood, "a");
      current_log_file_normal = fopen(name_logfile_normal, "a");
      if (current_log_file_flood == NULL || current_log_file_normal == NULL)
      {
        perror("Error opening log file");
        pthread_exit(NULL);
      }

      stop_writing = false;
    }
    // printf("\nTong so packet tan cong: %d", test);
    // printf("\nTong so packet binh thuong: %d", test1);
    sleep(3);
  }
  pthread_exit(NULL);
}

// void *log_buffer_thread(void *arg)
// {
//   while (1)
//   {

//     if (log_buffer_pos >= 0)
//     { // //printf("\nbye\n");
//       fwrite(log_buffer, 1, log_buffer_pos, current_log_file_flood);
//       log_buffer_pos = 0; // Reset buffer
//     }
//     // pthread_mutex_unlock(&log_mutex);

//     sleep(1);
//   }
//   return NULL;
//}

void handle_signal(int sig)
{
  char key;
  if (sig == SIGTSTP)
  {
    printf("\nSwitching to menu...\n");
    sleep(3);
    // signal(SIGTSTP, SIG_DFL);
    //  execlp("sudo", "sudo", "./menu.sh", NULL);
    //  execl("/bin/bash", "/bin/bash", "./menu.sh", NULL);
    //  perror("execlp failed");
    exit(1);
  }
}
void send_reset(int serial_port)
{
  char key = 03;
  char enter = '\r';
  write(serial_port, &key, sizeof(key));
  usleep(10000);
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
    // //printf("data :%c\n",data);
    usleep(500000);
  }
  write(serial_port, &enter, sizeof(enter));
}
void check_connect_eth()
{
  int sockfd;
  struct ifreq ifr;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1)
  {
    perror("socket");
    pthread_exit(NULL);
  }
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
  if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
  {
    perror("ioctl");
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

void previous_mode_fc()
{
  FILE *file = fopen(previous_mode, "w");
  if (file == NULL)
  {
    printf("Cannot open file %s\n", previous_mode);
    exit(1);
  }
  fprintf(file, "%c\n", '1');

  fclose(file);
}
void *run(void *arg)
{
  wiringPiSetup();
  lcd_init(LCD_ADDR);
  ClrLcd();
  int argc;
  char **argv;
  /*Khai bao wolfssl*/
  func_args args;

  int return_value;
  init_http_resource();
  StartTCP();
  args.argc = argc;
  args.argv = argv;
  args.return_code = 0;
  return_value = wolfSSL_Init();
  return_value = ChangeToWolfRoot();

  if (return_value == WOLFSSL_SUCCESS)
    printf("ChangeToWolfRoot successfully\n");
  else
    printf("return value ChangeToWolfRoot(): %d\n", return_value);

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
  pthread_t wolfssl_thread;
  pthread_create(&wolfssl_thread, NULL, process_https, &args);
  // Create thread for log buffer
  // pthread_t log_buffer_thread_id;
  // pthread_create(&log_buffer_thread_id, NULL, log_buffer_thread, NULL);
  int sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (sock_raw < 0)
  {
    perror("Socket Error");
    exit(1);
  }
  // Set socket to non-blocking
  int flags = fcntl(sock_raw, F_GETFL, 0);
  if (flags == -1)
  {
    perror("fcntl(F_GETFL)");
    exit(1);
  }
  if (fcntl(sock_raw, F_SETFL, flags | O_NONBLOCK) == -1)
  {
    perror("fcntl(F_SETFL)");
    exit(1);
  }

  // Configure interface in promiscuous mode
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
  if (ioctl(sock_raw, SIOCGIFFLAGS, &ifr) == -1)
  {
    perror("ioctl error");
    close(sock_raw);
    exit(1);
  }
  ifr.ifr_flags |= IFF_PROMISC;
  if (ioctl(sock_raw, SIOCSIFFLAGS, &ifr) == -1)
  {
    perror("ioctl error");
    close(sock_raw);
    exit(1);
  }
  unsigned char *buffer = (unsigned char *)malloc(BUFFER_SIZE);
  if (buffer == NULL)
  {
    perror("Failed to allocate memory");
    exit(1);
  }
  struct sockaddr saddr;
  int saddr_len = sizeof(saddr);

  pthread_mutex_lock(&mutex_new_log);
  create_new_log_file();
  pthread_mutex_unlock(&mutex_new_log);
  while (1)
  {
    int data_size = recvfrom(sock_raw, buffer, BUFFER_SIZE, 0, &saddr, (socklen_t *)&saddr_len);
    if (data_size > 0)
    {
      stop_scrolling = 1;
      struct ethhdr *eth = (struct ethhdr *)buffer;
      if ((memcmp(eth->h_dest, target_mac_attack, 6) == 0))
      {
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
      perror("Recvfrom Error");
      exit(1);
    }
    time_t current_time = time(NULL);
    if (current_time - last_packet_time > 2)
    {

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

// Main C
int main()
{
  serial_port = configure_serial_port("/dev/ttyUSB0", B115200);

  /*===================================================================*/

  // signal(SIGINT, handle_signal);
  // SIGTSTP
  signal(SIGTSTP, handle_signal);
  /******************************************************************/
  read_config_mode_save_logfile();
  read_threshold_from_file();
  read_threshold_timecounter_from_file();
  // create_http_filelog(LOGFILE_HTTP_IPv4);
  // create_http_filelog(LOGFILE_HTTP_IPv6);
  // ip_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  // batch_queue = g_queue_new();
  // load_ips_from_file(LOGFILE_HTTP_IPv4);
  // load_ips_from_file(LOGFILE_HTTP_IPv6);
  // send_http_ipv4_start(serial_port, LOGFILE_HTTP_IPv4);
  // sleep(1);
  // send_http_ipv6_start(serial_port, LOGFILE_HTTP_IPv6);
  connect_to_server();
  previous_mode_fc();

  /*****************************************************************/
  pthread_mutex_lock(&run_mutex);
  if (!is_run_running)
  {
    if (pthread_create(&run_thread, NULL, run, NULL) != 0)
    {
      perror("pthread_create");
      is_run_running = true;
      exit(1);
    }
  }
  pthread_mutex_unlock(&run_mutex);
  /******************************************************************/
  // Sync Time
  // send_data_sync_time(serial_port);

  // Sync GUI
  // send_data_sync_gui(serial_port);
  ModeGui(serial_port);
  /*************************************************************************/
  // flush_batch_to_file(LOGFILE_HTTP_IPv4);
  // flush_batch_to_file(LOGFILE_HTTP_IPv6);
  // g_queue_free(batch_queue);
  // g_hash_table_destroy(ip_table);
  if (pthread_join(run_thread, NULL) != 0)
  {
    perror("pthread_join");
    exit(1);
  }
  close(serial_port);
  close_connection();
  return 0;
}

// 24/6 thi