/******************************************************************************
 *
 * Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   960 0
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <malloc.h>

#include "platform.h"
#include "xil_printf.h"
#include "xil_cache.h"

#include "xuartlite.h"

#include "xuartlite_l.h"
#include "xintc.h"
#include "xparameters.h"

#include "define_eeprom.h"
#include "EEPROM.h"
#include "time.h"

#define IPV6_SEGMENTS 8
#define MAC_SEGMENTS 6
#define IPV4_Version 4
#define IPV6_Version 6
// #define XPAR_DDOS_DEFENDER_0_BASEADDR 	XPAR_ANTI_DDOS_1GB_0_I_ANTI_DDOS_AXI_BASEADDR

#define XPAR_DDOS_DEFENDER_0_BASEADDR 0x44A00000
#define XPAR_NF1_CML_ETHFMC_INTERFACE_0_BASEADDR XPAR_NF1_CML_INTERFACE_0_BASEADDR
#define XPAR_NF1_CML_ETHFMC_INTERFACE_1_BASEADDR XPAR_NF1_CML_INTERFACE_1_BASEADDR
#define XPAR_IIC_1_BASEADDR XPAR_IIC_0_BASEADDR
#define XPAR_AXI_INTC_0_MDIO_CTRL_FINAL_0_INTERRUPT_INTR XPAR_MDIO_CTRL_FINAL_0_BASEADDR
#define XPAR_AXI_INTC_0_MDIO_CTRL_FINAL_1_INTERRUPT_INTR XPAR_MDIO_CTRL_FINAL_1_BASEADDR
#define XPAR_UARTLITE_1_DEVICE_ID XPAR_UARTLITE_0_DEVICE_ID
#define XPAR_AXI_INTC_0_RS232_UART_1_INTERRUPT_INTR XPAR_INTC_0_UARTLITE_0_VEC_ID

#define INTC_DEVICE_ID XPAR_INTC_0_DEVICE_ID

#define MDIO_BASE_ADDR_0 XPAR_MDIO_CTRL_FINAL_0_BASEADDR
#define MDIO_BASE_ADDR_1 XPAR_MDIO_CTRL_FINAL_1_BASEADDR

#define SET_MAC_HI_REG 0x00000400
#define SET_MAC_LO_REG 0x00000404
#define SET_FLOWCTRL 0x0000040c
#define SET_JUMBO 0x00000408

#define CLS      // xil_printf("\033[2J\033[1;1H");
#define CLS_BUFF // xil_printf("\033[3J");
#define CLS_2    // xil_printf("\033\143");

#define SPEED_CONFIG_REG 0x00000410

#define COPPER_CONTROL_REG 0
#define COPPER_STATUS_REG 0
#define COPPER_SPECIFIC_CONTROL_REG_1 16
#define COPPER_SPECIFIC_STATUS_REG_1 17
#define COPPER_SPECIFIC_INTERRUPT_ENABLE_REG 18
#define COPPER_INTERRUPT_STATUS_REG 19

#define T_OUT 100
#define OP_RD 1
#define OP_WR 0
#define ENB 1
#define DIS 0
#define INT 1
#define RDY 1
#define IPV6_SEGMENTS 8

#define IP_TIMEOUT 1
#define MAX_USERNAME_LENGTH 16

#define NUM_USERS 5

EEPROM24C myDevice_EEPROM;
XUartLite xuartLite;
XIntc InterruptController;

u8 key;
u32 _time_ter = 2000000;
unsigned int *Attacker_List;
int Num_of_IP = 300;
int Num_of_URL = 100;
int Num_in_PAL = 100;

#define BUFFER_SIZE_IPV4 20
#define BUFFER_SIZE_IPV6 40
#define LARGE_BUFFER_SIZE 8192

volatile static int intc_processed0 = FALSE;
volatile static int intc_processed1 = FALSE;
volatile static int Status;

int VolumeSYN = 0;
int VolumeSYNON = 0;
int VolumeUDP = 0;
int VolumeDNS = 0;
int VolumeICMP = 0;
int VolumeIPSEC = 0;
int VolumeTCPFrag = 0;
int VolumeUDPFrag = 0;

int IPTarget1 = 0;
int IPTarget2 = 0;
int IPTarget3 = 0;
int IPTarget4 = 0;
int IPv6Target[4] = {0};
int TimeFlood1 = 0;
int TimeFlood2 = 0;
int TimeFlood3 = 0;
int TimeFlood4 = 0;
int TimeFlood5 = 0;
int TimeFlood6 = 0;
int TimeFlood7 = 0;
int TimeFlood8 = 0;

int TimeDelete = 0;
// Thresholds
int SynThreshold1 = 0;
int SynThreshold2 = 0;
int SynThreshold3 = 0;
int SynThreshold4 = 0;
int SynThreshold5 = 0;
int SynThreshold6 = 0;
int SynThreshold7 = 0;
int SynThreshold8 = 0;

// AckThreshold
int AckThreshold1 = 0;
int AckThreshold2 = 0;
int AckThreshold3 = 0;
int AckThreshold4 = 0;
int AckThreshold5 = 0;
int AckThreshold6 = 0;
int AckThreshold7 = 0;
int AckThreshold8 = 0;

// UDPThreshold
int UDPThreshold1 = 0;
int UDPThreshold2 = 0;
int UDPThreshold3 = 0;
int UDPThreshold4 = 0;
int UDPThreshold5 = 0;
int UDPThreshold6 = 0;
int UDPThreshold7 = 0;
int UDPThreshold8 = 0;
// DNSThreshold
int DNSThreshold1 = 0;
int DNSThreshold2 = 0;
int DNSThreshold3 = 0;
int DNSThreshold4 = 0;
int DNSThreshold5 = 0;
int DNSThreshold6 = 0;
int DNSThreshold7 = 0;
int DNSThreshold8 = 0;
// ICMPThreshold
int ICMPThreshold1 = 0;
int ICMPThreshold2 = 0;
int ICMPThreshold3 = 0;
int ICMPThreshold4 = 0;
int ICMPThreshold5 = 0;
int ICMPThreshold6 = 0;
int ICMPThreshold7 = 0;
int ICMPThreshold8 = 0;
// IPSecThreshold
int IPSecThreshold1 = 0;
int IPSecThreshold2 = 0;
int IPSecThreshold3 = 0;
int IPSecThreshold4 = 0;
int IPSecThreshold5 = 0;
int IPSecThreshold6 = 0;
int IPSecThreshold7 = 0;
int IPSecThreshold8 = 0;
// UDPThresh_1s
int UDPThresh_1s1 = 0;
int UDPThresh_1s2 = 0;
int UDPThresh_1s3 = 0;
int UDPThresh_1s4 = 0;
int UDPThresh_1s5 = 0;
int UDPThresh_1s6 = 0;
int UDPThresh_1s7 = 0;
int UDPThresh_1s8 = 0;
// ICMPThresh_1s
int ICMPThresh_1s1 = 0;
int ICMPThresh_1s2 = 0;
int ICMPThresh_1s3 = 0;
int ICMPThresh_1s4 = 0;
int ICMPThresh_1s5 = 0;
int ICMPThresh_1s6 = 0;
int ICMPThresh_1s7 = 0;
int ICMPThresh_1s8 = 0;
// Defender Port
int DefenderPort = 0;

// Enable Defender
int EnableDefender1 = 0;
int EnableDefender2 = 0;
int EnableDefender3 = 0;
int EnableDefender4 = 0;
int EnableDefender5 = 0;
int EnableDefender6 = 0;
int EnableDefender7 = 0;
int EnableDefender8 = 0;

int Duration_time_export;

u8 SYNFlood_en1 = 0;
u8 SYNFlood_en2 = 0;
u8 SYNFlood_en3 = 0;
u8 SYNFlood_en4 = 0;
u8 SYNFlood_en5 = 0;
u8 SYNFlood_en6 = 0;
u8 SYNFlood_en7 = 0;
u8 SYNFlood_en8 = 0;

u8 UDPFlood_en1 = 0;
u8 UDPFlood_en2 = 0;
u8 UDPFlood_en3 = 0;
u8 UDPFlood_en4 = 0;
u8 UDPFlood_en5 = 0;
u8 UDPFlood_en6 = 0;
u8 UDPFlood_en7 = 0;
u8 UDPFlood_en8 = 0;

u8 LANDATTACK_en1 = 0;
u8 LANDATTACK_en2 = 0;
u8 LANDATTACK_en3 = 0;
u8 LANDATTACK_en4 = 0;
u8 LANDATTACK_en5 = 0;
u8 LANDATTACK_en6 = 0;
u8 LANDATTACK_en7 = 0;
u8 LANDATTACK_en8 = 0;

u8 DNSFlood_en1 = 0;
u8 DNSFlood_en2 = 0;
u8 DNSFlood_en3 = 0;
u8 DNSFlood_en4 = 0;
u8 DNSFlood_en5 = 0;
u8 DNSFlood_en6 = 0;
u8 DNSFlood_en7 = 0;
u8 DNSFlood_en8 = 0;

u8 ICMPFlood_en1 = 0;
u8 ICMPFlood_en2 = 0;
u8 ICMPFlood_en3 = 0;
u8 ICMPFlood_en4 = 0;
u8 ICMPFlood_en5 = 0;
u8 ICMPFlood_en6 = 0;
u8 ICMPFlood_en7 = 0;
u8 ICMPFlood_en8 = 0;

u8 IPSecFlood_en1 = 0;
u8 IPSecFlood_en2 = 0;
u8 IPSecFlood_en3 = 0;
u8 IPSecFlood_en4 = 0;
u8 IPSecFlood_en5 = 0;
u8 IPSecFlood_en6 = 0;
u8 IPSecFlood_en7 = 0;
u8 IPSecFlood_en8 = 0;

u8 TCPFragFlood_en1 = 0;
u8 TCPFragFlood_en2 = 0;
u8 TCPFragFlood_en3 = 0;
u8 TCPFragFlood_en4 = 0;
u8 TCPFragFlood_en5 = 0;
u8 TCPFragFlood_en6 = 0;
u8 TCPFragFlood_en7 = 0;
u8 TCPFragFlood_en8 = 0;

u8 UDPFragFlood_en1 = 0;
u8 UDPFragFlood_en2 = 0;
u8 UDPFragFlood_en3 = 0;
u8 UDPFragFlood_en4 = 0;
u8 UDPFragFlood_en5 = 0;
u8 UDPFragFlood_en6 = 0;
u8 UDPFragFlood_en7 = 0;
u8 UDPFragFlood_en8 = 0;

u8 HTTPGETFlood_en1 = 0;
u8 HTTPGETFlood_en2 = 0;
u8 HTTPGETFlood_en3 = 0;
u8 HTTPGETFlood_en4 = 0;
u8 HTTPGETFlood_en5 = 0;
u8 HTTPGETFlood_en6 = 0;
u8 HTTPGETFlood_en7 = 0;
u8 HTTPGETFlood_en8 = 0;

u8 HTTPSGETFlood_en1 = 0;
u8 HTTPSGETFlood_en2 = 0;
u8 HTTPSGETFlood_en3 = 0;
u8 HTTPSGETFlood_en4 = 0;
u8 HTTPSGETFlood_en5 = 0;
u8 HTTPSGETFlood_en6 = 0;
u8 HTTPSGETFlood_en7 = 0;
u8 HTTPSGETFlood_en8 = 0;

u8 DefenderPort_en1 = 0;
u8 DefenderPort_en2 = 0;
u8 DefenderPort_en3 = 0;
u8 DefenderPort_en4 = 0;
u8 DefenderPort_en5 = 0;
u8 DefenderPort_en6 = 0;
u8 DefenderPort_en7 = 0;
u8 DefenderPort_en8 = 0;

// Nhieu port
u8 DefenderPort_en_1 = 0;
u8 DefenderPort_en_2 = 0;
u8 DefenderPort_en_3 = 0;
u8 DefenderPort_en_4 = 0;
u8 DefenderPort_en_5 = 0;
u8 DefenderPort_en_6 = 0;
u8 DefenderPort_en_7 = 0;
u8 DefenderPort_en_8 = 0;

int Tmp_IPv6Target[4] = {0};
int Tmp_IPTarget = 0;
int Tmp_TimeFlood = 0;
int Tmp_TimeDelete = 0;
int Tmp_SynThreshold = 0;
int Tmp_AckThreshold = 0;
int Tmp_UDPThreshold = 0;
int Tmp_DNSThreshold = 0;
int Tmp_ICMPThreshold = 0;
int Tmp_IPSecThreshold = 0;
int Tmp_UDPThresh_1s = 0;
int Tmp_ICMPThresh_1s = 0;
int Tmp_DefenderPort = 0;
int Tmp_EnableDefender = 0;

int IPAttacker_rm = 0;
int IPAttacker = 0;
int isCompleted_AssignData = 0;
int isCompleted_SaveEEPROM = 0;
int isCompleted_ConfigurationIPcore = 0;
int isCompleted_add_account = 0;
int isCompleted_delete_account = 0;
int isCompleted_change_account = 0;
int isCompleted_change_root = 0;
int isCompleted_load_default = 0;
int is_delete_http = 0;
int is_set_http = 0;
int is_delete_http_v6 = 0;
int is_set_http_v6 = 0;
int is_addVPN = 0;
int is_deleteVPN = 0;
int is_addVPN_IP6 = 0;
int is_deleteVPN_IP6 = 0;
u8 user1[16];
u8 pass1[16];
u8 user2[16];
u8 pass2[16];
u8 user3[16];
u8 pass3[16];
u8 user4[16];
u8 pass4[16];
u8 user5[16];
u8 pass5[16];

u8 passroot[16];
u8 zero = 0;
u8 username_login[16];

u8 countCheck = 0;
// u8 username_last[16];

uint32_t time_value;
struct tm *timeinfo;

u8 mode_monitored;   // 2 hex digit (8 bit)
u8 port_monitored;   // 6 hex digit (24 bit, ví dụ 0xaaaaaa)
u8 port_monitored_1; // 6 hex digit (24 bit, ví dụ 0xaaaaaa)
u8 port_monitored_2; // 6 hex digit (24 bit, ví dụ 0xaaaaaa)
u8 port_monitored_3; // 6 hex digit (24 bit, ví dụ 0xaaaaaa)
u8 port_monitored_4; // 6 hex digit (24 bit, ví dụ 0xaaaaaa)

// *********************** HTTP Flood ******************//
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
  unsigned int hash_url;
  unsigned int request_counter;
  struct Potential_IP_List *PAL;
};
struct URL_Connection *URL_Conn_Table;

struct target_URL
{
  unsigned int url;
  int index;
  int max_URL_cnt;
  int num_URLs;

} Atk_URL;

struct Layer7_Parameter
{
  unsigned int connected_IP;
  unsigned int attacked_URL;
  unsigned int just_detected_IP;
} Atk_HTTP;

//============== FUNCTIONS ==========================//

void UartLiteInterupt();
void RecvHandler(void *CallBackRef);
int SetupInterruptSystem(XUartLite *UartLitePtr);

void EnableCaches();
void DisableCaches();
void init_peripheral();
void ModeStart();
void modeOperation();
void interupt();
void ReadData();
void options_mode1();
void mode_login_config_user();
void SetIPv6Target();
int count_digits(char *str);
void mydelay();
void clear_screen();
void display_logo1();
void display_logo2();
void delay_1s();
void delay_1ms();
void delay_ter(u32 _time_in);
void mode1();
// void mode2();
void ReturnMode2(int port);
// void mode3();
void ReturnMode3();

void load_user1();
void load_user2();
void load_pass1();
void load_pass2();

void load_user();
void load_pass();

void load_passroot();
u8 check(u8 *text1, u8 *text2);

void LoadEEPROM();
void LoadEEPROM_defender();
void SaveEEPROM();
void DisplayTable();

// Admin permissions
void check_account1();
void check_account();

void admin_mode();
void admin_change_info();
void add_acount();
void delete_account();
void delete_account_ver_tmn();
void reset_account();
void reset_user_account();
void change_root();
void setload_default();
void load_default();
void AssignData();
void Reset_System();

// User permissions
void user_mode();
void user_change_info();
void change_user_pass();
void reconfig();
// Configure IPCore
void SetDateTime();
void ConfigurationIPcore(int port);
void SetIPTarget();
void Add_IPv4_Protect_Server();
void Add_IPv6_Protect_Server();
void Delete_IPv6_Protect_Server();
void Delete_IPv4_Protect_Server();

void Add_IPv4_Block_Attacker();
void Add_IPv6_Block_Attacker();
void Delete_IPv4_Block_Attacker();
void Delete_IPv6_Block_Attacker();

// void SetIPTarget1(const char *ip_str);
// void SetIPTarget2(const char *ip_str);
// void SetIPTarget3(const char *ip_str);
// void SetIPTarget4(const char *ip_str);
void Set_IPv4_Write_Protect_Server(const char *ip_str);
void Set_IPv4_Remove_Protect_Server(const char *ip_str);

void Set_IPv4_Write_Block_Attacker(const char *ip_str);
void Set_IPv4_Remove_Block_Attacker(const char *ip_str);

void ReceiveIPv4String_protect(char *buffer_IPV4, char *header_buffer_IPV4, size_t buffer_size_IPV4);
void ReceiveIPv4String(char *buffer_IPV4, size_t buffer_size_IPV4);
void SetPortDefender();
void SetDefenderPort();
void SetTimeflood();
void SetTimeDelete();
void SetSynThresh();
void SetAckThresh();
void SetUDPThresh();
void SetICMPThresh();
void SetDNSThresh();
void SetIPSecThresh();
void AddIpv4VPN();
void AddIpv4VPN1(const char *ip_str);
void ProcessAddIpv4VPN();
void RemoveIpv4VPN1(const char *ip_str);
void ProcessRemoveIpv4VPN();
void RemoveIpv4VPN();
void DisplayIpVPN();
void SetUDPThresh1s();
void SetICMPThresh1s();
void SetSynDefender();
void SetSynonymousDefender();
void SetUDPDefender();
void SetDNSDefender();
void SetICMPDefender();
void SetIPSecDefender();
void SetTCPFragDefender();
void SetUDPFragDefender();
void AddIpVPNtoCore();
void SetHTTPGETDefender();
void show_Atk_IP_Table();
void show_info_SDcard();
void add_Atk_IP_Table();
void add_ipv4_table_http_blacklist(const char *ip_str);
void add_ipv4_table_http_whitelist(const char *ip_str);
void add_Atk_IP_Table1(const char *ip_str);
// from file

void Add_IPv4_HTTP_fromFile();
void Add_IPv4_HTTP_Table_From_File(const char *ip_str);
void ProcessBuffer_IPv4_HTTP_Add(char *buffer);
void Process_IPv4_HTTP_Add(const char *ip);

void Clear_IPv4_HTTP_fromFile();
void ProcessBuffer_IPv4_HTTP_Clear(char *buffer);
void Clear_IPv4_HTTP_Table_From_File(const char *ip_str);
void Process_IPv4_HTTP_Clear(const char *ip);

void Add_IPv6_HTTP_fromFile();
void Process_IPv6_HTTP_Add(const char *ip);
void ProcessBuffer_IPv6_HTTP_Add(char *buffer);
void Add_IPv6_HTTP_Table_From_File(const char *ip_str);
void Add_IPv6_Protect_Table_From_File(const char *ip_str);
void Clear_IPv6_HTTP_fromFile();
void Process_IPv6_HTTP_Clear(const char *ip);
void ProcessBuffer_IPv6_HTTP_Clear(char *buffer);
void Clear_IPv6_HTTP_Table_From_File(const char *ip_str);
//
void ADD_IPv4_HTTP_BLACK_LIST();
void ADD_IPv4_HTTP_WHITE_LIST();
void Processadd_ipv4_http();
void ConvertTimestamp();
void change_duration_time_export();
void ADD_IPv6_HTTP_BLACK_LIST();
void REMOVE_IPv6_HTTP_BLACK_LIST();
void AddIpVPN_IPV6();
void RemoveIpVPN_IPV6();
void change_user_pass_admin_mode();
void change_user_pass_admin_mode_ter();

// ************************* HTTP Flood Functions ************//
void set_HTTP_IP_Table();
void load_HTTP_IP_EEPROM(u8 *ip);
void clear_Atk_IP_Table();
void remove_ipv4_table_http_blacklist(const char *ip_str);
void remove_ipv4_table_http_whitelist(const char *ip_str);
void REMOVE_IPv4_HTTP_BLACK_LIST();
void REMOVE_IPv4_HTTP_WHITE_LIST();
void Clear_Atk_IP(unsigned int *Atk_List, unsigned int atk_ip);
void init_http_resource();
void cleanup_http_resource();
int get_HTTP_Data();
int URL_Management(struct URL_Connection *URL_Conn_Table, unsigned int url, unsigned int IP_cnt, int *numURL);
int IP_Management(struct IP_Connection *IP_Conn_Table, unsigned int ip, int timer, int *numIP);
int Blacklist(unsigned int atk_ip, unsigned int *Atk_List);
int Attacker_Detection(struct URL_Connection *URL_Conn_Table, unsigned int ip, int *numPAL, unsigned int *Atk_List);
void reset_HTTP_Table(struct IP_Connection *IP_Conn_Table, struct URL_Connection *URL_Conn_Table, int *numURL, int *numIP, int *numPAL);

//=======================================================
void DisplayAccount();
void show_inf_flow();
void init_interrupt();
void int_handler0();
void int_handler1();
void Auto_negotial();
u16 mdio_functions(u16 addr, u16 *linkStatus, u16 *speed);
int PhyRead(u32 base, u32 PhyAddr, u32 RegAddr, u16 *ReadData);
int PhyWrite(u32 base, u32 PhyAddr, u32 RegAddr, u16 WriteData);
u16 getPhySpeed(u32 base_addr);
void MBdone(u32 base_addr);
void Send_data_gui();

//=======================================================
void Open_port();
void Port_mode(int port_value);
void Add_IPv4_into_port(int port_value, int dst_or_src);
void Add_or_Remove_IPv4_into_port(const char *ip_str, int part_ip1_addr, int part_ip2_addr, int part_ip3_addr, int part_ip4_addr, int ver_ip_addr, int signal_en_addr, int ver_ip_val, int signal_en);
void Remove_IPv4_into_port(int port_value, int dst_or_src);
void Add_IPv6_into_port(int port_value, int dst_or_src);
void Remove_IPv6_into_port(int port_value, int dst_or_src);
void Write_IPv6_into_port(int IPv6_Protect_ADD_0_addr, int IPv6_Protect_ADD_1_addr, int IPv6_Protect_ADD_2_addr, int IPv6_Protect_ADD_3_addr, int IPv6_Protect_ADD_0, int IPv6_Protect_ADD_1, int IPv6_Protect_ADD_2, int IPv6_Protect_ADD_3, int ip_ver_addr, int signal_en_addr, int ver_ip_val, int signal_en);

//=======================================================
// MIRRORING
void Choose_port_monitored();
void Port_mirroring_src_mac();
void Port_mirroring_dst_mac();
void Port_mirroring_src_ipv4();
void Set_src_IP4_Mirroring(const char *ip_str, int i, int add_or_remove);
void Port_mirroring_dst_ipv4();
void Set_dst_IP4_Mirroring(const char *ip_str, int i);
void Port_mirroring_src_dst_ipv6(int dst_or_src);
void Write_IPv6_into_port_mirroring(int IPv6_Protect_ADD_0_addr, int IPv6_Protect_ADD_1_addr, int IPv6_Protect_ADD_2_addr, int IPv6_Protect_ADD_3_addr, int IPv6_Protect_ADD_0, int IPv6_Protect_ADD_1, int IPv6_Protect_ADD_2, int IPv6_Protect_ADD_3);
void Port_mirroring_src_dst_port();
void Port_mirroring_protocol();
void Port_mirroring_mode();
//=======================================================

void change_user_pass_admin_mode()
{
  // DisplayAccount();
  load_user();
  load_pass();
  load_passroot();
  u8 count, count_p;
  u8 key;
  u8 i;
  u8 countCheck = 0;
  u8 pass[16];
  u8 *password[16];
  key = 0;
  count_p = 0;
  while (1)
  {
    // xil_printf("\r\n ===============+========================================================================================================================+");
    // xil_printf("\r\n       | Username: ");
    count = 0;
    key = 0;
    while (key != 13 && count < 16)
    {
      key = XUartLite_RecvByte(0x40600000);
      // xil_printf("%c",key);
      if (key > 47)
      {
        username_login[count] = key;
        count = count + 1;
        username_login[count] = 0;
      }
      else if (key == 03)
      {
        Reset_System();
      }
    }
    // xil_printf("\r\n\t\t|                                                                                                                        |");
    // xil_printf("\r\n\t\t| Password: ");
    count = 0;
    key = 0;
    while (key != 13 || count_p == 0)
    {
      key = XUartLite_RecvByte(0x40600000);
      if (key > 47 && count_p < 15)
      {
        // xil_printf("%c",key);
        password[count_p] = key;
        count_p = count_p + 1;
        password[count_p] = 0;
      }
      else if (key == 03)
      {
        Reset_System();
      }
    }
    // xil_printf("\r\n\t\t|                                                                                                                        |");
    // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------+");
    if (check(user1, username_login))
    {
      for (i = 0; i <= count_p; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, pass1_addr + i, password + i, 1);
        delay_1ms();
      }

      //	XUartLite_SendByte(0x40600000, 'Y');
      isCompleted_change_account = 1;

      break;
    }
    else if (check(user2, username_login))
    {
      for (i = 0; i <= count_p; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, pass2_addr + i, password + i, 1);
        delay_1ms();
      }

      // XUartLite_SendByte(0x40600000, 'Y');
      isCompleted_change_account = 1;

      break;
    }
    else if (check(user3, username_login))
    {
      for (i = 0; i <= count_p; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, pass3_addr + i, password + i, 1);
        delay_1ms();
      }

      //	XUartLite_SendByte(0x40600000, 'Y');
      isCompleted_change_account = 1;

      break;
    }
    else if (check(user4, username_login))
    {
      for (i = 0; i <= count_p; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, pass4_addr + i, password + i, 1);
        delay_1ms();
      }

      //	XUartLite_SendByte(0x40600000, 'Y');
      isCompleted_change_account = 1;
      break;
    }

    else if (check(user5, username_login))
    {
      for (i = 0; i <= count_p; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, pass5_addr + i, password + i, 1);
        delay_1ms();
      }

      // XUartLite_SendByte(0x40600000, 'Y');
      isCompleted_change_account = 1;
      break;
    }
  }

  ReturnMode3();
}

void change_user_pass_admin_mode_ter()
{
  u8 count, count_p;
  u8 key;
  u8 i;
  // u8 countCheck = 0;
  u8 pass[16];
  u8 *password[16];
  key = 0;
  count_p = 0;
  // while (1)
  // {
  // xil_printf("\r\n ===============+========================================================================================================================+");
  // xil_printf("\r\n       | Username: ");
  count = 0;
  key = 0;
  while (key != 13 && count < 16)
  {
    key = XUartLite_RecvByte(0x40600000);
    // xil_printf("%c",key);
    if (key > 47)
    {
      username_login[count] = key;
      count = count + 1;
      username_login[count] = 0;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
  // xil_printf("\r\n\t\t|                                                                                                                        |");
  // xil_printf("\r\n\t\t| Password: ");
  count = 0;
  key = 0;
  while (key != 13 || count_p == 0)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key > 47 && count_p < 15)
    {
      // xil_printf("%c",key);
      password[count_p] = key;
      count_p = count_p + 1;
      password[count_p] = 0;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
  if (check(user1, username_login))
  {
    for (i = 0; i <= count_p; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, pass1_addr + i, password + i, 1);
      delay_1ms();
    }
    XUartLite_SendByte(0x40600000, 'Y');
    // delay_1s();
    //  break;
  }
  else if (check(user2, username_login))
  {
    for (i = 0; i <= count_p; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, pass2_addr + i, password + i, 1);
      delay_1ms();
    }
    XUartLite_SendByte(0x40600000, 'Y');
    // delay_1s();
    //  break;
  }
  else if (check(user3, username_login))
  {
    for (i = 0; i <= count_p; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, pass3_addr + i, password + i, 1);
      delay_1ms();
    }
    XUartLite_SendByte(0x40600000, 'Y');
    // delay_1s();
    // break;
  }
  else if (check(user4, username_login))
  {
    for (i = 0; i <= count_p; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, pass4_addr + i, password + i, 1);
      delay_1ms();
    }
    XUartLite_SendByte(0x40600000, 'Y');
    // delay_1s();
    //  break;
  }

  else if (check(user5, username_login))
  {
    for (i = 0; i <= count_p; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, pass5_addr + i, password + i, 1);
      delay_1ms();
    }
    XUartLite_SendByte(0x40600000, 'Y');
    // delay_1s();
    // break;
  }
  else
  {
    XUartLite_SendByte(0x40600000, 'X');
    // delay_1s();
    // break;
  }
}
//}

void Reset_System()
{
  // //xil_printf("Loading Bitstream.........");
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 312, 1);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 312, 0);
}

void show_info_SDcard()
{
  u8 key = 0;
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 'e' || key == 'E')
    {
      XUartLite_SendByte(0x40600000, 'E');
      delay_1s();
      break;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
}

void ConvertTimestamp()
{
  u8 key;
  // uint32_t time_value;
  time_value = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 8);

  time_t timestamp = (time_t)time_value;
  // struct tm *timeinfo;
  timeinfo = localtime(&timestamp);

  if (timeinfo == NULL)
  {
    // xil_printf("Error: Unable to convert timestamp.\n");
    return;
  }
}

void SetDateTime()
{
  struct tm timeinfo = {0};
  u8 Dtime;
  int temp;

Return:
  temp = 0;
  temp = 1;

  while (temp > 0)
  {
    switch (temp)
    {
    case 1:
      // xil_printf("\r\n\t\t");
      // xil_printf("\r\n================+================================================================================================================+");
      // xil_printf("\r\n   Time - Date  |               For RTC module                                                                                   |");
      // xil_printf("\r\n\t\t+================================================================================================================+");
      // xil_printf("\r\n\t\t| Enter the time in the format (YYYY-MM-DD HH:MM:SS) : ");
      temp = 2;

    case 2: //////////////////// YEAR ////////////////////
      while (1)
      {
        // xil_printf("\r\n\t\t| Enter Year: ");
        Dtime = (XUartLite_RecvByte(0x40600000) - '0') * 10;
        Dtime += XUartLite_RecvByte(0x40600000) - '0';
        if (Dtime > 0)
        {
          timeinfo.tm_year = (Dtime + 2000) - 1900;
          temp = 3;
          // xil_printf("Year set to: 20%x\n", Dtime);
          break;
        }
        else
        {
          // xil_printf("\r\n\t\t| Warning: Invalid year %x", Dtime);
          goto Return;
        }
      }

    case 3:
      temp = 4;

    case 4:
      while (1)
      {
        // xil_printf("\r\n\t\t| Enter Month: ");
        Dtime = (XUartLite_RecvByte(0x40600000) - '0') * 10;
        Dtime += XUartLite_RecvByte(0x40600000) - '0';
        if (Dtime > 0 && Dtime <= 12)
        {
          timeinfo.tm_mon = Dtime - 1;
          temp = 5;
          // xil_printf("Month set to: %x\n", Dtime);
          break;
        }
        else
        {
          // xil_printf("\r\n\t\t| Warning: Invalid month %x ", Dtime);
          goto Return;
        }
      }

    case 5:
      temp = 6;

    case 6:
      while (1)
      {
        // xil_printf("\r\n\t\t| Enter Day: ");
        Dtime = (XUartLite_RecvByte(0x40600000) - '0') * 10;
        Dtime += XUartLite_RecvByte(0x40600000) - '0';
        if (Dtime > 0 && Dtime <= 31)
        {
          timeinfo.tm_mday = Dtime;
          temp = 7;
          // xil_printf("Day set to: %x\n", Dtime);
          break;
        }
        else
        {
          // xil_printf("\r\n\t\t| Warning: Invalid date %x ", Dtime);
          goto Return;
        }
      }

    case 7:
      temp = 8;

    case 8:
      while (1)
      {
        // xil_printf("\r\n\t\t| Enter Hour: ");
        Dtime = (XUartLite_RecvByte(0x40600000) - '0') * 10;
        Dtime += XUartLite_RecvByte(0x40600000) - '0';
        if (Dtime >= 0 && Dtime <= 23)
        {
          timeinfo.tm_hour = Dtime;
          temp = 9;
          // xil_printf("Hour set to: %x\n", Dtime);
          break;
        }
        else
        {
          // xil_printf("\r\n\t\t| Warning: Invalid hour %x", Dtime);
          goto Return;
        }
      }

    case 9:
      temp = 10;

    case 10:
      while (1)
      {
        // xil_printf("\r\n\t\t| Enter Minute: ");
        Dtime = (XUartLite_RecvByte(0x40600000) - '0') * 10;
        Dtime += XUartLite_RecvByte(0x40600000) - '0';
        if (Dtime >= 0 && Dtime <= 59)
        {
          timeinfo.tm_min = Dtime;
          temp = 11;
          // xil_printf("Minute set to: %x\n", Dtime);
          break;
        }
        else
        {
          // xil_printf("\r\n\t\t| Warning: Invalid minute %x", Dtime);
          goto Return;
        }
      }

    case 11:
      temp = 12;

    case 12:
      while (1)
      {
        // xil_printf("\r\n\t\t| Enter Second: ");
        Dtime = (XUartLite_RecvByte(0x40600000) - '0') * 10;
        Dtime += XUartLite_RecvByte(0x40600000) - '0';
        if (Dtime >= 0 && Dtime <= 59)
        {
          timeinfo.tm_sec = Dtime;
          temp = 0;
          // xil_printf("Second set to: %x\n", Dtime);
          break;
        }
        else
        {
          // xil_printf("\r\n\t\t| Warning: Invalid second %x", Dtime);
          goto Return;
        }
      }
      break;
    }
    break;
  }

  time_t timestamp = mktime(&timeinfo);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 8, timestamp);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 352, 1);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 352, 0);
  ReturnMode2(1);
}

void mydelay(int a)
{
  int b = 0;
  while (1)
  {
    b = b + 1;
    if (b == a)
    {
      break;
    }
  }
}

int main()
{
  init_platform();
  EnableCaches();
  init_peripheral();

  init_http_resource();
  LoadEEPROM();
  //   EnableDefender1 = 12543;
  load_default();
  // ConfigurationIPcore();

  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 600, 0x00000007);

  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 604, 0xc0a80164);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 608, 0xe45f010d);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 612, 0x0000ec66);

  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 172, 0xc0a80159);
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000010);
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000000);

  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 576, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 580, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 584, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 588, 0xc0a80142);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 592, 0x00000004);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 572, 0x00000001);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 572, 0x00000000);

  // XUartLite_SendByte(0x40600000, 'k');
  // XUartLite_SendByte(0x40600000, 'L');
  // xil_printf("\r\n ppppp %d", EnableDefender1);
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 524, 0x11110000);
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 528, 0x00000000);
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 532, 0x00000000);
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 536, 0x00001112); // ip_dst_protect port 0
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 540, 0x00000006);
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 544, 0x00000001);
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 544, 0x00000000);

  //
  ModeStart();
  // show_inf_flow();

  cleanup_http_resource();
  cleanup_platform();
  return 0;
}

void EnableCaches()
{
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
  Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
  Xil_DCacheEnable();
#endif
#endif
}

void DisableCaches()
{
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_DCACHE
  Xil_DCacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_ICACHE
  Xil_ICacheDisable();
#endif
#endif
}

void delay_1s()
{
  u32 _time;
  _time = 9000000;
  // _time = 1000000;
  while (_time)
  {
    _time = _time - 1;
  }
}
void delay_0_5s()
{
  u32 _time;
  _time = 4500000;
  // _time = 1000000;
  while (_time)
  {
    _time = _time - 1;
  }
}
void delay_1ms()
{
  u32 _time;
  _time = 200000;
  while (_time)
  {
    _time = _time - 1;
    // //xil_printf("_time: %d\r\n", _time);
  }
}

void delay_ter(u32 _time_in)
{
  //	u32 _time;
  //	_time = 5000000;
  ////xil_printf("\r\n%d",_time_in);
  while (_time_in)
  {
    _time_in = _time_in - 1;
  }
}

//    RTCC_begin(&myDevice, XPAR_IIC_0_BASEADDR, 0x68);		// RTC
void init_peripheral()
{
  // EEPROM_begin(&myDevice_EEPROM, XPAR_IIC_1_BASEADDR, 0x57);		// 0x57 is chip Addr for EEPROM on ML605
  // EEPROM_begin(&myDevice_EEPROM, XPAR_IIC_1_BASEADDR, 0x54);		// 0x54 is chip Addr for EEPROM on ALINX
  EEPROM_begin(&myDevice_EEPROM, XPAR_IIC_1_BASEADDR, 0x50);

  // delay_1ms();
  // EEPROM_ReadIIC(&myDevice_EEPROM, number_ip_vpn, &number_ip, 1);

  // Set MAC ADDR
  u32 mac_addr_1_hi = 0xDDB8A89A;
  u32 mac_addr_1_lo = 0x5000d1dd;

  u32 tx_flowctrl = 0x40000000;
  u32 jumboframe = 0x50000000;

  u32 mac_addr_2_hi = 0xDDB8A89A;
  u32 mac_addr_2_lo = 0x5000d2dd;

  // Xil_Out32((XPAR_NF1_CML_ETHFMC_INTERFACE_0_BASEADDR) + (SET_MAC_HI_REG), (u32)(mac_addr_1_hi));
  // Xil_Out32((XPAR_NF1_CML_ETHFMC_INTERFACE_0_BASEADDR) + (SET_MAC_LO_REG), (u32)(mac_addr_1_lo));

  // Xil_Out32((XPAR_NF1_CML_ETHFMC_INTERFACE_0_BASEADDR) + (SET_FLOWCTRL), (u32)(tx_flowctrl));
  // Xil_Out32((XPAR_NF1_CML_ETHFMC_INTERFACE_0_BASEADDR) + (SET_JUMBO), (u32)(jumboframe));

  // Xil_Out32((XPAR_NF1_CML_ETHFMC_INTERFACE_1_BASEADDR) + (SET_MAC_HI_REG), (u32)(mac_addr_2_hi));
  // Xil_Out32((XPAR_NF1_CML_ETHFMC_INTERFACE_1_BASEADDR) + (SET_MAC_LO_REG), (u32)(mac_addr_2_lo));

  // Xil_Out32((XPAR_NF1_CML_ETHFMC_INTERFACE_1_BASEADDR) + (SET_FLOWCTRL), (u32)(tx_flowctrl));
  // Xil_Out32((XPAR_NF1_CML_ETHFMC_INTERFACE_1_BASEADDR) + (SET_JUMBO), (u32)(jumboframe));
}

void UartLiteInterupt()
{
  int Status;
  // Initial
  Status = XUartLite_Initialize(&xuartLite, XPAR_UARTLITE_1_DEVICE_ID);
  // Seltest
  Status = XUartLite_SelfTest(&xuartLite);
  // Setup Interrupt
  Status = SetupInterruptSystem(&xuartLite);

  XUartLite_SetRecvHandler(&xuartLite, RecvHandler, &xuartLite);

  XUartLite_EnableInterrupt(&xuartLite);
}

void RecvHandler(void *CallBackRef)
{
  u32 IPdst = 0;
  int RecvTemp = 0;
  u16 Recv = 0;
  while (1)
  {
    Recv = XUartLite_RecvByte(0x40600000);
    if (Recv == 33)
    { //"!" SetIP Target
      IPdst = RecvTemp;
      RecvTemp = 0;
    }
    else if (Recv == 35)
    { //"#" SetTimeFlood
      TimeFlood1 = RecvTemp;
      RecvTemp = 0;
    }
    else if (Recv == 36)
    { //"$" SetSynThreshold
      SynThreshold1 = RecvTemp;
      RecvTemp = 0;
    }
    else if (Recv == 37)
    { //"%" SetAckThreshold
      AckThreshold1 = RecvTemp;
      RecvTemp = 0;
    }
    else if (Recv == 38)
    { //"&" SetUDPThreshold
      UDPThreshold1 = RecvTemp;
      RecvTemp = 0;
    }
    else if (Recv == 64)
    { //"@" SetICMPThreshold
      ICMPThreshold1 = RecvTemp;
      RecvTemp = 0;
    }
    else if (Recv == 47)
    { //"/" SetDNSThreshold
      DNSThreshold1 = RecvTemp;
      RecvTemp = 0;
    }
    else if (Recv == 63)
    { //"?" SetUDPThreshold 1s
      UDPThresh_1s1 = RecvTemp;
      RecvTemp = 0;
    }
    else if (Recv == 126)
    { //"~" SetICMPThreshold 1s
      ICMPThresh_1s1 = RecvTemp;
      RecvTemp = 0;
    }
    else if (Recv == 92)
    { //"\" SetEnable
      EnableDefender1 = RecvTemp;
      RecvTemp = 0;
    }
    else if (Recv == 62)
    { //">" Write Lite to IPcore
      // Set Ip taget:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 4, IPdst);
      // Set Timeflood
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 16, TimeFlood1);
      // Set Syn Threshold
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 20, SynThreshold1);
      // Set Ack Threshold
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 24, AckThreshold1);
      // Set Udp Threshold
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 28, UDPThreshold1);
      // Set Icmp Threshold
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 32, ICMPThreshold1);
      // Set Dns Thrshold
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 36, DNSThreshold1);
      // Set Udp Threshold each sencond.
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 112, UDPThresh_1s1);
      // Set Icmp Threshold each second
      // Set Dns Threshold each second.
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 120, ICMPThresh_1s1);
      delay_1s();
      // Set Enable DDoS Defender
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 40, EnableDefender1); // all
      break;
    }
    else
    {
      RecvTemp = (Recv - 48) + RecvTemp * 10;
    }
  }
}

int SetupInterruptSystem(XUartLite *UartLitePtr)
{

  int Status;

  Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
  if (Status != XST_SUCCESS)
  {
    return XST_FAILURE;
  }

  Status = XIntc_Connect(&InterruptController, XPAR_AXI_INTC_0_RS232_UART_1_INTERRUPT_INTR,
                         (XInterruptHandler)XUartLite_InterruptHandler,
                         (void *)UartLitePtr);
  if (Status != XST_SUCCESS)
  {
    return XST_FAILURE;
  }

  Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
  if (Status != XST_SUCCESS)
  {
    return XST_FAILURE;
  }

  XIntc_Enable(&InterruptController, XPAR_AXI_INTC_0_RS232_UART_1_INTERRUPT_INTR);

  Xil_ExceptionInit();

  Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                               (Xil_ExceptionHandler)XIntc_InterruptHandler,
                               &InterruptController);

  Xil_ExceptionEnable();

  return XST_SUCCESS;
}

void show_inf_flow()
{
  int Number_of_IP;
  // int Num_of_IP = 300;
  // int Num_of_URL = 100;
  // int Num_in_PAL = 100;

  // xil_printf("\t\t\t\t\t\t\t\t\t\t                                         |");
  // 		    xil_printf("\r\n\t\t|                                                                                                                                                                        |");
  // 		    xil_printf("\r\n\t\t|------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
  // 			xil_printf("\r\n\t\t|========================================================================================================================================================================|");
  // 			xil_printf("\r\n\t\t+-------------------------------------------------------------------------  infomation about flow attacker  -------------------------------------------------------------------+");
  // 			xil_printf("\r\n\t\t|------------------------------------------------------------------------------------------------------------------------------------------------------------------------|\r\n");
  // init_interrupt();
  while (1)
  {
    // Auto_negotial();
    // UartLiteInterupt();
    if (XUartLite_IsReceiveEmpty(0x40600000))
    {
      // modeOperation();
      print_process_Info();

      Number_of_IP = get_HTTP_Data(&Num_of_URL, &Num_of_IP, &Num_in_PAL);
      // //xil_printf("| Num_of_IP: %d", Num_of_IP);
      // //xil_printf("| WR_DDoS: %u", ANTI_DDOS_10GB_mReadReg(ptr + offset, WR_DDOS_FIFO_CNT));
      // //xil_printf("| RD_DDoS: %u", ANTI_DDOS_10GB_mReadReg(ptr + offset, RD_DDOS_FIFO_CNT));
      if (Number_of_IP == Num_of_IP)
      {
        Num_of_IP += 100;
        IP_Conn_Table = (struct IP_Connection *)realloc(IP_Conn_Table, Num_of_IP * sizeof(struct IP_Connection));
        if (IP_Conn_Table == NULL)
        {
          // xil_printf("IP malloc failed\n");
        }
      }
    }
    else
    {
      interupt();
    }
  }
}

void init_interrupt()
{
  intc_processed0 = FALSE;
  intc_processed1 = FALSE;

  Status = XIntc_Initialize(&InterruptController, XPAR_INTC_0_DEVICE_ID);

  Status = XIntc_Connect(&InterruptController,
                         XPAR_AXI_INTC_0_MDIO_CTRL_FINAL_0_INTERRUPT_INTR, int_handler0, 0);

  Status = XIntc_Connect(&InterruptController,
                         XPAR_AXI_INTC_0_MDIO_CTRL_FINAL_1_INTERRUPT_INTR, int_handler1, 0);
  XIntc_Enable(&InterruptController,
               XPAR_AXI_INTC_0_MDIO_CTRL_FINAL_1_INTERRUPT_INTR);

  XIntc_Enable(&InterruptController,
               XPAR_AXI_INTC_0_MDIO_CTRL_FINAL_0_INTERRUPT_INTR);

  Xil_ExceptionInit();
  Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                               (Xil_ExceptionHandler)XIntc_InterruptHandler,
                               (void *)&InterruptController);

  Xil_ExceptionEnable();

  XIntc_Start(&InterruptController, XIN_REAL_MODE);
}

void int_handler0()
{
  intc_processed0 = TRUE;
}

void int_handler1()
{
  intc_processed1 = TRUE;
}

void Auto_negotial()
{
  u16 link_status_0;
  u16 link_status_1;

  u16 link_speed_0;
  u16 link_speed_1;

  /*	u16 speed_0 ;
  u16 speed_1 ;*/
  //==================================== PROCESS PORT 0 ============================================================
  if (intc_processed0)
  {
    intc_processed0 = FALSE;
    // Note:	// link_speed_0 = 0 - 10Mbps
    //  link_speed_0 = 1 - 100Mbps
    //  link_speed_0 = 2 - 1Gbps
    mdio_functions(0, &link_status_0, &link_speed_0);

    u32 speed_config_0;
    speed_config_0 = Xil_In32((XPAR_NF1_CML_ETHFMC_INTERFACE_0_BASEADDR) + (SPEED_CONFIG_REG));
    speed_config_0 = speed_config_0 >> 30;

    if (link_speed_0 != speed_config_0)
    {
      speed_config_0 = link_speed_0;
      speed_config_0 = (speed_config_0 << 30);
      Xil_Out32((XPAR_NF1_CML_ETHFMC_INTERFACE_0_BASEADDR) + (SPEED_CONFIG_REG), (u32)(speed_config_0));
    }

    MBdone(MDIO_BASE_ADDR_0);
  }

  //==================================== PROCESS PORT 1 ============================================================

  if (intc_processed1)
  {
    intc_processed1 = FALSE;

    mdio_functions(1, &link_status_1, &link_speed_1);

    u32 speed_config_1;
    speed_config_1 = Xil_In32((XPAR_NF1_CML_ETHFMC_INTERFACE_1_BASEADDR) + (SPEED_CONFIG_REG));
    speed_config_1 = speed_config_1 >> 30;

    if (link_speed_1 != speed_config_1)
    {
      speed_config_1 = link_speed_1;
      speed_config_1 = (speed_config_1 << 30);
      Xil_Out32((XPAR_NF1_CML_ETHFMC_INTERFACE_1_BASEADDR) + (SPEED_CONFIG_REG), (u32)(speed_config_1));
    }

    MBdone(MDIO_BASE_ADDR_1);
  }
}

u16 mdio_functions(u16 addr, u16 *linkStatus, u16 *speed)
{
  u16 energy_detect_status;
  u16 status = 0;

  switch (addr)
  {
  case 0:
    PhyRead(MDIO_BASE_ADDR_0, 0, COPPER_SPECIFIC_STATUS_REG_1,
            &energy_detect_status);
    status = (energy_detect_status >> 10) & 0x1;

    if (status == 1)
    {
      *speed = getPhySpeed(MDIO_BASE_ADDR_0);
      // setCMLSpeed(CML_BASE_ADDR_0, speed);
    }
    *linkStatus = status;

    break;
  case 1:
    PhyRead(MDIO_BASE_ADDR_1, 0, COPPER_SPECIFIC_STATUS_REG_1,
            &energy_detect_status);
    status = (energy_detect_status >> 10) & 0x1;

    if (status == 1)
    {
      *speed = getPhySpeed(MDIO_BASE_ADDR_1);
      // setCMLSpeed(CML_BASE_ADDR_1, speed);
    }
    *linkStatus = status;

    break;

  default:
    break;
  }

  return status;
}

////======================================================================================================================================================
//// PHY interaction
////======================================================================================================================================================
int PhyRead(u32 base, u32 PhyAddr, u32 RegAddr, u16 *ReadData)
{
  u32 stat, timeout;

  u32 ctrl = base + 0x0c;
  u32 addr = base + 0x00;
  u32 rd = base + 0x08;

  // wait for ready
  timeout = T_OUT;
  while ((stat = Xil_In16(ctrl) & RDY) && (timeout > 0))
  {
    // PhySleep(10);
    timeout--;
    //		//xil_printf("\r\nPhyRead ERROR: MAC Not Ready1 0x%04x\r\n",
    //				Xil_In16(ctrl) & RDY);
  }
  if (timeout == 0)
  {
    //		//xil_printf("\r\nPhyRead ERROR: MAC Not Ready 0x%04x\r\n", stat);
    return XST_FAILURE;
  }

  // initiate request
  Xil_Out32(addr, (OP_RD << 10) | (PhyAddr << 5) | RegAddr);
  Xil_Out32(ctrl, (ENB << 3) | INT);
https: // www.mkyong.com/java/java-display-double-in-2-decimal-points/

  // wait for response
  timeout = T_OUT;
  while ((stat = Xil_In16(ctrl) & RDY) && (timeout > 0))
  {
    // PhySleep(10);
    timeout--;
  }
  if (timeout == 0)
  {
    //		//xil_printf("\r\nPhyRead ERROR: Response Timeout 0x%04x\r\n", stat);
    return XST_FAILURE;
  }

  // read data from input register
  *ReadData = Xil_In16(rd);

  return XST_SUCCESS;
}

int PhyWrite(u32 base, u32 PhyAddr, u32 RegAddr, u16 WriteData)
{
  u32 stat, timeout;

  u32 ctrl = base + 0x0c;
  u32 addr = base + 0x00;
  u32 wr = base + 0x04;

  // wait for ready
  timeout = T_OUT;
  while ((stat = Xil_In16(ctrl) & RDY) && (timeout > 0))
  {
    // PhySleep(1);
    timeout--;
  }
  if (timeout == 0)
  {
    //		//xil_printf("\r\nPhyWrite ERROR: MAC Not Ready\r\n");
    return XST_FAILURE;
  }

  // put data in MDIO write data register and initiate request
  Xil_Out32(addr, (OP_WR << 10) | (PhyAddr << 5) | RegAddr);
  Xil_Out32(wr, WriteData);
  Xil_Out32(ctrl, (ENB << 3) | INT);

  // wait for response
  timeout = T_OUT;
  while ((stat = Xil_In16(ctrl) & RDY) && (timeout > 0))
  {
    // PhySleep(1);
    timeout--;
  }
  if (timeout == 0)
  {
    //		//xil_printf("\r\nPhyWrite ERROR: Response Timeout\r\n");
    return XST_FAILURE;
  }

  return XST_SUCCESS;
}

////======================================================================================================================================================
//// PHY interaction
////======================================================================================================================================================
u16 getPhySpeed(u32 base_addr)
{
  u16 read_data;
  u16 speed;

  PhyRead(base_addr, 0, COPPER_SPECIFIC_STATUS_REG_1, &read_data);
  speed = (read_data >> 14) & 0x3;

  return speed;
}

void MBdone(u32 base_addr)
{
  Xil_Out32(base_addr, 255);
}

void interupt()
{
  u8 key = 0;
  u8 count = 0;
  u32 time_inter;
  while (count < 1)
  {
    key = XUartLite_RecvByte(0x40600000);
    // //xil_printf("\r\n%02x", key);
    if (key == 13)
    {
      options_mode1();
      // xil_printf("\r\n");
      ModeStart();
      break;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else if (key == '+')
    {
      time_inter = _time_ter;
      if (time_inter > 50000000)
      {
        _time_ter = 50000000;
        break;
      }
      ////xil_printf("\r\n Cong");
      _time_ter = time_inter + 200000;
      break;
    }
    else if (key == '-')
    {
      time_inter = _time_ter;
      if (time_inter < 0)
      {
        _time_ter = 0;
        break;
      }
      ////xil_printf("\r\n Tru");
      _time_ter = time_inter - 200000;
      break;
    }
    else
    {
      break;
    }
  }
}

void options_mode1()
{
  u8 key1, done;
  u8 key2, key3;
start:
  key1 = 0;
  done = 0;
  key2 = 0;
  key3 = 0;
  //   xil_printf("\r\n                                                                                                                                         |");
  // 		  xil_printf("\r\n ================+===========+===========================================================================================================+");
  // 		  xil_printf("\r\n     DISPLAY     |           |                                                                                                           |");
  // 					  xil_printf("\r\n\t\t | Key Enter |                  Mode                                                                                     |");
  // 					  xil_printf("\r\n\t\t +-----------+-----------------------------------------------------------------------------------------------------------+");
  // 					  xil_printf("\r\n\t\t |     1:    | Return mode start                                                                                         |");
  // 					  xil_printf("\r\n\t\t +-----------+-----------------------------------------------------------------------------------------------------------+");
  // 					  xil_printf("\r\n\t\t |     2:    | Continue show terminal                                                                                    |");
  // 					  xil_printf("\r\n\t\t +-----------+-----------------------------------------------------------------------------------------------------------+");
  // 					  xil_printf("\r\n\t\t |     3:    | Show config curent                                                                                        |");
  // 					  xil_printf("\r\n\t\t +-----------+-----------------------------------------------------------------------------------------------------------+");
  // 					  xil_printf("\r\n\t\t |     4:    | Show condition SD card                                                                                    |");
  // 					  xil_printf("\r\n\t\t +-----------+-----------------------------------------------------------------------------------------------------------+");
  // 		 xil_printf("\r\n ----------------+-----------------------------------------------------------------------------------------------------------------------+");
  // 		 xil_printf("\r\n ================+=======================================================================================================================+");
  //  		 xil_printf("\r\n     SETTING     | --> Please choose Mode: ");
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    // xil_printf("%c", key1);
    key2 = key1 - 48;
    key3 = key1;
    if (key3 == 03)
    {
      Reset_System();
    }
    else if (key2 == 1)
    {
      XUartLite_SendByte(0x40600000, '1');
      break;
    }
    else if (key2 == 2)
    {
      XUartLite_SendByte(0x40600000, '2');
      delay_1s();
      show_inf_flow();
    }
    else if (key2 == 3)
    {
      XUartLite_SendByte(0x40600000, '3');
      delay_1s();
      LoadEEPROM();
      delay_1s();
      DisplayTable();
      mode_login_config_user();
      goto start;
    }
    else if (key2 == 4)
    {
    }
  }
}

void mode_login_config_user()
{
  u8 key = 0;
  // xil_printf (" Do you want login to config: Y or N: ");
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    // xil_printf("%c", key);
    if (key == 'y' || key == 'Y')
    {
      XUartLite_SendByte(0x40600000, 'Y');
      check_account();
      ConfigurationIPcore(1);
      break;
    }
    else if (key == 'n' || key == 'N')
    {
      XUartLite_SendByte(0x40600000, 'N');
      break;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
}

void SetIPv6Target()
{
  u8 key;
  u16 IPv6Segment[IPV6_SEGMENTS] = {0};
  u8 next = 0;
  u8 flag_error = 0;
  char input_buffer[40] = {0};
  u8 count = 0;
  int double_colon_position = -1;

  // xil_printf("\r\nNh?p d?a ch? IP Server IPv6 c?n b?o v?: ");
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 13)
    {
      break;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else
    {
      // xil_printf("%c", key);
      input_buffer[count++] = key;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  input_buffer[count] = '\0';

  char *token = strtok(input_buffer, ":");
  while (token != NULL && next < IPV6_SEGMENTS)
  {
    if (strcmp(token, "") == 0)
    {
      if (double_colon_position == -1)
      {
        double_colon_position = next;
        token = strtok(NULL, ":");
        continue;
      }
      else
      {
        flag_error = 1;
        break;
      }
    }
    else
    {
      if (count_digits(token) > 4)
      {
        flag_error = 1;
        break;
      }
      u16 segment_value = (u16)strtol(token, NULL, 16);
      IPv6Segment[next++] = segment_value;
      // xil_printf("\r\nSegment %d set to %04x", next - 1, segment_value);
    }
    token = strtok(NULL, ":");
  }

  if (double_colon_position != -1)
  {
    int segments_to_add = IPV6_SEGMENTS - next;
    for (int i = 0; i < segments_to_add; i++)
    {
      IPv6Segment[double_colon_position + i] = 0;
    }
    for (int i = 0; token != NULL; i++)
    {
      if (next < IPV6_SEGMENTS)
      {
        u16 segment_value = (u16)strtol(token, NULL, 16);
        IPv6Segment[double_colon_position + segments_to_add + i] = segment_value;
        next++;
      }
      token = strtok(NULL, ":");
    }
  }

  if (flag_error == 1)
  {
    // xil_printf("\r\n error");
    // return;
  }
  else
  {
    // xil_printf("\r\n ip  ");
    for (int i = 0; i < IPV6_SEGMENTS; i++)
    {
      // xil_printf("%04x", IPv6Segment[i]);
      if (i < IPV6_SEGMENTS - 1)
      {
        // xil_printf(":");
      }
    }
  }
  IPv6Target[0] = (IPv6Segment[0] << 16) | IPv6Segment[1];
  IPv6Target[1] = (IPv6Segment[2] << 16) | IPv6Segment[3];
  IPv6Target[2] = (IPv6Segment[4] << 16) | IPv6Segment[5];
  IPv6Target[3] = (IPv6Segment[6] << 16) | IPv6Segment[7];

  // XUartLite_SendByte(0x40600000, 'I');
  ReturnMode2(1);
}
void Add_IPv6_Protect_Server()
{
  int IPv6_Protect_ADD[4] = {0};
  u8 key;
  u16 IPv6Segment[IPV6_SEGMENTS] = {0};
  u8 next = 0;
  u8 flag_error = 0;
  char input_buffer[40] = {0};
  u8 count = 0;
  int double_colon_position = -1;

  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 13)
    {
      break;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else
    {
      // xil_printf("%c", key);
      input_buffer[count++] = key;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  input_buffer[count] = '\0';

  char *token = strtok(input_buffer, ":");
  while (token != NULL && next < IPV6_SEGMENTS)
  {
    if (strcmp(token, "") == 0)
    {
      if (double_colon_position == -1)
      {
        double_colon_position = next;
        token = strtok(NULL, ":");
        continue;
      }
      else
      {
        flag_error = 1;
        break;
      }
    }
    else
    {
      if (count_digits(token) > 4)
      {
        flag_error = 1;
        break;
      }
      u16 segment_value = (u16)strtol(token, NULL, 16);
      IPv6Segment[next++] = segment_value;
      // xil_printf("\r\nSegment %d set to %04x", next - 1, segment_value);
    }
    token = strtok(NULL, ":");
    // XUartLite_SendByte(0x40600000, 'H');
  }

  if (double_colon_position != -1)
  {
    int segments_to_add = IPV6_SEGMENTS - next;
    for (int i = 0; i < segments_to_add; i++)
    {
      IPv6Segment[double_colon_position + i] = 0;
    }
    for (int i = 0; token != NULL; i++)
    {
      if (next < IPV6_SEGMENTS)
      {
        u16 segment_value = (u16)strtol(token, NULL, 16);
        IPv6Segment[double_colon_position + segments_to_add + i] = segment_value;
        next++;
      }
      token = strtok(NULL, ":");
    }
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  else
  {
    //  // xil_printf("\r\n ip  ");
    //   for (int i = 0; i < IPV6_SEGMENTS; i++)
    //   {
    //     xil_printf("%04x", IPv6Segment[i]);
    //     if (i < IPV6_SEGMENTS - 1)
    //     {
    //       xil_printf(":");
    //     }
    //   }
    //}
    IPv6_Protect_ADD[0] = (IPv6Segment[0] << 16) | IPv6Segment[1];
    IPv6_Protect_ADD[1] = (IPv6Segment[2] << 16) | IPv6Segment[3];
    IPv6_Protect_ADD[2] = (IPv6Segment[4] << 16) | IPv6Segment[5];
    IPv6_Protect_ADD[3] = (IPv6Segment[6] << 16) | IPv6Segment[7];
    delay_1ms();

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 524, IPv6_Protect_ADD[0]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 528, IPv6_Protect_ADD[1]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 532, IPv6_Protect_ADD[2]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 536, IPv6_Protect_ADD[3]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 540, IPV6_Version);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 544, 1);
    // delay_1s();
    delay_1ms();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 544, 0);
    XUartLite_SendByte(0x40600000, 'Y');
  }
}

void Delete_IPv6_Protect_Server()
{
  int IPv6_Protect_ADD[4] = {0};
  u8 key;
  u16 IPv6Segment[IPV6_SEGMENTS] = {0};
  u8 next = 0;
  u8 flag_error = 0;
  char input_buffer[40] = {0};
  u8 count = 0;
  int double_colon_position = -1;

  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 13)
    {
      break;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else
    {
      // xil_printf("%c", key);
      input_buffer[count++] = key;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  input_buffer[count] = '\0';

  char *token = strtok(input_buffer, ":");
  while (token != NULL && next < IPV6_SEGMENTS)
  {
    if (strcmp(token, "") == 0)
    {
      if (double_colon_position == -1)
      {
        double_colon_position = next;
        token = strtok(NULL, ":");
        continue;
      }
      else
      {
        flag_error = 1;
        break;
      }
    }
    else
    {
      if (count_digits(token) > 4)
      {
        flag_error = 1;
        break;
      }
      u16 segment_value = (u16)strtol(token, NULL, 16);
      IPv6Segment[next++] = segment_value;
      // xil_printf("\r\nSegment %d set to %04x", next - 1, segment_value);
    }
    token = strtok(NULL, ":");
    // XUartLite_SendByte(0x40600000, 'H');
  }

  if (double_colon_position != -1)
  {
    int segments_to_add = IPV6_SEGMENTS - next;
    for (int i = 0; i < segments_to_add; i++)
    {
      IPv6Segment[double_colon_position + i] = 0;
    }
    for (int i = 0; token != NULL; i++)
    {
      if (next < IPV6_SEGMENTS)
      {
        u16 segment_value = (u16)strtol(token, NULL, 16);
        IPv6Segment[double_colon_position + segments_to_add + i] = segment_value;
        next++;
      }
      token = strtok(NULL, ":");
    }
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  else
  {
    //  // xil_printf("\r\n ip  ");
    //   for (int i = 0; i < IPV6_SEGMENTS; i++)
    //   {
    //     xil_printf("%04x", IPv6Segment[i]);
    //     if (i < IPV6_SEGMENTS - 1)
    //     {
    //       xil_printf(":");
    //     }
    //   }
    //}
    IPv6_Protect_ADD[0] = (IPv6Segment[0] << 16) | IPv6Segment[1];
    IPv6_Protect_ADD[1] = (IPv6Segment[2] << 16) | IPv6Segment[3];
    IPv6_Protect_ADD[2] = (IPv6Segment[4] << 16) | IPv6Segment[5];
    IPv6_Protect_ADD[3] = (IPv6Segment[6] << 16) | IPv6Segment[7];
    delay_1ms();

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 524, IPv6_Protect_ADD[0]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 528, IPv6_Protect_ADD[1]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 532, IPv6_Protect_ADD[2]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 536, IPv6_Protect_ADD[3]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 540, IPV6_Version);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 544, 0x00000011);
    // delay_1s();
    delay_1ms();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 544, 0);
    XUartLite_SendByte(0x40600000, 'Y');
  }
}

// Block IPv6 Attacker
void Add_IPv6_Block_Attacker()
{
  int IPv6_Protect_ADD[4] = {0};
  u8 key;
  u16 IPv6Segment[IPV6_SEGMENTS] = {0};
  u8 next = 0;
  u8 flag_error = 0;
  char input_buffer[40] = {0};
  u8 count = 0;
  int double_colon_position = -1;

  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 13)
    {
      break;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else
    {
      // xil_printf("%c", key);
      input_buffer[count++] = key;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  input_buffer[count] = '\0';

  char *token = strtok(input_buffer, ":");
  while (token != NULL && next < IPV6_SEGMENTS)
  {
    if (strcmp(token, "") == 0)
    {
      if (double_colon_position == -1)
      {
        double_colon_position = next;
        token = strtok(NULL, ":");
        continue;
      }
      else
      {
        flag_error = 1;
        break;
      }
    }
    else
    {
      if (count_digits(token) > 4)
      {
        flag_error = 1;
        break;
      }
      u16 segment_value = (u16)strtol(token, NULL, 16);
      IPv6Segment[next++] = segment_value;
      // xil_printf("\r\nSegment %d set to %04x", next - 1, segment_value);
    }
    token = strtok(NULL, ":");
    // XUartLite_SendByte(0x40600000, 'H');
  }

  if (double_colon_position != -1)
  {
    int segments_to_add = IPV6_SEGMENTS - next;
    for (int i = 0; i < segments_to_add; i++)
    {
      IPv6Segment[double_colon_position + i] = 0;
    }
    for (int i = 0; token != NULL; i++)
    {
      if (next < IPV6_SEGMENTS)
      {
        u16 segment_value = (u16)strtol(token, NULL, 16);
        IPv6Segment[double_colon_position + segments_to_add + i] = segment_value;
        next++;
      }
      token = strtok(NULL, ":");
    }
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  else
  {
    //  // xil_printf("\r\n ip  ");
    //   for (int i = 0; i < IPV6_SEGMENTS; i++)
    //   {
    //     xil_printf("%04x", IPv6Segment[i]);
    //     if (i < IPV6_SEGMENTS - 1)
    //     {
    //       xil_printf(":");
    //     }
    //   }
    //}
    IPv6_Protect_ADD[0] = (IPv6Segment[0] << 16) | IPv6Segment[1];
    IPv6_Protect_ADD[1] = (IPv6Segment[2] << 16) | IPv6Segment[3];
    IPv6_Protect_ADD[2] = (IPv6Segment[4] << 16) | IPv6Segment[5];
    IPv6_Protect_ADD[3] = (IPv6Segment[6] << 16) | IPv6Segment[7];
    delay_1ms();

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 548, IPv6_Protect_ADD[0]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 552, IPv6_Protect_ADD[1]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 556, IPv6_Protect_ADD[2]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 560, IPv6_Protect_ADD[3]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 564, IPV6_Version);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 568, 1);
    // delay_1s();
    delay_1ms();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 568, 0);
    XUartLite_SendByte(0x40600000, 'K');
  }
}

void Delete_IPv6_Block_Attacker()
{
  int IPv6_Protect_ADD[4] = {0};
  u8 key;
  u16 IPv6Segment[IPV6_SEGMENTS] = {0};
  u8 next = 0;
  u8 flag_error = 0;
  char input_buffer[40] = {0};
  u8 count = 0;
  int double_colon_position = -1;

  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 13)
    {
      break;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else
    {
      // xil_printf("%c", key);
      input_buffer[count++] = key;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  input_buffer[count] = '\0';

  char *token = strtok(input_buffer, ":");
  while (token != NULL && next < IPV6_SEGMENTS)
  {
    if (strcmp(token, "") == 0)
    {
      if (double_colon_position == -1)
      {
        double_colon_position = next;
        token = strtok(NULL, ":");
        continue;
      }
      else
      {
        flag_error = 1;
        break;
      }
    }
    else
    {
      if (count_digits(token) > 4)
      {
        flag_error = 1;
        break;
      }
      u16 segment_value = (u16)strtol(token, NULL, 16);
      IPv6Segment[next++] = segment_value;
      // xil_printf("\r\nSegment %d set to %04x", next - 1, segment_value);
    }
    token = strtok(NULL, ":");
    // XUartLite_SendByte(0x40600000, 'H');
  }

  if (double_colon_position != -1)
  {
    int segments_to_add = IPV6_SEGMENTS - next;
    for (int i = 0; i < segments_to_add; i++)
    {
      IPv6Segment[double_colon_position + i] = 0;
    }
    for (int i = 0; token != NULL; i++)
    {
      if (next < IPV6_SEGMENTS)
      {
        u16 segment_value = (u16)strtol(token, NULL, 16);
        IPv6Segment[double_colon_position + segments_to_add + i] = segment_value;
        next++;
      }
      token = strtok(NULL, ":");
    }
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  else
  {
    //  // xil_printf("\r\n ip  ");
    //   for (int i = 0; i < IPV6_SEGMENTS; i++)
    //   {
    //     xil_printf("%04x", IPv6Segment[i]);
    //     if (i < IPV6_SEGMENTS - 1)
    //     {
    //       xil_printf(":");
    //     }
    //   }
    //}
    IPv6_Protect_ADD[0] = (IPv6Segment[0] << 16) | IPv6Segment[1];
    IPv6_Protect_ADD[1] = (IPv6Segment[2] << 16) | IPv6Segment[3];
    IPv6_Protect_ADD[2] = (IPv6Segment[4] << 16) | IPv6Segment[5];
    IPv6_Protect_ADD[3] = (IPv6Segment[6] << 16) | IPv6Segment[7];
    delay_1ms();

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 548, IPv6_Protect_ADD[0]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 552, IPv6_Protect_ADD[1]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 556, IPv6_Protect_ADD[2]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 560, IPv6_Protect_ADD[3]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 564, IPV6_Version);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 568, 0x00000011);
    // delay_1s();
    delay_1ms();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 544, 0);
    XUartLite_SendByte(0x40600000, 'K');
  }
}

int count_digits(char *str)
{
  int count = 0;
  while (*str)
  {
    if ((('0' <= *str && *str <= '9') ||
         ('a' <= *str && *str <= 'f') ||
         ('A' <= *str && *str <= 'F')))
    {
      count++;
    }
    else
    {
      break;
    }
    str++;
  }
  return count;
}
void modeOperation()
{
  ReadData();
}

void ReadData()
{
  // time_t rawtime;
  // struct tm  ts;
  char buf[80];

  u32 SynPPS, SynBPS, Conv_SynPPS, Conv_SynBPS, Conp_SynPPS;
  char *Dv_SynPPS;
  char *Dv_SynBPS;

  u32 UdpPPS, UdpBPS, Conv_UdpPPS, Conv_UdpBPS, Conp_UdpPPS;
  char *Dv_UdpPPS;
  char *Dv_UdpBPS;

  u32 IcmpPPS, IcmpBPS, Conv_IcmpPPS, Conv_IcmpBPS, Conp_IcmpPPS;
  char *Dv_IcmpPPS;
  char *Dv_IcmpBPS;

  u32 DnsPPS, DnsBPS, Conv_DnsPPS, Conv_DnsBPS, Conp_DnsPPS;
  char *Dv_DnsPPS;
  char *Dv_DnsBPS;

  u32 SynonymousPPS, SynonymousBPS, Conv_SynonymousPPS, Conv_SynonymousBPS, Conp_SynonymousPPS;
  char *Dv_SynonymousPPS;
  char *Dv_SynonymousBPS;

  u32 IPSecPPS, IPSecBPS, Conv_IPSecPPS, Conv_IPSecBPS, Conp_IPSecPPS;
  char *Dv_IPSecPPS;
  char *Dv_IPSecBPS;

  u32 TCPFragPPS, TCPFragBPS, Conv_TCPFragPPS, Conv_TCPFragBPS, Conp_TCPFragPPS;
  char *Dv_TCPFragPPS;
  char *Dv_TCPFragBPS;

  u32 UDPFragPPS, UDPFragBPS, Conv_UDPFragPPS, Conv_UDPFragBPS, Conp_UDPFragPPS;
  char *Dv_UDPFragPPS;
  char *Dv_UDPFragBPS;

  u32 Attacked;
  u32 PortNumber;
  u32 SourceIP;
  u32 DestIP;
  u32 TimeAttack;
  u32 VolumeAttack;
  u8 More_frag;
  //	u32 TotalBWD;
  //	u32 TotalPKT;
  u8 Protocol;
  char *SynRiskAttack;
  char *SynonymousRiskAttack;
  char *UdpRiskAttack;
  char *DnsRiskAttack;
  char *IcmpRiskAttack;
  char *IPSecRiskAttack;
  char *TCPFragRiskAttack;
  char *UDPFragRiskAttack;

  u8 StatusAttack;
  u8 StarusReg;
  u32 SynPPSThreshold;
  u32 UdpPPSThreshold;
  u32 DnsPPSThreshold;
  u32 IcmpPPSThreshold;
  u32 IPSecPPSThreshold;

  SynPPSThreshold = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 20);
  UdpPPSThreshold = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 28);
  DnsPPSThreshold = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 36);
  IcmpPPSThreshold = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 32);
  IPSecPPSThreshold = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 37);

  SynPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 68);
  // Convert SYN PPS
  if (SynPPS > 999999)
  {
    Conv_SynPPS = SynPPS / 1000000;
    Conp_SynPPS = (SynPPS % 1000000) / 100000;
    Dv_SynPPS = "MPPS";
  }
  else if (SynPPS <= 999999 && SynPPS > 999)
  {
    Conv_SynPPS = SynPPS / 1000;
    Conp_SynPPS = (SynPPS % 1000) / 100;
    Dv_SynPPS = "KPPS";
  }
  else if (SynPPS <= 999)
  {
    Conv_SynPPS = SynPPS;
    Conp_SynPPS = 0;
    Dv_SynPPS = "PPS";
  }

  SynBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 72);
  // Convert SYN BPS
  if (SynBPS > 1073741823)
  {
    Conv_SynBPS = SynBPS / 1073741824;
    Dv_SynBPS = "Gbit";
  }
  else if (SynBPS <= 1073741824 && SynBPS > 1048575)
  {
    Conv_SynBPS = SynBPS / 1048576;
    Dv_SynBPS = "Mbit";
  }
  else if (SynBPS <= 1048575 && SynBPS > 1023)
  {
    Conv_SynBPS = SynBPS / 1024;
    Dv_SynBPS = "Kbit";
  }
  else if (SynBPS <= 1023)
  {
    Conv_SynBPS = SynBPS;
    Dv_SynBPS = "bit";
  }

  if (SynPPS < SynPPSThreshold)
  {
    SynRiskAttack = "Low"; // low
  }
  else if (SynPPS > SynPPSThreshold && SynPPS < SynPPSThreshold * 5)
  {
    SynRiskAttack = "Medium"; // Medium
  }
  else if (SynPPS >= SynPPSThreshold * 5)
  {
    SynRiskAttack = "High"; // High
  }

  UdpPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 76);
  // Convert UDP PPS
  if (UdpPPS > 999999)
  {
    Conv_UdpPPS = UdpPPS / 1000000;
    Conp_UdpPPS = (UdpPPS % 1000000) / 100000;
    Dv_UdpPPS = "MPPS";
  }
  else if (UdpPPS <= 999999 && UdpPPS > 999)
  {
    Conv_UdpPPS = UdpPPS / 1000;
    Conp_UdpPPS = (UdpPPS % 1000) / 100;
    Dv_UdpPPS = "KPPS";
  }
  else if (UdpPPS <= 999)
  {
    Conv_UdpPPS = UdpPPS;
    Conp_UdpPPS = 0;
    Dv_UdpPPS = "PPS";
  }

  UdpBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 80);
  // Convert UDP BPS
  if (UdpBPS > 1073741823)
  {
    Conv_UdpBPS = UdpBPS / 1073741824;
    Dv_UdpBPS = "Gbit";
  }
  else if (UdpBPS <= 1073741823 && UdpBPS > 1048575)
  {
    Conv_UdpBPS = UdpBPS / 1048576;
    Dv_UdpBPS = "Mbit";
  }
  else if (UdpBPS <= 1048575 && UdpBPS > 1023)
  {
    Conv_UdpBPS = UdpBPS / 1024;
    Dv_UdpBPS = "Kbit";
  }
  else if (SynBPS <= 1023)
  {
    Conv_UdpBPS = UdpBPS;
    Dv_UdpBPS = "bit";
  }
  if (UdpPPS < UdpPPSThreshold)
  {
    UdpRiskAttack = "Low"; // low
  }
  else if (UdpPPS > UdpPPSThreshold && UdpPPS < UdpPPSThreshold * 5)
  {
    UdpRiskAttack = "Medium"; // Medium
  }
  else if (UdpPPS >= UdpPPSThreshold * 5)
  {
    UdpRiskAttack = "High"; // High
  }

  IcmpPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 84);
  // Convert ICMP PPS
  if (IcmpPPS > 999999)
  {
    Conv_IcmpPPS = IcmpPPS / 1000000;
    Conp_IcmpPPS = (IcmpPPS % 1000000) / 100000;
    Dv_IcmpPPS = "MPPS";
  }
  else if (IcmpPPS <= 999999 && IcmpPPS > 999)
  {
    Conv_IcmpPPS = IcmpPPS / 1000;
    Conp_IcmpPPS = (IcmpPPS % 1000) / 100;
    Dv_IcmpPPS = "KPPS";
  }
  else if (IcmpPPS <= 999)
  {
    Conv_IcmpPPS = IcmpPPS;
    Conp_IcmpPPS = 0;
    Dv_IcmpPPS = "PPS";
  }

  IcmpBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 88);
  // Convert ICMP BPS
  if (IcmpBPS > 1073741823)
  {
    Conv_IcmpBPS = IcmpBPS / 1073741824;
    Dv_IcmpBPS = "Gbit";
  }
  else if (IcmpBPS <= 1073741823 && IcmpBPS > 1048575)
  {
    Conv_IcmpBPS = IcmpBPS / 1048576;
    Dv_IcmpBPS = "Mbit";
  }
  else if (IcmpBPS <= 1048575 && IcmpBPS > 1023)
  {
    Conv_IcmpBPS = IcmpBPS / 1024;
    Dv_IcmpBPS = "Kbit";
  }
  else if (IcmpBPS <= 1023)
  {
    Conv_IcmpBPS = IcmpBPS;
    Dv_IcmpBPS = "bit";
  }

  if (IcmpPPS < IcmpPPSThreshold)
  {
    IcmpRiskAttack = "Low"; // low
  }
  else if (IcmpPPS > IcmpPPSThreshold && IcmpPPS < IcmpPPSThreshold * 5)
  {
    IcmpRiskAttack = "Medium"; // Medium
  }
  else if (IcmpPPS >= IcmpPPSThreshold * 5)
  {
    IcmpRiskAttack = "High"; // High
  }

  DnsPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 92);
  // Convert DNS PPS
  if (DnsPPS > 999999)
  {
    Conv_DnsPPS = DnsPPS / 1000000;
    Conp_DnsPPS = (DnsPPS % 1000000) / 100000;
    Dv_DnsPPS = "MPPS";
  }
  else if (DnsPPS <= 999999 && DnsPPS > 999)
  {
    Conv_DnsPPS = DnsPPS / 1000;
    Conp_DnsPPS = (DnsPPS % 1000) / 100;
    Dv_DnsPPS = "KPPS";
  }
  else if (DnsPPS <= 999)
  {
    Conv_DnsPPS = DnsPPS;
    Conp_DnsPPS = 0;
    Dv_DnsPPS = "PPS";
  }

  DnsBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 96);
  // Convert DNS BPS
  if (DnsBPS > 1073741823)
  {
    Conv_DnsBPS = DnsBPS / 1073741824;
    Dv_DnsBPS = "Gbit";
  }
  else if (DnsBPS <= 1073741823 && DnsBPS > 1048575)
  {
    Conv_DnsBPS = DnsBPS / 1048576;
    Dv_DnsBPS = "Mbit";
  }
  else if (DnsBPS <= 1048575 && DnsBPS > 1023)
  {
    Conv_DnsBPS = DnsBPS / 1024;
    Dv_DnsBPS = "Kbit";
  }
  else if (DnsBPS <= 1023)
  {
    Conv_DnsBPS = DnsBPS;
    Dv_DnsBPS = "bit";
  }

  if (DnsPPS < DnsPPSThreshold)
  {
    DnsRiskAttack = "Low"; // low
  }
  else if (DnsPPS > DnsPPSThreshold && DnsPPS < DnsPPSThreshold * 5)
  {
    DnsRiskAttack = "Medium"; // Medium
  }
  else if (DnsPPS >= DnsPPSThreshold * 5)
  {
    DnsRiskAttack = "High"; // High
  }

  SynonymousPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 100);
  // Convert LAND ATTACK PPS
  if (SynonymousPPS > 999999)
  {
    Conv_SynonymousPPS = SynonymousPPS / 1000000;
    Conp_SynonymousPPS = (SynonymousPPS % 1000000) / 100000;
    Dv_SynonymousPPS = "MPPS";
  }
  else if (SynonymousPPS <= 999999 && SynonymousPPS > 999)
  {
    Conv_SynonymousPPS = SynonymousPPS / 1000;
    Conp_SynonymousPPS = (SynonymousPPS % 1000) / 100;
    Dv_SynonymousPPS = "KPPS";
  }
  else if (SynonymousPPS <= 999)
  {
    Conv_SynonymousPPS = SynonymousPPS;
    Conp_SynonymousPPS = 0;
    Dv_SynonymousPPS = "PPS";
  }

  SynonymousBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 104);
  // Convert LAND ATTACK BPS
  if (SynonymousBPS > 1073741823)
  {
    Conv_SynonymousBPS = SynonymousBPS / 1073741824;
    Dv_SynonymousBPS = "Gbit";
  }
  else if (SynonymousBPS <= 1073741823 && SynonymousBPS > 1048575)
  {
    Conv_SynonymousBPS = SynonymousBPS / 1048576;
    Dv_SynonymousBPS = "Mbit";
  }
  else if (SynonymousBPS <= 1048575 && SynonymousBPS > 1023)
  {
    Conv_SynonymousBPS = SynonymousBPS / 1024;
    Dv_SynonymousBPS = "Kbit";
  }
  else if (SynonymousBPS <= 1023)
  {
    Conv_SynonymousBPS = SynonymousBPS;
    Dv_SynonymousBPS = "bit";
  }
  if (SynonymousPPS < SynPPSThreshold)
  {
    SynonymousRiskAttack = "Low"; // low
  }
  else if (SynonymousPPS > SynPPSThreshold && SynonymousPPS < SynPPSThreshold * 5)
  {
    SynonymousRiskAttack = "Medium"; // Medium
  }
  else if (SynonymousPPS >= SynPPSThreshold * 5)
  {
    SynonymousRiskAttack = "High"; // High
  }

  IPSecPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 93);
  // Convert IPSec PPS
  if (IPSecPPS > 999999)
  {
    Conv_IPSecPPS = IPSecPPS / 1000000;
    Conp_IPSecPPS = (IPSecPPS % 100000) / 100000;
    Dv_IPSecPPS = "MPPS";
  }
  else if (IPSecPPS <= 999999 && IPSecPPS > 999)
  {
    Conv_IPSecPPS = IPSecPPS / 1000;
    Conp_IPSecPPS = (IPSecPPS % 1000) / 100;
    Dv_IPSecPPS = "KPPS";
  }
  else if (IPSecPPS <= 999)
  {
    Conv_IPSecPPS = IPSecPPS;
    Conp_IPSecPPS = 0;
    Dv_IPSecPPS = "PPS";
  }

  IPSecBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 97);
  // Convert IPSec BPS
  if (IPSecBPS > 1073741823)
  {
    Conv_IPSecBPS = IPSecBPS / 1073741824;
    Dv_IPSecBPS = "Gbit";
  }
  else if (IPSecBPS <= 1073741823 && IPSecBPS > 1048575)
  {
    Conv_IPSecBPS = IPSecBPS / 1048576;
    Dv_IPSecBPS = "Mbit";
  }
  else if (IPSecBPS <= 1048575 && IPSecBPS > 1023)
  {
    Conv_IPSecBPS = IPSecBPS / 1024;
    Dv_IPSecBPS = "Kbit";
  }
  else if (IPSecBPS <= 1023)
  {
    Conv_IPSecBPS = IPSecBPS;
    Dv_IPSecBPS = "bit";
  }

  if (IPSecPPS < IPSecPPSThreshold)
  {
    IPSecRiskAttack = "Low"; // low
  }
  else if (IPSecPPS > IPSecPPSThreshold && IPSecPPS < IPSecPPSThreshold * 5)
  {
    IPSecRiskAttack = "Medium"; // Medium
  }
  else if (IPSecPPS >= IPSecPPSThreshold * 5)
  {
    IPSecRiskAttack = "High"; // High
  }

  TCPFragPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 105);
  // Convert TCP Fragment PPS
  if (TCPFragPPS > 999999)
  {
    Conv_TCPFragPPS = TCPFragPPS / 1000000;
    Conp_TCPFragPPS = (TCPFragPPS % 1000000) / 100000;
    Dv_TCPFragPPS = "MPPS";
  }
  else if (TCPFragPPS <= 999999 && TCPFragPPS > 999)
  {
    Conv_TCPFragPPS = TCPFragPPS / 1000;
    Conp_TCPFragPPS = (TCPFragPPS % 1000) / 100;
    Dv_TCPFragPPS = "KPPS";
  }
  else if (TCPFragPPS <= 999)
  {
    Conv_TCPFragPPS = TCPFragPPS;
    Conp_TCPFragPPS = 0;
    Dv_TCPFragPPS = "PPS";
  }

  TCPFragBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 106);
  // Convert TCPFragment BPS
  if (TCPFragBPS > 1073741823)
  {
    Conv_TCPFragBPS = TCPFragBPS / 1073741824;
    Dv_TCPFragBPS = "Gbit";
  }
  else if (TCPFragBPS <= 1073741823 && TCPFragBPS > 1048575)
  {
    Conv_TCPFragBPS = TCPFragBPS / 1048576;
    Dv_TCPFragBPS = "Mbit";
  }
  else if (TCPFragBPS <= 1048575 && TCPFragBPS > 1023)
  {
    Conv_TCPFragBPS = TCPFragBPS / 1024;
    Dv_TCPFragBPS = "Kbit";
  }
  else if (TCPFragBPS <= 1023)
  {
    Conv_TCPFragBPS = TCPFragBPS;
    Dv_TCPFragBPS = "bit";
  }

  if (TCPFragPPS < SynPPSThreshold)
  {
    TCPFragRiskAttack = "Low"; // low
  }
  else if (TCPFragPPS > SynPPSThreshold && TCPFragPPS < SynPPSThreshold * 5)
  {
    TCPFragRiskAttack = "Medium"; // Medium
  }
  else if (TCPFragPPS >= SynPPSThreshold * 5)
  {
    TCPFragRiskAttack = "High"; // High
  }

  UDPFragPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 109);
  // Convert UDP Fragment PPS
  if (UDPFragPPS > 999999)
  {
    Conv_UDPFragPPS = UDPFragPPS / 1000000;
    Conp_UDPFragPPS = (UDPFragPPS % 1000000) / 100000;
    Dv_UDPFragPPS = "MPPS";
  }
  else if (UDPFragPPS <= 999999 && UDPFragPPS > 999)
  {
    Conv_UDPFragPPS = UDPFragPPS / 1000;
    Conp_UDPFragPPS = (UDPFragPPS % 1000) / 100;
    Dv_UDPFragPPS = "KPPS";
  }
  else if (UDPFragPPS <= 999)
  {
    Conv_UDPFragPPS = UDPFragPPS;
    Conp_UDPFragPPS = 0;
    Dv_UDPFragPPS = "PPS";
  }

  UDPFragBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 110);
  // Convert TCPFragment BPS
  if (UDPFragBPS > 1073741823)
  {
    Conv_UDPFragBPS = UDPFragBPS / 1073741824;
    Dv_UDPFragBPS = "Gbit";
  }
  else if (UDPFragBPS <= 1073741823 && UDPFragBPS > 1048575)
  {
    Conv_UDPFragBPS = UDPFragBPS / 1048576;
    Dv_UDPFragBPS = "Mbit";
  }
  else if (UDPFragBPS <= 1048575 && UDPFragBPS > 1023)
  {
    Conv_UDPFragBPS = UDPFragBPS / 1024;
    Dv_UDPFragBPS = "Kbit";
  }
  else if (UDPFragBPS <= 1023)
  {
    Conv_UDPFragBPS = UDPFragBPS;
    Dv_UDPFragBPS = "bit";
  }

  if (UDPFragPPS < UdpPPSThreshold)
  {
    UDPFragRiskAttack = "Low"; // low
  }
  else if (UDPFragPPS > UdpPPSThreshold && UDPFragPPS < UdpPPSThreshold * 5)
  {
    UDPFragRiskAttack = "Medium"; // Medium
  }
  else if (UDPFragPPS >= UdpPPSThreshold * 5)
  {
    UDPFragRiskAttack = "High"; // High
  }

  Attacked = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 44);
  StarusReg = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 108);

  //	TotalBWD = SynBPS + UdpBPS + IcmpBPS + DnsBPS + SynonymousBPS + IPSecBPS + TCPFragBPS + UDPFragBPS;
  //	TotalPKT = SynPPS + UdpPPS + IcmpPPS + DnsPPS + SynonymousPPS + IPSecPPS + TCPFragPPS + UDPFragPPS;

  /*if(StarusReg != 0)*/ {
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 1, 1);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 1, 0);
    PortNumber = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 48);
    SourceIP = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 52);
    DestIP = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 56);
    TimeAttack = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 60);
    VolumeAttack = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 124);
    Protocol = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 128);
    More_frag = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 61);
    //  rawtime      = TimeAttack;
    // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
    //  ts = *localtime(&rawtime);
    //  strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %p", &ts);
    // xil_printf("\r\n*****************************************************************************************************************************************************************************************************");
    // xil_printf("\r\n* +===============================================================================================================================================================================================+ *");
    // xil_printf("\r\n* | =====>{ Press Ctrl+C => Device configuration }                                                                                                                                                | *");
    // xil_printf("\r\n* | =====> INFORMATION ATTACKER:                                                                                                                                                                  | *");
    // xil_printf("\r\n* +----------------------------+-------------------+-------------------+------------+------------+----------+-------------+---------------------+--------+--------+-------------------------------| *");
    // xil_printf("\r\n* |        Time attack         |    Src IP addr    |    Dst IP addr    |  Src port  |  Dst port  | Protocol |    Status   |     Type attack     | Volume |  Risk  |              Action           | *");
    // xil_printf("\r\n* +----------------------------+-------------------+-------------------+------------+------------+----------+-------------+---------------------+--------+--------+-------------------------------| *");
    // xil_printf("\r\n* | %26s |  %-3d.%-3d.%-3d.%-3d   | %-3d.%-3d.%-3d.%-3d  |    %5d   |   %5d    ", buf,(u8)(SourceIP>>24), (u8)(SourceIP>>16), (u8)(SourceIP>>8), (u8)(SourceIP), (u8)(DestIP>>24),
    //(u8)(DestIP>>16), (u8)(DestIP>>8), (u8)(DestIP), (u16)(PortNumber>>16),(u16)(PortNumber));

    if (Protocol == 0x06 && More_frag == 1)
    {
      StatusAttack = (StarusReg >> 1) % 2;
      VolumeTCPFrag = VolumeTCPFrag + VolumeAttack;
      xil_printf("|   %4s   |  %8s   |  %17s  |   %1d    | %6s ", "TCP", StatusAttack ? "Ongoing" : "Attacked", "TCP Frag Flood", VolumeAttack, TCPFragRiskAttack);
    }
    else if (Protocol == 0x11 && More_frag == 1)
    {
      StatusAttack = StarusReg % 2;
      VolumeUDPFrag = VolumeUDPFrag + VolumeAttack;
      xil_printf("|   %4s   |  %8s   |  %17s  |   %1d    | %6s ", "UDP", StatusAttack ? "Ongoing" : "Attacked", "UDP Frag Flood", VolumeAttack, UDPFragRiskAttack);
    }
    else if (Protocol == 0x06 && SourceIP != DestIP)
    {
      StatusAttack = (StarusReg >> 7) % 2;
      VolumeSYN = VolumeSYN + VolumeAttack;
      xil_printf("|   %4s   |  %8s   |  %17s  |   %1d    | %6s ", "TCP", StatusAttack ? "Ongoing" : "Attacked", "SYN-Flood", VolumeAttack, SynRiskAttack);
    }
    else if (Protocol == 0x06 && SourceIP == DestIP)
    {
      StatusAttack = (StarusReg >> 6) % 2;
      VolumeSYNON = VolumeSYNON + VolumeAttack;
      xil_printf("|   %4s   |  %8s   |  %17s  |   %1d    | %6s ", "TCP", StatusAttack ? "Ongoing" : "Attacked", "LAND-Attack", VolumeAttack, SynonymousRiskAttack);
    }
    else if (Protocol == 0x11 && ((u8)(PortNumber >> 16) != 53) && ((u16)(PortNumber >> 16) != 500) && ((u16)(PortNumber >> 16) != 4500))
    {
      StatusAttack = (StarusReg >> 5) % 2;
      VolumeUDP = VolumeUDP + VolumeAttack;
      xil_printf("|   %4s   |  %8s   |  %17s  |   %1d    | %6s ", "UDP", StatusAttack ? "Ongoing" : "Attacked", "UDP-Flood", VolumeAttack, UdpRiskAttack);
    }
    else if (Protocol == 0x11 && (((u16)(PortNumber >> 16) == 500) || ((u16)(PortNumber >> 16) == 4500)))
    {
      StatusAttack = (StarusReg >> 2) % 2;
      VolumeIPSEC = VolumeIPSEC + VolumeAttack;
      xil_printf("|   %4s   |  %8s   |  %17s  |   %1d    | %6s ", "UDP", StatusAttack ? "Ongoing" : "Attacked", "IPSec-Flood", VolumeAttack, IPSecRiskAttack);
    }
    else if (Protocol == 0x11 && ((u16)(PortNumber >> 16) == 53))
    {
      StatusAttack = (StarusReg >> 4) % 2;
      VolumeDNS = VolumeDNS + VolumeAttack;
      xil_printf("|   %4s   |  %8s   |  %17s  |   %1d    | %6s ", "UDP", StatusAttack ? "Ongoing" : "Attacked", "DNS-Amplification", VolumeAttack, DnsRiskAttack);
    }
    else if (Protocol == 0x01)
    {
      StatusAttack = (StarusReg >> 3) % 2;
      VolumeICMP = VolumeICMP + VolumeAttack;
      xil_printf("|   %4s   |  %8s   |  %17s  |   %1d    | %6s ", "ICMP", StatusAttack ? "Ongoing" : "Attacked", "IMCP-Flood", VolumeAttack, IcmpRiskAttack);
    }
    else
    {
      VolumeSYN = VolumeSYN;
      VolumeSYNON = VolumeSYNON;
      VolumeUDP = VolumeUDP;
      VolumeDNS = VolumeDNS;
      VolumeICMP = VolumeICMP;
      VolumeIPSEC = VolumeIPSEC;
      VolumeTCPFrag = VolumeTCPFrag;
      VolumeUDPFrag = VolumeUDPFrag;
    }
    //  xil_printf("| %4s                          | *", "Drop");
    //  xil_printf("\r\n* +============================+===================+===================+============+============+==========+=============+=====================+========+========+===============================| *");
    //  xil_printf("\r\n* +===============================================================================================================================================================================================+ *");
    //  xil_printf("\r\n* | =====> BANDWIDTH ATTACK:                                                                                                                                                                      | *");
    //  xil_printf("\r\n* +-----------------------+-----------------------+-----------------------+-----------------------+-----------------------+-----------------------+-----------------------+-----------------------+ *");
    //  xil_printf("\r\n* |       SYN flood       |      LAND attack      |       UDP flood       |   DNS amplification   |      ICMP flood       |    IPSec IKE flood    |  TCP fragmentation    |   UDP fragmentation   | *");
    //  xil_printf("\r\n* +------------+----------+------------+----------+------------+----------+------------+----------+------------+----------+------------+----------+------------+----------+------------+----------+ *");
    //  xil_printf("\r\n* | %3d.%1d %4s | %3d %4s | %3d.%1d %4s | %3d %4s | %3d.%1d %4s | %3d %4s | %3d.%1d %4s | %3d %4s | %3d.%1d %4s | %3d %4s | %3d.%1d %4s | %3d %4s | %3d.%1d %4s | %3d %4s | %3d.%1d %4s | %3d %4s | *",
    //  Conv_SynPPS,Conp_SynPPS, Dv_SynPPS,Conv_SynBPS, Dv_SynBPS,
    //  Conv_SynonymousPPS,Conp_SynonymousPPS, Dv_SynonymousPPS, Conv_SynonymousBPS, Dv_SynonymousBPS,
    //  Conv_UdpPPS ,Conp_UdpPPS, Dv_UdpPPS, Conv_UdpBPS, Dv_UdpBPS,
    //  Conv_DnsPPS, Conp_DnsPPS, Dv_DnsPPS, Conv_DnsBPS, Dv_DnsBPS,
    //  Conv_IcmpPPS, Conp_IcmpPPS, Dv_IcmpPPS, Conv_IcmpBPS, Dv_IcmpBPS,
    //  Conv_IPSecPPS,Conp_IPSecPPS, Dv_IPSecPPS, Conv_IPSecBPS, Dv_IPSecBPS,
    //  Conv_TCPFragPPS, Conp_TCPFragPPS, Dv_TCPFragPPS, Conv_TCPFragBPS, Dv_TCPFragBPS,
    //  Conv_UDPFragPPS , Conp_UDPFragPPS, Dv_UDPFragPPS, Conv_UDPFragBPS, Dv_UDPFragBPS);
    //  xil_printf("\r\n* +============+==========+============+==========+============+==========+============+==========+============+==========+============+==========+============+==========+============+==========+ *");
    //  xil_printf("\r\n*****************************************************************************************************************************************************************************************************");
    //  xil_printf("\r\n*                                                                                                                                                                                                   *");
    delay_ter(1000000);
    ////xil_printf("%c[18A", 30);
  }
}

void Port_mode(int port_value)
{
  u8 key = 0;
  u8 done = 0;

  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);

    // key = 1 bao ve port
    if (key == '1')
    {

      switch (port_value)
      {
      case 1:
        DefenderPort_en_1 = 0;
        break;
      case 2:
        DefenderPort_en_2 = 0;
        break;
      case 3:
        DefenderPort_en_3 = 0;
        break;
      case 4:
        DefenderPort_en_4 = 0;
        break;
      case 5:
        DefenderPort_en_5 = 0;
        break;
      case 6:
        DefenderPort_en_6 = 0;
        break;
      case 7:
        DefenderPort_en_7 = 0;
        break;
      case 8:
        DefenderPort_en_8 = 0;
        break;
      default:
        break;
      }
      done = 1;
    }
    // key = 0 bao ve IP
    else if (key == '0')
    {
      switch (port_value)
      {
      case 1:
        DefenderPort_en_1 = 1;
        break;
      case 2:
        DefenderPort_en_2 = 1;
        break;
      case 3:
        DefenderPort_en_3 = 1;
        break;
      case 4:
        DefenderPort_en_4 = 1;
        break;
      case 5:
        DefenderPort_en_5 = 1;
        break;
      case 6:
        DefenderPort_en_6 = 1;
        break;
      case 7:
        DefenderPort_en_7 = 1;
        break;
      case 8:
        DefenderPort_en_8 = 1;
        break;
      default:
        break;
      }
      done = 1;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else if (key == 13 && done == 1)
    {
      break;
    }
  }
  switch (port_value)
  {
  case 1:
    EnableDefender1 = (HTTPSGETFlood_en1 << 10) + (HTTPGETFlood_en1 << 9) + (UDPFragFlood_en1 << 8) + (TCPFragFlood_en1 << 7) + (IPSecFlood_en1 << 6) + (ICMPFlood_en1 << 5) + (DNSFlood_en1 << 4) + (UDPFlood_en1 << 3) + (LANDATTACK_en1 << 2) + (SYNFlood_en1 << 1) + DefenderPort_en_1;
    break;
  case 2:
    EnableDefender2 = (HTTPSGETFlood_en2 << 10) + (HTTPGETFlood_en2 << 9) + (UDPFragFlood_en2 << 8) + (TCPFragFlood_en2 << 7) + (IPSecFlood_en2 << 6) + (ICMPFlood_en2 << 5) + (DNSFlood_en2 << 4) + (UDPFlood_en2 << 3) + (LANDATTACK_en2 << 2) + (SYNFlood_en2 << 1) + DefenderPort_en_2;
    break;
  case 3:
    EnableDefender3 = (HTTPSGETFlood_en3 << 10) + (HTTPGETFlood_en3 << 9) + (UDPFragFlood_en3 << 8) + (TCPFragFlood_en3 << 7) + (IPSecFlood_en3 << 6) + (ICMPFlood_en3 << 5) + (DNSFlood_en3 << 4) + (UDPFlood_en3 << 3) + (LANDATTACK_en3 << 2) + (SYNFlood_en3 << 1) + DefenderPort_en_3;
    break;
  case 4:
    EnableDefender4 = (HTTPSGETFlood_en4 << 10) + (HTTPGETFlood_en4 << 9) + (UDPFragFlood_en4 << 8) + (TCPFragFlood_en4 << 7) + (IPSecFlood_en4 << 6) + (ICMPFlood_en4 << 5) + (DNSFlood_en4 << 4) + (UDPFlood_en4 << 3) + (LANDATTACK_en4 << 2) + (SYNFlood_en4 << 1) + DefenderPort_en_4;
    break;
  case 5:
    EnableDefender5 = (HTTPSGETFlood_en5 << 10) + (HTTPGETFlood_en5 << 9) + (UDPFragFlood_en5 << 8) + (TCPFragFlood_en5 << 7) + (IPSecFlood_en5 << 6) + (ICMPFlood_en5 << 5) + (DNSFlood_en5 << 4) + (UDPFlood_en5 << 3) + (LANDATTACK_en5 << 2) + (SYNFlood_en5 << 1) + DefenderPort_en_5;
    break;
  case 6:
    EnableDefender6 = (HTTPSGETFlood_en6 << 10) + (HTTPGETFlood_en6 << 9) + (UDPFragFlood_en6 << 8) + (TCPFragFlood_en6 << 7) + (IPSecFlood_en6 << 6) + (ICMPFlood_en6 << 5) + (DNSFlood_en6 << 4) + (UDPFlood_en6 << 3) + (LANDATTACK_en6 << 2) + (SYNFlood_en6 << 1) + DefenderPort_en_6;
    break;
  case 7:
    EnableDefender7 = (HTTPSGETFlood_en7 << 10) + (HTTPGETFlood_en7 << 9) + (UDPFragFlood_en7 << 8) + (TCPFragFlood_en7 << 7) + (IPSecFlood_en7 << 6) + (ICMPFlood_en7 << 5) + (DNSFlood_en7 << 4) + (UDPFlood_en7 << 3) + (LANDATTACK_en7 << 2) + (SYNFlood_en7 << 1) + DefenderPort_en_7;
    break;
  case 8:
    EnableDefender8 = (HTTPSGETFlood_en8 << 10) + (HTTPGETFlood_en8 << 9) + (UDPFragFlood_en8 << 8) + (TCPFragFlood_en8 << 7) + (IPSecFlood_en8 << 6) + (ICMPFlood_en8 << 5) + (DNSFlood_en8 << 4) + (UDPFlood_en8 << 3) + (LANDATTACK_en8 << 2) + (SYNFlood_en8 << 1) + DefenderPort_en_8;
    break;
  default:
    break;
  }
  ReturnMode2(key - '0');
}
//
void Add_or_Remove_IPv4_into_port(const char *ip_str, int part_ip1_addr, int part_ip2_addr, int part_ip3_addr, int part_ip4_addr, int ver_ip_addr, int signal_en_addr, int ver_ip_val, int signal_en)
{
  u16 IP_atk_layerA = 0;
  u16 IP_atk_layerB = 0;
  u16 IP_atk_layerC = 0;
  u16 IP_atk_layerD = 0;
  u8 flag_error = 0;
  if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IP_atk_layerA, &IP_atk_layerB, &IP_atk_layerC, &IP_atk_layerD) != 4)
  {
    flag_error = 1;
  }
  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
    return;
  }

  IPAttacker = IP_atk_layerA * 16777216 + IP_atk_layerB * 65536 + IP_atk_layerC * 256 + IP_atk_layerD;
  delay_1ms();
  //
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + part_ip1_addr, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + part_ip2_addr, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + part_ip3_addr, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + part_ip4_addr, IPAttacker);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + ver_ip_addr, ver_ip_val);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + signal_en_addr, signal_en);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + signal_en_addr, 0x00000000);
  //

  // Ghi them vao white list Cho https
  delay_1ms();
  //
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 576, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 580, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 584, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 588, IPAttacker);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 592, 0x00000004);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 572, 0x00000001);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 572, 0x00000000);
  //

  delay_1ms();
  XUartLite_SendByte(0x40600000, 'Y');
}

void Add_IPv6_into_port(int port_value, int dst_or_src)
{
  int ipv6_1 = 0;
  int ipv6_2 = 0;
  int ipv6_3 = 0;
  int ipv6_4 = 0;
  Cacul_IPv6_into_port(&ipv6_1, &ipv6_2, &ipv6_3, &ipv6_4);

  if (dst_or_src)
  {
    switch (port_value)
    {
    case 1:
      Write_IPv6_into_port(524, 528, 532, 536, ipv6_1, ipv6_2, ipv6_3, ipv6_4, 540, 544, 6, 1);
      break;
    case 2:

      break;
    case 3:
      Write_IPv6_into_port(632, 636, 640, 644, ipv6_1, ipv6_2, ipv6_3, ipv6_4, 540, 544, 96, 256);
      break;
    case 4:
      Write_IPv6_into_port(648, 652, 656, 660, ipv6_1, ipv6_2, ipv6_3, ipv6_4, 540, 544, 1536, 65536);
      break;

    case 5:
      break;
    case 6:
      break;
    case 7:
      break;
    default:
      break;
    }
  }
  // Them IP src
  else
  {
    switch (port_value)
    {

    case 1:
      Write_IPv6_into_port(548, 552, 556, 560, ipv6_1, ipv6_2, ipv6_3, ipv6_4, 564, 568, 22, 1);
      break;
    case 2:
      break;
    case 3:
      Write_IPv6_into_port(548, 552, 556, 560, ipv6_1, ipv6_2, ipv6_3, ipv6_4, 564, 568, 134, 1);
      break;
    case 4:
      Write_IPv6_into_port(548, 552, 556, 560, ipv6_1, ipv6_2, ipv6_3, ipv6_4, 564, 568, 262, 1);
      break;
    case 5:
      break;
    case 6:
      break;
    case 7:
      break;
    default:
      break;
    }
  }
}
void Remove_IPv6_into_port(int port_value, int dst_or_src)
{
  int ipv6_1 = 0;
  int ipv6_2 = 0;
  int ipv6_3 = 0;
  int ipv6_4 = 0;
  Cacul_IPv6_into_port(&ipv6_1, &ipv6_2, &ipv6_3, &ipv6_4);

  if (dst_or_src)
  {
    switch (port_value)
    {
    case 1:
      Write_IPv6_into_port(524, 528, 532, 536, ipv6_1, ipv6_2, ipv6_3, ipv6_4, 540, 544, 6, 17);
      break;
    case 2:

      break;
    case 3:
      Write_IPv6_into_port(632, 636, 640, 644, ipv6_1, ipv6_2, ipv6_3, ipv6_4, 540, 544, 96, 4352);
      break;
    case 4:
      Write_IPv6_into_port(648, 652, 656, 660, ipv6_1, ipv6_2, ipv6_3, ipv6_4, 540, 544, 1536, 1114112);
      break;

    case 5:
      break;
    case 6:
      break;
    case 7:
      break;
    default:
      break;
    }
  }
  // xoa IP src
  else
  {
    switch (port_value)
    {

    case 1:
      Write_IPv6_into_port(548, 552, 556, 560, ipv6_1, ipv6_2, ipv6_3, ipv6_4, 564, 568, 22, 17);
      break;
    case 2:
      break;
    case 3:
      Write_IPv6_into_port(548, 552, 556, 560, ipv6_1, ipv6_2, ipv6_3, ipv6_4, 564, 568, 134, 17);
      break;
    case 4:
      Write_IPv6_into_port(548, 552, 556, 560, ipv6_1, ipv6_2, ipv6_3, ipv6_4, 564, 568, 262, 17);
      break;
    case 5:
      break;
    case 6:
      break;
    case 7:
      break;
    default:
      break;
    }
  }
}
void Write_IPv6_into_port(int IPv6_Protect_ADD_0_addr, int IPv6_Protect_ADD_1_addr, int IPv6_Protect_ADD_2_addr, int IPv6_Protect_ADD_3_addr, int IPv6_Protect_ADD_0, int IPv6_Protect_ADD_1, int IPv6_Protect_ADD_2, int IPv6_Protect_ADD_3, int ip_ver_addr, int signal_en_addr, int ver_ip_val, int signal_en)
{

  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + IPv6_Protect_ADD_0_addr, IPv6_Protect_ADD_0);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + IPv6_Protect_ADD_1_addr, IPv6_Protect_ADD_1);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + IPv6_Protect_ADD_2_addr, IPv6_Protect_ADD_2);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + IPv6_Protect_ADD_3_addr, IPv6_Protect_ADD_3);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + ip_ver_addr, ver_ip_val);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + signal_en_addr, signal_en);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + signal_en_addr, 0x00000000);
  delay_1ms();

  XUartLite_SendByte(0x40600000, 'Y');
}
void Write_IPv6_into_port_mirroring(int IPv6_Protect_ADD_0_addr, int IPv6_Protect_ADD_1_addr, int IPv6_Protect_ADD_2_addr, int IPv6_Protect_ADD_3_addr, int IPv6_Protect_ADD_0, int IPv6_Protect_ADD_1, int IPv6_Protect_ADD_2, int IPv6_Protect_ADD_3)
{

  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + IPv6_Protect_ADD_0_addr, IPv6_Protect_ADD_0);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + IPv6_Protect_ADD_1_addr, IPv6_Protect_ADD_1);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + IPv6_Protect_ADD_2_addr, IPv6_Protect_ADD_2);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + IPv6_Protect_ADD_3_addr, IPv6_Protect_ADD_3);
  delay_1ms();
  //
  // XUartLite_SendByte(0x40600000, 'Y');
}
void Cacul_IPv6_into_port(int *ipv6_1, int *ipv6_2, int *ipv6_3, int *ipv6_4)
{
  int IPv6_Protect_ADD[4] = {0};
  u8 key;
  u16 IPv6Segment[IPV6_SEGMENTS] = {0};
  u8 next = 0;
  u8 flag_error = 0;
  char input_buffer[40] = {0};
  u8 count = 0;
  int double_colon_position = -1;
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 13)
    {
      break;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else
    {
      // xil_printf("%c", key);
      input_buffer[count++] = key;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  input_buffer[count] = '\0';

  char *token = strtok(input_buffer, ":");
  while (token != NULL && next < IPV6_SEGMENTS)
  {
    if (strcmp(token, "") == 0)
    {
      if (double_colon_position == -1)
      {
        double_colon_position = next;
        token = strtok(NULL, ":");
        continue;
      }
      else
      {
        flag_error = 1;
        break;
      }
    }
    else
    {
      if (count_digits(token) > 4)
      {
        flag_error = 1;
        break;
      }
      u16 segment_value = (u16)strtol(token, NULL, 16);
      IPv6Segment[next++] = segment_value;
      // xil_printf("\r\nSegment %d set to %04x", next - 1, segment_value);
    }
    token = strtok(NULL, ":");
    // XUartLite_SendByte(0x40600000, 'H');
  }

  if (double_colon_position != -1)
  {
    int segments_to_add = IPV6_SEGMENTS - next;
    for (int i = 0; i < segments_to_add; i++)
    {
      IPv6Segment[double_colon_position + i] = 0;
    }
    for (int i = 0; token != NULL; i++)
    {
      if (next < IPV6_SEGMENTS)
      {
        u16 segment_value = (u16)strtol(token, NULL, 16);
        IPv6Segment[double_colon_position + segments_to_add + i] = segment_value;
        next++;
      }
      token = strtok(NULL, ":");
    }
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  else
  {

    IPv6_Protect_ADD[0] = (IPv6Segment[0] << 16) | IPv6Segment[1];
    IPv6_Protect_ADD[1] = (IPv6Segment[2] << 16) | IPv6Segment[3];
    IPv6_Protect_ADD[2] = (IPv6Segment[4] << 16) | IPv6Segment[5];
    IPv6_Protect_ADD[3] = (IPv6Segment[6] << 16) | IPv6Segment[7];
    delay_1ms();
    *ipv6_1 = IPv6_Protect_ADD[0];
    *ipv6_2 = IPv6_Protect_ADD[1];
    *ipv6_3 = IPv6_Protect_ADD[2];
    *ipv6_4 = IPv6_Protect_ADD[3];
  }
}
// //**=========== */

void Add_IPv4_into_port(int port_value, int dst_or_src)
{
  char buffer_IPV4[BUFFER_SIZE_IPV4];
  ReceiveIPv4String(buffer_IPV4, BUFFER_SIZE_IPV4);
  // If them iP dst
  if (dst_or_src)
  {
    switch (port_value)
    {
    case 1:
      Add_or_Remove_IPv4_into_port(buffer_IPV4, 524, 528, 532, 536, 540, 544, 4, 1);
      break;

    case 3:
      Add_or_Remove_IPv4_into_port(buffer_IPV4, 632, 636, 640, 644, 540, 544, 64, 256);
      break;
    case 4:
      Add_or_Remove_IPv4_into_port(buffer_IPV4, 648, 652, 656, 660, 540, 544, 1024, 65536);
      break;
    case 5:
      break;
    case 6:
      break;
    case 7:
      break;
    default:
      break;
    }
  }
  // Them IP src
  if (!dst_or_src)

  {
    switch (port_value)
    {

    case 1:
      Add_or_Remove_IPv4_into_port(buffer_IPV4, 548, 552, 556, 560, 564, 568, 20, 1);
      break;
    case 2:
      break;
    case 3:
      Add_or_Remove_IPv4_into_port(buffer_IPV4, 548, 552, 556, 560, 564, 568, 132, 1);
      break;
    case 4:
      Add_or_Remove_IPv4_into_port(buffer_IPV4, 548, 552, 556, 560, 564, 568, 260, 1);
      break;
    case 5:
      break;
    case 6:
      break;
    case 7:
      break;
    default:
      break;
    }
  }
}

void Remove_IPv4_into_port(int port_value, int dst_or_src)
{
  char buffer_IPV4[BUFFER_SIZE_IPV4];
  ReceiveIPv4String(buffer_IPV4, BUFFER_SIZE_IPV4);
  // If xoa iP dst
  if (dst_or_src)
  {
    switch (port_value)
    {
    case 1:
      Add_or_Remove_IPv4_into_port(buffer_IPV4, 524, 528, 532, 536, 540, 544, 4, 17);
      break;
    case 2:

      break;
    case 3:
      Add_or_Remove_IPv4_into_port(buffer_IPV4, 632, 636, 640, 644, 540, 544, 64, 4352);
      break;
    case 4:
      Add_or_Remove_IPv4_into_port(buffer_IPV4, 648, 652, 656, 660, 540, 544, 1024, 1114112);
      break;

    case 5:
      break;
    case 6:
      break;
    case 7:
      break;
    default:
      break;
    }
  }
  // xoa IP src
  if (!dst_or_src)

  {
    switch (port_value)
    {

    case 1:
      Add_or_Remove_IPv4_into_port(buffer_IPV4, 548, 552, 556, 560, 564, 568, 20, 17);
      break;
    case 2:
      break;
    case 3:
      Add_or_Remove_IPv4_into_port(buffer_IPV4, 548, 552, 556, 560, 564, 568, 132, 17);
      break;
    case 4:
      Add_or_Remove_IPv4_into_port(buffer_IPV4, 548, 552, 556, 560, 564, 568, 260, 17);
      break;
    case 5:
      break;
    case 6:
      break;
    case 7:
      break;
    default:
      break;
    }
  }
}
// void Delete_IPv4_into_port(int port_value)
// {
// }
// void Cacul_IPv6_into_port(int port_value)
// {
// }
// void Delete_IPv6_into_port(int port_value)
// {
// }

// Port mirroring

void Port_mirroring()
{
  u8 key = 0;
  u8 done = 0;
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);

    if (key == 'A')
    {
      Port_mirroring_src_mac(1); // 1 la them
      break;
    }
    if (key == '1')
    {
      Port_mirroring_src_mac(0); // 0 la xoa
      break;
    }
    else if (key == 'B')
    {
      Port_mirroring_dst_mac(1);
      break;
    }
    else if (key == '2')
    {
      Port_mirroring_dst_mac(0); //
      break;
    }
    else if (key == 'C')
    {
      Port_mirroring_src_ipv4(1);
      break;
    }
    else if (key == '3')
    {
      Port_mirroring_src_ipv4(0);
      break;
    }
    else if (key == 'D')
    {
      Port_mirroring_dst_ipv4(1);
      break;
    }
    else if (key == '4')
    {
      Port_mirroring_dst_ipv4(0);
      break;
    }

    else if (key == 'E') // them
    {
      Port_mirroring_src_dst_ipv6(0, 1);
      break;
    }
    else if (key == 'L') // xoa
    {
      Port_mirroring_src_dst_ipv6(0, 0);
      break;
    }
    else if (key == 'F') // them
    {
      Port_mirroring_src_dst_ipv6(1, 1);
      break;
    }
    else if (key == 'M') // xoa
    {
      Port_mirroring_src_dst_ipv6(1, 0);
      break;
    }
    //
    else if (key == 'G')
    {
      Port_mirroring_src_dst_port(1); // them
      break;
    }
    else if (key == '7')
    {
      Port_mirroring_src_dst_port(0); // xoa
      break;
    }
    else if (key == 'H')
    {
      // Port_mirroring_src_dst_port();
      break;
    }
    else if (key == 'I')
    {
      Port_mirroring_protocol(1); // them
      break;
    }
    else if (key == '8')
    {
      Port_mirroring_protocol(0); // xoa
      break;
    }
    else if (key == 'K')
    {
      Port_mirroring_mode(1); // them
      break;
    }
    else if (key == '9')
    {
      Port_mirroring_mode(0); // xoa
      break;
    }
    else if (key == 'X')
    {
      Choose_port_monitored();
      break;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
  XUartLite_SendByte(0x40600000, 'Y');
}

// Port mirroring src mac
void Port_mirroring_src_mac(int add_or_remove)
{
  u8 key = 0;
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);

    if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8')
    {
      break;
    }
  }

  int SRC_MAC[2] = {0};
  u8 key1 = 0;
  u16 SRC_MACSegment[MAC_SEGMENTS] = {0};
  u8 next = 0;
  u8 flag_error = 0;
  char input_buffer[19] = {0};
  u8 count = 0;
  int double_colon_position = -1;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == 13)
    {
      break;
    }
    else if (key1 == 03)
    {
      Reset_System();
    }
    else
    {
      // xil_printf("%c", key);
      input_buffer[count++] = key1;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  input_buffer[count] = '\0';
  char *token = strtok(input_buffer, ":");
  while (token != NULL && next < MAC_SEGMENTS)
  {
    if (strcmp(token, "") == 0)
    {
      if (double_colon_position == -1)
      {
        double_colon_position = next;
        token = strtok(NULL, ":");
        continue;
      }
      else
      {
        flag_error = 1;
        break;
      }
    }
    else
    {
      if (count_digits(token) > 2)
      {
        flag_error = 1;
        break;
      }
      u16 segment_value = (u16)strtol(token, NULL, 16);
      SRC_MACSegment[next++] = segment_value;
      // xil_printf("\r\nSegment %d set to %04x", next - 1, segment_value);
    }
    token = strtok(NULL, ":");
    // XUartLite_SendByte(0x40600000, 'H');
  }
  if (double_colon_position != -1)
  {
    int segments_to_add = MAC_SEGMENTS - next;
    for (int i = 0; i < segments_to_add; i++)
    {
      SRC_MACSegment[double_colon_position + i] = 0;
    }
    for (int i = 0; token != NULL; i++)
    {
      if (next < MAC_SEGMENTS)
      {
        u16 segment_value = (u16)strtol(token, NULL, 16);
        SRC_MACSegment[double_colon_position + segments_to_add + i] = segment_value;
        next++;
      }
      token = strtok(NULL, ":");
    }
  }
  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  else
  {
    if (add_or_remove)
    {
      SRC_MAC[0] = (SRC_MACSegment[0] << 24) | SRC_MACSegment[1] << 16 | (SRC_MACSegment[2] << 8) | SRC_MACSegment[3];
      SRC_MAC[1] = (SRC_MACSegment[4] << 8) | SRC_MACSegment[5];
    }
    else
    {
      SRC_MAC[0] = 0x00000000;
      SRC_MAC[1] = 0x0000;
    }
    delay_1ms();
    switch (key - '0')
    {
    case 1:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 684, SRC_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 688, SRC_MAC[1]);
      break;
    case 2:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 684, SRC_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 688, SRC_MAC[1]);
      break;
    case 3:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 684, SRC_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 688, SRC_MAC[1]);
      break;
    case 4:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 684, SRC_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 688, SRC_MAC[1]);
      break;
    case 5:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 684, SRC_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 688, SRC_MAC[1]);
      break;
    case 6:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 684, SRC_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 688, SRC_MAC[1]);
      break;
    case 7:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 684, SRC_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 688, SRC_MAC[1]);
      break;
    case 8:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 684, SRC_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 688, SRC_MAC[1]);
      break;
      //
    }
  }
}

// Port mirroring dst mac
void Port_mirroring_dst_mac(int add_or_remove)
{
  u8 key = 0;
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);

    if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8')
    {
      break;
    }
  }

  int DST_MAC[2] = {0};
  u8 key1 = 0;
  u16 DST_MACSegment[MAC_SEGMENTS] = {0};
  u8 next = 0;
  u8 flag_error = 0;
  char input_buffer[19] = {0};
  u8 count = 0;
  int double_colon_position = -1;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == 13)
    {
      break;
    }
    else if (key1 == 03)
    {
      Reset_System();
    }
    else
    {
      // xil_printf("%c", key);
      input_buffer[count++] = key1;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  input_buffer[count] = '\0';
  char *token = strtok(input_buffer, ":");
  while (token != NULL && next < MAC_SEGMENTS)
  {
    if (strcmp(token, "") == 0)
    {
      if (double_colon_position == -1)
      {
        double_colon_position = next;
        token = strtok(NULL, ":");
        continue;
      }
      else
      {
        flag_error = 1;
        break;
      }
    }
    else
    {
      if (count_digits(token) > 2)
      {
        flag_error = 1;
        break;
      }
      u16 segment_value = (u16)strtol(token, NULL, 16);
      DST_MACSegment[next++] = segment_value;
      // xil_printf("\r\nSegment %d set to %04x", next - 1, segment_value);
    }
    token = strtok(NULL, ":");
    // XUartLite_SendByte(0x40600000, 'H');
  }
  if (double_colon_position != -1)
  {
    int segments_to_add = MAC_SEGMENTS - next;
    for (int i = 0; i < segments_to_add; i++)
    {
      DST_MACSegment[double_colon_position + i] = 0;
    }
    for (int i = 0; token != NULL; i++)
    {
      if (next < MAC_SEGMENTS)
      {
        u16 segment_value = (u16)strtol(token, NULL, 16);
        DST_MACSegment[double_colon_position + segments_to_add + i] = segment_value;
        next++;
      }
      token = strtok(NULL, ":");
    }
  }
  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  else
  {
    if (add_or_remove)
    {
      DST_MAC[0] = (DST_MACSegment[0] << 24) | DST_MACSegment[1] << 16 | (DST_MACSegment[2] << 8) | DST_MACSegment[3];
      DST_MAC[1] = (DST_MACSegment[4] << 8) | DST_MACSegment[5];
    }
    else
    {
      DST_MAC[0] = 0x00000000;
      DST_MAC[1] = 0x00000000;
    }
    delay_1ms();
    switch (key - '0')
    {
    case 1:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 692, DST_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 696, DST_MAC[1]);
      break;
    case 2:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 692, DST_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 696, DST_MAC[1]);
      break;
    case 3:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 692, DST_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 696, DST_MAC[1]);
      break;
    case 4:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 692, DST_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 696, DST_MAC[1]);
      break;
    case 5:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 692, DST_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 696, DST_MAC[1]);
      break;
    case 6:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 692, DST_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 696, DST_MAC[1]);
      break;
    case 7:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 692, DST_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 696, DST_MAC[1]);
      break;
    case 8:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 692, DST_MAC[0]);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 696, DST_MAC[1]);
      break;
      //
    }
  }
}
// Port mirroring src ipv4
void Set_src_IP4_Mirroring(const char *ip_str, int i, int add_or_remove)
{
  u16 IPlayerA = 0;
  u16 IPlayerB = 0;
  u16 IPlayerC = 0;
  u16 IPlayerD = 0;
  u8 flag_error = 0;

  if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IPlayerA, &IPlayerB, &IPlayerC, &IPlayerD) != 4)
  {
    flag_error = 1;
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
    return;
  }
  if (add_or_remove)
  {
    IPTarget1 = IPlayerA * 16777216 + IPlayerB * 65536 + IPlayerC * 256 + IPlayerD;
  }
  else
  {
    IPTarget1 = 0x00000000;
  }
  switch (i)
  {
  case 1:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 700, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 704, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 708, 0x00000000); // ip_src_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 712, IPTarget1);
    break;
  case 2:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 700, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 704, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 708, 0x00000000); // ip_src_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 712, IPTarget1);
    break;
  case 3:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 700, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 704, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 708, 0x00000000); // ip_src_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 712, IPTarget1);
    break;
  case 4:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 700, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 704, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 708, 0x00000000); // ip_src_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 712, IPTarget1);
    break;
  case 5:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 700, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 704, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 708, 0x00000000); // ip_src_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 712, IPTarget1);
    break;
  case 6:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 700, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 704, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 708, 0x00000000); // ip_src_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 712, IPTarget1);
    break;
  case 7:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 700, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 704, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 708, 0x00000000); // ip_src_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 712, IPTarget1);
    break;
  case 8:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 700, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 704, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 708, 0x00000000); // ip_src_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 712, IPTarget1);
    break;
  default:
    break;
  }
  // ReturnMode2();
}

// Port mirroring src ipv4
void Port_mirroring_src_ipv4(int add_or_remove)
{
  u8 key = 0;
  char buffer_IPV4[BUFFER_SIZE_IPV4];
  char header_buffer_IPV4[1];
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);

    if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8')
    {
      break;
    }
  }

  ReceiveIPv4String_protect(buffer_IPV4, header_buffer_IPV4, BUFFER_SIZE_IPV4);
  Set_src_IP4_Mirroring(buffer_IPV4, key - '0', add_or_remove);
}

// set dst ip4 mir
void Set_dst_IP4_Mirroring(const char *ip_str, int i)
{
  u16 IPlayerA = 0;
  u16 IPlayerB = 0;
  u16 IPlayerC = 0;
  u16 IPlayerD = 0;
  u8 flag_error = 0;

  if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IPlayerA, &IPlayerB, &IPlayerC, &IPlayerD) != 4)
  {
    flag_error = 1;
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
    return;
  }
  if (add_or_remove)
  {
    IPTarget1 = IPlayerA * 16777216 + IPlayerB * 65536 + IPlayerC * 256 + IPlayerD;
  }
  else
  {
    IPTarget1 = 0x00000000;
  }
  switch (i)
  {
  case 1:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 716, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 720, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 724, 0x00000000); // ip_dst_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 728, IPTarget1);
    break;
  case 2:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 716, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 720, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 724, 0x00000000); // ip_dst_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 728, IPTarget1);
    break;
  case 3:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 716, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 720, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 724, 0x00000000); // ip_dst_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 728, IPTarget1);
    break;
  case 4:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 716, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 720, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 724, 0x00000000); // ip_dst_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 728, IPTarget1);
    break;
  case 5:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 716, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 720, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 724, 0x00000000); // ip_dst_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 728, IPTarget1);
    break;
  case 6:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 716, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 720, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 724, 0x00000000); // ip_dst_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 728, IPTarget1);
    break;
  case 7:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 716, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 720, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 724, 0x00000000); // ip_dst_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 728, IPTarget1);
    break;
  case 8:

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 716, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 720, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 724, 0x00000000); // ip_dst_filter_mirror
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 728, IPTarget1);
    break;
  default:
    break;
  }
  // ReturnMode2();
}

// Port mirroring dst ipv4
void Port_mirroring_dst_ipv4(int add_or_remove)
{
  u8 key = 0;
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);

    if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8')
    {
      break;
    }
  }

  char buffer_IPV4[BUFFER_SIZE_IPV4];
  char header_buffer_IPV4[1];
  ReceiveIPv4String_protect(buffer_IPV4, header_buffer_IPV4, BUFFER_SIZE_IPV4);
  Set_dst_IP4_Mirroring(buffer_IPV4, key - '0', add_or_remove);
}

// set src ipv6 mir
void Port_mirroring_src_dst_ipv6(int dst_or_src, int add_or_remove)
{
  u8 key = 0;
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);

    if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8')
    {
      break;
    }
  }

  int ipv6_1 = 0;
  int ipv6_2 = 0;
  int ipv6_3 = 0;
  int ipv6_4 = 0;
  if (add_or_remove == 0)
  {
    ipv6_1 = 0;
    ipv6_2 = 0;
    ipv6_3 = 0;
    ipv6_4 = 0;
  }
  else
  {
    Cacul_IPv6_into_port(&ipv6_1, &ipv6_2, &ipv6_3, &ipv6_4);
  }

  if (dst_or_src)
  {
    switch (key - '0')
    {
    case 1:
      Write_IPv6_into_port_mirroring(716, 720, 724, 728, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;
    case 2:
      Write_IPv6_into_port_mirroring(716, 720, 724, 728, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;
    case 3:
      Write_IPv6_into_port_mirroring(716, 720, 724, 728, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;
    case 4:
      Write_IPv6_into_port_mirroring(716, 720, 724, 728, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;

    case 5:
      Write_IPv6_into_port_mirroring(716, 720, 724, 728, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;
    case 6:
      Write_IPv6_into_port_mirroring(716, 720, 724, 728, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;
    case 7:
      Write_IPv6_into_port_mirroring(716, 720, 724, 728, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;
    case 8:
      Write_IPv6_into_port_mirroring(716, 720, 724, 728, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;
    default:
      break;
    }
  }
  // Them IP src ip6
  else
  {
    switch (key - '0')
    {
    case 1:
      Write_IPv6_into_port_mirroring(700, 704, 708, 712, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;
    case 2:
      Write_IPv6_into_port_mirroring(700, 704, 708, 712, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;
    case 3:
      Write_IPv6_into_port_mirroring(700, 704, 708, 712, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;
    case 4:
      Write_IPv6_into_port_mirroring(700, 704, 708, 712, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;

    case 5:
      Write_IPv6_into_port_mirroring(700, 704, 708, 712, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;
    case 6:
      Write_IPv6_into_port_mirroring(700, 704, 708, 712, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;
    case 7:
      Write_IPv6_into_port_mirroring(700, 704, 708, 712, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;
    case 8:
      Write_IPv6_into_port_mirroring(700, 704, 708, 712, ipv6_1, ipv6_2, ipv6_3, ipv6_4);
      break;
    default:
      break;
    }
  }
}

// port mirroring src port
void Port_mirroring_src_dst_port(int add_or_remove)
{
  char buffer[16] = {0};
  u8 key;
  int idx = 0;
  // Nhận chuỗi từ UART
  u8 key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);

    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
    {
      break;
    }
  }
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 13 || key == '\n' || idx >= 15) // Enter hoặc quá dài
    {
      buffer[idx] = '\0';
      break;
    }
    buffer[idx++] = key;
  }

  unsigned int src = 0, dst = 0;
  if (strchr(buffer, '-') != NULL)
  {
    // Có dấu '-'
    if (buffer[0] == '-')
    {
      // "-dst"
      sscanf(buffer, "-%u", &dst);
      src = 0;
    }
    else if (buffer[strlen(buffer) - 1] == '-')
    {
      // "src-"
      sscanf(buffer, "%u-", &src);
      dst = 0;
    }
    else
    {
      // "src-dst"
      sscanf(buffer, "%u-%u", &src, &dst);
    }
  }
  else
  {
    // Không có dấu '-'
    dst = atoi(buffer);
    src = 0;
  }

  if (src <= 0xFFFF && dst <= 0xFFFF)
  {
    unsigned int value = (src << 16) | (dst & 0xFFFF);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 732, value);
    // XUartLite_SendByte(0x40600000, 'Y');
  }
  else
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
}

// Port_mirroring_protocol
void Port_mirroring_protocol(int add_or_remove)
{
  u8 key = 0;
  char buffer[16] = {0};
  int idx = 0;
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);

    if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8')
    {
      break;
    }
  }

  u8 protocol = 0;

  while (1)
  {
    protocol = XUartLite_RecvByte(0x40600000);
    if (protocol == '1' || protocol == '2' || protocol == '3') // Enter hoặc quá dài
    {
      break;
    }
  }
  if (add_or_remove == 0)
  {
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000000); // Clear previous protocol
  }
  else
  {
    switch (key - '0')
    {
    case 1:
      if (protocol == '1')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000006); // TCP
      }
      else if (protocol == '2')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000011); // UDP
      }
      else if (protocol == '3')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000001); // ICMP
      }

      break;
    case 2:
      if (protocol == '1')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000006); // TCP
      }
      else if (protocol == '2')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000011); // UDP
      }
      else if (protocol == '3')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000001); // ICMP
      }
      break;
    case 3:
      if (protocol == '1')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000006); // TCP
      }
      else if (protocol == '2')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000011); // UDP
      }
      else if (protocol == '3')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000001); // ICMP
      }
      break;
    case 4:
      if (protocol == '1')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000006); // TCP
      }
      else if (protocol == '2')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000011); // UDP
      }
      else if (protocol == '3')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000001); // ICMP
      }
      break;
    case 5:
      if (protocol == '1')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000006); // TCP
      }
      else if (protocol == '2')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000011); // UDP
      }
      else if (protocol == '3')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000001); // ICMP
      }
      break;
    case 6:
      if (protocol == '1')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000006); // TCP
      }
      else if (protocol == '2')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000011); // UDP
      }
      else if (protocol == '3')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000001); // ICMP
      }
      break;
    case 7:
      if (protocol == '1')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000006); // TCP
      }
      else if (protocol == '2')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000011); // UDP
      }
      else if (protocol == '3')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000001); // ICMP
      }
      break;
    case 8:
      if (protocol == '1')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000006); // TCP
      }
      else if (protocol == '2')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000011); // UDP
      }
      else if (protocol == '3')
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, 0x00000001); // ICMP
      }
      break;
    default:
      break;
    }
  }
}
// Port_mirroring_protocol
void Port_mirroring_mode(int add_or_remove)
{
  u8 key = 0;
  char buffer[16] = {0};
  int idx = 0;
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);

    if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8')
    {
      break;
    }
  }

  u8 mode = 0;

  while (1)
  {
    mode = XUartLite_RecvByte(0x40600000);
    if (mode == '1' || mode == '2' || mode == '3' || mode == '0')
    {
      break;
    }
  }

  switch (key - '0')
  {
  case 1:
    if (mode == '1')
    {

      mode_monitored = '1';

      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I
    }
    else if (mode == '2')
    {
      mode_monitored = '2';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // E
    }
    else if (mode == '3')
    {
      mode_monitored = '3';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I+E
    }

    break;
  case 2:
    if (mode == '1')
    {

      mode_monitored = '1';

      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I
    }
    else if (mode == '2')
    {
      mode_monitored = '2';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // E
    }
    else if (mode == '3')
    {
      mode_monitored = '3';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I+E
    }
    break;
  case 3:
    if (mode == '1')
    {

      mode_monitored = '1';

      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I
    }
    else if (mode == '2')
    {
      mode_monitored = '2';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // E
    }
    else if (mode == '3')
    {
      mode_monitored = '3';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I+E
    }
    break;
  case 4:
    if (mode == '1')
    {

      mode_monitored = '1';

      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I
    }
    else if (mode == '2')
    {
      mode_monitored = '2';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // E
    }
    else if (mode == '3')
    {
      mode_monitored = '3';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I+E
    }
    break;
  case 5:
    if (mode == '1')
    {

      mode_monitored = '1';

      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I
    }
    else if (mode == '2')
    {
      mode_monitored = '2';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // E
    }
    else if (mode == '3')
    {
      mode_monitored = '3';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I+E
    }
    break;
  case 6:
    if (mode == '1')
    {

      mode_monitored = '1';

      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I
    }
    else if (mode == '2')
    {
      mode_monitored = '2';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // E
    }
    else if (mode == '3')
    {
      mode_monitored = '3';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I+E
    }
    break;
  case 7:
    if (mode == '1')
    {

      mode_monitored = '1';

      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I
    }
    else if (mode == '2')
    {
      mode_monitored = '2';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // E
    }
    else if (mode == '3')
    {
      mode_monitored = '3';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I+E
    }
    break;
  case 8:
    if (mode == '1')
    {

      mode_monitored = '1';

      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I
    }
    else if (mode == '2')
    {
      mode_monitored = '2';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // E
    }
    else if (mode == '3')
    {
      mode_monitored = '3';
      u8 value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value); // I+E
    }
    break;
  default:
    break;
  }
}

// Chose Port monitored
void Choose_port_monitored()
{
  u8 key = 0;
  u8 key1;
  char buffer[16] = {0};
  int idx = 0;
  u8 value;
  uint8_t port_monitored;
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);

    if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8')
    {
      break;
    }
  }
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);

    if (key1 == '1' || key1 == '0')
    {
      break;
    }
  }
  if (key1 == "1")
  {
    switch (key - '0')
    {
    case 1:
      port_monitored_1 = 1;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);
      break;
    case 2:
      port_monitored_2 = 1;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);
      break;
    case 3:
      port_monitored_3 = 1;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);

      break;
    case 4:
      port_monitored_4 = 1;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);
      break;
    case 5:
      port_monitored_1 = 1;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);
      break;
    case 6:
      port_monitored_1 = 1;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);
      break;
    case 7:
      port_monitored_1 = 1;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);
      break;
    case 8:
      port_monitored_1 = 1;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);
      break;
    default:
      break;
    }
  }
  else if (key1 == "0")
  {
    switch (key - '0')
    {
    case 1:
      port_monitored_1 = 0;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);
      break;
    case 2:
      port_monitored_2 = 0;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);
      break;
    case 3:
      port_monitored_3 = 0;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);
      break;
    case 4:
      port_monitored_4 = 0;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);
      break;
    case 5:
      port_monitored_1 = 1;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);
      break;
    case 6:
      port_monitored_1 = 1;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);
      break;
    case 7:
      port_monitored_1 = 1;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);
      break;
    case 8:
      port_monitored_1 = 1;
      port_monitored = (uint8_t)((port_monitored_1 << 3) | (port_monitored_2 << 2) | (port_monitored_3 << 1) | (port_monitored_4 << 0));
      value = ((port_monitored & 0x0F) << 2) | (mode_monitored & 0x03);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 676, value);
      break;
    default:
      break;
    }
  }
}
//

void Open_port()
{
  u8 key = 0;
  u8 key_rst = 0;
  u8 count = 0;
  u8 key1 = 0;
  while (1)
  {

    key = 0;
    count = 0;

    while (key != 13 || count < 1)
    {
      while (1)
      {
        if (XUartLite_IsReceiveEmpty(0x40600000) == 0)
        {
          key = XUartLite_RecvByte(0x40600000);
          break;
        }
      }
      if (key != 13 && count == 0)
      {
        key_rst = key;
        key1 = key;
        count = 1;
      }
    }

    if (key1 == 'A')
    {
      // MODE PORT 1
      Port_mode(1);
      break;
    }
    else if (key1 == 'B')
    {
      // MODE PORT 2
      Port_mode(2);
      break;
    }
    else if (key1 == 'C')
    {
      // MODE PORT 3
      Port_mode(3);
      break;
    }
    else if (key1 == 'D')
    {
      // MODE PORT 4
      Port_mode(4);
      break;
    }
    else if (key1 == 'E')
    {
      // MODE PORT 5
      Port_mode(5);
      break;
    }
    else if (key1 == 'F')
    {
      // MODE PORT 6
      Port_mode(6);
      break;
    }
    else if (key1 == 'G')
    {
      // MODE PORT 7
      Port_mode(7);
      break;
    }
    else if (key1 == 0xFF)
    {
      // MODE PORT 8
      Port_mode(8);
      break;
    }
    // Add ipv4 vao port
    //========================

    else if (key1 == 'H')
    {
      // Add ipv4 vao port 1
      Add_IPv4_into_port(1, 1);
      break;
    }
    else if (key1 == 'I')
    {
      // Add ipv4 vao port 2
      // Add_IPv4_into_port(2, 1);
    }
    else if (key1 == 'J')
    {
      // Add ipv4 vao port 3
      Add_IPv4_into_port(3, 1);
      break;
    }
    else if (key1 == 'K')
    {
      // Add ipv4 vao port 4
      Add_IPv4_into_port(4, 1);
      break;
    }
    else if (key1 == 'L')
    {
      // Add ipv4 vao port 5
      Add_IPv4_into_port(1, 0);
      break;
    }
    else if (key1 == 'M')
    {
      // Add ipv4 vao port 6
      Add_IPv4_into_port(3, 0);
      break;
    }
    else if (key1 == 'N')
    {
      // Add ipv4 vao port 7
      Add_IPv4_into_port(4, 0);
      break;
    }

    // Xoa ipv4 vao port
    //========================
    else if (key1 == 'O')
    {
      // Xoa ipv4 vao port 1
      Remove_IPv4_into_port(1, 1);
      break;
    }
    else if (key1 == 'P')
    {
      // Xoa ipv4 vao port 2
      // Remove_IPv4_into_port(2, 1);
      // break;
    }
    else if (key1 == 'Q')
    {
      // Xoa ipv4 vao port 3
      Remove_IPv4_into_port(3, 1);
      break;
    }
    else if (key1 == 'R')
    {
      // Xoa ipv4 vao port 4
      Remove_IPv4_into_port(4, 1);
      break;
    }
    else if (key1 == 'S')
    {
      // Xoa ipv4 vao port 5
      Remove_IPv4_into_port(1, 0);
      break;
    }
    else if (key1 == 'T')
    {
      // Xoa ipv4 vao port 6
      Remove_IPv4_into_port(3, 0);
      break;
    }
    else if (key1 == 'U')
    {
      // Xoa ipv4 vao port 7
      Remove_IPv4_into_port(4, 0);
      break;
    }

    // Add ipv6 vao port
    //========================
    else if (key1 == 'V')
    {
      // Add ipv6 vao port 1
      Add_IPv6_into_port(1, 1);
      break;
    }
    else if (key1 == 'W')
    {
      // Add ipv6 vao port 2
    }
    else if (key1 == 'X')
    {
      // Add ipv6 vao port 3
      Add_IPv6_into_port(3, 1);
      break;
    }

    else if (key1 == 'Y')
    {
      // Add ipv6 vao port 4
      Add_IPv6_into_port(4, 1);
      break;
    }
    else if (key1 == 'Z')
    {
      // Add ipv6 vao port 5
      // Tam thoi lay 5 6 7 lam src 1 3 4
      Add_IPv6_into_port(1, 0);
      break;
    }

    else if (key1 == ';')
    {
      // Add ipv6 vao port 6
      Add_IPv6_into_port(3, 0);
      break;
    }
    else if (key1 == '+')
    {
      // Add ipv6 vao port 7
      Add_IPv6_into_port(4, 0);
      break;
    }

    //========================
    // Xoa ipv6 vao port

    else if (key1 == '-')
    {
      // Xoa ipv6 vao port 1
      Remove_IPv6_into_port(1, 1);
      break;
    }
    else if (key1 == 0x02)
    {
      // Xoa ipv6 vao port 2
    }
    else if (key1 == 0x0D)
    {
      // Xoa ipv6 vao port 3
      Remove_IPv6_into_port(3, 1);
      break;
    }
    else if (key1 == 0x04)
    {
      // Xoa ipv6 vao port 4
      Remove_IPv6_into_port(4, 1);
      break;
    }

    else if (key1 == 0x05)
    {
      // Xoa ipv6 vao port 5
      Remove_IPv6_into_port(1, 0);
      break;
    }

    else if (key1 == 0x06)
    {
      // Xoa ipv6 vao port 6
      Remove_IPv6_into_port(3, 0);
      break;
    }

    else if (key1 == 0x07)
    {
      // Xoa ipv6 vao port 2
      Remove_IPv6_into_port(4, 0);
      break;
    }
  }
}

void ModeStart()
{

  // clear_screen();
  u8 time1, key1, key_rst, setupOK;
  u8 key2 = 0;
  setupOK = 0;
  key1 = 0;
  key = 0;
  key_rst = 0;
  u8 count = 0;
  int Number_of_IP;
  while (setupOK == 0)
  {

  start:
    key = 0;
    count = 0;
    LoadEEPROM_defender();
    while (key != 13 || count < 1)
    {
      while (1)
      {
        Number_of_IP = get_HTTP_Data(&Num_of_URL, &Num_of_IP, &Num_in_PAL);
        if (Number_of_IP == Num_of_IP)
        {
          Num_of_IP += 100;
          IP_Conn_Table = (struct IP_Connection *)realloc(IP_Conn_Table, Num_of_IP * sizeof(struct IP_Connection));
          if (IP_Conn_Table == NULL)
          {
          }
        }

        if (XUartLite_IsReceiveEmpty(0x40600000) == 0)
        {
          key = XUartLite_RecvByte(0x40600000);
          break;
        }
      }
      if (key != 13 && count == 0)
      {
        key_rst = key;
        key1 = key;
        count = 1;
      }
      // xil_printf("\r\n\t\t|ggg:: %d", EnableDefender1);
    }

    if (key1 == '1')
    {
      SetDateTime();
      goto start;
    }
    else if (key1 == ',')
    {
      while (1)
      {
        SetDateTime();
        XUartLite_SendByte(0x40600000, 'S');
        delay_1ms();
        break;
      }
      goto start;
    }
    else if (key1 == '2')
    {
      SetDefenderPort();
      goto start;
    }
    else if (key1 == '3')
    {
      SetPortDefender();
      goto start;
    }
    else if (key1 == '4')
    {
      Add_IPv4_Protect_Server();
      goto start;
    }
    else if (key1 == 'R')
    {
      // SetIPv6Target();
      Add_IPv6_Protect_Server();
      // ReturnMode2();
      goto start;
    }
    //
    else if (key1 == 05)
    {
      Delete_IPv4_Protect_Server();
      goto start;
    }
    else if (key1 == 06)
    {
      // SetIPv6Target();
      Delete_IPv6_Protect_Server();
      // ReturnMode2();
      goto start;
    }
    // Block IP Client
    //=========================
    // else if (key1 == 0x08)
    // {
    //   Add_IPv4_Block_Attacker();
    //   goto start;
    // }
    else if (key1 == 0x09)
    {

      Add_IPv6_Block_Attacker();
      goto start;
    }
    //
    else if (key1 == 0x0A)
    {
      Delete_IPv4_Block_Attacker();
      goto start;
    }
    else if (key1 == 0x0D)
    {
      Delete_IPv6_Block_Attacker();
      goto start;
    }
    //==============================

    //
    else if (key1 == '5')
    {
      SetTimeflood(); //
      goto start;
    }
    else if (key1 == '6')
    {
      SetSynDefender(); //
      goto start;
    }
    else if (key1 == '7')
    {
      SetSynThresh(); //
      goto start;
    }
    else if (key1 == '8')
    {
      SetAckThresh(); //
      goto start;
    }
    else if (key1 == '9')
    {
      SetTimeDelete();
      goto start;
    }
    else if (key1 == 'A' || key1 == 'a')
    {
      SetSynonymousDefender(); //
      goto start;
    }
    else if (key1 == 'B' || key1 == 'b')
    {
      SetUDPDefender(); //
      goto start;
    }
    else if (key1 == 'C' || key1 == 'c')
    {

      SetUDPThresh(); //
      goto start;
    }
    else if (key1 == 'D' || key1 == 'd')
    {

      SetUDPThresh1s(); //
      goto start;
    }
    else if (key1 == 'E' || key1 == 'e')
    {

      SetDNSDefender(); //
      goto start;
    }
    else if (key1 == 'F' || key1 == 'f')
    {

      SetDNSThresh(); //
      goto start;
    }
    else if (key1 == 'G' || key1 == 'g')
    {

      SetICMPDefender(); //
      goto start;
    }
    else if (key1 == 'H' || key1 == 'h')
    {

      SetICMPThresh(); //
      goto start;
    }
    else if (key1 == 'I' || key1 == 'i')
    {

      SetICMPThresh1s(); //
      goto start;
    }
    else if (key1 == 'J' || key1 == 'j')
    {

      SetIPSecDefender(); //
      goto start;
    }
    else if (key1 == 'K' || key1 == 'k')
    {

      SetIPSecThresh(); //
      goto start;
    }
    else if (key1 == 'L' || key1 == 'l')
    {

      ProcessAddIpv4VPN();
      goto start;
    }
    else if (key1 == 'M' || key1 == 'm')
    {
      ProcessRemoveIpv4VPN();
      goto start;
    }
    else if (key1 == '#')
    {

      AddIpVPN_IPV6();

      goto start;
    }
    else if (key1 == '^')
    {
      RemoveIpVPN_IPV6();

      goto start;
    }
    else if (key1 == 'N' || key1 == 'n')
    {

      SetTCPFragDefender(); //
      goto start;
    }
    else if (key1 == 'O' || key1 == 'o')
    {

      SetUDPFragDefender(); //
      goto start;
    }
    else if (key1 == 'P' || key1 == 'p')
    {

      SetHTTPGETDefender();
      goto start;
    }
    else if (key1 == 0xFF)
    {

      SetHTTPSGETDefender();
      goto start;
    }
    else if (key1 == 'T')
    {
      // delay_1s();
      ADD_IPv4_HTTP_BLACK_LIST();
      goto start;
    }
    else if (key1 == 'X' || key1 == 'x')
    {
      REMOVE_IPv4_HTTP_BLACK_LIST();

      goto start;
    }
    else if (key1 == '{')
    {
      // delay_1s();
      ADD_IPv6_HTTP_BLACK_LIST();

      goto start;
    }
    else if (key1 == '}')
    {
      REMOVE_IPv6_HTTP_BLACK_LIST();
      goto start;
    }
    // WhiteList HTTPS
    else if (key1 == 04)
    {
      // delay_1s();
      ADD_IPv4_HTTP_WHITE_LIST();
      goto start;
    }
    else if (key1 == 05)
    {
      REMOVE_IPv4_HTTP_WHITE_LIST();
      goto start;
    }
    // else if (key1 == '{')
    // {
    //   // delay_1s();
    //   ADD_IPv6_HTTP_WHITE_LIST();

    //   goto start;
    // }
    // else if (key1 == '}')
    // {
    //   REMOVE_IPv6_HTTP_WHITE_LIST();
    //   goto start;
    // }
    /*=====================================*/

    else if (key1 == '-')
    {
      delete_account();
      goto start;
    }
    else if (key1 == '+')
    {
      add_acount();
      goto start;
    }
    else if (key1 == '=')
    {
      change_user_pass_admin_mode();
      goto start;
    }
    else if (key1 == ';')
    {
      change_root();
      goto start;
      //
    }
    else if (key1 == '?')
    {
      LoadEEPROM();
      load_default();
      ReturnMode3();
      goto start;
    }
    else if (key1 == '<')
    {
      DisplayTable();
      goto start;
    }
    else if (key1 == '>')
    {
      DisplayAccount();
      goto start;
      //
    }
    else if (key1 == '/')
    {
      check_account();
      goto start;
      // ok
    }
    else if (key1 == '|')
    {
      user_change_info();
      goto start;
      // ok
    }
    else if (key1 == '[')
    {
      reset_account();
      // ok
      goto start;
    }
    else if (key1 == ']')
    {
      setload_default();
      goto start;
      // ok
    }
    else if (key1 == '(')
    {
      change_user_pass_admin_mode_ter();
      goto start;
      // ok
    }
    else if (key1 == ')')
    {
      delete_account_ver_tmn();
      goto start;
      // ok
    }
    else if (key1 == '*')
    {
      show_inf_flow();
      goto start;
    }
    else if (key1 == '&')
    {

      loadData();
      goto start;
    }
    else if (key1 == '%')
    {
      change_duration_time_export();
      goto start;
    }
    else if (key1 == '.')
    {
      Add_IPv4_HTTP_fromFile();
      goto start;
    }
    else if (key1 == '`')
    {
      Add_IPv6_HTTP_fromFile();
      goto start;
    }
    else if (key1 == 01)
    {
      Clear_IPv4_HTTP_fromFile();
      goto start;
    }
    else if (key1 == 02)
    {
      Clear_IPv6_HTTP_fromFile();
      goto start;
    }
    else if (key1 == 07)
    {
      Open_port();
      goto start;
    }
    // Select port Mirroring
    else if (key1 == 0x08)
    {
      Port_mirroring();
      goto start;
    }
    else if (key_rst == 03)
    {
      Reset_System();
    }
  }
}
void change_duration_time_export()
{
  u8 key;
  u8 count;
Return:
  key = 0;
  count = 0;
  u32 duration_time = 0;
  while (key != 13 && count < 16)
  {
    key = XUartLite_RecvByte(0x40600000);
    if ((count == 0) && (key == 48))
    {
      key = key - 48;
      duration_time = duration_time + key;
    }
    else if (key > 47 && key < 58)
    {
      key = key - 48;
      duration_time = duration_time * 10 + key;
      count++;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
  Duration_time_export = duration_time;

  AssignData();
  SaveEEPROM();
  ConfigurationIPcore(1);
  // ReturnMode2();
}
void ConfigurationIPcore(int port)
{
  // EnableDefender1 = 12799;
  //   EnableDefender1 = 1279;
  //   EnableDefender1 = 3327;
  //   EnableDefender1 = 12543;

  // Set Ip taget:
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 356, IPv6Target[0]);
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 360, IPv6Target[1]);
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 364, IPv6Target[2]);
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 368, IPv6Target[3]);
  // Set Ip taget:
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 4, IPTarget1);
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 4, IPTarget2);
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 4, IPTarget3);
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 4, IPTarget4);

  // Set time condition timeout white list
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 140, TimeDelete);

  u32 TimeFloodCore;
  switch (port)
  {
  case 1:
    // Set Timeflood
    TimeFloodCore = TimeFlood1 * 1000;
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 16, TimeFloodCore);
    // Set Syn Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 20, SynThreshold1);
    // Set Ack Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 24, AckThreshold1);
    // Set Udp Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 28, UDPThreshold1);
    // Set Icmp Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 32, ICMPThreshold1);
    // Set Dns Thrshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 36, DNSThreshold1);
    // Set IPSec Thrshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 93, IPSecThreshold1);
    // Set Udp Threshold each sencond.
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 112, UDPThresh_1s1);
    // Set Icmp Threshold each second
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 116, ICMPThresh_1s1);
    delay_1s();
    // Set Enable DDoS Defender
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 40, EnableDefender1);
    break;
  case 2:
    // Set Timeflood
    TimeFloodCore = TimeFlood2 * 1000;
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 740, TimeFloodCore);
    // Set Syn Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 744, SynThreshold2);
    // Set Ack Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 748, AckThreshold2);
    // Set Udp Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 752, UDPThreshold2);
    // Set Icmp Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 760, ICMPThreshold2);
    // Set Dns Thrshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 768, DNSThreshold2);
    // Set IPSec Thrshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 772, IPSecThreshold2);
    // Set Udp Threshold each sencond.
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 756, UDPThresh_1s2);
    // Set Icmp Threshold each second
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 764, ICMPThresh_1s2);
    delay_1s();
    // Set Enable DDoS Defender
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 736, EnableDefender2);
    break;
  case 3:
    // Set Timeflood
    TimeFloodCore = TimeFlood3 * 1000;
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 780, TimeFloodCore);
    // Set Syn Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 784, SynThreshold3);
    // Set Ack Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 788, AckThreshold3);
    // Set Udp Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 792, UDPThreshold3);
    // Set Icmp Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 800, ICMPThreshold3);
    // Set Dns Thrshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 808, DNSThreshold3);
    // Set IPSec Thrshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 812, IPSecThreshold3);
    // Set Udp Threshold each sencond.
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 796, UDPThresh_1s3);
    // Set Icmp Threshold each second
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 804, ICMPThresh_1s3);
    delay_1s();
    // Set Enable DDoS Defender
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 776, EnableDefender3);
    break;
  case 4:
    // Set Timeflood
    TimeFloodCore = TimeFlood4 * 1000;
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 820, TimeFloodCore);
    // Set Syn Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 824, SynThreshold4);
    // Set Ack Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 828, AckThreshold4);
    // Set Udp Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 832, UDPThreshold4);
    // Set Icmp Threshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 840, ICMPThreshold4);
    // Set Dns Thrshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 848, DNSThreshold4);
    // Set IPSec Thrshold
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 852, IPSecThreshold4);
    // Set Udp Threshold each sencond.
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 836, UDPThresh_1s4);
    // Set Icmp Threshold each second
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 844, ICMPThresh_1s4);
    delay_1s();
    // Set Enable DDoS Defender
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 816, EnableDefender4);
    break;

  default:
    break;
  }
  isCompleted_ConfigurationIPcore = 1;
}

void AddIpVPNtoCore()
{
  u8 done = 0;
  u8 number_ip = 0;
  u8 Rd_LA;
  u8 Rd_LB;
  u8 Rd_LC;
  u8 Rd_LD;
  u32 IpVPN;
  while (done == 0)
  {
    EEPROM_ReadIIC(&myDevice_EEPROM, (number_ip * 4) + 32 + 0, &Rd_LA, 1);
    // delay_1ms();
    EEPROM_ReadIIC(&myDevice_EEPROM, (number_ip * 4) + 32 + 1, &Rd_LB, 1);
    // delay_1ms();
    EEPROM_ReadIIC(&myDevice_EEPROM, (number_ip * 4) + 32 + 2, &Rd_LC, 1);
    // delay_1ms();
    EEPROM_ReadIIC(&myDevice_EEPROM, (number_ip * 4) + 32 + 3, &Rd_LD, 1);
    if (number_ip >= 32)
    {
      done = 1;
    }
    if (Rd_LA == 0 && Rd_LB == 0 && Rd_LC == 0 && Rd_LD == 0)
    {
      number_ip = number_ip + 1;
    }
    else
    {
      IpVPN = Rd_LA * 16777216 + Rd_LB * 65536 + Rd_LC * 256 + Rd_LD;
      // Set Ip VPN:
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 132, IpVPN);
      // Set enable add :
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 136, 1);
      // delay_1ms();
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 136, 0);
      number_ip = number_ip + 1;
    }
  }
}
void check_account1()
{
  admin_mode();
}
void loadData()
{
  XUartLite_SendByte(0x40600000, 'Y');
  delay_0_5s();
  Send_data_gui();
}
void sendString(u32 BaseAddress, const char *str)
{
  while (*str != '\0')
  {
    XUartLite_SendByte(BaseAddress, *str);
    str++;
  }
}
void Send_data_gui()
{
  LoadEEPROM();
  load_user();
  load_pass();
  load_passroot();
  char name_protect[5];
  char buffer_str[1024];
  if (DefenderPort_en1)
  {
    strcpy(name_protect, "IP");
  }
  else if (DefenderPort_en1 == 0)
  {
    strcpy(name_protect, "PORT");
  }
  sprintf(buffer_str, "TIME_DETECT_ATTACK$%d$TIME_WHITE_LIST$%d$SYN_THR$%d$ACK_THR$%d$UDP_THR$%d$DNS_THR$%d$ICMP_THR$%d$IPSEC_IKE_THR$%d$UDP_THR_PS$%d$ICMP_THR_PS$%d$SYN_EN_DIS$%d$LAND_EN_DIS$%d$UDP_EN_DIS$%d$DNS_EN_DIS$%d$ICMP_EN_DIS$%d$IPSEC_IKE_EN_DIS$%d$TCP_FRA_EN_DIS$%d$UDP_FRA_EN_DIS$%d$PROTECT$%s$PROTECT_PORT$%d$HTTP_EN_DIS$%d$HTTPS_EN_DIS$%d$PROTECT_IPV6$%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x$accounts1$%s$acpasswords1$%s$accounts2$%s$acpasswords2$%s$accounts3$%s$acpasswords3$%s$accounts4$%s$acpasswords4$%s$accounts5$%s$acpasswords5$%s$passwordADMIN$%s^",
          TimeFlood1, TimeDelete, SynThreshold1, AckThreshold1, UDPThreshold1, DNSThreshold1, ICMPThreshold1, IPSecThreshold1, UDPThresh_1s1, ICMPThresh_1s1, SYNFlood_en1, LANDATTACK_en1, UDPFlood_en1, DNSFlood_en1, ICMPFlood_en1, IPSecFlood_en1, TCPFragFlood_en1, UDPFragFlood_en1, name_protect, DefenderPort, HTTPGETFlood_en1, HTTPSGETFlood_en1, (u16)(IPv6Target[0] >> 16), (u16)(IPv6Target[0] & 0xFFFF), (u16)(IPv6Target[1] >> 16), (u16)(IPv6Target[1] & 0xFFFF), (u16)(IPv6Target[2] >> 16), (u16)(IPv6Target[2] & 0xFFFF), (u16)(IPv6Target[3] >> 16), (u16)(IPv6Target[3] & 0xFFFF), user1, pass1, user2, pass2, user3, pass3, user4, pass4, user5, pass5, passroot);

  sendString(0x40600000, buffer_str);
}
void display_logo1()
{
  // xil_printf("\r\n******************************************************************************************************************************************************************************************");
  // xil_printf("\r\n**                                                                                                                                                                                      **");
  // xil_printf("\r\n**\t\t\t\t\t\t\t ____  ____       ____        ____        __                _                                                                   **");
  // xil_printf("\r\n**\t\t\t\t\t\t\t|  _ '|  _ '  ___/ ___|      |  _ '  ___ / _| ___ _ __   __| | ___ _ __                                                         **");
  // xil_printf("\r\n**\t\t\t\t\t\t\t| | | | | | |/ _ '___ '  ___ | | | |/ _ ' |_ / _ ' '_ ' / _` |/ _ ' '__|                                                        **");
  // xil_printf("\r\n**\t\t\t\t\t\t\t| |_| | |_| | (_) |_ ) )|___|| |_| | |/_/  _|  __/ | | | (_| |  __/ |                                                           **");
  // xil_printf("\r\n**\t\t\t\t\t\t\t|____/|____/ '___/____/      |____/ '___|_|  '___|_| |_|'__,_|'___|_|                                                           **");
  // xil_printf("\r\n**                                                                                                                                                                                      **");
  // xil_printf("\r\n******************************************************************************************************************************************************************************************");
  // xil_printf("\r\n                                                                                                                                            ***** DDoS Defender by Acronics Solutions ****");
}

void display_logo2()
{
  // xil_printf("\r\n******************************************************************************************************************************************");
  // xil_printf("\r\n**                                                                                                                                      **");
  // xil_printf("\r\n**\t\t\t\t ____  ____       ____        ____        __                _                                           **");
  // xil_printf("\r\n**\t\t\t\t|  _ '|  _ '  ___/ ___|      |  _ '  ___ / _| ___ _ __   __| | ___ _ __                                 **");
  // xil_printf("\r\n**\t\t\t\t| | | | | | |/ _ '___ '  ___ | | | |/ _ ' |_ / _ ' '_ ' / _` |/ _ ' '__|                                **");
  // xil_printf("\r\n**\t\t\t\t| |_| | |_| | (_) |_ ) )|___|| |_| | |/_/  _|  __/ | | | (_| |  __/ |                                   **");
  // xil_printf("\r\n**\t\t\t\t|____/|____/ '___/____/      |____/ '___|_|  '___|_| |_|'__,_|'___|_|                                   **");
  // xil_printf("\r\n**                                                                                                                                      **");
  // xil_printf("\r\n******************************************************************************************************************************************");
  // xil_printf("\r\n                                                                                            ***** DDoS Defender by Acronics Solutions ****");
}

void clear_screen()
{
  CLS;
  CLS_BUFF
}

void mode1()
{
  // xil_printf("\r\n");
  // xil_printf("\r\n ========================================================================================================================================================================================+");
  // xil_printf("\r\n ==> Mode 1 is selected                                                                                                                                                                  |");
  // xil_printf("\r\n                                                                                                                                                                                         |");
  LoadEEPROM();
  DisplayTable();
}

void check_account()
{
  load_user();
  load_pass();
  load_passroot();
  u8 count;
  u8 key;
  // u8 countCheck = 0;
  u8 pass[16];
  u8 i;
  // while (countCheck < 3)
  // {
  // xil_printf("\r\n ===============+========================================================================================================================+");
  // xil_printf("\r\n      LOG IN    | Username: ");
  count = 0;
  key = 0;
  while (key != 13 && count < 16)
  {
    key = XUartLite_RecvByte(0x40600000);
    // xil_printf("%c",key);
    if (key > 47)
    {
      username_login[count] = key;
      count = count + 1;
      username_login[count] = 0;
    }
    else if (key == 03)
    {
      // // xil_printf("Loading Bitstream.........");
      // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 144 ,1 );
      // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 144 ,0 );
      Reset_System();
    }
  }
  // xil_printf("\r\n\t\t|                                                                                                                        |");
  // xil_printf("\r\n\t\t| Password: ");
  count = 0;
  key = 0;
  while (key != 13 && count < 16)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key > 47)
    {
      pass[count] = key;
      count = count + 1;
      pass[count] = 0;
      // xil_printf("*");
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
  // xil_printf("\r\n\t\t|                                                                                                                        |");
  // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------+");
  if ((check(user1, username_login) && check(pass1, pass)) || (check(user2, username_login) && check(pass2, pass)) || (check(user3, username_login) && check(pass3, pass)) || (check(user4, username_login) && check(pass4, pass)) || (check(user5, username_login) && check(pass5, pass)))
  {
    // while(i < MAX_USERNAME_LENGTH) {
    //     EEPROM_WriteIIC(&myDevice_EEPROM, last_user_reg + i, username_login + i, 1);
    //     delay_1ms();
    //     if(username_login[i] == 0)
    //         break;
    //     i++;
    // 	}
    XUartLite_SendByte(0x40600000, 'U');
    // delay_1s();
    // user_mode();
    // break;
  }
  else if (check("admin", username_login) && check(passroot, pass))
  {
    // while(i < MAX_USERNAME_LENGTH) {
    //     EEPROM_WriteIIC(&myDevice_EEPROM, last_user_reg + i, username_login + i, 1);
    //     delay_1ms();
    //     if(username_login[i] == 0)
    //         break;
    //     i++;
    // 	}
    XUartLite_SendByte(0x40600000, 'A');
    // delay_1s();
    // admin_mode();
    // break;
  }

  else if (countCheck == 0 || countCheck == 1)
  {
    countCheck = countCheck + 1;
    XUartLite_SendByte(0x40600000, 'F');
    // xil_printf("\r\n\t\t+========================================================================================================================+");
    // xil_printf("\r\n\t\t| Warning: Incorrect password/username. Retry!                                                                           |");
  }
  else if (countCheck == 2)
  {
    XUartLite_SendByte(0x40600000, 'Y');
    countCheck = 0;
    // break;
  }
  //}
}

void load_user()
{
  u8 *users[NUM_USERS] = {user1, user2, user3, user4, user5};
  u32 user_addrs[NUM_USERS] = {
      user1_addr,
      user2_addr,
      user3_addr,
      user4_addr,
      user5_addr,
  };

  for (u8 user_idx = 0; user_idx < NUM_USERS; user_idx++)
  {
    u8 i = 0;
    while (1)
    {
      EEPROM_ReadIIC(&myDevice_EEPROM, user_addrs[user_idx] + i, users[user_idx] + i, 1);
      delay_1ms();
      if (users[user_idx][i] == 0)
        break;
      i++;
    }
    // xil_printf("%d", user_idx + 1);
  }
}

void load_pass()
{
  u8 *pass[NUM_USERS] = {pass1, pass2, pass3, pass4, pass5};
  u32 pass_addrs[NUM_USERS] = {pass1_addr, pass2_addr, pass3_addr, pass4_addr, pass5_addr};

  for (u8 pass_idx = 0; pass_idx < NUM_USERS; pass_idx++)
  {
    u8 i = 0;
    while (1)
    {
      EEPROM_ReadIIC(&myDevice_EEPROM, pass_addrs[pass_idx] + i, pass[pass_idx] + i, 1);
      delay_1ms();
      if (pass[pass_idx][i] == 0)
        break;
      i++;
    }
    // xil_printf("%d", pass_idx + 1);
  }
}

void load_user1()
{
  u8 i = 0;
  while (1)
  {
    EEPROM_ReadIIC(&myDevice_EEPROM, user1_addr + i, user1 + i, 1);
    delay_1ms();
    // //xil_printf("%02x\r\n",*(user1+i));
    if (user1[i] == 0)
      break;
    i++;
  }
}
void load_user2()
{
  u8 i = 0;
  while (1)
  {
    EEPROM_ReadIIC(&myDevice_EEPROM, user2_addr + i, user2 + i, 1);
    delay_1ms();
    if (user2[i] == 0)
      break;
    i++;
  }
}
void load_pass1()
{
  u8 i = 0;
  while (1)
  {
    EEPROM_ReadIIC(&myDevice_EEPROM, pass1_addr + i, pass1 + i, 1);
    delay_1ms();
    if (pass1[i] == 0)
      break;
    i++;
  }
}
void load_pass2()
{
  u8 i = 0;
  while (1)
  {
    EEPROM_ReadIIC(&myDevice_EEPROM, pass2_addr + i, pass2 + i, 1);
    delay_1ms();
    if (pass2[i] == 0)
      break;
    i++;
  }
}
void load_passroot()
{
  u8 i = 0;
  while (1)
  {
    EEPROM_ReadIIC(&myDevice_EEPROM, passRoot_addr + i, passroot + i, 1);
    delay_1ms();
    // //xil_printf("%02x\r\n", *(passroot+i));
    if (passroot[i] == 0)
      break;
    i++;
  }
}

// Function compare between 2 string. if two strings are similar -> 1 else -> 0
u8 check(u8 *text1, u8 *text2)
{
  u8 count;
  count = 0;
  while (text1[count] > 0)
  {
    if (text1[count] != text2[count])
      return 0;
    count = count + 1;
  }
  if (text2[count] > 0)
    return 0;
  return 1;
}

void AssignData()
{
  Tmp_IPv6Target[4] = IPv6Target[4];
  // Tmp_IPTarget = IPTarget;
  Tmp_TimeFlood = TimeFlood1;
  Tmp_TimeDelete = TimeDelete;
  Tmp_SynThreshold = SynThreshold1;
  Tmp_AckThreshold = AckThreshold1;
  Tmp_UDPThreshold = UDPThreshold1;
  Tmp_DNSThreshold = DNSThreshold1;
  Tmp_ICMPThreshold = ICMPThreshold1;
  Tmp_IPSecThreshold = IPSecThreshold1;
  Tmp_UDPThresh_1s = UDPThresh_1s1;
  Tmp_ICMPThresh_1s = ICMPThresh_1s1;
  Tmp_DefenderPort = DefenderPort;
  Tmp_EnableDefender = EnableDefender1;

  isCompleted_AssignData = 1;
}

void LoadEEPROM()
{

  u8 Rd_IPTargetA;
  u8 Rd_IPTargetB;
  u8 Rd_IPTargetC;
  u8 Rd_IPTargetD;
  u8 Rd_IPv6Target[16];

  u8 Rd_SynThreshold_lo;
  u8 Rd_SynThreshold_hi;
  u8 Rd_AckThreshold_lo;
  u8 Rd_AckThreshold_hi;
  u8 Rd_UdpThreshold_lo;
  u8 Rd_UdpThreshold_hi;
  u8 Rd_DNSThreshold_lo;
  u8 Rd_DNSThreshold_hi;
  u8 Rd_ICMPThreshold_lo;
  u8 Rd_ICMPThreshold_hi;
  u8 Rd_IPSecThreshold_lo;
  u8 Rd_IPSecThreshold_hi;
  u8 Rd_UDPThreshold_1s_lo;
  u8 Rd_UDPThreshold_1s_hi;
  u8 Rd_ICMPThreshold_1s_lo;
  u8 Rd_ICMPThreshold_1s_hi;
  u8 Rd_EnableDefender_lo;
  u8 Rd_EnableDefender_hi;
  u8 Rd_TimeFlood_lo;
  u8 Rd_TimeFlood_hi;
  u8 Rd_TimeDelete_lo;
  u8 Rd_TimeDelete_hi;

  ////xil_printf("\r\n Waiting: 0%% - 10%% -");
  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr1, &Rd_IPv6Target[0], 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr2, &Rd_IPv6Target[1], 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr3, &Rd_IPv6Target[2], 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr4, &Rd_IPv6Target[3], 1);
  delay_1ms();

  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr5, &Rd_IPv6Target[4], 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr6, &Rd_IPv6Target[5], 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr7, &Rd_IPv6Target[6], 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr8, &Rd_IPv6Target[7], 1);
  delay_1ms();

  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr9, &Rd_IPv6Target[8], 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr10, &Rd_IPv6Target[9], 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr11, &Rd_IPv6Target[10], 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr12, &Rd_IPv6Target[11], 1);
  delay_1ms();

  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr13, &Rd_IPv6Target[12], 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr14, &Rd_IPv6Target[13], 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr15, &Rd_IPv6Target[14], 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ipv6_targer_addr16, &Rd_IPv6Target[15], 1);
  delay_1ms();

  IPv6Target[0] = (Rd_IPv6Target[0] << 24) | (Rd_IPv6Target[1] << 16) | (Rd_IPv6Target[2] << 8) | Rd_IPv6Target[3];
  IPv6Target[1] = (Rd_IPv6Target[4] << 24) | (Rd_IPv6Target[5] << 16) | (Rd_IPv6Target[6] << 8) | Rd_IPv6Target[7];
  IPv6Target[2] = (Rd_IPv6Target[8] << 24) | (Rd_IPv6Target[9] << 16) | (Rd_IPv6Target[10] << 8) | Rd_IPv6Target[11];
  IPv6Target[3] = (Rd_IPv6Target[12] << 24) | (Rd_IPv6Target[13] << 16) | (Rd_IPv6Target[14] << 8) | Rd_IPv6Target[15];

  EEPROM_ReadIIC(&myDevice_EEPROM, ip_targer_layer_a_addr, &Rd_IPTargetA, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ip_targer_layer_b_addr, &Rd_IPTargetB, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ip_targer_layer_c_addr, &Rd_IPTargetC, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ip_targer_layer_d_addr, &Rd_IPTargetD, 1);
  delay_1ms();
  // IPTarget = Rd_IPTargetA * 16777216 + Rd_IPTargetB * 65536 + Rd_IPTargetC * 256 + Rd_IPTargetD;

  EEPROM_ReadIIC(&myDevice_EEPROM, time_flood_lo_addr, &Rd_TimeFlood_lo, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, time_flood_hi_addr, &Rd_TimeFlood_hi, 1);
  delay_1ms();
  TimeFlood1 = (Rd_TimeFlood_hi << 8) + Rd_TimeFlood_lo;

  EEPROM_ReadIIC(&myDevice_EEPROM, time_delete_lo_addr, &Rd_TimeDelete_lo, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, time_delete_hi_addr, &Rd_TimeDelete_hi, 1);
  delay_1ms();
  TimeDelete = (Rd_TimeDelete_hi << 8) + Rd_TimeDelete_lo;

  ////xil_printf(" 20%% - 30%% -");
  EEPROM_ReadIIC(&myDevice_EEPROM, syn_threshold_lo_addr, &Rd_SynThreshold_lo, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, syn_threshold_hi_addr, &Rd_SynThreshold_hi, 1);
  delay_1ms();
  SynThreshold1 = (Rd_SynThreshold_hi << 8) + Rd_SynThreshold_lo;

  ////xil_printf(" 40%% - 50%% -");
  EEPROM_ReadIIC(&myDevice_EEPROM, ack_threshold_lo_addr, &Rd_AckThreshold_lo, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ack_threshold_hi_addr, &Rd_AckThreshold_hi, 1);
  delay_1ms();
  AckThreshold1 = (Rd_AckThreshold_hi << 8) + Rd_AckThreshold_lo;

  EEPROM_ReadIIC(&myDevice_EEPROM, udp_threshold_lo_addr, &Rd_UdpThreshold_lo, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, udp_threshold_hi_addr, &Rd_UdpThreshold_hi, 1);
  delay_1ms();
  UDPThreshold1 = (Rd_UdpThreshold_hi << 8) + Rd_UdpThreshold_lo;

  ////xil_printf(" 60%% - 70%% -");
  EEPROM_ReadIIC(&myDevice_EEPROM, dns_threshold_lo_addr, &Rd_DNSThreshold_lo, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, dns_threshold_hi_addr, &Rd_DNSThreshold_hi, 1);
  delay_1ms();
  DNSThreshold1 = (Rd_DNSThreshold_hi << 8) + Rd_DNSThreshold_lo;

  EEPROM_ReadIIC(&myDevice_EEPROM, icmp_threshold_lo_addr, &Rd_ICMPThreshold_lo, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, icmp_threshold_hi_addr, &Rd_ICMPThreshold_hi, 1);
  delay_1ms();
  ICMPThreshold1 = (Rd_ICMPThreshold_hi << 8) + Rd_ICMPThreshold_lo;

  EEPROM_ReadIIC(&myDevice_EEPROM, ipsec_threshold_lo_addr, &Rd_IPSecThreshold_lo, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, ipsec_threshold_hi_addr, &Rd_IPSecThreshold_hi, 1);
  delay_1ms();
  IPSecThreshold1 = (Rd_IPSecThreshold_hi << 8) + Rd_IPSecThreshold_lo;

  ////xil_printf(" 80%% - 90%% -");
  EEPROM_ReadIIC(&myDevice_EEPROM, udp_threshold1s_lo_addr, &Rd_UDPThreshold_1s_lo, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, udp_threshold1s_hi_addr, &Rd_UDPThreshold_1s_hi, 1);
  delay_1ms();
  UDPThresh_1s1 = (Rd_UDPThreshold_1s_hi << 8) + Rd_UDPThreshold_1s_lo;

  EEPROM_ReadIIC(&myDevice_EEPROM, icmp_threshold1s_lo_addr, &Rd_ICMPThreshold_1s_lo, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, icmp_threshold1s_hi_addr, &Rd_ICMPThreshold_1s_hi, 1);
  delay_1ms();
  ICMPThresh_1s1 = (Rd_ICMPThreshold_1s_hi << 8) + Rd_ICMPThreshold_1s_lo;

  EEPROM_ReadIIC(&myDevice_EEPROM, enable_defender_lo_addr, &Rd_EnableDefender_lo, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, enable_defender_hi_addr, &Rd_EnableDefender_hi, 1);
  EnableDefender1 = (Rd_EnableDefender_hi << 8) + Rd_EnableDefender_lo;

  delay_1ms();
  ////xil_printf(" 100%%");
}

void LoadEEPROM_defender()
{
  u8 Rd_EnableDefender_lo;
  u8 Rd_EnableDefender_hi;

  EEPROM_ReadIIC(&myDevice_EEPROM, enable_defender_lo_addr, &Rd_EnableDefender_lo, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, enable_defender_hi_addr, &Rd_EnableDefender_hi, 1);

  EnableDefender1 = (Rd_EnableDefender_hi << 8) + Rd_EnableDefender_lo;

  delay_1ms();

  // EnableDefender1 = (HTTPSGETFlood_en1 << 13) + (HTTPGETFlood_en1 << 12) + (DefenderPort_en_5 << 11) + (DefenderPort_en_4 << 10) + (DefenderPort_en_1 << 9) + (DefenderPort_en1 << 8) + (UDPFragFlood_en1 << 7) + (TCPFragFlood_en1 << 6) + (IPSecFlood_en1 << 5) + (ICMPFlood_en1 << 4) + (DNSFlood_en1 << 3) + (UDPFlood_en1 << 2) + (LANDATTACK_en1 << 1) + SYNFlood_en1;

  HTTPSGETFlood_en1 = (EnableDefender1 >> 13) % 2;
  HTTPGETFlood_en1 = (EnableDefender1 >> 12) % 2;
  DefenderPort_en_5 = (EnableDefender1 >> 11) % 2;
  DefenderPort_en_4 = (EnableDefender1 >> 10) % 2;
  DefenderPort_en_1 = (EnableDefender1 >> 9) % 2;
  DefenderPort_en1 = (EnableDefender1 >> 8) % 2;
  UDPFragFlood_en1 = (EnableDefender1 >> 7) % 2;
  TCPFragFlood_en1 = (EnableDefender1 >> 6) % 2;
  IPSecFlood_en1 = (EnableDefender1 >> 5) % 2;
  ICMPFlood_en1 = (EnableDefender1 >> 4) % 2;
  DNSFlood_en1 = (EnableDefender1 >> 3) % 2;
  UDPFlood_en1 = (EnableDefender1 >> 2) % 2;
  LANDATTACK_en1 = (EnableDefender1 >> 1) % 2;
  SYNFlood_en1 = EnableDefender1 % 2;
}

void SaveEEPROM()
{
  int Tmp_IPv6Target[4];
  u8 Wr_IPTargetA;
  u8 Wr_IPTargetB;
  u8 Wr_IPTargetC;
  u8 Wr_IPTargetD;
  u8 Wr_SynThreshold_lo;
  u8 Wr_SynThreshold_hi;
  u8 Wr_AckThreshold_lo;
  u8 Wr_AckThreshold_hi;
  u8 Wr_UdpThreshold_lo;
  u8 Wr_UdpThreshold_hi;
  u8 Wr_DNSThreshold_lo;
  u8 Wr_DNSThreshold_hi;
  u8 Wr_ICMPThreshold_lo;
  u8 Wr_ICMPThreshold_hi;
  u8 Wr_IPSecThreshold_lo;
  u8 Wr_IPSecThreshold_hi;
  u8 Wr_UDPThreshold_1s_lo;
  u8 Wr_UDPThreshold_1s_hi;
  u8 Wr_ICMPThreshold_1s_lo;
  u8 Wr_ICMPThreshold_1s_hi;
  u8 Wr_EnableDefender_lo;
  u8 Wr_EnableDefender_hi;
  u8 Wr_TimeFlood_lo;
  u8 Wr_TimeFlood_hi;
  u8 Wr_TimeDelete_lo;
  u8 Wr_TimeDelete_hi;

  ////xil_printf("\r\n");
  ////xil_printf("\r\n\t\t| Waiting: 0%% - 10%% - ");
  Wr_IPTargetA = (u8)(Tmp_IPTarget >> 24);
  Wr_IPTargetB = (u8)(Tmp_IPTarget >> 16);
  Wr_IPTargetC = (u8)(Tmp_IPTarget >> 8);
  Wr_IPTargetD = (u8)(Tmp_IPTarget);

  for (int i = 0; i < 4; i++)
  {
    Tmp_IPv6Target[i] = IPv6Target[i];
  }

  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr1, (u8 *)&Tmp_IPv6Target[0] + 3, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr2, (u8 *)&Tmp_IPv6Target[0] + 2, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr3, (u8 *)&Tmp_IPv6Target[0] + 1, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr4, (u8 *)&Tmp_IPv6Target[0], 1);
  delay_1ms();

  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr5, (u8 *)&Tmp_IPv6Target[1] + 3, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr6, (u8 *)&Tmp_IPv6Target[1] + 2, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr7, (u8 *)&Tmp_IPv6Target[1] + 1, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr8, (u8 *)&Tmp_IPv6Target[1], 1);
  delay_1ms();

  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr9, (u8 *)&Tmp_IPv6Target[2] + 3, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr10, (u8 *)&Tmp_IPv6Target[2] + 2, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr11, (u8 *)&Tmp_IPv6Target[2] + 1, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr12, (u8 *)&Tmp_IPv6Target[2], 1);
  delay_1ms();

  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr13, (u8 *)&Tmp_IPv6Target[3] + 3, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr14, (u8 *)&Tmp_IPv6Target[3] + 2, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr15, (u8 *)&Tmp_IPv6Target[3] + 1, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ipv6_targer_addr16, (u8 *)&Tmp_IPv6Target[3], 1);
  delay_1ms();
  //////////////////

  EEPROM_WriteIIC(&myDevice_EEPROM, ip_targer_layer_a_addr, &Wr_IPTargetA, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ip_targer_layer_b_addr, &Wr_IPTargetB, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ip_targer_layer_c_addr, &Wr_IPTargetC, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ip_targer_layer_d_addr, &Wr_IPTargetD, 1);
  delay_1ms();
  ////xil_printf("20%% - 30%% - ");

  Wr_TimeFlood_lo = (u8)(Tmp_TimeFlood);
  Wr_TimeFlood_hi = (u8)(Tmp_TimeFlood >> 8);
  EEPROM_WriteIIC(&myDevice_EEPROM, time_flood_lo_addr, &Wr_TimeFlood_lo, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, time_flood_hi_addr, &Wr_TimeFlood_hi, 1);
  delay_1ms();

  Wr_TimeDelete_lo = (u8)(Tmp_TimeDelete);
  Wr_TimeDelete_hi = (u8)(Tmp_TimeDelete >> 8);
  EEPROM_WriteIIC(&myDevice_EEPROM, time_delete_lo_addr, &Wr_TimeDelete_lo, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, time_delete_hi_addr, &Wr_TimeDelete_hi, 1);
  delay_1ms();

  Wr_SynThreshold_lo = (u8)(Tmp_SynThreshold);
  Wr_SynThreshold_hi = (u8)(Tmp_SynThreshold >> 8);
  EEPROM_WriteIIC(&myDevice_EEPROM, syn_threshold_lo_addr, &Wr_SynThreshold_lo, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, syn_threshold_hi_addr, &Wr_SynThreshold_hi, 1);
  delay_1ms();

  ////xil_printf("40%% - 50%% - ");
  Wr_AckThreshold_lo = (u8)(Tmp_AckThreshold);
  Wr_AckThreshold_hi = (u8)(Tmp_AckThreshold >> 8);
  EEPROM_WriteIIC(&myDevice_EEPROM, ack_threshold_lo_addr, &Wr_AckThreshold_lo, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ack_threshold_hi_addr, &Wr_AckThreshold_hi, 1);
  delay_1ms();

  ////xil_printf("60%% - 70%% - ");
  Wr_UdpThreshold_lo = (u8)(Tmp_UDPThreshold);
  Wr_UdpThreshold_hi = (u8)(Tmp_UDPThreshold >> 8);
  EEPROM_WriteIIC(&myDevice_EEPROM, udp_threshold_lo_addr, &Wr_UdpThreshold_lo, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, udp_threshold_hi_addr, &Wr_UdpThreshold_hi, 1);
  delay_1ms();

  ////xil_printf("80%% - 90%% - ");
  Wr_DNSThreshold_lo = (u8)(Tmp_DNSThreshold);
  Wr_DNSThreshold_hi = (u8)(Tmp_DNSThreshold >> 8);
  EEPROM_WriteIIC(&myDevice_EEPROM, dns_threshold_lo_addr, &Wr_DNSThreshold_lo, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, dns_threshold_hi_addr, &Wr_DNSThreshold_hi, 1);
  delay_1ms();

  Wr_ICMPThreshold_lo = (u8)(Tmp_ICMPThreshold);
  Wr_ICMPThreshold_hi = (u8)(Tmp_ICMPThreshold >> 8);
  EEPROM_WriteIIC(&myDevice_EEPROM, icmp_threshold_lo_addr, &Wr_ICMPThreshold_lo, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, icmp_threshold_hi_addr, &Wr_ICMPThreshold_hi, 1);
  delay_1ms();

  Wr_IPSecThreshold_lo = (u8)(Tmp_IPSecThreshold);
  Wr_IPSecThreshold_hi = (u8)(Tmp_IPSecThreshold >> 8);
  EEPROM_WriteIIC(&myDevice_EEPROM, ipsec_threshold_lo_addr, &Wr_IPSecThreshold_lo, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, ipsec_threshold_hi_addr, &Wr_IPSecThreshold_hi, 1);
  delay_1ms();

  Wr_UDPThreshold_1s_lo = (u8)(Tmp_UDPThresh_1s);
  Wr_UDPThreshold_1s_hi = (u8)(Tmp_UDPThresh_1s >> 8);
  EEPROM_WriteIIC(&myDevice_EEPROM, udp_threshold1s_lo_addr, &Wr_UDPThreshold_1s_lo, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, udp_threshold1s_hi_addr, &Wr_UDPThreshold_1s_hi, 1);
  delay_1ms();

  Wr_ICMPThreshold_1s_lo = (u8)(Tmp_ICMPThresh_1s);
  Wr_ICMPThreshold_1s_hi = (u8)(Tmp_ICMPThresh_1s >> 8);
  EEPROM_WriteIIC(&myDevice_EEPROM, icmp_threshold1s_lo_addr, &Wr_ICMPThreshold_1s_lo, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, icmp_threshold1s_hi_addr, &Wr_ICMPThreshold_1s_hi, 1);
  delay_1ms();

  Wr_EnableDefender_lo = (u8)(Tmp_EnableDefender);
  Wr_EnableDefender_hi = (u8)(Tmp_EnableDefender >> 8);
  EEPROM_WriteIIC(&myDevice_EEPROM, enable_defender_lo_addr, &Wr_EnableDefender_lo, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, enable_defender_hi_addr, &Wr_EnableDefender_hi, 1);
  delay_1ms();
  ////xil_printf("100%% ");
  isCompleted_SaveEEPROM = 1;
}

void user_mode()
{
  u8 key, key1 = 0;
  u8 count;
start:
  LoadEEPROM();
  DisplayTable();
  // xil_printf("\r\n\t\t===============+===========+=========================== Session-User ============================================================================+\r\n");
  // 			xil_printf("    DISPLAY     |           |                                                                                                            |\r\n");
  // 						xil_printf("\t\t| Key Enter | Please choose 1 option below:                                                                              |\r\n");
  // 						xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
  // 						xil_printf("\t\t|     1.    | Reconfig Anti-DDoS                                                                                         |\r\n");
  // 						xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
  // 						xil_printf("\t\t|     2.    | Change Password account                                                                                |\r\n");
  // 						xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
  // 						xil_printf("\t\t|     3.    | Exit                                                                                                       |\r\n");
  // 						xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
  // 						xil_printf("\t\tPlease choose mode:  ");
  count = 0;
  while (key != 13 || count < 1)
  {
    key = XUartLite_RecvByte(0x40600000);
    // if ((key < 49 || key > 52)){
    // }
    if (key != 13 && count == 0)
    {
      // xil_printf("%c",key);
      key1 = key;
      count = 1;
    }
  }
  if (key1 == '1')
  {
    XUartLite_SendByte(0x40600000, '1');
    reconfig();
    delay_1s();
    ;
    goto start;
  }
  else if (key1 == '2')
  {
    XUartLite_SendByte(0x40600000, '2');
    user_change_info();
    goto start;
  }
  else if (key1 == '3')
  {

    XUartLite_SendByte(0x40600000, '3');
    delay_1s();
    show_info_SDcard();
    goto start;
  }
  else if (key1 == '4')
  {
    XUartLite_SendByte(0x40600000, '4');
    // delay_1s();
  }
  else if (key1 == 03)
  {
    Reset_System();
  }
}

void admin_mode()
{
  u8 key, key1 = 0;
  u8 count;
start_check_account:
  // LoadEEPROM();
  // DisplayTable();
  //  displaytable_admin_mode();
  //  DisplayTable();
  //   xil_printf("\r\n\t\t===============+===========+=========================== Session-Admin ============================================================================+\r\n");
  //   			xil_printf("    DISPLAY     |           |                                                                                                            |\r\n");
  //   						xil_printf("\t\t| Key Enter | Please choose 1 option below:                                                                              |\r\n");
  //   						xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
  //   						xil_printf("\t\t|     1.    | Reconfig Anti-DDoS                                                                                         |\r\n");
  //   						xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
  //   						xil_printf("\t\t|     2.    | Change informations account                                                                                |\r\n");
  //   						xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
  //   						xil_printf("\t\t|     3.    | Exit                                                                                                       |\r\n");
  //   						xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
  //   						xil_printf("\t\tPlease choose mode:  ");
  count = 0;
  while (key != 13 || count < 1)
  {
    key = XUartLite_RecvByte(0x40600000);
    // if ((key < 49 || key > 52)){
    // }
    if (key != 13 && count == 0)
    {
      // xil_printf("%c",key);
      key1 = key;
      count = 1;
    }
  }
  if (key1 == '1')
  {
    // XUartLite_SendByte(0x40600000, '1');
    reconfig();
    // delay_1s();
    goto start_check_account;
  }
  else if (key1 == '2')
  {
    // XUartLite_SendByte(0x40600000, '2');
    admin_change_info();
    // delay_1s();
    goto start_check_account;
  }
  else if (key1 == '3')
  {
    // XUartLite_SendByte(0x40600000, '3');
    // delay_1s();
    show_info_SDcard();
    goto start_check_account;
  }
  else if (key1 == '4')
  {
    XUartLite_SendByte(0x40600000, '4');
    // delay_1s();
  }
  else if (key1 == 03)
  {
    Reset_System();
  }
}

void DisplayTable()
{
  LoadEEPROM();
  ConvertTimestamp();
  xil_printf("\r\n ============================================================================================================================================================================================================+");
  xil_printf("\r\n ------------------------------------------------------------------------------------ System Configuration --------------------------------------------------------------------------------------------------+");
  xil_printf("\r\n ---------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
  xil_printf("\r\n     DISPLAY    | Company                : Acronics Solutions                                                                                                                            		     |");
  xil_printf("\r\n\t\t| Device                 : DDoS Defender                                                                                                                                 		     |");
  xil_printf("\r\n\t\t| Model                  : Bandwidth 1Gbps                                                                                                                               		     |");
  xil_printf("\r\n\t\t| Version                : 1.0                                                                                                                                           		     |");
  xil_printf("\r\n\t\t| Current date and time  : %2dh.%2dm.%2ds  %2d.%2d.%4d ", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
  // xil_printf("\r\n%4d.%2d.%2d.%2d.%2d.%2d",timeinfo->tm_year + 1900,timeinfo->tm_mon + 1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
  // xil_printf("\t\t\t\t\t\t\t\t\t Hello!: %-16s",username_login);
  xil_printf("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t     |");
  if (EnableDefender1 == 0)
  {
    xil_printf("\r\n\t\t| +==========================================================================================================================================================================================+");
    xil_printf("\r\n\t\t| | DDoS Defender status            :  OFF.                                                                                                                               		     |");
    xil_printf("\r\n\t\t| +==========================================================================================================================================================================================+");
    xil_printf("\r\n\t\t| | SYN Flood | LAND ATTACK |  UDP Flood  |  DNS Amplification  |  ICMP Flood  | IPSec IKE Flood  | TCP Fragmentation Flood  | UDP Fragmentation Flood  |  HTTP GET Flood  | HTTPS GET Flood |");
    xil_printf("\r\n\t\t| +------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
    xil_printf("\r\n\t\t| | %8s  |   %8s  |   %8s  |      %8s       |  %8s    |   %8s       |         %8s         |         %8s         |     %8s     |     %8s    |", (EnableDefender1 % 2) ? "Enable" : "Disable",
               ((EnableDefender1 >> 1) % 2) ? "Enable" : "Disable", ((EnableDefender1 >> 2) % 2) ? "Enable" : "Disable", ((EnableDefender1 >> 3) % 2) ? "Enable" : "Disable", ((EnableDefender1 >> 4) % 2) ? "Enable" : "Disable",
               ((EnableDefender1 >> 5) % 2) ? "Enable" : "Disable", ((EnableDefender1 >> 6) % 2) ? "Enable" : "Disable", ((EnableDefender1 >> 7) % 2) ? "Enable" : "Disable", ((EnableDefender1 >> 10) % 2) ? "Enable" : "Disable", ((EnableDefender1 >> 11) % 2) ? "Enable" : "Disable");
    xil_printf("\r\n\t\t| +------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------");
    XUartLite_SendByte(0x40600000, '*');
  }
  else
  {
    xil_printf("\r\n\t\t| +==========================================================================================================================================================================================+");
    xil_printf("\r\n\t\t| | DDoS Defender status            :  ON.                                                                                                                               		     |");
    xil_printf("\r\n\t\t| +==========================================================================================================================================================================================+");
    xil_printf("\r\n\t\t| | SYN Flood | LAND ATTACK |  UDP Flood  |  DNS Amplification  |  ICMP Flood  | IPSec IKE Flood  | TCP Fragmentation Flood  | UDP Fragmentation Flood  |  HTTP GET Flood  | HTTPS GET Flood |");
    xil_printf("\r\n\t\t| +------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
    xil_printf("\r\n\t\t| | %8s  |   %8s  |   %8s  |      %8s       |  %8s    |   %8s       |         %8s         |         %8s         |     %8s     |     %8s    |", (EnableDefender1 % 2) ? "Enable" : "Disable",
               ((EnableDefender1 >> 1) % 2) ? "Enable" : "Disable", ((EnableDefender1 >> 2) % 2) ? "Enable" : "Disable", ((EnableDefender1 >> 3) % 2) ? "Enable" : "Disable", ((EnableDefender1 >> 4) % 2) ? "Enable" : "Disable",
               ((EnableDefender1 >> 5) % 2) ? "Enable" : "Disable", ((EnableDefender1 >> 6) % 2) ? "Enable" : "Disable", ((EnableDefender1 >> 7) % 2) ? "Enable" : "Disable", ((EnableDefender1 >> 10) % 2) ? "Enable" : "Disable", ((EnableDefender1 >> 11) % 2) ? "Enable" : "Disable");
    xil_printf("\r\n\t\t| +------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------");
    xil_printf("\r\n\t\t|                                                                                                                                                                        		     |");
    if (((EnableDefender1 >> 8) % 2) == 1)
    {
      xil_printf("\r\n\t\t| +=============================================================================+                                                                                                            |");
      // xil_printf("\r\n\t\t| | Server IP: %3d.%3d.%3d.%3d                                                  |                                                                                                            |", ipv4_1);
      // xil_printf("\r\n\t\t| | Server IP: %3d.%3d.%3d.%3d                                                  |                                                                                                            |", ipv4_2);
      // xil_printf("\r\n\t\t| | Server IP: %3d.%3d.%3d.%3d                                                  |                                                                                                            |",ipv4_3 );
      // xil_printf("\r\n\t\t| | Server IP: %3d.%3d.%3d.%3d                                                  |                                                                                                            |", ipv4_1);

      xil_printf("\r\n\t\t| |                                                                             |                                                                                                            |");
      xil_printf("\r\n\t\t| | Server IPv6: %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x                        |                                                                                                            |",
                 (u16)(IPv6Target[0] >> 16), (u16)(IPv6Target[0] & 0xFFFF),
                 (u16)(IPv6Target[1] >> 16), (u16)(IPv6Target[1] & 0xFFFF),
                 (u16)(IPv6Target[2] >> 16), (u16)(IPv6Target[2] & 0xFFFF),
                 (u16)(IPv6Target[3] >> 16), (u16)(IPv6Target[3] & 0xFFFF));
    }
    else if (((EnableDefender1 >> 8) % 2) == 0)
    {
      xil_printf("\r\n\t\t| +=============================================================================+                                                                                        		     |");
      xil_printf("\r\n\t\t| | Client port number (Internet side): %d                                       |                                                                                        		     |", (((EnableDefender1 >> 9) % 2) + 1));
      xil_printf("\r\n\t\t| |                                                                             |                                                                                        		     |");
    }
    xil_printf("\r\n\t\t| +==============================================================+==============+                                                                                        		     |");
    xil_printf("\r\n\t\t| |                    Configured Parameters                     |     Valid    |                                                                                        		     |");
    xil_printf("\r\n\t\t| +==============================================================+==============+                                                                                        		     |");
    xil_printf("\r\n\t\t| | Attack detection time (seconds):                             |  %-10d  |                                                                                        		     |", TimeFlood1);
    xil_printf("\r\n\t\t| +==============================================================+==============+                                                                                        		     |");
    xil_printf("\r\n\t\t| | SYN Flood attack detection throughput (PPS):                 |  %-10d  |                                                                                        		     |", SynThreshold1);
    xil_printf("\r\n\t\t| +--------------------------------------------------------------+--------------+                                                                                        		     |");
    xil_printf("\r\n\t\t| | ACK Flood attack detection throughput (PPS):                 |  %-10d  |                                                                                        		     |", AckThreshold1);
    xil_printf("\r\n\t\t| +--------------------------------------------------------------+--------------+                                                                                        		     |");
    xil_printf("\r\n\t\t| | Delete information white list time (seconds):                |  %-10d  |                                                                                        		     |", TimeDelete);
    xil_printf("\r\n\t\t| +==============================================================+==============+                                                                                        		     |");
    xil_printf("\r\n\t\t| | UDP Flood attack detection throughput (PPS):                 |  %-10d  |                                                                                        		     |", UDPThreshold1);
    xil_printf("\r\n\t\t| +--------------------------------------------------------------+--------------+                                                                                        		     |");
    xil_printf("\r\n\t\t| | Valid UDP packet throughput allowed (PPS):                   |  %-10d  |                                                                                        		     |", UDPThresh_1s1);
    xil_printf("\r\n\t\t| +==============================================================+==============+                                                                                        		     |");
    xil_printf("\r\n\t\t| | DNS Amplification attack detection throughput (PPS):         |  %-10d  |                                                                                        		     |", DNSThreshold1);
    xil_printf("\r\n\t\t| +==============================================================+==============+                                                                                        		     |");
    xil_printf("\r\n\t\t| | ICMP Flood attack detection throughput (PPS):                |  %-10d  |                                                                                        		     |", ICMPThreshold1);
    xil_printf("\r\n\t\t| +--------------------------------------------------------------+--------------+                                                                                        		     |");
    xil_printf("\r\n\t\t| | Valid ICMP packet throughput allowed (PPS):                  |  %-10d  |                                                                                        		     |", ICMPThresh_1s1);
    xil_printf("\r\n\t\t| +==============================================================+==============+                                                                                        		     |");
    xil_printf("\r\n\t\t| | IPSec Flood attack detection throughput (PPS):               |  %-10d  |                                                                                        		     |", IPSecThreshold1);
    xil_printf("\r\n\t\t| +==============================================================+==============+                                                                                        		     |");
    xil_printf("\r\n----------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------");
    XUartLite_SendByte(0x40600000, '*');
  }
}

void admin_change_info()
{
  u8 key1 = 0;
  while (key1 != 8)
  {
    // clear_screen();
    // display_logo2();
    // xil_printf("\r\n ========================================================================================================================================+");
    // xil_printf("\r\n ==> Mode 3 is selected.                                                                                                                 |");
    // xil_printf("\r\n                                                                                                                                         |");
    // xil_printf("\r\n================+===========+==========================================+=================================================================+");
    // xil_printf("\r\n    DISPLAY     |           |                                          |                                                                 |");
    // xil_printf("\r\n\t\t| Key Enter |           Choose 1 option below:         |                                                                 |");
    // xil_printf("\r\n\t\t+-----------+------------------------------------------+                                                                 |");
    // xil_printf("\r\n\t\t|     1.    | Add new user account.                    |                                                                 |");
    // xil_printf("\r\n\t\t+-----------+------------------------------------------+                                                                 |");
    // xil_printf("\r\n\t\t|     2.    | Display saved user account.              |                                                                 |");
    // xil_printf("\r\n\t\t+-----------+------------------------------------------+                                                                 |");
    // xil_printf("\r\n\t\t|     3.    | Delete user account.                     |                                                                 |");
    // xil_printf("\r\n\t\t+-----------+------------------------------------------+                                                                 |");
    // xil_printf("\r\n\t\t|     4.    | Reset all user accounts.                 |                                                                 |");
    // xil_printf("\r\n\t\t+-----------+------------------------------------------+                                                                 |");
    // xil_printf("\r\n\t\t|     5.    | Change root password.                    |                                                                 |");
    // xil_printf("\r\n\t\t+-----------+------------------------------------------+                                                                 |");
    // xil_printf("\r\n\t\t|     6.    | Load default setting from manufacturer.  |                                                                 |");
    // xil_printf("\r\n\t\t+-----------+------------------------------------------+                                                                 |");
    // xil_printf("\r\n\t\t|     7.    | ==> Exit.                                |                                                                 |");
    // xil_printf("\r\n\t\t+-----------+------------------------------------------+                                                                 |");
    // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------+");
    // xil_printf("\r\n================+========================================================================================================================+");
    // xil_printf("\r\n    SETTING     |  --> Your choice: "); delay_1s();

    u8 choice = 0;
    u8 count = 0;
    u8 key2 = 0;
    while (key1 != 13 || count < 1)
    {
      key1 = XUartLite_RecvByte(0x40600000);
      if (key1 != 13 && count == 0)
      {
        key2 = key1;
        // xil_printf("%c",key1);
        choice = 10 * choice + key1;
        count = 1;
      }
    }
    // xil_printf("\r\n-----------------------------------------------------------------------------------------------------------------------------------------+");
    key1 = choice;

    if (key1 == '1')
    {
      // XUartLite_SendByte(0x40600000, '1');
      // delay_1s();
      add_acount();
    }
    else if (key1 == '2')
    {
      u8 count = 0;
      key = 0;
      while (count == 0)
      {
        // XUartLite_SendByte(0x40600000, '2');
        // delay_1s();
        // DisplayAccount();
        ReturnMode3();
        count = 1;
        // break;
      }
    }
    else if (key1 == '7')
    {
      // XUartLite_SendByte(0x40600000, '7');
      // delay_1s();
      LoadEEPROM();
      // DisplayTable();
      break;
    }
    else if (key1 == '0')
    {
      // XUartLite_SendByte(0x40600000, '0');
      // delay_1s();
      change_user_pass_admin_mode();
    }
    else if (key1 == '6')
    {
      // XUartLite_SendByte(0x40600000, '6');
      // delay_1s();
      setload_default();
      // break;
    }

    else if (key1 == '5')
    {
      // XUartLite_SendByte(0x40600000, '5');
      // delay_1s();
      change_root();
    }
    else if (key1 == '3')
    {
      // XUartLite_SendByte(0x40600000, '3');
      // delay_1s();
      delete_account();
    }
    else if (key1 == '4')
    {
      // XUartLite_SendByte(0x40600000, '4');
      // delay_1s();
      reset_account();
    }
    else if (key2 == 03)
    {
      // xil_printf("Loading Bitstream.........");
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 144, 1);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 144, 0);
    }
  }
}

void add_acount()
{
  // DisplayAccount();
  u8 count_u, count_p;
  u8 i;
  count_u = 0;
  count_p = 0;
  u8 *username[16];
  u8 *password[16];
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================+");
  // xil_printf("\r\n    Account     | Enter Username: ");
  key = 0;
  while (key != 13 || count_u == 0)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key > 47 && count_u < 15)
    {
      // xil_printf("%c",key);
      username[count_u] = key;
      count_u = count_u + 1;
      username[count_u] = 0;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
  // xil_printf("\r\n\t\t|                                                                                                                        |");
  // xil_printf("\r\n\t\t| Enter Password: ");

  while (key != 13 || count_p == 0)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key > 47 && count_p < 15)
    {
      // xil_printf("%c",key);
      password[count_p] = key;
      count_p = count_p + 1;
      password[count_p] = 0;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }

  // xil_printf("\r\n\t\t| --> Please wait . ");
  u8 rd_user1;
  u8 rd_user2;
  u8 rd_user3;
  u8 rd_user4;
  u8 rd_user5;
  u8 rd_user6;
  u8 rd_user7;

  EEPROM_ReadIIC(&myDevice_EEPROM, user1_addr, &rd_user1, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, user2_addr, &rd_user2, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, user3_addr, &rd_user3, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, user4_addr, &rd_user4, 1);
  delay_1ms();
  EEPROM_ReadIIC(&myDevice_EEPROM, user5_addr, &rd_user5, 1);
  delay_1ms();

  if (rd_user1 == 0)
  {
    for (i = 0; i <= count_p; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, pass1_addr + i, password + i, 1);
      delay_1ms();
    }
    for (i = 0; i <= count_u; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, user1_addr + i, username + i, 1);
      delay_1ms();
    }
    isCompleted_add_account = 1;
  }
  else if (rd_user2 == 0)
  {
    for (i = 0; i <= count_p; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, pass2_addr + i, password + i, 1);
      delay_1ms();
    }
    for (i = 0; i <= count_u; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, user2_addr + i, username + i, 1);
      delay_1ms();
    }
    // XUartLite_SendByte(0x40600000, 'Y');
    isCompleted_add_account = 1;
  }
  else if (rd_user3 == 0)
  {
    for (i = 0; i <= count_p; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, pass3_addr + i, password + i, 1);
      delay_1ms();
    }
    for (i = 0; i <= count_u; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, user3_addr + i, username + i, 1);
      delay_1ms();
    }
    //	XUartLite_SendByte(0x40600000, 'Y');
    isCompleted_add_account = 1;
  }
  else if (rd_user4 == 0)
  {
    for (i = 0; i <= count_p; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, pass4_addr + i, password + i, 1);
      delay_1ms();
    }
    for (i = 0; i <= count_u; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, user4_addr + i, username + i, 1);
      delay_1ms();
    }
    //	XUartLite_SendByte(0x40600000, 'Y');
    isCompleted_add_account = 1;
  }

  else if (rd_user5 == 0)
  {
    for (i = 0; i <= count_p; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, pass5_addr + i, password + i, 1);
      delay_1ms();
    }
    for (i = 0; i <= count_u; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, user5_addr + i, username + i, 1);
      delay_1ms();
    }
    // XUartLite_SendByte(0x40600000, 'Y');
    isCompleted_add_account = 1;
  }

  else if (rd_user1 == 0 && check(user2, username))
  {
    //	XUartLite_SendByte(0x40600000, 'V');
    isCompleted_add_account = 0;
  }
  else if (rd_user2 == 0 && check(user1, username))
  {
    //	XUartLite_SendByte(0x40600000, 'V');
    isCompleted_add_account = 0;
  }
  else
  {
    // load_user1();
    // load_user2();
    // load_user3();
    // load_user4();
    load_user();
    if (check(user1, username))
    {
      for (i = 0; i <= count_p; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, pass1_addr + i, password + i, 1);
        delay_1ms();
      }
      for (i = 0; i <= count_u; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, user1_addr + i, username + i, 1);
        delay_1ms();
      }
      //	XUartLite_SendByte(0x40600000, 'G');
      isCompleted_add_account = 0;
    }
    else if (check(user2, username))
    {
      for (i = 0; i <= count_p; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, pass2_addr + i, password + i, 1);
        delay_1ms();
      }
      for (i = 0; i <= count_u; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, user2_addr + i, username + i, 1);
        delay_1ms();
      }
      // XUartLite_SendByte(0x40600000, 'G');
      isCompleted_add_account = 0;
    }
    else if (check(user3, username))
    {
      for (i = 0; i <= count_p; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, pass3_addr + i, password + i, 1);
        delay_1ms();
      }
      for (i = 0; i <= count_u; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, user3_addr + i, username + i, 1);
        delay_1ms();
      }
      // XUartLite_SendByte(0x40600000, 'G');
      isCompleted_add_account = 0;
    }
    else if (check(user4, username))
    {
      for (i = 0; i <= count_p; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, pass4_addr + i, password + i, 1);
        delay_1ms();
      }
      for (i = 0; i <= count_u; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, user4_addr + i, username + i, 1);
        delay_1ms();
      }
      // XUartLite_SendByte(0x40600000, 'G');
      isCompleted_add_account = 0;
    }

    else if (check(user5, username))
    {
      for (i = 0; i <= count_p; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, pass5_addr + i, password + i, 1);
        delay_1ms();
      }
      for (i = 0; i <= count_u; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, user5_addr + i, username + i, 1);
        delay_1ms();
      }
      // XUartLite_SendByte(0x40600000, 'G');
      isCompleted_add_account = 0;
    }
    else
    {
      //	XUartLite_SendByte(0x40600000, 'F');
      isCompleted_add_account = 0;
      // xil_printf("\r\n\t\t|                                                                                                                        |");
      // xil_printf("\r\n\t\t+========================================================================================================================+");
      // xil_printf("\r\n\t\t| Cannot add new user. The number of user accounts is full.                                                              |");
      // xil_printf("\r\n\t\t|                                                                                                                        |");
      // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------+");
    }
  }
  // //clear_screen();
  // display_logo2();
  // delay_1s();
  // DisplayAccount();
  ReturnMode3();
}

void delete_account()
{
  // DisplayAccount();
  //  xil_printf("\r\n");
  //  xil_printf("\r\n================+========================================================================================================================+");
  //  xil_printf("\r\n    Account     | Enter an account username account need to be deleted: ");
  u8 del_user[16];
  u8 count = 0;
  while (key != 13 || count == 0)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key > 47 && count < 15)
    {
      // xil_printf("%c",key);
      del_user[count] = key;
      count = count + 1;
      del_user[count] = 0;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
  load_user();
  if (check(user1, del_user))
  {
    EEPROM_WriteIIC(&myDevice_EEPROM, user1_addr, &zero, 1);
    delay_1ms();
    EEPROM_WriteIIC(&myDevice_EEPROM, pass1_addr, &zero, 1);
    isCompleted_delete_account = 1;
    //	XUartLite_SendByte(0x40600000, 'Y');
  }
  else if (check(user2, del_user))
  {
    EEPROM_WriteIIC(&myDevice_EEPROM, user2_addr, &zero, 1);
    delay_1ms();
    EEPROM_WriteIIC(&myDevice_EEPROM, pass2_addr, &zero, 1);
    isCompleted_delete_account = 1;
    //	XUartLite_SendByte(0x40600000, 'Y');
  }
  else if (check(user3, del_user))
  {
    EEPROM_WriteIIC(&myDevice_EEPROM, user3_addr, &zero, 1);
    delay_1ms();
    EEPROM_WriteIIC(&myDevice_EEPROM, pass3_addr, &zero, 1);
    isCompleted_delete_account = 1;
    //	XUartLite_SendByte(0x40600000, 'Y');
  }
  else if (check(user4, del_user))
  {
    EEPROM_WriteIIC(&myDevice_EEPROM, user4_addr, &zero, 1);
    delay_1ms();
    EEPROM_WriteIIC(&myDevice_EEPROM, pass4_addr, &zero, 1);
    isCompleted_delete_account = 1;
    //	XUartLite_SendByte(0x40600000, 'Y');
  }
  else if (check(user5, del_user))
  {
    EEPROM_WriteIIC(&myDevice_EEPROM, user5_addr, &zero, 1);
    delay_1ms();
    EEPROM_WriteIIC(&myDevice_EEPROM, pass5_addr, &zero, 1);
    isCompleted_delete_account = 1;
    //	XUartLite_SendByte(0x40600000, 'Y');
  }
  else
  {
    isCompleted_delete_account = 0;
    // XUartLite_SendByte(0x40600000, 'F');
    //  xil_printf("\r\n\t\t|                                                                                                                        |");
    //  xil_printf("\r\n\t\t+========================================================================================================================+");
    //  xil_printf("\r\n\t\t| Wrong user account.                                                                                                    |");
    //  xil_printf("\r\n\t\t|                                                                                                                        |");
    //  xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------+");
  }
  // delay_1s();
  // DisplayAccount();
  ReturnMode3();
}

void delete_account_ver_tmn()
{
  // DisplayAccount();
  //  xil_printf("\r\n");
  //  xil_printf("\r\n================+========================================================================================================================+");
  //  xil_printf("\r\n    Account     | Enter an account username account need to be deleted: ");
  u8 del_user[16];
  u8 count = 0;
  while (key != 13 || count == 0)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key > 47 && count < 15)
    {
      // xil_printf("%c",key);
      del_user[count] = key;
      count = count + 1;
      del_user[count] = 0;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
  load_user();
  if (check(user1, del_user))
  {
    EEPROM_WriteIIC(&myDevice_EEPROM, user1_addr, &zero, 1);
    delay_1ms();
    EEPROM_WriteIIC(&myDevice_EEPROM, pass1_addr, &zero, 1);
    XUartLite_SendByte(0x40600000, 'Y');
  }
  else if (check(user2, del_user))
  {
    EEPROM_WriteIIC(&myDevice_EEPROM, user2_addr, &zero, 1);
    delay_1ms();
    EEPROM_WriteIIC(&myDevice_EEPROM, pass2_addr, &zero, 1);
    XUartLite_SendByte(0x40600000, 'Y');
  }
  else if (check(user3, del_user))
  {
    EEPROM_WriteIIC(&myDevice_EEPROM, user3_addr, &zero, 1);
    delay_1ms();
    EEPROM_WriteIIC(&myDevice_EEPROM, pass3_addr, &zero, 1);
    XUartLite_SendByte(0x40600000, 'Y');
  }
  else if (check(user4, del_user))
  {
    EEPROM_WriteIIC(&myDevice_EEPROM, user4_addr, &zero, 1);
    delay_1ms();
    EEPROM_WriteIIC(&myDevice_EEPROM, pass4_addr, &zero, 1);
    XUartLite_SendByte(0x40600000, 'Y');
  }
  else if (check(user5, del_user))
  {
    EEPROM_WriteIIC(&myDevice_EEPROM, user5_addr, &zero, 1);
    delay_1ms();
    EEPROM_WriteIIC(&myDevice_EEPROM, pass5_addr, &zero, 1);
    XUartLite_SendByte(0x40600000, 'Y');
  }
  else
  {
    XUartLite_SendByte(0x40600000, 'F');
    //  xil_printf("\r\n\t\t|                                                                                                                        |");
    //  xil_printf("\r\n\t\t+========================================================================================================================+");
    //  xil_printf("\r\n\t\t| Wrong user account.                                                                                                    |");
    //  xil_printf("\r\n\t\t|                                                                                                                        |");
    //  xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------+");
  }
  // delay_1s();
  // DisplayAccount();
}

void reset_account()
{
  // DisplayAccount();
  u8 flag = 0;
  // xil_printf("\r\n================+========================================================================================================================+");
  // xil_printf("\r\n    SETTING     | Do you want to reset all user account? (Y): ");
  while (flag == 0)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 'y' || key == 'Y' || key == 13)
    {
      // xil_printf("%c",key);
      reset_user_account();
      flag = 1;
      // XUartLite_SendByte(0x40600000, 'R');
    }
    else if (key == 03)
    {
      Reset_System();
    }
    // else
    // {
    //   flag = 1;
    //   XUartLite_SendByte(0x40600000, 'R');
    // }
  }
  // delay_1s();

  // DisplayAccount();
  // delay_1s();
  // ReturnMode3();
}

void reset_user_account()
{
  EEPROM_WriteIIC(&myDevice_EEPROM, user1_addr, &zero, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, pass1_addr, &zero, 1);
  delay_1ms();

  EEPROM_WriteIIC(&myDevice_EEPROM, user2_addr, &zero, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, pass2_addr, &zero, 1);
  delay_1ms();

  EEPROM_WriteIIC(&myDevice_EEPROM, user3_addr, &zero, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, pass3_addr, &zero, 1);
  delay_1ms();

  EEPROM_WriteIIC(&myDevice_EEPROM, user4_addr, &zero, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, pass4_addr, &zero, 1);
  delay_1ms();

  EEPROM_WriteIIC(&myDevice_EEPROM, user5_addr, &zero, 1);
  delay_1ms();
  EEPROM_WriteIIC(&myDevice_EEPROM, pass5_addr, &zero, 1);
  delay_1ms();
}

void DisplayAccount()
{
  load_user();
  load_pass();
  load_passroot();
  xil_printf("\r\n");
  xil_printf("\r\n================+=========================================================================+==================================================================================================================+");
  xil_printf("\r\n    Account     | ----->                                                                  |                                                                                                                  |");
  xil_printf("\r\n\t\t+========+================================================================+                                                                                                                  |");
  xil_printf("\r\n\t\t|        |                  User account information                      |                                                                                                                  |");
  xil_printf("\r\n\t\t|   No.  +-----------------------------+----------------------------------+                                                                                                                  |");
  xil_printf("\r\n\t\t|        |           Username          |             Password             |                                                                                                                  |");
  xil_printf("\r\n\t\t+--------+-----------------------------+----------------------------------+                                                                                                                  |");
  xil_printf("\r\n\t\t|   1.   |       %-16s      |         %-16s         |                                                                                                                  |", user1, pass1);
  xil_printf("\r\n\t\t+--------+-----------------------------+----------------------------------+                                                                                                                  |");
  xil_printf("\r\n\t\t|   2.   |       %-16s      |         %-16s         |                                                                                                                  |", user2, pass2);
  xil_printf("\r\n\t\t+--------+-----------------------------+----------------------------------+                                                                                                                  |");
  xil_printf("\r\n\t\t|   3.   |       %-16s      |         %-16s         |                                                                                                                  |", user3, pass3);
  xil_printf("\r\n\t\t+--------+-----------------------------+----------------------------------+                                                                                                                  |");
  xil_printf("\r\n\t\t|   4.   |       %-16s      |         %-16s         |                                                                                                                  |", user4, pass4);
  xil_printf("\r\n\t\t+--------+-----------------------------+----------------------------------+                                                                                                                  |");
  xil_printf("\r\n\t\t|   5.   |       %-16s      |         %-16s         |                                                                                                                  |", user5, pass5);
  xil_printf("\r\n\t\t+--------+-----------------------------+----------------------------------+                                                                                                                  |");
  xil_printf("\r\n\t\t|        |                   Root account information                     |                                                                                                                  |");
  xil_printf("\r\n\t\t|   No.  +-----------------------------+----------------------------------+                                                                                                                  |");
  xil_printf("\r\n\t\t|        |           Rootname          |             Password             |                                                                                                                  |");
  xil_printf("\r\n\t\t+--------+-----------------------------+----------------------------------+                                                                                                                  |");
  xil_printf("\r\n\t\t|   1.   |       %-16s      |         %-16s         |                                                                                                                  |", "admin", passroot);
  xil_printf("\r\n\t\t+--------+-----------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------");
  XUartLite_SendByte(0x40600000, '*');
}

void change_root()
{
  // DisplayAccount();
  //  xil_printf("\r\n================+========================================================================================================================+");
  //  xil_printf("\r\n    Account     | Enter new password for root: ");
  u8 *root_pass[16];
  u8 count = 0;
  u8 i;
  while (key != 13 || count == 0)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key > 47 && count < 15)
    {
      // xil_printf("%c",key);
      root_pass[count] = key;
      count = count + 1;
      root_pass[count] = 0;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
  for (i = 0; i <= count; i++)
  {
    EEPROM_WriteIIC(&myDevice_EEPROM, passRoot_addr + i, root_pass + i, 1);
    delay_1ms();
  }
  i = 0;
  u8 rd_pass1 = 1;
  while (rd_pass1 != 0)
  {
    EEPROM_ReadIIC(&myDevice_EEPROM, passRoot_addr + i, &rd_pass1, 1);
    delay_1ms();
    ////xil_printf("%c",rd_pass1);
    i++;
  }
  isCompleted_change_root = 1;
  // XUartLite_SendByte(0x40600000, 'N');
  //  delay_1s();
  //  DisplayAccount();
  ReturnMode3();
}

void setload_default()
{
  u8 flag = 0;
  // LoadEEPROM();
  //	DisplayTable();
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    SETTING     | Do you want to load default device configuration from manufacturer? (Y/N): ");
  while (flag == 0)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 'y' || key == 'Y' || key == 13)
    {
      // xil_printf("%c",key);
      load_default();
      delay_0_5s();
      XUartLite_SendByte(0x40600000, 'Y');
      flag = 1;
    }
    else if (key == 'n' || key == 'N' || key == 13)
    {
      XUartLite_SendByte(0x40600000, 'N');
      delay_1s();
      // DisplayTable();
      flag = 1;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
  // flag = 0;
  // // xil_printf("\r\n================+========================================================================================================================================================================+");
  // // xil_printf("\r\n    SETTING     | Do you want to return to the device configuration menu? (Y): ");
  // while (flag == 0)
  // {
  //   key = XUartLite_RecvByte(0x40600000);
  //   if (key == 'y' || key == 'Y' || key == 13)
  //   {
  //     XUartLite_SendByte(0x40600000, 'X');
  //     //  xil_printf("%c",key);
  //     flag = 1;
  //   }
  //   else if (key == 03)
  //   {
  //     Reset_System();
  //   }
  // }
}

void load_default()
{
  // IPTarget = IPTarget;
  TimeFlood1 = 1;
  TimeDelete = 30;
  SynThreshold1 = 1000;
  AckThreshold1 = 100;
  UDPThreshold1 = 1000;
  DNSThreshold1 = 1000;
  ICMPThreshold1 = 1000;
  IPSecThreshold1 = 1000;
  UDPThresh_1s1 = 100;
  ICMPThresh_1s1 = 100;
  DefenderPort = 0;
  DefenderPort_en_1 = 0;
  DefenderPort_en_4 = 0;
  DefenderPort_en_5 = 0;
  // EnableDefender1 = 1279;
  EnableDefender1 = 12543;
  AssignData();
  SaveEEPROM();
  // xil_printf("\r\n");
  // xil_printf("\r\n ======================================================================+");
  // xil_printf("\r\n Manufacturer information.                                             |");
  // xil_printf("\r\n");
  LoadEEPROM();
  ConfigurationIPcore(1);
  // DisplayTable();
  isCompleted_load_default = 1;
}

void ReturnMode3()
{
  if ((isCompleted_add_account == 1) || (isCompleted_delete_account == 1) || (isCompleted_change_account == 1) || (isCompleted_load_default == 1) || (isCompleted_change_root == 1))
  {
    XUartLite_SendByte(0x40600000, 'Y');
  }
  else
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  isCompleted_add_account = 0;
  isCompleted_delete_account = 0;
  isCompleted_change_account = 0;
  isCompleted_load_default = 0;
  isCompleted_change_root = 0;
}

void user_change_info()
{
  change_user_pass();
  // add_acount();
}

void change_user_pass()
{
  u8 count_u, count_p;
  u8 i;
  count_u = 0;
  count_p = 0;
  u8 *password[16];
  key = 0;
  // xil_printf("\r\n\t\t|                                                                                                                        |");
  // xil_printf("\r\n\t\t| Enter Password: ");

  while (key != 13 || count_p == 0)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key > 47 && count_p < 15)
    {
      // xil_printf("%c",key);
      password[count_p] = key;
      count_p = count_p + 1;
      password[count_p] = 0;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }

  load_user();
  if (check(user1, username_login))
  {
    for (i = 0; i <= count_p; i++)
    {
      for (i = 0; i <= count_p; i++)
      {
        EEPROM_WriteIIC(&myDevice_EEPROM, pass1_addr + i, password + i, 1);
        delay_1ms();
      }
    }
  }
  else if (check(user2, username_login))
  {
    for (i = 0; i <= count_p; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, pass2_addr + i, password + i, 1);
      delay_1ms();
    }
  }
  else if (check(user3, username_login))
  {
    for (i = 0; i <= count_p; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, pass3_addr + i, password + i, 1);
      delay_1ms();
    }
  }
  else if (check(user4, username_login))
  {
    for (i = 0; i <= count_p; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, pass4_addr + i, password + i, 1);
      delay_1ms();
    }
  }

  else if (check(user5, username_login))
  {
    for (i = 0; i <= count_p; i++)
    {
      EEPROM_WriteIIC(&myDevice_EEPROM, pass5_addr + i, password + i, 1);
      delay_1ms();
    }
  }
}

void reconfig()
{
  u8 key1, count = 0;
  u8 key2 = 0;

  HTTPSGETFlood_en1 = (EnableDefender1 >> 11) % 2;
  HTTPGETFlood_en1 = (EnableDefender1 >> 10) % 2;
  DefenderPort = (EnableDefender1 >> 9) % 2;
  DefenderPort_en1 = (EnableDefender1 >> 8) % 2;
  UDPFragFlood_en1 = (EnableDefender1 >> 7) % 2;
  TCPFragFlood_en1 = (EnableDefender1 >> 6) % 2;
  IPSecFlood_en1 = (EnableDefender1 >> 5) % 2;
  ICMPFlood_en1 = (EnableDefender1 >> 4) % 2;
  DNSFlood_en1 = (EnableDefender1 >> 3) % 2;
  UDPFlood_en1 = (EnableDefender1 >> 2) % 2;
  LANDATTACK_en1 = (EnableDefender1 >> 1) % 2;
  SYNFlood_en1 = EnableDefender1 % 2;
  // 			xil_printf("\r\n\t\t+========================================================================================================================+");
  // 			xil_printf("\r\n\t\t| User login successful.                                                                                                 |");
  // 			xil_printf("\r\n\t\t|                                                                                                                        |");
  // xil_printf("\r\n----------------+------------------------------------------------------------------------------------------------------------------------+");
  while (key1 != 'O')
  {
    // clear_screen();
    // display_logo2();
    // xil_printf("\r\n ========================================================================================================================================+");
    // xil_printf("\r\n ==> Mode 2 is selected                                                                                                                  |");
    // xil_printf("\r\n                                                                                                                                         |\r\n");
    // xil_printf(" ===============+===========+============================================================================================================+\r\n");
    // xil_printf("    DISPLAY     |           |                                                                                                            |\r\n");
    // xil_printf("\t\t| Key Enter | Please choose 1 option below:                                                                              |\r\n");
    // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
    // xil_printf("\t\t|     1.    | Setting RTC.                                                                                               |\r\n");
    // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
    // xil_printf("\t\t|     2.    | Setting Anti by Port mode(*).                                                                              |\r\n");
    // xil_printf("\t\t|           | 	->(Info: When port protection mode(*) is enabled, IP protected mode (**) is disabled and vice versa).    |\r\n");
    // xil_printf("\t\t|     3.    | Setting interface port is protect.                                                                         |\r\n");
    // xil_printf("\t\t|           | 	->(Info: Protected default port interface is 1).                                                         |\r\n");
    // xil_printf("\t\t|           +------------------------------------------------------------------------------------------------------------+\r\n");
    // xil_printf("\t\t|     4.    | Setting IP Server to protect(**).                                                                          |\r\n");
    // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
    // xil_printf("\t\t|     5.    | Setting attack detection time.                                                                             |\r\n");
    // xil_printf("\t\t|           | 	->(Info: The default value is: 1 second).                                                                |\r\n");
    // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
    // xil_printf("\t\t|     6.    | Setting Anti-SYN flood.                                                                                    |\r\n");
    // xil_printf("\t\t|     7.    | Setting SYN flood attack detection threshold.                                                              |\r\n");
    // xil_printf("\t\t|           | 	->(Info: The default value is: 1000 PPS).                                                                |\r\n");
    // xil_printf("\t\t|     8.    | Setting ACK flood attack detection threshold.                                                              |\r\n");
    // xil_printf("\t\t|           | 	->(Info: The default value is: 1000 PPS).                                                                |\r\n");
    // xil_printf("\t\t|     9.    | Setting the time to automatically delete the connection session information in the white list.             |\r\n");
    // xil_printf("\t\t|           | 	->(Info: The default value is: 30 second).                                                               |\r\n");
    // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
    // xil_printf("\t\t|     A.    | Setting Anti-LAND Attack.                                                                                  |\r\n");
    // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
    // xil_printf("\t\t|     B.    | Setting Anti-UDP flood.                                                                                    |\r\n");
    // xil_printf("\t\t|     C.    | Setting UDP flood attack detection threshold.                                                              |\r\n");
    // xil_printf("\t\t|           | 	->(Info: The default value is: 1000 PPS).                                                                |\r\n");
    // xil_printf("\t\t|     D.    | Setting threshold of valid UDP packer per second allowed.                                                  |\r\n");
    // xil_printf("\t\t|           | 	->(Info: The default value is: 1000 PPS).                                                                |\r\n");
    // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
    // xil_printf("\t\t|     E.    | Setting Anti-DNS Amplification attack.                                                                     |\r\n");
    // xil_printf("\t\t|     F.    | Setting DNS Amplification attack detection threshold.                                                      |\r\n");
    // xil_printf("\t\t|           | 	->(Info: The default value is: 1000 PPS).                                                                |\r\n");
    // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
    // xil_printf("\t\t|     G.    | Setting Anti-ICMP flood.                                                                                   |\r\n");
    // xil_printf("\t\t|     H.    | Setting ICMP flood attack detection threshold.                                                             |\r\n");
    // xil_printf("\t\t|           | 	->(Info: The default value is: 1000 PPS).                                                                |\r\n");
    // xil_printf("\t\t|     I.    | Setting threshold of valid ICMP packer per second allowed.                                                 |\r\n");
    // xil_printf("\t\t|           | 	->(Info: The default value is: 1000 PPS).                                                                |\r\n");
    // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
    // xil_printf("\t\t|     J.    | Setting Anti-IPSec IKE flood.                                                                              |\r\n");
    // xil_printf("\t\t|     K.    | Setting IPSEC IKE flood attack detection threshold.                                                        |\r\n");
    // xil_printf("\t\t|           | 	->(Info: The default value is: 1000 PPS).                                                                |\r\n");
    // xil_printf("\t\t|     L.    | Add VPN server name or address to legitimate VPN list.                                                     |\r\n");
    // xil_printf("\t\t|     M.    | Remove the VPN server name or address from the legal VPN list.                                             |\r\n");
    // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
    // xil_printf("\t\t|     N.    | Setting Anti-TCP fragmentation flood.                                                                      |\r\n");
    // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
    // xil_printf("\t\t|     O.    | Setting Anti-UDP fragmentation flood.                                                                      |\r\n");
    // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
    // xil_printf("\t\t|     Z.    | => Exit.                                                                                                   |\r\n");
    // xil_printf("----------------+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
    // xil_printf("    SETTING     | --> Your choice: ");
    key = 0;
    count = 0;
    // xil_printf(" ");
    while (key != 13 || count < 1)
    {
      key = XUartLite_RecvByte(0x40600000);
      // if ((key < 49 || key > 57) && key != 13 && (key < 65 || key > 79)&&(key <97 || key >111) && key != 90 && key != 122){
      // }
      if (key != 13 && count == 0)
      {
        // xil_printf("%c",key);
        key1 = key;
        count = 1;
      }
    }
    // xil_printf("\r\n");
    // if (key1 == '1')
    // {
    //   // XUartLite_SendByte(0x40600000, '1');
    //   // mydelay(10000000);
    //   //  clear_screen();
    //   //  display_logo1();
    //   // DisplayTable();
    //   SetDateTime();
    //   ReturnMode2();
    // }

    // else if (key1 == '2')
    // {
    //   // XUartLite_SendByte(0x40600000, '2');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   //    display_logo1();
    //   SetDefenderPort();
    // }
    // else if (key1 == '3')
    // {
    //   // XUartLite_SendByte(0x40600000, '3');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetPortDefender();
    // }
    // else if (key1 == '4')
    // {
    //   // XUartLite_SendByte(0x40600000, '4');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetIPTarget();
    // }
    // else if (key1 == 'R')
    // {
    //   // XUartLite_SendByte(0x40600000, 'R');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetIPv6Target();
    //   // ReturnMode2();
    // }
    // else if (key1 == '5')
    // {
    //   // XUartLite_SendByte(0x40600000, '5');
    //   // mydelay(10000000);
    //   // // clear_screen();
    //   // // display_logo1();
    //   // DisplayTable();
    //   SetTimeflood();
    // }
    // else if (key1 == '6')
    // {
    //   // XUartLite_SendByte(0x40600000, '6');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetSynDefender();
    // }
    // else if (key1 == '7')
    // {
    //   // XUartLite_SendByte(0x40600000, '7');
    //   // mydelay(10000000);
    //   // // clear_screen();
    //   // // display_logo1();
    //   // DisplayTable();
    //   SetSynThresh();
    // }
    // else if (key1 == '8')
    // {
    //   // XUartLite_SendByte(0x40600000, '8');
    //   // mydelay(10000000);
    //   // // clear_screen();
    //   // // display_logo1();
    //   // DisplayTable();
    //   SetAckThresh();
    // }
    // else if (key1 == '9')
    // {
    //   // XUartLite_SendByte(0x40600000, '9');
    //   // mydelay(10000000);
    //   // // clear_screen();
    //   // // display_logo1();
    //   // DisplayTable();
    //   SetTimeDelete();
    // }
    // else if (key1 == 'A' || key1 == 'a')
    // {
    //   // XUartLite_SendByte(0x40600000, 'A');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetSynonymousDefender();
    // }
    // else if (key1 == 'B' || key1 == 'b')
    // {
    //   // XUartLite_SendByte(0x40600000, 'B');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetUDPDefender();
    // }
    // else if (key1 == 'C' || key1 == 'c')
    // {
    //   // XUartLite_SendByte(0x40600000, 'C');
    //   // mydelay(10000000);
    //   // // clear_screen();
    //   // // display_logo1();
    //   // DisplayTable();
    //   SetUDPThresh();
    // }
    // else if (key1 == 'D' || key1 == 'd')
    // {
    //   // XUartLite_SendByte(0x40600000, 'D');
    //   // mydelay(10000000);
    //   // // clear_screen();
    //   // // display_logo1();
    //   // DisplayTable();
    //   SetUDPThresh1s();
    // }
    // else if (key1 == 'E' || key1 == 'e')
    // {
    //   // XUartLite_SendByte(0x40600000, 'E');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetDNSDefender();
    // }
    // else if (key1 == 'F' || key1 == 'f')
    // {
    //   // // clear_screen();
    //   // // display_logo1();
    //   // XUartLite_SendByte(0x40600000, 'F');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetDNSThresh();
    // }
    // else if (key1 == 'G' || key1 == 'g')
    // {
    //   // XUartLite_SendByte(0x40600000, 'G');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetICMPDefender();
    // }
    // else if (key1 == 'H' || key1 == 'h')
    // {
    //   // clear_screen();
    //   // display_logo1();
    //   // //XUartLite_SendByte(0x40600000, 'H');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetICMPThresh();
    // }
    // else if (key1 == 'I' || key1 == 'i')
    // {
    //   // clear_screen();
    //   // display_logo1();
    //   // XUartLite_SendByte(0x40600000, 'I');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetICMPThresh1s();
    // }
    // else if (key1 == 'J' || key1 == 'j')
    // {
    //   // XUartLite_SendByte(0x40600000, 'J');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetIPSecDefender();
    // }
    // else if (key1 == 'K' || key1 == 'k')
    // {
    //   // XUartLite_SendByte(0x40600000, 'K');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetIPSecThresh();
    // }
    // else if (key1 == 'L' || key1 == 'l')
    // {
    //   // XUartLite_SendByte(0x40600000, 'L');
    //   // mydelay(10000000);
    //   // clear_screen();
    //   // display_logo2();
    //   // DisplayTable();
    //   AddIpv4VPN();
    // }
    // else if (key1 == 'M' || key1 == 'm')
    // {
    //   // XUartLite_SendByte(0x40600000, 'M');
    //   // mydelay(10000000);
    //   // clear_screen();
    //   // display_logo2();
    //   RemoveIpv4VPN();
    // }
    // else if (key1 == 'N' || key1 == 'n')
    // {
    //   // XUartLite_SendByte(0x40600000, 'N');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetTCPFragDefender();
    // }
    // else if (key1 == 'O' || key1 == 'o')
    // {
    //   // XUartLite_SendByte(0x40600000, 'O');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetUDPFragDefender();
    // }
    // else if (key1 == 'P' || key1 == 'p')
    // {
    //   // XUartLite_SendByte(0x40600000, 'P');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   SetHTTPGETDefender();
    // }
    // else if (key1 == 'Q' || key1 == 'q')
    // {
    //   // XUartLite_SendByte(0x40600000, 'Q');
    //   // mydelay(10000000);
    //   // DisplayTable();
    //   set_HTTP_IP_Table();
    // }
    if (key1 == 03)
    {
      // xil_printf("Loading Bitstream.........");
      Reset_System();
    }
    else if (key1 == 'Z' || key1 == 'z')
    {
      // XUartLite_SendByte(0x40600000, 'Z');
      //  clear_screen();
      //  display_logo1();
      //  mydelay(10000000);
      //  DisplayTable();
      //  xil_printf("\r\n    SETTING     | Do you want to save your setting changes? (Y/N) ");
      u8 key = 0;
      u8 done1 = 0;
      while (key != 13)
      {
        key = XUartLite_RecvByte(0x40600000);
        if (key == 'y' || key == 'Y')
        {
          // XUartLite_SendByte(0x40600000, 'Y');
          //	xil_printf("%c",key);
          AssignData();
          SaveEEPROM();
          ConfigurationIPcore(1);
          count++;
        }
        else if (key == 'n' || key == 'N')
        {
          // XUartLite_SendByte(0x40600000, 'N');
          // xil_printf("%c", key);
          count++;
        }
        else if (key == 03)
        {
          Reset_System();
        }
      }
      break;
    }
    else
    {
    }
    EnableDefender1 = (HTTPSGETFlood_en1 << 13) + (HTTPGETFlood_en1 << 12) + (DefenderPort_en_5 << 11) + (DefenderPort_en_4 << 10) + (DefenderPort_en_1 << 9) + (DefenderPort_en1 << 8) + (UDPFragFlood_en1 << 7) + (TCPFragFlood_en1 << 6) + (IPSecFlood_en1 << 5) + (ICMPFlood_en1 << 4) + (DNSFlood_en1 << 3) + (UDPFlood_en1 << 2) + (LANDATTACK_en1 << 1) + SYNFlood_en1;
  }
}

void SetDefenderPort()
{
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Do you want to enable protect by interface port now (Y/N)?: ");
  u8 key = 0;
  u8 done3 = 0;

  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 'y' || key == 'Y')
    {
      // XUartLite_SendByte(0x40600000, 'Y');
      done3 = 1;
      // xil_printf("%c",key);
      DefenderPort_en1 = 0;
    }
    else if (key == 'n' || key == 'N')
    {
      // XUartLite_SendByte(0x40600000, 'N');
      done3 = 1;
      // xil_printf("%c",key);
      DefenderPort_en1 = 1;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else if (key == 13 && done3 == 1)
    {
      //  XUartLite_SendByte(0x40600000, 'E');
      break;
    }
  }
  // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
  // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
  EnableDefender1 = (HTTPSGETFlood_en1 << 13) + (HTTPGETFlood_en1 << 12) + (DefenderPort_en_5 << 11) + (DefenderPort_en_4 << 10) + (DefenderPort_en_1 << 9) + (DefenderPort_en1 << 8) + (UDPFragFlood_en1 << 7) + (TCPFragFlood_en1 << 6) + (IPSecFlood_en1 << 5) + (ICMPFlood_en1 << 4) + (DNSFlood_en1 << 3) + (UDPFlood_en1 << 2) + (LANDATTACK_en1 << 1) + SYNFlood_en1;
  ReturnMode2(1);
}

void SetPortDefender()
{
  u8 key;
  u8 count;
  u32 portnumber_valid;
Return:
  key = 0;
  count = 0;
  portnumber_valid = 0;
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Enter port number for clinet side (Internet side) (0/1): ");
  while (key != 13 && count < 1)
  {
    key = XUartLite_RecvByte(0x40600000);
    // if ((key <48 || key >49) && key!=13){
    // }
    if (key > 47 && key < 50)
    {
      // xil_printf("%c",key);
      key = key - 48;
      portnumber_valid = key;
      count++;
      if (key == 0)
      {
        // XUartLite_SendByte(0x40600000, '0');
      }
      else if (key == 1)
      {
        // XUartLite_SendByte(0x40600000, '1');
      }
      else if (key == 03)
      {
        Reset_System();
      }
    }
  }
  // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
  // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
  if (portnumber_valid > 1)
  {
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+========================================================================================================================================================================+");
    // xil_printf("\r\n\t\t| The client (Internet) port must be 0 or 1.                                                                                                                     |");
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
    goto Return;
  }
  else
    DefenderPort = portnumber_valid;
  EnableDefender1 = (HTTPSGETFlood_en1 << 13) + (HTTPGETFlood_en1 << 12) + (DefenderPort_en_5 << 11) + (DefenderPort_en_4 << 10) + (DefenderPort_en_1 << 9) + (DefenderPort_en1 << 8) + (UDPFragFlood_en1 << 7) + (TCPFragFlood_en1 << 6) + (IPSecFlood_en1 << 5) + (ICMPFlood_en1 << 4) + (DNSFlood_en1 << 3) + (UDPFlood_en1 << 2) + (LANDATTACK_en1 << 1) + SYNFlood_en1;
  ReturnMode2(1);
}

//============================================================================
// HTTP IP v4 add excel
void Add_IPv4_HTTP_fromFile()
{
  char large_buffer[LARGE_BUFFER_SIZE];
  size_t buffer_index = 0;
  char key;
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (buffer_index >= LARGE_BUFFER_SIZE - 1)
    {
      // printf("Buffer full, processing data...\n");
      break;
    }
    if (key == '\n' || key == '\r')
    {
      large_buffer[buffer_index] = '\0';
      break;
    }
    else
    {
      large_buffer[buffer_index++] = key;
    }
  }
  ProcessBuffer_IPv4_HTTP_Add(large_buffer);
  XUartLite_SendByte(0x40600000, 'Y');
}

void ProcessBuffer_IPv4_HTTP_Add(char *buffer)
{
  char *token = strtok(buffer, "$");
  while (token != NULL)
  {
    Process_IPv4_HTTP_Add(token);
    token = strtok(NULL, "$");
  }
}

void Process_IPv4_HTTP_Add(const char *ip)
{
  // Add_IPv4_HTTP_Table_From_File(ip);
  Set_IPv4_Write_Protect_Server(ip);
}

// Clear IPv4 HTTP excel
void Clear_IPv4_HTTP_fromFile()
{
  char large_buffer[LARGE_BUFFER_SIZE];
  size_t buffer_index = 0;
  char key;
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (buffer_index >= LARGE_BUFFER_SIZE - 1)
    {
      // printf("Buffer full, processing data...\n");
      break;
    }
    if (key == '\n' || key == '\r')
    {
      large_buffer[buffer_index] = '\0';
      break;
    }
    else
    {
      large_buffer[buffer_index++] = key;
    }
  }
  ProcessBuffer_IPv4_HTTP_Clear(large_buffer);
  XUartLite_SendByte(0x40600000, 'Y');
}

void ProcessBuffer_IPv4_HTTP_Clear(char *buffer)
{
  char *token = strtok(buffer, "$");
  while (token != NULL)
  {
    Process_IPv4_HTTP_Clear(token);
    token = strtok(NULL, "$");
  }
}

void Process_IPv4_HTTP_Clear(const char *ip)
{
  Clear_IPv4_HTTP_Table_From_File(ip);
}

// HTTP IP v6 add execl
void Add_IPv6_HTTP_fromFile()
{
  char large_buffer[LARGE_BUFFER_SIZE];
  size_t buffer_index = 0;
  char key;
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (buffer_index >= LARGE_BUFFER_SIZE - 1)
    {
      // printf("Buffer full, processing data...\n");
      break;
    }
    if (key == '\n' || key == '\r')
    {
      large_buffer[buffer_index] = '\0';
      break;
    }
    else
    {
      large_buffer[buffer_index++] = key;
    }
  }
  // XUartLite_SendByte(0x40600000, 'O');
  ProcessBuffer_IPv6_HTTP_Add(large_buffer);
  XUartLite_SendByte(0x40600000, 'Y');
}

void ProcessBuffer_IPv6_HTTP_Add(char *buffer)
{
  char *token = strtok(buffer, "$");
  while (token != NULL)
  {
    Process_IPv6_HTTP_Add(token);
    token = strtok(NULL, "$");
  }
}
void Process_IPv6_HTTP_Add(const char *ip)
{
  Add_IPv6_HTTP_Table_From_File(ip);
  // Add_IPv6_Protect_Table_From_File(ip);
}

// Clear HTTP IPv6 excel
void Clear_IPv6_HTTP_fromFile()
{
  char large_buffer[LARGE_BUFFER_SIZE];
  size_t buffer_index = 0;
  char key;
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (buffer_index >= LARGE_BUFFER_SIZE - 1)
    {
      // printf("Buffer full, processing data...\n");
      break;
    }
    if (key == '\n' || key == '\r')
    {
      large_buffer[buffer_index] = '\0';
      break;
    }
    else
    {
      large_buffer[buffer_index++] = key;
    }
  }
  ProcessBuffer_IPv6_HTTP_Clear(large_buffer);
  XUartLite_SendByte(0x40600000, 'Y');
}

void ProcessBuffer_IPv6_HTTP_Clear(char *buffer)
{
  char *token = strtok(buffer, "$");
  while (token != NULL)
  {
    Process_IPv6_HTTP_Clear(token);
    token = strtok(NULL, "$");
  }
}
void Process_IPv6_HTTP_Clear(const char *ip)
{
  Clear_IPv6_HTTP_Table_From_File(ip);
}
//============================================================================
void ReceiveIPv4String(char *buffer_IPV4, size_t buffer_size_IPV4)
{
  char key;
  size_t index = 0;
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);

    if (key == '\n' || key == '\r')
    {
      buffer_IPV4[index] = '\0';
      break;
    }
    else if (index < buffer_size_IPV4 - 1)
    {
      buffer_IPV4[index++] = key;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
}
void ReceiveIPv4String_protect(char *buffer_IPV4, char *header_buffer_IPV4, size_t buffer_size_IPV4)
{
  char key;
  size_t index = 0;
  int is_first_char = 1;

  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);

    if (key == '\n' || key == '\r')
    {
      buffer_IPV4[index] = '\0';
      break;
    }
    else if (key == 3)
    {
      Reset_System();
    }
    else if (index < buffer_size_IPV4 - 1)
    {
      // if (is_first_char)
      // {
      //   *header_buffer_IPV4 = key;
      //   is_first_char = 0;
      // }
      // else
      // {
      buffer_IPV4[index++] = key;
      //}
    }
  }
}

// void SetIPTarget1(const char *ip_str)
// {
//   u16 IPlayerA = 0;
//   u16 IPlayerB = 0;
//   u16 IPlayerC = 0;
//   u16 IPlayerD = 0;
//   u8 flag_error = 0;

//   if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IPlayerA, &IPlayerB, &IPlayerC, &IPlayerD) != 4)
//   {
//     flag_error = 1;
//   }

//   if (flag_error == 1)
//   {
//     XUartLite_SendByte(0x40600000, 'N');
//     return;
//   }

//   IPTarget1 = IPlayerA * 16777216 + IPlayerB * 65536 + IPlayerC * 256 + IPlayerD;
//   Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 4, IPTarget1);

//   ReturnMode2();
// }
// void SetIPTarget2(const char *ip_str)
// {
//   u16 IPlayerA = 0;
//   u16 IPlayerB = 0;
//   u16 IPlayerC = 0;
//   u16 IPlayerD = 0;
//   u8 flag_error = 0;

//   if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IPlayerA, &IPlayerB, &IPlayerC, &IPlayerD) != 4)
//   {
//     flag_error = 1;
//   }

//   if (flag_error == 1)
//   {
//     XUartLite_SendByte(0x40600000, 'N');
//     return;
//   }

//   IPTarget2 = IPlayerA * 16777216 + IPlayerB * 65536 + IPlayerC * 256 + IPlayerD;
//   Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 488, IPTarget1);
//   ReturnMode2();
// }
// void SetIPTarget3(const char *ip_str)
// {
//   u16 IPlayerA = 0;
//   u16 IPlayerB = 0;
//   u16 IPlayerC = 0;
//   u16 IPlayerD = 0;
//   u8 flag_error = 0;

//   if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IPlayerA, &IPlayerB, &IPlayerC, &IPlayerD) != 4)
//   {
//     flag_error = 1;
//   }

//   if (flag_error == 1)
//   {
//     XUartLite_SendByte(0x40600000, 'N');
//     return;
//   }

//   IPTarget3 = IPlayerA * 16777216 + IPlayerB * 65536 + IPlayerC * 256 + IPlayerD;
//   Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 492, IPTarget1);
//   ReturnMode2();
// }
// void SetIPTarget4(const char *ip_str)
// {
//   u16 IPlayerA = 0;
//   u16 IPlayerB = 0;
//   u16 IPlayerC = 0;
//   u16 IPlayerD = 0;
//   u8 flag_error = 0;

//   if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IPlayerA, &IPlayerB, &IPlayerC, &IPlayerD) != 4)
//   {
//     flag_error = 1;
//   }

//   if (flag_error == 1)
//   {
//     XUartLite_SendByte(0x40600000, 'N');
//     return;
//   }

//   IPTarget4 = IPlayerA * 16777216 + IPlayerB * 65536 + IPlayerC * 256 + IPlayerD;
//   Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 496, IPTarget1);
//   ReturnMode2();
// }
void Set_IPv4_Write_Protect_Server(const char *ip_str)
{
  u16 IPlayerA = 0;
  u16 IPlayerB = 0;
  u16 IPlayerC = 0;
  u16 IPlayerD = 0;
  u8 flag_error = 0;
  int IPTarget_Write = 0;
  if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IPlayerA, &IPlayerB, &IPlayerC, &IPlayerD) != 4)
  {
    flag_error = 1;
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
    return;
  }

  IPTarget_Write = IPlayerA * 16777216 + IPlayerB * 65536 + IPlayerC * 256 + IPlayerD;
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 524, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 528, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 532, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 536, IPTarget_Write);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 540, 0x00000004);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 544, 0x00000001);
  // delay_1ms();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 544, 0x00000000);
}
void Set_IPv4_Remove_Protect_Server(const char *ip_str)
{
  u16 IPlayerA = 0;
  u16 IPlayerB = 0;
  u16 IPlayerC = 0;
  u16 IPlayerD = 0;
  u8 flag_error = 0;
  int IPTarget_Write = 0;
  if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IPlayerA, &IPlayerB, &IPlayerC, &IPlayerD) != 4)
  {
    flag_error = 1;
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
    return;
  }

  IPTarget_Write = IPlayerA * 16777216 + IPlayerB * 65536 + IPlayerC * 256 + IPlayerD;
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 524, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 528, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 532, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 536, IPTarget_Write);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 540, 0x00000004);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 544, 0x00000011);
  // delay_1ms();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 544, 0x00000000);
}

// Block attcker
void Set_IPv4_Write_Block_Attacker(const char *ip_str)
{
  u16 IPlayerA = 0;
  u16 IPlayerB = 0;
  u16 IPlayerC = 0;
  u16 IPlayerD = 0;
  u8 flag_error = 0;
  int IPTarget_Write = 0;
  if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IPlayerA, &IPlayerB, &IPlayerC, &IPlayerD) != 4)
  {
    flag_error = 1;
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
    return;
  }

  IPTarget_Write = IPlayerA * 16777216 + IPlayerB * 65536 + IPlayerC * 256 + IPlayerD;
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 548, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 552, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 556, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 560, IPTarget_Write);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 564, 0x00000004);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 568, 0x00000001);
  // delay_1ms();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 568, 0x00000000);
}
void Set_IPv4_Remove_Block_Attacker(const char *ip_str)
{
  u16 IPlayerA = 0;
  u16 IPlayerB = 0;
  u16 IPlayerC = 0;
  u16 IPlayerD = 0;
  u8 flag_error = 0;
  int IPTarget_Write = 0;
  if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IPlayerA, &IPlayerB, &IPlayerC, &IPlayerD) != 4)
  {
    flag_error = 1;
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
    return;
  }

  IPTarget_Write = IPlayerA * 16777216 + IPlayerB * 65536 + IPlayerC * 256 + IPlayerD;
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 548, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 552, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 556, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 560, IPTarget_Write);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 564, 0x00000004);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 568, 0x00000011);
  // delay_1ms();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 568, 0x00000000);
}

void Add_IPv4_Protect_Server()
{
  char buffer_IPV4[BUFFER_SIZE_IPV4];
  char header_buffer_IPV4[1];
  ReceiveIPv4String_protect(buffer_IPV4, header_buffer_IPV4, BUFFER_SIZE_IPV4);
  Set_IPv4_Write_Protect_Server(buffer_IPV4);
  XUartLite_SendByte(0x40600000, 'Y');
  // if (strcmp(header_buffer_IPV4, "1") == 0)
  // {
  //   SetIPTarget1(buffer_IPV4);
  //   XUartLite_SendByte(0x40600000, '1');
  // }
  // else if (strcmp(header_buffer_IPV4, "2") == 0)
  // {
  //   SetIPTarget2(buffer_IPV4);
  //   XUartLite_SendByte(0x40600000, '2');
  // }

  // else if (strcmp(header_buffer_IPV4, "3") == 0)
  // {
  //   SetIPTarget3(buffer_IPV4);
  //   XUartLite_SendByte(0x40600000, '3');
  // }
  // else if (strcmp(header_buffer_IPV4, "4") == 0)
  // {
  //   SetIPTarget4(buffer_IPV4);
  //   XUartLite_SendByte(0x40600000, '4');
  // }
}
void Delete_IPv4_Protect_Server()
{
  char buffer_IPV4[BUFFER_SIZE_IPV4];
  char header_buffer_IPV4[1];
  ReceiveIPv4String_protect(buffer_IPV4, header_buffer_IPV4, BUFFER_SIZE_IPV4);
  Set_IPv4_Remove_Protect_Server(buffer_IPV4);
  XUartLite_SendByte(0x40600000, 'Y');
  // if (strcmp(header_buffer_IPV4, "1") == 0)
  // {
  //   SetIPTarget1(buffer_IPV4);
  //   XUartLite_SendByte(0x40600000, '1');
  // }
  // else if (strcmp(header_buffer_IPV4, "2") == 0)
  // {
  //   SetIPTarget2(buffer_IPV4);
  //   XUartLite_SendByte(0x40600000, '2');
  // }

  // else if (strcmp(header_buffer_IPV4, "3") == 0)
  // {
  //   SetIPTarget3(buffer_IPV4);
  //   XUartLite_SendByte(0x40600000, '3');
  // }
  // else if (strcmp(header_buffer_IPV4, "4") == 0)
  // {
  //   SetIPTarget4(buffer_IPV4);
  //   XUartLite_SendByte(0x40600000, '4');
  // }
}

void Add_IPv4_Block_Attacker()
{
  char buffer_IPV4[BUFFER_SIZE_IPV4];
  char header_buffer_IPV4[1];
  ReceiveIPv4String_protect(buffer_IPV4, header_buffer_IPV4, BUFFER_SIZE_IPV4);
  Set_IPv4_Write_Block_Attacker(buffer_IPV4);
  XUartLite_SendByte(0x40600000, 'K');
}
void Delete_IPv4_Block_Attacker()
{
  char buffer_IPV4[BUFFER_SIZE_IPV4];
  char header_buffer_IPV4[1];
  ReceiveIPv4String_protect(buffer_IPV4, header_buffer_IPV4, BUFFER_SIZE_IPV4);
  Set_IPv4_Remove_Block_Attacker(buffer_IPV4);
  XUartLite_SendByte(0x40600000, 'K');
}
// void SetIPTarget()
// {
//   u8 key;
//   u8 count_point = 0;
//   u8 count = 0;
//   u8 colect_ip[2];
//   u16 IPlayerA;
//   u16 IPlayerB;
//   u16 IPlayerC;
//   u16 IPlayerD;
//   u8 next;
//   u8 flag_error;
// Return:
//   key = 0;
//   count_point = 0;
//   count = 0;
//   colect_ip[2] = 0;
//   IPlayerA = 0;
//   IPlayerB = 0;
//   IPlayerC = 0;
//   IPlayerD = 0;
//   next = 0;
//   flag_error = 0;
//   // xil_printf("\r\n");
//   // xil_printf("\r\n================+========================================================================================================================================================================+");
//   // xil_printf("\r\n    Setting     | Enter Server IP address want to protect : ");
//   while (count_point < 5 && flag_error != 1)
//   {
//     key = XUartLite_RecvByte(0x40600000);
//     // if((key < '0' || key > '9')&& key != '.' && key != 13){
//     // 	//No dothing;
//     // }
//     if ((key >= '0' && key <= '9') || key == '.' || key == 13)
//     {
//       // xil_printf("%c", key);
//       if (key == '.' && count_point < 4)
//       {
//         count_point++;
//       }
//       if (key >= '0' && key <= '9')
//       {
//         key = key - 48;
//         count = count + 1;
//         colect_ip[count - 1] = key;
//       }
//       if (key == 03)
//       {
//         Reset_System();
//       }
//       if (count_point == 1 && next == 0)
//       {
//         if (count == 1)
//         {
//           IPlayerA = colect_ip[count - 1];
//         }
//         else if (count == 2)
//         {
//           IPlayerA = colect_ip[count - 2] * 10 + colect_ip[count - 1];
//         }
//         else if (count == 3)
//         {
//           IPlayerA = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
//         }
//         count = 0;
//         next = 1;
//       }
//       if (count_point == 2 && next == 1)
//       {
//         if (count == 1)
//         {
//           IPlayerB = colect_ip[count - 1];
//         }
//         else if (count == 2)
//         {
//           IPlayerB = colect_ip[count - 2] * 10 + colect_ip[count - 1];
//         }
//         else if (count == 3)
//         {
//           IPlayerB = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
//         }
//         count = 0;
//         next = 2;
//       }
//       if (count_point == 3 && next == 2)
//       {
//         if (count == 1)
//         {
//           IPlayerC = colect_ip[count - 1];
//         }
//         else if (count == 2)
//         {
//           IPlayerC = colect_ip[count - 2] * 10 + colect_ip[count - 1];
//         }
//         else if (count == 3)
//         {
//           IPlayerC = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
//         }
//         count_point = 4;
//         count = 0;
//         next = 3;
//       }
//       if (key == 13 && count_point == 4)
//       {
//         if (count == 1)
//         {
//           IPlayerD = colect_ip[count - 1];
//         }
//         else if (count == 2)
//         {
//           IPlayerD = colect_ip[count - 2] * 10 + colect_ip[count - 1];
//         }
//         else if (count == 3)
//         {
//           IPlayerD = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
//         }
//         count = 0;
//         count_point = 5;
//       }
//       if (IPlayerA > 255 || IPlayerA < 0 || IPlayerB > 255 || IPlayerB < 0 || IPlayerC > 255 || IPlayerC < 0 || IPlayerD > 255 || IPlayerD < 0)
//       {
//         flag_error = 1;
//       }
//     }
//   }
//   if (flag_error == 1)
//   {
//     // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
//     // xil_printf("\r\n\t\t+========================================================================================================================================================================+");
//     // xil_printf("\r\n\t\t| Warning: IP Server %d.%d.%d.%d invalid!", IPlayerA, IPlayerB, IPlayerC, IPlayerD);
//     // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
//     // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
//     // goto Return;
//   }
//   else
//   {
//     // XUartLite_SendByte(0x40600000, 'I');
//     IPTarget = IPlayerA * 16777216 + IPlayerB * 65536 + IPlayerC * 256 + IPlayerD;
//   }
//   ReturnMode2();
// }

void SetSynThresh()
{
  u8 count;
Return:
  key = 0;
  count = 0;
  u32 synthresh_valid = 0;
  u8 key1 = 0;

  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);

    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
    {
      break;
    }
  }

  while (key != 13 && count < 16)
  {
    key = XUartLite_RecvByte(0x40600000);
    // if ((key <48 || key >57) && key!=13){
    // }
    if ((count == 0) && (key == 48))
    {
      key = key - 48;
      synthresh_valid = synthresh_valid + key;
    }
    else if (key > 47 && key < 58)
    {
      key = key - 48;
      synthresh_valid = synthresh_valid * 10 + key;
      count++;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
  if (synthresh_valid > 65535)
  {
  }
  else
    switch (key1 - '0') // Convert char to int
    {
    case 1:
      SynThreshold1 = synthresh_valid;
      break;
    case 2:
      SynThreshold2 = synthresh_valid;
      break;
    case 3:
      SynThreshold3 = synthresh_valid;
      break;
    case 4:
      SynThreshold4 = synthresh_valid;
      break;
    case 5:
      SynThreshold5 = synthresh_valid;
      break;
    case 6:
      SynThreshold6 = synthresh_valid;
      break;
    case 7:
      SynThreshold7 = synthresh_valid;
      break;
    case 8:
      SynThreshold8 = synthresh_valid;
      break;
    default:
      break;
    }

  ReturnMode2(key1 - '0'); // Pass the selected option to ReturnMode2
}

void SetAckThresh()
{
  u8 key;
  u8 count;
Return:
  key = 0;
  count = 0;
  u32 ackthresh_valid = 0;
  u8 key1 = 0;

  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);

    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
    {
      break;
    }
  }
  while (key != 13 && count < 16)
  {
    key = XUartLite_RecvByte(0x40600000);
    // if ((key <48 || key >57) && key!=13){
    // }
    if ((count == 0) && (key == 48))
    {
      // xil_printf("%c", key);
      key = key - 48;
      ackthresh_valid = ackthresh_valid + key;
    }
    else if (key > 47 && key < 58)
    {
      // xil_printf("%c",key);
      key = key - 48;
      ackthresh_valid = ackthresh_valid * 10 + key;
      count++;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
  if (ackthresh_valid > 65535)
  {
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+========================================================================================================================================================================+");
    // xil_printf("\r\n\t\t| The value must be less than 65536.                                                                                                                                     |");
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
    // goto Return;
  }
  else
  {
    switch (key1 - '0') // Convert char to int
    {
    case 1:
      AckThreshold1 = ackthresh_valid;
      break;
    case 2:
      AckThreshold2 = ackthresh_valid;
      break;
    case 3:
      AckThreshold3 = ackthresh_valid;
      break;
    case 4:
      AckThreshold4 = ackthresh_valid;
      break;
    case 5:
      AckThreshold5 = ackthresh_valid;
      break;
    case 6:
      AckThreshold6 = ackthresh_valid;
      break;
    case 7:
      AckThreshold7 = ackthresh_valid;
      break;
    case 8:
      AckThreshold8 = ackthresh_valid;
      break;
    }
  }
  ReturnMode2(key1 - '0'); // Pass the selected option to ReturnMode2
}

void SetUDPThresh()
{
  u8 key;
  u8 count;
Return:
  key = 0;
  count = 0;
  u32 udpthresh_valid = 0;
  u8 key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }
  while (key != 13 && count < 16)
  {
    key = XUartLite_RecvByte(0x40600000);
    // if ((key <48 || key >57) && key!=13){
    // }
    if ((count == 0) && (key == 48))
    {
      // xil_printf("%c", key);
      key = key - 48;
      udpthresh_valid = udpthresh_valid + key;
    }
    else if (key > 47 && key < 58)
    {
      // xil_printf("%c",key);
      key = key - 48;
      udpthresh_valid = udpthresh_valid * 10 + key;
      count++;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
  if (udpthresh_valid > 65535)
  {
  }
  else
    switch (key1 - '0')
    {
    case 1:
      UDPThreshold1 = udpthresh_valid;
      break;
    case 2:
      UDPThreshold2 = udpthresh_valid;
      break;
    case 3:
      UDPThreshold3 = udpthresh_valid;
      break;
    case 4:
      UDPThreshold4 = udpthresh_valid;
      break;
    case 5:
      UDPThreshold5 = udpthresh_valid;
      break;
    case 6:
      UDPThreshold6 = udpthresh_valid;
      break;
    case 7:
      UDPThreshold7 = udpthresh_valid;
      break;
    case 8:
      UDPThreshold8 = udpthresh_valid;
      break;
    }

  ReturnMode2(key1 - '0'); // Pass the selected option to ReturnMode2
}

void SetUDPThresh1s()
{
  u8 key;
  u8 count;
Return:
  key = 0;
  count = 0;
  u32 udpthresh1s_valid = 0;
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Enter the limit of incoming UDP packet per second (PPS): ");
  u8 key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }
  while (key != 13 && count < 16)
  {
    key = XUartLite_RecvByte(0x40600000);
    // if ((key >57) && key!=13){
    // }
    if ((count == 0) && (key == 48))
    {
      // xil_printf("%c", key);
      key = key - 48;
      udpthresh1s_valid = udpthresh1s_valid + key;
    }
    else if (key > 47 && key < 58)
    {
      // xil_printf("%c",key);
      key = key - 48;
      udpthresh1s_valid = udpthresh1s_valid * 10 + key;
      count++;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
  if (udpthresh1s_valid > 65535)
  {
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+========================================================================================================================================================================+");
    // xil_printf("\r\n\t\t| The value must be less than 65536.                                                                                                                                     |");
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
    goto Return;
  }
  else
  {
    switch (key1 - '0') // Convert char to int
    {
    case 1:
      UDPThresh_1s1 = udpthresh1s_valid;
      break;

    case 2:
      UDPThresh_1s2 = udpthresh1s_valid;
      break;
    case 3:
      UDPThresh_1s3 = udpthresh1s_valid;
      break;
    case 4:
      UDPThresh_1s4 = udpthresh1s_valid;
      break;
    case 5:
      UDPThresh_1s5 = udpthresh1s_valid;
      break;
    case 6:
      UDPThresh_1s6 = udpthresh1s_valid;
      break;
    case 7:
      UDPThresh_1s7 = udpthresh1s_valid;
      break;
    case 8:
      UDPThresh_1s8 = udpthresh1s_valid;
      break;
    }
    ReturnMode2(key1 - '0'); // Pass the selected option to ReturnMode2
  }
}

void SetICMPThresh()
{
  u8 key;
  u8 count;
Return:
  key = 0;
  count = 0;
  u32 icmpthresh_valid = 0;

  u8 key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }
  while (key != 13 && count < 16)
  {
    key = XUartLite_RecvByte(0x40600000);
    // if ((key <48 || key >57) && key!=13){
    // }
    if ((count == 0) && (key == 48))
    {
      // xil_printf("%c", key);
      key = key - 48;
      icmpthresh_valid = icmpthresh_valid + key;
    }
    else if (key > 47 && key < 58)
    {
      // xil_printf("%c",key);
      key = key - 48;
      icmpthresh_valid = icmpthresh_valid * 10 + key;
      count++;
    }
    else if (key == 03)
    {
      Reset_System();
    }
  }
  if (icmpthresh_valid > 65535)
  {
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+========================================================================================================================================================================+");
    // xil_printf("\r\n\t\t| The value must be less than 65536.                                                                                                                                     |");
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
    // goto Return;
  }
  else
  {
    switch (key1 - '0') // Convert char to int
    {
    case 1:
      ICMPThreshold1 = icmpthresh_valid;
      break;
    case 2:
      ICMPThreshold2 = icmpthresh_valid;
      break;
    case 3:
      ICMPThreshold3 = icmpthresh_valid;
      break;
    case 4:
      ICMPThreshold4 = icmpthresh_valid;
      break;
    case 5:
      ICMPThreshold5 = icmpthresh_valid;
      break;
    case 6:
      ICMPThreshold6 = icmpthresh_valid;
      break;
    case 7:
      ICMPThreshold7 = icmpthresh_valid;
      break;
    case 8:
      ICMPThreshold8 = icmpthresh_valid;
      break;
    }
    ReturnMode2(key1 - '0');
  }
}

void SetICMPThresh1s()
{
  u8 key;
  u8 count;
Return:
  key = 0;
  count = 0;
  u32 icmpthresh1s_valid = 0;
  u8 key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }
  while (key != 13 && count < 16)
  {
    key = XUartLite_RecvByte(0x40600000);
    if ((count == 0) && (key == 48))
    {
      // xil_printf("%c", key);
      key = key - 48;
      icmpthresh1s_valid = icmpthresh1s_valid + key;
    }
    else if (key > 47 && key < 58)
    {
      // xil_printf("%c",key);
      key = key - 48;
      icmpthresh1s_valid = icmpthresh1s_valid * 10 + key;
      count++;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  if (icmpthresh1s_valid > 65535)
  {
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+========================================================================================================================================================================+");
    // xil_printf("\r\n\t\t| The value must be less than 65536.                                                                                                                                     |");
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
    // goto Return;
  }
  else
  {
    switch (key1 - '0') // Convert char to int
    {
    case 1:
      ICMPThresh_1s1 = icmpthresh1s_valid;
      break;
    case 2:
      ICMPThresh_1s2 = icmpthresh1s_valid;
      break;
    case 3:
      ICMPThresh_1s3 = icmpthresh1s_valid;
      break;
    case 4:
      ICMPThresh_1s4 = icmpthresh1s_valid;
      break;
    case 5:
      ICMPThresh_1s5 = icmpthresh1s_valid;
      break;
    case 6:
      ICMPThresh_1s6 = icmpthresh1s_valid;
      break;
    case 7:
      ICMPThresh_1s7 = icmpthresh1s_valid;
      break;
    case 8:
      ICMPThresh_1s8 = icmpthresh1s_valid;
      break;
    }
    ReturnMode2(key1 - '0');
  }
}

void SetDNSThresh()
{
  u8 key;
  u8 count;
  u8 key1;
Return:
  key = 0;
  count = 0;
  u32 dnsthresh_valid = 0;
start:
  key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }
  while (key != 13 && count < 16)
  {
    key = XUartLite_RecvByte(0x40600000);
    // if (( key >57) && key!=13){
    // }
    if ((count == 0) && (key == 48))
    {
      // xil_printf("%c", key);
      key = key - 48;
      dnsthresh_valid = dnsthresh_valid + key;
    }
    else if (key > 47 && key < 58)
    {
      // xil_printf("%c",key);
      key = key - 48;
      dnsthresh_valid = dnsthresh_valid * 10 + key;
      count++;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }

  if (dnsthresh_valid > 65535)
  {
  }
  else
  {
    switch (key1 - '0') // Convert char to int
    {
    case 1:
      DNSThreshold1 = dnsthresh_valid;
      break;
    case 2:
      DNSThreshold2 = dnsthresh_valid;
      break;
    case 3:
      DNSThreshold3 = dnsthresh_valid;
      break;
    case 4:
      DNSThreshold4 = dnsthresh_valid;
      break;
    case 5:
      DNSThreshold5 = dnsthresh_valid;
      break;
    case 6:
      DNSThreshold6 = dnsthresh_valid;
      break;
    case 7:
      DNSThreshold7 = dnsthresh_valid;
      break;
    case 8:
      DNSThreshold8 = dnsthresh_valid;
      break;
    default:
      break;
    }
  }
  ReturnMode2(key1 - '0'); // Pass the selected option to ReturnMode2
}

void SetIPSecThresh()
{
  u8 key;
  u8 count;
  u8 key1;
Return:
  key = 0;
  count = 0;
  u32 ipsecthresh_valid = 0;
start:
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Enter the value of incoming IPSec IKE packet threshold (PPS): ");
  key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }
  while (key != 13 && count < 16)
  {
    key = XUartLite_RecvByte(0x40600000);
    // if (( key >57) && key!=13){
    // }
    if ((count == 0) && (key == 48))
    {
      // xil_printf("%c", key);
      key = key - 48;
      ipsecthresh_valid = ipsecthresh_valid + key;
    }
    else if (key > 47 && key < 58)
    {
      // xil_printf("%c",key);
      key = key - 48;
      ipsecthresh_valid = ipsecthresh_valid * 10 + key;
      count++;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  if (ipsecthresh_valid > 65535)
  {
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+========================================================================================================================================================================+");
    // xil_printf("\r\n\t\t| The value must be less than 65536.                                                                                                                                     |");
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
    //  goto Return;
  }
  else
  {
    switch (key1 - '0') // Convert char to int
    {
    case 1:
      IPSecThreshold1 = ipsecthresh_valid;
      break;
    case 2:
      IPSecThreshold2 = ipsecthresh_valid;
      break;
    case 3:
      IPSecThreshold3 = ipsecthresh_valid;
      break;
    case 4:
      IPSecThreshold4 = ipsecthresh_valid;
      break;
    case 5:
      IPSecThreshold5 = ipsecthresh_valid;
      break;
    case 6:
      IPSecThreshold6 = ipsecthresh_valid;
      break;
    case 7:
      IPSecThreshold7 = ipsecthresh_valid;
      break;
    case 8:
      IPSecThreshold8 = ipsecthresh_valid;
      break;
    default:
      break;
    }
    ReturnMode2(key1 - '0'); // Pass the selected option to ReturnMode2
  }
}

void AddIpv4VPN()
{
  int IPv4Target_ADD = 0;
  u8 key;
  u8 count_point = 0;
  u8 count = 0;
  u8 colect_ip[2];
  u16 IPlayerA;
  u16 IPlayerB;
  u16 IPlayerC;
  u16 IPlayerD;
  u8 next;
  u8 flag_error;
Return:
  key = 0;
  count_point = 0;
  count = 0;
  colect_ip[2] = 0;
  IPlayerA = 0;
  IPlayerB = 0;
  IPlayerC = 0;
  IPlayerD = 0;
  next = 0;
  flag_error = 0;
  //	xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Enter Server IPv4 VPN address want to add : ");
  while (count_point < 5 && flag_error != 1)
  {
    key = XUartLite_RecvByte(0x40600000);
    // if((key < '0' || key > '9')&& key != '.' && key != 13){
    // 	//No dothing;
    // }
    if (key == 03)
    {
      Reset_System();
    }
    else if ((key >= '0' && key <= '9') || key == '.' || key == 13)
    {
      // xil_printf("%c", key);
      if (key == '.' && count_point < 4)
      {
        count_point++;
      }
      if (key >= '0' && key <= '9')
      {
        key = key - 48;
        count = count + 1;
        colect_ip[count - 1] = key;
      }
      if (count_point == 1 && next == 0)
      {
        if (count == 1)
        {
          IPlayerA = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IPlayerA = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IPlayerA = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count = 0;
        next = 1;
      }
      if (count_point == 2 && next == 1)
      {
        if (count == 1)
        {
          IPlayerB = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IPlayerB = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IPlayerB = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count = 0;
        next = 2;
      }
      if (count_point == 3 && next == 2)
      {
        if (count == 1)
        {
          IPlayerC = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IPlayerC = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IPlayerC = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count_point = 4;
        count = 0;
        next = 3;
      }
      if (key == 13 && count_point == 4)
      {
        if (count == 1)
        {
          IPlayerD = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IPlayerD = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IPlayerD = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count = 0;
        count_point = 5;
      }
      if (IPlayerA > 255 || IPlayerA < 0 || IPlayerB > 255 || IPlayerB < 0 || IPlayerC > 255 || IPlayerC < 0 || IPlayerD > 255 || IPlayerD < 0)
      {
        flag_error = 1;
      }
    }
  }
  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  else
  {
    IPv4Target_ADD = IPlayerA * 16777216 + IPlayerB * 65536 + IPlayerC * 256 + IPlayerD;
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 132, IPV4_Version);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 136, IPv4Target_ADD);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 404, 0x00000001);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 404, 0x00000000);
    XUartLite_SendByte(0x40600000, 'Y');
    // xil_printf("\r\n\t\t| | Server IP: %3d.%3d.%3d.%3d |", (u8)(IPv4Target_ADD >> 24), (u8)(IPv4Target_ADD >> 16), (u8)(IPv4Target_ADD >> 8), (u8)(IPv4Target_ADD));
  }
}
void AddIpv4VPN1(const char *ip_str)
{
  int IPv4Target_ADD = 0;
  u16 IPlayerA = 0;
  u16 IPlayerB = 0;
  u16 IPlayerC = 0;
  u16 IPlayerD = 0;
  u8 flag_error = 0;

  if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IPlayerA, &IPlayerB, &IPlayerC, &IPlayerD) != 4)
  {
    flag_error = 1;
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
    return;
  }

  IPv4Target_ADD = IPlayerA * 16777216 + IPlayerB * 65536 + IPlayerC * 256 + IPlayerD;
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 132, IPV4_Version);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 136, IPv4Target_ADD);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 404, 0x00000001);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 404, 0x00000000);

  XUartLite_SendByte(0x40600000, 'Y');
}
void ProcessAddIpv4VPN()
{
  char buffer_IPV4[BUFFER_SIZE_IPV4];
  ReceiveIPv4String(buffer_IPV4, BUFFER_SIZE_IPV4);
  AddIpv4VPN1(buffer_IPV4);
}
void DisplayIpVPN()
{
  u8 Rd_layerA;
  u8 Rd_layerB;
  u8 Rd_layerC;
  u8 Rd_layerD;
  u8 number_ip = 0;
  u8 done = 0;
  // xil_printf("\r\n================+=========================================================================+==============================================+");
  // xil_printf("\r\n    Display     |                         IP VPN White list Table                         |                                              |");
  // xil_printf("\r\n\t\t+=================+=======================================================+                                              |");
  // xil_printf("\r\n\t\t| Numerical Order |                    IP VPN Address                     |                                              |");
  // xil_printf("\r\n\t\t+-----------------+-------------------------------------------------------+                                              |");
  while (done == 0)
  {
    if (number_ip >= 32)
    {
      // xil_printf("\r\n\t\t+-----------------+-------------------------------------------------------+                                              |");
      done = 1;
    }
    else
    {
      /*			EEPROM_WriteIIC(&myDevice_EEPROM, (number_ip*4)+32+0, 0,1);
            delay_1ms();
            EEPROM_WriteIIC(&myDevice_EEPROM, (number_ip*4)+32+1, 0,1);
            delay_1ms();
            EEPROM_WriteIIC(&myDevice_EEPROM, (number_ip*4)+32+2, 0,1);
            delay_1ms();
            EEPROM_WriteIIC(&myDevice_EEPROM, (number_ip*4)+32+3, 0,1);
            delay_1ms();*/

      EEPROM_ReadIIC(&myDevice_EEPROM, (number_ip * 4) + 32 + 0, &Rd_layerA, 1);
      // delay_1ms();
      EEPROM_ReadIIC(&myDevice_EEPROM, (number_ip * 4) + 32 + 1, &Rd_layerB, 1);
      // delay_1ms();
      EEPROM_ReadIIC(&myDevice_EEPROM, (number_ip * 4) + 32 + 2, &Rd_layerC, 1);
      // delay_1ms();
      EEPROM_ReadIIC(&myDevice_EEPROM, (number_ip * 4) + 32 + 3, &Rd_layerD, 1);
      // delay_1ms();
      if (Rd_layerA == 0 && Rd_layerB == 0 && Rd_layerC == 0 && Rd_layerD == 0)
      {
        // xil_printf("\r\n\t\t|        %2d       |                         (null)                        |                                              |", number_ip);
      }
      else
      {
        // xil_printf("\r\n\t\t|        %2d       |                    %3d.%3d.%3d.%3d                    |                                              |", number_ip, Rd_layerA, Rd_layerB, Rd_layerC, Rd_layerD);
      }
      number_ip = number_ip + 1;
    }
  }
  // XUartLite_SendByte(0x40600000, '*');
}

void RemoveIpv4VPN()
{
  int IPv4Target_ADD = 0;
  u8 key;
  u8 count_point = 0;
  u8 count = 0;
  u8 colect_ip[2];
  u16 IPlayerA;
  u16 IPlayerB;
  u16 IPlayerC;
  u16 IPlayerD;
  u8 next;
  u8 flag_error;
Return:
  key = 0;
  count_point = 0;
  count = 0;
  colect_ip[2] = 0;
  IPlayerA = 0;
  IPlayerB = 0;
  IPlayerC = 0;
  IPlayerD = 0;
  next = 0;
  flag_error = 0;
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Enter Server IPv4 VPN address want to remove : ");
  while (count_point < 5 && flag_error != 1)
  {
    key = XUartLite_RecvByte(0x40600000);
    // if((key < '0' || key > '9')&& key != '.' && key != 13){
    // 	//No dothing;
    // }
    if (key == 03)
    {
      Reset_System();
    }
    else if ((key >= '0' && key <= '9') || key == '.' || key == 13)
    {
      // xil_printf("%c", key);
      if (key == '.' && count_point < 4)
      {
        count_point++;
      }
      if (key >= '0' && key <= '9')
      {
        key = key - 48;
        count = count + 1;
        colect_ip[count - 1] = key;
      }
      if (count_point == 1 && next == 0)
      {
        if (count == 1)
        {
          IPlayerA = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IPlayerA = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IPlayerA = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count = 0;
        next = 1;
      }
      if (count_point == 2 && next == 1)
      {
        if (count == 1)
        {
          IPlayerB = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IPlayerB = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IPlayerB = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count = 0;
        next = 2;
      }
      if (count_point == 3 && next == 2)
      {
        if (count == 1)
        {
          IPlayerC = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IPlayerC = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IPlayerC = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count_point = 4;
        count = 0;
        next = 3;
      }
      if (key == 13 && count_point == 4)
      {
        if (count == 1)
        {
          IPlayerD = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IPlayerD = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IPlayerD = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count = 0;
        count_point = 5;
      }
      if (IPlayerA > 255 || IPlayerA < 0 || IPlayerB > 255 || IPlayerB < 0 || IPlayerC > 255 || IPlayerC < 0 || IPlayerD > 255 || IPlayerD < 0)
      {
        flag_error = 1;
      }
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  else
  {
    IPv4Target_ADD = IPlayerA * 16777216 + IPlayerB * 65536 + IPlayerC * 256 + IPlayerD;
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 132, IPV4_Version);
    delay_1ms();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 136, IPv4Target_ADD);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 404, 0x00000002);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 404, 0x00000000);
    XUartLite_SendByte(0x40600000, 'Y');
    // xil_printf("\r\n\t\t| | Server IP: %3d.%3d.%3d.%3d |", (u8)(IPv4Target_ADD >> 24), (u8)(IPv4Target_ADD >> 16), (u8)(IPv4Target_ADD >> 8), (u8)(IPv4Target_ADD));
  }
}
void RemoveIpv4VPN1(const char *ip_str)
{
  int IPv4Target_ADD = 0;
  u16 IPlayerA = 0;
  u16 IPlayerB = 0;
  u16 IPlayerC = 0;
  u16 IPlayerD = 0;
  u8 flag_error = 0;

  if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IPlayerA, &IPlayerB, &IPlayerC, &IPlayerD) != 4)
  {
    flag_error = 1;
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
    return;
  }

  IPv4Target_ADD = IPlayerA * 16777216 + IPlayerB * 65536 + IPlayerC * 256 + IPlayerD;
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 132, IPV4_Version);
  delay_1ms();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 136, IPv4Target_ADD);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 404, 0x00000002);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 404, 0x00000000);
  XUartLite_SendByte(0x40600000, 'Y');
}
void ProcessRemoveIpv4VPN()
{
  char buffer_IPV4[BUFFER_SIZE_IPV4];
  ReceiveIPv4String(buffer_IPV4, BUFFER_SIZE_IPV4);
  RemoveIpv4VPN1(buffer_IPV4);
}
void AddIpVPN_IPV6()
{
  int IPv6VPN_ADD[4] = {0};
  u8 key;
  u16 IPv6Segment[IPV6_SEGMENTS] = {0};
  u8 next = 0;
  u8 flag_error = 0;
  char input_buffer[40] = {0};
  u8 count = 0;
  int double_colon_position = -1;

  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 13)
    {
      break;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else
    {
      // xil_printf("%c", key);
      input_buffer[count++] = key;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  input_buffer[count] = '\0';

  char *token = strtok(input_buffer, ":");
  while (token != NULL && next < IPV6_SEGMENTS)
  {
    if (strcmp(token, "") == 0)
    {
      if (double_colon_position == -1)
      {
        double_colon_position = next;
        token = strtok(NULL, ":");
        continue;
      }
      else
      {
        flag_error = 1;
        break;
      }
    }
    else
    {
      if (count_digits(token) > 4)
      {
        flag_error = 1;
        break;
      }
      u16 segment_value = (u16)strtol(token, NULL, 16);
      IPv6Segment[next++] = segment_value;
      // xil_printf("\r\nSegment %d set to %04x", next - 1, segment_value);
    }
    token = strtok(NULL, ":");
    // XUartLite_SendByte(0x40600000, 'H');
  }

  if (double_colon_position != -1)
  {
    int segments_to_add = IPV6_SEGMENTS - next;
    for (int i = 0; i < segments_to_add; i++)
    {
      IPv6Segment[double_colon_position + i] = 0;
    }
    for (int i = 0; token != NULL; i++)
    {
      if (next < IPV6_SEGMENTS)
      {
        u16 segment_value = (u16)strtol(token, NULL, 16);
        IPv6Segment[double_colon_position + segments_to_add + i] = segment_value;
        next++;
      }
      token = strtok(NULL, ":");
    }
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  else
  {
    //  // xil_printf("\r\n ip  ");
    //   for (int i = 0; i < IPV6_SEGMENTS; i++)
    //   {
    //     xil_printf("%04x", IPv6Segment[i]);
    //     if (i < IPV6_SEGMENTS - 1)
    //     {
    //       xil_printf(":");
    //     }
    //   }
    //}
    IPv6VPN_ADD[0] = (IPv6Segment[0] << 16) | IPv6Segment[1];
    IPv6VPN_ADD[1] = (IPv6Segment[2] << 16) | IPv6Segment[3];
    IPv6VPN_ADD[2] = (IPv6Segment[4] << 16) | IPv6Segment[5];
    IPv6VPN_ADD[3] = (IPv6Segment[6] << 16) | IPv6Segment[7];
    delay_1ms();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 132, IPV6_Version);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 136, IPv6VPN_ADD[0]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 342, IPv6VPN_ADD[1]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 346, IPv6VPN_ADD[2]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 350, IPv6VPN_ADD[3]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 354, 1);
    delay_1s();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 354, 0);
    XUartLite_SendByte(0x40600000, 'Y');
  }
}
void RemoveIpVPN_IPV6()
{
  int IPv6VPN_RM[4] = {0};
  u8 key;
  u16 IPv6Segment[IPV6_SEGMENTS] = {0};
  u8 next = 0;
  u8 flag_error = 0;
  char input_buffer[40] = {0};
  u8 count = 0;
  int double_colon_position = -1;

  // xil_printf("\r\n IPv6 : ");
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 13)
    {
      break;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else
    {
      // xil_printf("%c", key);
      input_buffer[count++] = key;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  input_buffer[count] = '\0';

  char *token = strtok(input_buffer, ":");
  while (token != NULL && next < IPV6_SEGMENTS)
  {
    if (strcmp(token, "") == 0)
    {
      if (double_colon_position == -1)
      {
        double_colon_position = next;
        token = strtok(NULL, ":");
        continue;
      }
      else
      {
        flag_error = 1;
        break;
      }
    }
    else
    {
      if (count_digits(token) > 4)
      {
        flag_error = 1;
        break;
      }
      u16 segment_value = (u16)strtol(token, NULL, 16);
      IPv6Segment[next++] = segment_value;
      // xil_printf("\r\nSegment %d set to %04x", next - 1, segment_value);
    }
    token = strtok(NULL, ":");
    // XUartLite_SendByte(0x40600000, 'H');
  }

  if (double_colon_position != -1)
  {
    int segments_to_add = IPV6_SEGMENTS - next;
    for (int i = 0; i < segments_to_add; i++)
    {
      IPv6Segment[double_colon_position + i] = 0;
    }
    for (int i = 0; token != NULL; i++)
    {
      if (next < IPV6_SEGMENTS)
      {
        u16 segment_value = (u16)strtol(token, NULL, 16);
        IPv6Segment[double_colon_position + segments_to_add + i] = segment_value;
        next++;
      }
      token = strtok(NULL, ":");
    }
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  else
  {
    // //xil_printf("\r\n ip  ");
    // for (int i = 0; i < IPV6_SEGMENTS; i++)
    // {
    //   xil_printf("%04x", IPv6Segment[i]);
    //   if (i < IPV6_SEGMENTS - 1)
    //   {
    //     xil_printf(":");
    //   }
    // }

    IPv6VPN_RM[0] = (IPv6Segment[0] << 16) | IPv6Segment[1];
    IPv6VPN_RM[1] = (IPv6Segment[2] << 16) | IPv6Segment[3];
    IPv6VPN_RM[2] = (IPv6Segment[4] << 16) | IPv6Segment[5];
    IPv6VPN_RM[3] = (IPv6Segment[6] << 16) | IPv6Segment[7];
    delay_1ms();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 132, IPV6_Version);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 136, IPv6VPN_RM[0]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 342, IPv6VPN_RM[1]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 346, IPv6VPN_RM[2]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 350, IPv6VPN_RM[3]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 354, 2);
    delay_1s();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 354, 0);
    XUartLite_SendByte(0x40600000, 'Y');
  }
}

void SetSynDefender()
{
  // XUartLite_SendByte(0x40600000, 'x');
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Do you want to enable SYN flood protect now (Y/N)?: ");
  // u8 key ;
  u8 key = 0;
  u8 done = 0;
  u8 key1 = 0;

  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);

    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
    {
      break;
    }
  }
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 'y' || key == 'Y')
    {
      switch (key1 - '0')
      {
      case 1:
        SYNFlood_en1 = 1;
        break;
      case 2:
        SYNFlood_en2 = 1;
        break;
      case 3:
        SYNFlood_en3 = 1;
        break;
      case 4:
        SYNFlood_en4 = 1;
        break;
      case 5:
        SYNFlood_en5 = 1;
        break;
      case 6:
        SYNFlood_en6 = 1;
        break;
      case 7:
        SYNFlood_en7 = 1;
        break;
      case 8:
        SYNFlood_en8 = 1;
        break;
      default:
        break;
      }
      done = 1;
    }

    else if (key == 'n' || key == 'N')
    {
      switch (key1 - '0')
      {
      case 1:
        SYNFlood_en1 = 0;
        break;
      case 2:
        SYNFlood_en2 = 0;
        break;
      case 3:
        SYNFlood_en3 = 0;
        break;
      case 4:
        SYNFlood_en4 = 0;
        break;
      case 5:
        SYNFlood_en5 = 0;
        break;
      case 6:
        SYNFlood_en6 = 0;
        break;
      case 7:

        SYNFlood_en7 = 0;
        break;
      case 8:
        SYNFlood_en8 = 0;
        break;
      }
      done = 1;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    if (key == 13 && done == 1)
    {
      break;
    }
  }
  switch (key1 - '0')
  {
  case 1:
    EnableDefender1 = (HTTPSGETFlood_en1 << 10) + (HTTPGETFlood_en1 << 9) + (UDPFragFlood_en1 << 8) + (TCPFragFlood_en1 << 7) + (IPSecFlood_en1 << 6) + (ICMPFlood_en1 << 5) + (DNSFlood_en1 << 4) + (UDPFlood_en1 << 3) + (LANDATTACK_en1 << 2) + (SYNFlood_en1 << 1) + DefenderPort_en_1;
    break;
  case 2:
    EnableDefender2 = (HTTPSGETFlood_en2 << 10) + (HTTPGETFlood_en2 << 9) + (UDPFragFlood_en2 << 8) + (TCPFragFlood_en2 << 7) + (IPSecFlood_en2 << 6) + (ICMPFlood_en2 << 5) + (DNSFlood_en2 << 4) + (UDPFlood_en2 << 3) + (LANDATTACK_en2 << 2) + (SYNFlood_en2 << 1) + DefenderPort_en_2;
    break;
  case 3:
    EnableDefender3 = (HTTPSGETFlood_en3 << 10) + (HTTPGETFlood_en3 << 9) + (UDPFragFlood_en3 << 8) + (TCPFragFlood_en3 << 7) + (IPSecFlood_en3 << 6) + (ICMPFlood_en3 << 5) + (DNSFlood_en3 << 4) + (UDPFlood_en3 << 3) + (LANDATTACK_en3 << 2) + (SYNFlood_en3 << 1) + DefenderPort_en_3;
    break;
  case 4:
    EnableDefender4 = (HTTPSGETFlood_en4 << 10) + (HTTPGETFlood_en4 << 9) + (UDPFragFlood_en4 << 8) + (TCPFragFlood_en4 << 7) + (IPSecFlood_en4 << 6) + (ICMPFlood_en4 << 5) + (DNSFlood_en4 << 4) + (UDPFlood_en4 << 3) + (LANDATTACK_en4 << 2) + (SYNFlood_en4 << 1) + DefenderPort_en_4;
    break;
  case 5:
    EnableDefender5 = (HTTPSGETFlood_en5 << 10) + (HTTPGETFlood_en5 << 9) + (UDPFragFlood_en5 << 8) + (TCPFragFlood_en5 << 7) + (IPSecFlood_en5 << 6) + (ICMPFlood_en5 << 5) + (DNSFlood_en5 << 4) + (UDPFlood_en5 << 3) + (LANDATTACK_en5 << 2) + (SYNFlood_en5 << 1) + DefenderPort_en_5;
    break;
  case 6:
    EnableDefender6 = (HTTPSGETFlood_en6 << 10) + (HTTPGETFlood_en6 << 9) + (UDPFragFlood_en6 << 8) + (TCPFragFlood_en6 << 7) + (IPSecFlood_en6 << 6) + (ICMPFlood_en6 << 5) + (DNSFlood_en6 << 4) + (UDPFlood_en6 << 3) + (LANDATTACK_en6 << 2) + (SYNFlood_en6 << 1) + DefenderPort_en_6;
    break;
  case 7:
    EnableDefender7 = (HTTPSGETFlood_en7 << 10) + (HTTPGETFlood_en7 << 9) + (UDPFragFlood_en7 << 8) + (TCPFragFlood_en7 << 7) + (IPSecFlood_en7 << 6) + (ICMPFlood_en7 << 5) + (DNSFlood_en7 << 4) + (UDPFlood_en7 << 3) + (LANDATTACK_en7 << 2) + (SYNFlood_en7 << 1) + DefenderPort_en_7;
    break;
  case 8:
    EnableDefender8 = (HTTPSGETFlood_en8 << 10) + (HTTPGETFlood_en8 << 9) + (UDPFragFlood_en8 << 8) + (TCPFragFlood_en8 << 7) + (IPSecFlood_en8 << 6) + (ICMPFlood_en8 << 5) + (DNSFlood_en8 << 4) + (UDPFlood_en8 << 3) + (LANDATTACK_en8 << 2) + (SYNFlood_en8 << 1) + DefenderPort_en_8;
    break;
  default:
    break;
  }
  ReturnMode2(key1 - '0');
}

void SetSynonymousDefender()
{
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Do you want to enable LAND Attack protect now (Y/N)?: ");
  u8 key = 0;
  u8 done4 = 0;
  u8 key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 'y' || key == 'Y')
    {
      // XUartLite_SendByte(0x40600000, 'Y');
      //  xil_printf("%c",key);
      done4 = 1;
      switch (key1 - '0')
      {
      case 1:
        LANDATTACK_en1 = 1;
        break;
      case 2:
        LANDATTACK_en2 = 1;
        break;
      case 3:
        LANDATTACK_en3 = 1;
        break;
      case 4:
        LANDATTACK_en4 = 1;
        break;
      case 5:
        LANDATTACK_en5 = 1;
        break;
      case 6:
        LANDATTACK_en6 = 1;
        break;
      case 7:
        LANDATTACK_en7 = 1;
        break;
      case 8:
        LANDATTACK_en8 = 1;
        break;
      default:
        break;
      }
    }
    else if (key == 'n' || key == 'N')
    {
      // XUartLite_SendByte(0x40600000, 'N');
      done4 = 1;
      switch (key1 - '0')
      {
      case 1:
        LANDATTACK_en1 = 0;
        break;
      case 2:
        LANDATTACK_en2 = 0;
        break;
      case 3:
        LANDATTACK_en3 = 01;
        break;
      case 4:
        LANDATTACK_en4 = 0;
        break;
      case 5:
        LANDATTACK_en5 = 0;
        break;
      case 6:
        LANDATTACK_en6 = 0;
        break;
      case 7:
        LANDATTACK_en7 = 0;
        break;
      case 8:
        LANDATTACK_en8 = 0;
        break;
      default:
        break;
      }
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else if (key == 13 && done4 == 1)
    {
      // XUartLite_SendByte(0x40600000, 'E');
      break;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  switch (key1 - '0')
  {
  case 1:
    EnableDefender1 = (HTTPSGETFlood_en1 << 10) + (HTTPGETFlood_en1 << 9) + (UDPFragFlood_en1 << 8) + (TCPFragFlood_en1 << 7) + (IPSecFlood_en1 << 6) + (ICMPFlood_en1 << 5) + (DNSFlood_en1 << 4) + (UDPFlood_en1 << 3) + (LANDATTACK_en1 << 2) + (SYNFlood_en1 << 1) + DefenderPort_en_1;
    break;
  case 2:
    EnableDefender2 = (HTTPSGETFlood_en2 << 10) + (HTTPGETFlood_en2 << 9) + (UDPFragFlood_en2 << 8) + (TCPFragFlood_en2 << 7) + (IPSecFlood_en2 << 6) + (ICMPFlood_en2 << 5) + (DNSFlood_en2 << 4) + (UDPFlood_en2 << 3) + (LANDATTACK_en2 << 2) + (SYNFlood_en2 << 1) + DefenderPort_en_2;
    break;
  case 3:
    EnableDefender3 = (HTTPSGETFlood_en3 << 10) + (HTTPGETFlood_en3 << 9) + (UDPFragFlood_en3 << 8) + (TCPFragFlood_en3 << 7) + (IPSecFlood_en3 << 6) + (ICMPFlood_en3 << 5) + (DNSFlood_en3 << 4) + (UDPFlood_en3 << 3) + (LANDATTACK_en3 << 2) + (SYNFlood_en3 << 1) + DefenderPort_en_3;
    break;
  case 4:
    EnableDefender4 = (HTTPSGETFlood_en4 << 10) + (HTTPGETFlood_en4 << 9) + (UDPFragFlood_en4 << 8) + (TCPFragFlood_en4 << 7) + (IPSecFlood_en4 << 6) + (ICMPFlood_en4 << 5) + (DNSFlood_en4 << 4) + (UDPFlood_en4 << 3) + (LANDATTACK_en4 << 2) + (SYNFlood_en4 << 1) + DefenderPort_en_4;
    break;
  case 5:
    EnableDefender5 = (HTTPSGETFlood_en5 << 10) + (HTTPGETFlood_en5 << 9) + (UDPFragFlood_en5 << 8) + (TCPFragFlood_en5 << 7) + (IPSecFlood_en5 << 6) + (ICMPFlood_en5 << 5) + (DNSFlood_en5 << 4) + (UDPFlood_en5 << 3) + (LANDATTACK_en5 << 2) + (SYNFlood_en5 << 1) + DefenderPort_en_5;
    break;
  case 6:
    EnableDefender6 = (HTTPSGETFlood_en6 << 10) + (HTTPGETFlood_en6 << 9) + (UDPFragFlood_en6 << 8) + (TCPFragFlood_en6 << 7) + (IPSecFlood_en6 << 6) + (ICMPFlood_en6 << 5) + (DNSFlood_en6 << 4) + (UDPFlood_en6 << 3) + (LANDATTACK_en6 << 2) + (SYNFlood_en6 << 1) + DefenderPort_en_6;
    break;
  case 7:
    EnableDefender7 = (HTTPSGETFlood_en7 << 10) + (HTTPGETFlood_en7 << 9) + (UDPFragFlood_en7 << 8) + (TCPFragFlood_en7 << 7) + (IPSecFlood_en7 << 6) + (ICMPFlood_en7 << 5) + (DNSFlood_en7 << 4) + (UDPFlood_en7 << 3) + (LANDATTACK_en7 << 2) + (SYNFlood_en7 << 1) + DefenderPort_en_7;
    break;
  case 8:
    EnableDefender8 = (HTTPSGETFlood_en8 << 10) + (HTTPGETFlood_en8 << 9) + (UDPFragFlood_en8 << 8) + (TCPFragFlood_en8 << 7) + (IPSecFlood_en8 << 6) + (ICMPFlood_en8 << 5) + (DNSFlood_en8 << 4) + (UDPFlood_en8 << 3) + (LANDATTACK_en8 << 2) + (SYNFlood_en8 << 1) + DefenderPort_en_8;
    break;
  default:
    break;
  }
  ReturnMode2(key1 - '0');
}

void SetUDPDefender()
{
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Do you want to enable UDP flood protect now (Y/N)?: ");
  u8 key = 0;
  u8 done = 0;
  u8 key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }

  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 'y' || key == 'Y')
    {
      switch (key1 - '0')
      {
      case 1:
        UDPFlood_en1 = 1;
        break;
      case 2:
        UDPFlood_en2 = 1;
        break;
      case 3:
        UDPFlood_en3 = 1;
        break;
      case 4:
        UDPFlood_en4 = 1;
        break;
      case 5:
        UDPFlood_en5 = 1;
        break;
      case 6:
        UDPFlood_en6 = 1;
        break;
      case 7:
        UDPFlood_en7 = 1;
        break;
      case 8:
        UDPFlood_en8 = 1;
        break;
      default:
        break;
      }

      done = 1;
    }
    else if (key == 'n' || key == 'N')
    {
      switch (key1 - '0')
      {
      case 1:
        UDPFlood_en1 = 0;
        break;
      case 2:
        UDPFlood_en2 = 0;
        break;
      case 3:
        UDPFlood_en3 = 0;
        break;
      case 4:
        UDPFlood_en4 = 0;
        break;
      case 5:
        UDPFlood_en5 = 0;
        break;
      case 6:
        UDPFlood_en6 = 0;
        break;
      case 7:
        UDPFlood_en7 = 0;
        break;
      case 8:
        UDPFlood_en8 = 0;
        break;
      default:
        break;
      }
      done = 1;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else if (key == 13 && done == 1)
    {
      // XUartLite_SendByte(0x40600000, 'E');
      break;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  switch (key1 - '0')
  {
  case 1:
    EnableDefender1 = (HTTPSGETFlood_en1 << 10) + (HTTPGETFlood_en1 << 9) + (UDPFragFlood_en1 << 8) + (TCPFragFlood_en1 << 7) + (IPSecFlood_en1 << 6) + (ICMPFlood_en1 << 5) + (DNSFlood_en1 << 4) + (UDPFlood_en1 << 3) + (LANDATTACK_en1 << 2) + (SYNFlood_en1 << 1) + DefenderPort_en_1;
    break;
  case 2:
    EnableDefender2 = (HTTPSGETFlood_en2 << 10) + (HTTPGETFlood_en2 << 9) + (UDPFragFlood_en2 << 8) + (TCPFragFlood_en2 << 7) + (IPSecFlood_en2 << 6) + (ICMPFlood_en2 << 5) + (DNSFlood_en2 << 4) + (UDPFlood_en2 << 3) + (LANDATTACK_en2 << 2) + (SYNFlood_en2 << 1) + DefenderPort_en_2;
    break;
  case 3:
    EnableDefender3 = (HTTPSGETFlood_en3 << 10) + (HTTPGETFlood_en3 << 9) + (UDPFragFlood_en3 << 8) + (TCPFragFlood_en3 << 7) + (IPSecFlood_en3 << 6) + (ICMPFlood_en3 << 5) + (DNSFlood_en3 << 4) + (UDPFlood_en3 << 3) + (LANDATTACK_en3 << 2) + (SYNFlood_en3 << 1) + DefenderPort_en_3;
    break;
  case 4:
    EnableDefender4 = (HTTPSGETFlood_en4 << 10) + (HTTPGETFlood_en4 << 9) + (UDPFragFlood_en4 << 8) + (TCPFragFlood_en4 << 7) + (IPSecFlood_en4 << 6) + (ICMPFlood_en4 << 5) + (DNSFlood_en4 << 4) + (UDPFlood_en4 << 3) + (LANDATTACK_en4 << 2) + (SYNFlood_en4 << 1) + DefenderPort_en_4;
    break;
  case 5:
    EnableDefender5 = (HTTPSGETFlood_en5 << 10) + (HTTPGETFlood_en5 << 9) + (UDPFragFlood_en5 << 8) + (TCPFragFlood_en5 << 7) + (IPSecFlood_en5 << 6) + (ICMPFlood_en5 << 5) + (DNSFlood_en5 << 4) + (UDPFlood_en5 << 3) + (LANDATTACK_en5 << 2) + (SYNFlood_en5 << 1) + DefenderPort_en_5;
    break;
  case 6:
    EnableDefender6 = (HTTPSGETFlood_en6 << 10) + (HTTPGETFlood_en6 << 9) + (UDPFragFlood_en6 << 8) + (TCPFragFlood_en6 << 7) + (IPSecFlood_en6 << 6) + (ICMPFlood_en6 << 5) + (DNSFlood_en6 << 4) + (UDPFlood_en6 << 3) + (LANDATTACK_en6 << 2) + (SYNFlood_en6 << 1) + DefenderPort_en_6;
    break;
  case 7:
    EnableDefender7 = (HTTPSGETFlood_en7 << 10) + (HTTPGETFlood_en7 << 9) + (UDPFragFlood_en7 << 8) + (TCPFragFlood_en7 << 7) + (IPSecFlood_en7 << 6) + (ICMPFlood_en7 << 5) + (DNSFlood_en7 << 4) + (UDPFlood_en7 << 3) + (LANDATTACK_en7 << 2) + (SYNFlood_en7 << 1) + DefenderPort_en_7;
    break;
  case 8:
    EnableDefender8 = (HTTPSGETFlood_en8 << 10) + (HTTPGETFlood_en8 << 9) + (UDPFragFlood_en8 << 8) + (TCPFragFlood_en8 << 7) + (IPSecFlood_en8 << 6) + (ICMPFlood_en8 << 5) + (DNSFlood_en8 << 4) + (UDPFlood_en8 << 3) + (LANDATTACK_en8 << 2) + (SYNFlood_en8 << 1) + DefenderPort_en_8;
    break;
  default:
    break;
  }

  ReturnMode2(key1 - '0');
}

void SetDNSDefender()
{
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Do you want to enable DNS flood protect now (Y/N)?: ");
  u8 key = 0;
  u8 done = 0;
  u8 key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 'y' || key == 'Y')
    {
      // XUartLite_SendByte(0x40600000, 'Y');
      done = 1;
      switch (key1 - '0')
      {
      case 1:
        DNSFlood_en1 = 1;
        break;
      case 2:
        DNSFlood_en2 = 1;
        break;
      case 3:
        DNSFlood_en3 = 1;
        break;
      case 4:
        DNSFlood_en4 = 1;
        break;
      case 5:
        DNSFlood_en5 = 1;
        break;
      case 6:
        DNSFlood_en6 = 1;
        break;
      case 7:
        DNSFlood_en7 = 1;
        break;
      case 8:
        DNSFlood_en8 = 1;
        break;
      default:
        break;
      }
    }
    else if (key == 'n' || key == 'N')
    {
      // XUartLite_SendByte(0x40600000, 'N');
      done = 1;
      switch (key1 - '0')
      {
      case 1:
        DNSFlood_en1 = 0;
        break;
      case 2:
        DNSFlood_en2 = 0;
        break;
      case 3:
        DNSFlood_en3 = 0;
        break;
      case 4:
        DNSFlood_en4 = 0;
        break;
      case 5:
        DNSFlood_en5 = 0;
        break;
      case 6:
        DNSFlood_en6 = 0;
        break;
      case 7:
        DNSFlood_en7 = 0;
        break;
      case 8:
        DNSFlood_en8 = 0;
        break;
      default:
        break;
      }
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else if (key == 13 && done == 1)
    {
      break;
      // XUartLite_SendByte(0x40600000, 'E');
    }
  }
  // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
  // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
  switch (key1 - '0')
  {
  case 1:
    EnableDefender1 = (HTTPSGETFlood_en1 << 10) + (HTTPGETFlood_en1 << 9) + (UDPFragFlood_en1 << 8) + (TCPFragFlood_en1 << 7) + (IPSecFlood_en1 << 6) + (ICMPFlood_en1 << 5) + (DNSFlood_en1 << 4) + (UDPFlood_en1 << 3) + (LANDATTACK_en1 << 2) + (SYNFlood_en1 << 1) + DefenderPort_en_1;
    break;
  case 2:
    EnableDefender2 = (HTTPSGETFlood_en2 << 10) + (HTTPGETFlood_en2 << 9) + (UDPFragFlood_en2 << 8) + (TCPFragFlood_en2 << 7) + (IPSecFlood_en2 << 6) + (ICMPFlood_en2 << 5) + (DNSFlood_en2 << 4) + (UDPFlood_en2 << 3) + (LANDATTACK_en2 << 2) + (SYNFlood_en2 << 1) + DefenderPort_en_2;
    break;
  case 3:
    EnableDefender3 = (HTTPSGETFlood_en3 << 10) + (HTTPGETFlood_en3 << 9) + (UDPFragFlood_en3 << 8) + (TCPFragFlood_en3 << 7) + (IPSecFlood_en3 << 6) + (ICMPFlood_en3 << 5) + (DNSFlood_en3 << 4) + (UDPFlood_en3 << 3) + (LANDATTACK_en3 << 2) + (SYNFlood_en3 << 1) + DefenderPort_en_3;
    break;
  case 4:
    EnableDefender4 = (HTTPSGETFlood_en4 << 10) + (HTTPGETFlood_en4 << 9) + (UDPFragFlood_en4 << 8) + (TCPFragFlood_en4 << 7) + (IPSecFlood_en4 << 6) + (ICMPFlood_en4 << 5) + (DNSFlood_en4 << 4) + (UDPFlood_en4 << 3) + (LANDATTACK_en4 << 2) + (SYNFlood_en4 << 1) + DefenderPort_en_4;
    break;
  case 5:
    EnableDefender5 = (HTTPSGETFlood_en5 << 10) + (HTTPGETFlood_en5 << 9) + (UDPFragFlood_en5 << 8) + (TCPFragFlood_en5 << 7) + (IPSecFlood_en5 << 6) + (ICMPFlood_en5 << 5) + (DNSFlood_en5 << 4) + (UDPFlood_en5 << 3) + (LANDATTACK_en5 << 2) + (SYNFlood_en5 << 1) + DefenderPort_en_5;
    break;
  case 6:
    EnableDefender6 = (HTTPSGETFlood_en6 << 10) + (HTTPGETFlood_en6 << 9) + (UDPFragFlood_en6 << 8) + (TCPFragFlood_en6 << 7) + (IPSecFlood_en6 << 6) + (ICMPFlood_en6 << 5) + (DNSFlood_en6 << 4) + (UDPFlood_en6 << 3) + (LANDATTACK_en6 << 2) + (SYNFlood_en6 << 1) + DefenderPort_en_6;
    break;
  case 7:
    EnableDefender7 = (HTTPSGETFlood_en7 << 10) + (HTTPGETFlood_en7 << 9) + (UDPFragFlood_en7 << 8) + (TCPFragFlood_en7 << 7) + (IPSecFlood_en7 << 6) + (ICMPFlood_en7 << 5) + (DNSFlood_en7 << 4) + (UDPFlood_en7 << 3) + (LANDATTACK_en7 << 2) + (SYNFlood_en7 << 1) + DefenderPort_en_7;
    break;
  case 8:
    EnableDefender8 = (HTTPSGETFlood_en8 << 10) + (HTTPGETFlood_en8 << 9) + (UDPFragFlood_en8 << 8) + (TCPFragFlood_en8 << 7) + (IPSecFlood_en8 << 6) + (ICMPFlood_en8 << 5) + (DNSFlood_en8 << 4) + (UDPFlood_en8 << 3) + (LANDATTACK_en8 << 2) + (SYNFlood_en8 << 1) + DefenderPort_en_8;
    break;
  default:
    break;
  }
  ReturnMode2(key1 - '0');
}

void SetICMPDefender()
{
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Do you want to enable ICMP flood protect now (Y/N)?: ");
  u8 key = 0;
  u8 done = 0;
  u8 key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 'y' || key == 'Y')
    {
      // XUartLite_SendByte(0x40600000, 'Y');
      done = 1;
      switch (key1 - '0')
      {
      case 1:
        ICMPFlood_en1 = 1;
        break;
      case 2:
        ICMPFlood_en2 = 1;
        break;
      case 3:
        ICMPFlood_en3 = 1;
        break;
      case 4:
        ICMPFlood_en4 = 1;
        break;
      case 5:
        ICMPFlood_en5 = 1;
        break;
      case 6:
        ICMPFlood_en6 = 1;
        break;
      case 7:
        ICMPFlood_en7 = 1;
        break;
      case 8:
        ICMPFlood_en8 = 1;
        break;
      default:
        break;
      }
    }
    else if (key == 'n' || key == 'N')
    {
      // XUartLite_SendByte(0x40600000, 'N');
      done = 1;
      switch (key1 - '0')
      {
      case 1:
        ICMPFlood_en1 = 0;
        break;
      case 2:
        ICMPFlood_en2 = 0;
        break;
      case 3:
        ICMPFlood_en3 = 0;
        break;
      case 4:
        ICMPFlood_en4 = 0;
        break;
      case 5:
        ICMPFlood_en5 = 0;
        break;
      case 6:
        ICMPFlood_en6 = 0;
        break;
      case 7:
        ICMPFlood_en7 = 0;
        break;
      case 8:
        ICMPFlood_en8 = 0;
        break;
      default:
        break;
      }
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else if (key == 13 && done == 1)
    {
      // XUartLite_SendByte(0x40600000, 'E');
      break;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
  // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
  switch (key1 - '0')
  {
  case 1:
    EnableDefender1 = (HTTPSGETFlood_en1 << 10) + (HTTPGETFlood_en1 << 9) + (UDPFragFlood_en1 << 8) + (TCPFragFlood_en1 << 7) + (IPSecFlood_en1 << 6) + (ICMPFlood_en1 << 5) + (DNSFlood_en1 << 4) + (UDPFlood_en1 << 3) + (LANDATTACK_en1 << 2) + (SYNFlood_en1 << 1) + DefenderPort_en_1;
    break;
  case 2:
    EnableDefender2 = (HTTPSGETFlood_en2 << 10) + (HTTPGETFlood_en2 << 9) + (UDPFragFlood_en2 << 8) + (TCPFragFlood_en2 << 7) + (IPSecFlood_en2 << 6) + (ICMPFlood_en2 << 5) + (DNSFlood_en2 << 4) + (UDPFlood_en2 << 3) + (LANDATTACK_en2 << 2) + (SYNFlood_en2 << 1) + DefenderPort_en_2;
    break;
  case 3:
    EnableDefender3 = (HTTPSGETFlood_en3 << 10) + (HTTPGETFlood_en3 << 9) + (UDPFragFlood_en3 << 8) + (TCPFragFlood_en3 << 7) + (IPSecFlood_en3 << 6) + (ICMPFlood_en3 << 5) + (DNSFlood_en3 << 4) + (UDPFlood_en3 << 3) + (LANDATTACK_en3 << 2) + (SYNFlood_en3 << 1) + DefenderPort_en_3;
    break;
  case 4:
    EnableDefender4 = (HTTPSGETFlood_en4 << 10) + (HTTPGETFlood_en4 << 9) + (UDPFragFlood_en4 << 8) + (TCPFragFlood_en4 << 7) + (IPSecFlood_en4 << 6) + (ICMPFlood_en4 << 5) + (DNSFlood_en4 << 4) + (UDPFlood_en4 << 3) + (LANDATTACK_en4 << 2) + (SYNFlood_en4 << 1) + DefenderPort_en_4;
    break;
  case 5:
    EnableDefender5 = (HTTPSGETFlood_en5 << 10) + (HTTPGETFlood_en5 << 9) + (UDPFragFlood_en5 << 8) + (TCPFragFlood_en5 << 7) + (IPSecFlood_en5 << 6) + (ICMPFlood_en5 << 5) + (DNSFlood_en5 << 4) + (UDPFlood_en5 << 3) + (LANDATTACK_en5 << 2) + (SYNFlood_en5 << 1) + DefenderPort_en_5;
    break;
  case 6:
    EnableDefender6 = (HTTPSGETFlood_en6 << 10) + (HTTPGETFlood_en6 << 9) + (UDPFragFlood_en6 << 8) + (TCPFragFlood_en6 << 7) + (IPSecFlood_en6 << 6) + (ICMPFlood_en6 << 5) + (DNSFlood_en6 << 4) + (UDPFlood_en6 << 3) + (LANDATTACK_en6 << 2) + (SYNFlood_en6 << 1) + DefenderPort_en_6;
    break;
  case 7:
    EnableDefender7 = (HTTPSGETFlood_en7 << 10) + (HTTPGETFlood_en7 << 9) + (UDPFragFlood_en7 << 8) + (TCPFragFlood_en7 << 7) + (IPSecFlood_en7 << 6) + (ICMPFlood_en7 << 5) + (DNSFlood_en7 << 4) + (UDPFlood_en7 << 3) + (LANDATTACK_en7 << 2) + (SYNFlood_en7 << 1) + DefenderPort_en_7;
    break;
  case 8:
    EnableDefender8 = (HTTPSGETFlood_en8 << 10) + (HTTPGETFlood_en8 << 9) + (UDPFragFlood_en8 << 8) + (TCPFragFlood_en8 << 7) + (IPSecFlood_en8 << 6) + (ICMPFlood_en8 << 5) + (DNSFlood_en8 << 4) + (UDPFlood_en8 << 3) + (LANDATTACK_en8 << 2) + (SYNFlood_en8 << 1) + DefenderPort_en_8;
    break;
  default:
    break;
  }
  ReturnMode2(key1 - '0');
}

void SetIPSecDefender()
{
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Do you want to enable IPSec IKE flood protect now (Y/N)?: ");
  u8 key = 0;
  u8 done = 0;
  u8 key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 'y' || key == 'Y')
    {
      // XUartLite_SendByte(0x40600000, 'Y');
      done = 1;

      switch (key1 - '0')
      {
      case 1:
        IPSecFlood_en1 = 1;
        break;
      case 2:
        IPSecFlood_en2 = 1;
        break;
      case 3:
        IPSecFlood_en3 = 1;
        break;
      case 4:
        IPSecFlood_en4 = 1;
        break;
      case 5:
        IPSecFlood_en5 = 1;
        break;
      case 6:
        IPSecFlood_en6 = 1;
        break;
      case 7:
        IPSecFlood_en7 = 1;
        break;
      case 8:
        IPSecFlood_en8 = 1;
        break;
      default:
        break;
      }
    }
    else if (key == 'n' || key == 'N')
    {
      // XUartLite_SendByte(0x40600000, 'N');
      done = 1;
      switch (key1 - '0')
      {
      case 1:
        IPSecFlood_en1 = 0;
        break;
      case 2:
        IPSecFlood_en2 = 0;
        break;
      case 3:
        IPSecFlood_en3 = 0;
        break;
      case 4:
        IPSecFlood_en4 = 0;
        break;
      case 5:
        IPSecFlood_en5 = 0;
        break;
      case 6:
        IPSecFlood_en6 = 0;
        break;
      case 7:
        IPSecFlood_en7 = 0;
        break;
      case 8:
        IPSecFlood_en8 = 0;
        break;
      default:
        break;
      }
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else if (key == 13 && done == 1)
    {
      // XUartLite_SendByte(0x40600000, 'E');
      break;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
  // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
  switch (key1 - '0')
  {
  case 1:
    EnableDefender1 = (HTTPSGETFlood_en1 << 10) + (HTTPGETFlood_en1 << 9) + (UDPFragFlood_en1 << 8) + (TCPFragFlood_en1 << 7) + (IPSecFlood_en1 << 6) + (ICMPFlood_en1 << 5) + (DNSFlood_en1 << 4) + (UDPFlood_en1 << 3) + (LANDATTACK_en1 << 2) + (SYNFlood_en1 << 1) + DefenderPort_en_1;
    break;
  case 2:
    EnableDefender2 = (HTTPSGETFlood_en2 << 10) + (HTTPGETFlood_en2 << 9) + (UDPFragFlood_en2 << 8) + (TCPFragFlood_en2 << 7) + (IPSecFlood_en2 << 6) + (ICMPFlood_en2 << 5) + (DNSFlood_en2 << 4) + (UDPFlood_en2 << 3) + (LANDATTACK_en2 << 2) + (SYNFlood_en2 << 1) + DefenderPort_en_2;
    break;
  case 3:
    EnableDefender3 = (HTTPSGETFlood_en3 << 10) + (HTTPGETFlood_en3 << 9) + (UDPFragFlood_en3 << 8) + (TCPFragFlood_en3 << 7) + (IPSecFlood_en3 << 6) + (ICMPFlood_en3 << 5) + (DNSFlood_en3 << 4) + (UDPFlood_en3 << 3) + (LANDATTACK_en3 << 2) + (SYNFlood_en3 << 1) + DefenderPort_en_3;
    break;
  case 4:
    EnableDefender4 = (HTTPSGETFlood_en4 << 10) + (HTTPGETFlood_en4 << 9) + (UDPFragFlood_en4 << 8) + (TCPFragFlood_en4 << 7) + (IPSecFlood_en4 << 6) + (ICMPFlood_en4 << 5) + (DNSFlood_en4 << 4) + (UDPFlood_en4 << 3) + (LANDATTACK_en4 << 2) + (SYNFlood_en4 << 1) + DefenderPort_en_4;
    break;
  case 5:
    EnableDefender5 = (HTTPSGETFlood_en5 << 10) + (HTTPGETFlood_en5 << 9) + (UDPFragFlood_en5 << 8) + (TCPFragFlood_en5 << 7) + (IPSecFlood_en5 << 6) + (ICMPFlood_en5 << 5) + (DNSFlood_en5 << 4) + (UDPFlood_en5 << 3) + (LANDATTACK_en5 << 2) + (SYNFlood_en5 << 1) + DefenderPort_en_5;
    break;
  case 6:
    EnableDefender6 = (HTTPSGETFlood_en6 << 10) + (HTTPGETFlood_en6 << 9) + (UDPFragFlood_en6 << 8) + (TCPFragFlood_en6 << 7) + (IPSecFlood_en6 << 6) + (ICMPFlood_en6 << 5) + (DNSFlood_en6 << 4) + (UDPFlood_en6 << 3) + (LANDATTACK_en6 << 2) + (SYNFlood_en6 << 1) + DefenderPort_en_6;
    break;
  case 7:
    EnableDefender7 = (HTTPSGETFlood_en7 << 10) + (HTTPGETFlood_en7 << 9) + (UDPFragFlood_en7 << 8) + (TCPFragFlood_en7 << 7) + (IPSecFlood_en7 << 6) + (ICMPFlood_en7 << 5) + (DNSFlood_en7 << 4) + (UDPFlood_en7 << 3) + (LANDATTACK_en7 << 2) + (SYNFlood_en7 << 1) + DefenderPort_en_7;
    break;
  case 8:
    EnableDefender8 = (HTTPSGETFlood_en8 << 10) + (HTTPGETFlood_en8 << 9) + (UDPFragFlood_en8 << 8) + (TCPFragFlood_en8 << 7) + (IPSecFlood_en8 << 6) + (ICMPFlood_en8 << 5) + (DNSFlood_en8 << 4) + (UDPFlood_en8 << 3) + (LANDATTACK_en8 << 2) + (SYNFlood_en8 << 1) + DefenderPort_en_8;
    break;
  default:
    break;
  }
  ReturnMode2(key1 - '0');
}

void SetTCPFragDefender()
{
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Do you want to enable TCP Fragmentation flood protect now (Y/N)?: ");
  u8 key = 0;
  u8 done = 0;
  u8 key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 'y' || key == 'Y')
    {
      // XUartLite_SendByte(0x40600000, 'Y');
      done = 1;
      switch (key1 - '0')
      {
      case 1:
        TCPFragFlood_en1 = 1;
        break;
      case 2:
        TCPFragFlood_en2 = 1;
        break;
      case 3:
        TCPFragFlood_en3 = 1;
        break;
      case 4:
        TCPFragFlood_en4 = 1;
        break;
      case 5:
        TCPFragFlood_en5 = 1;
        break;
      case 6:
        TCPFragFlood_en6 = 1;
        break;
      case 7:
        TCPFragFlood_en7 = 1;
        break;
      case 8:
        TCPFragFlood_en8 = 1;
        break;
      default:
        break;
      }
    }
    else if (key == 'n' || key == 'N')
    {
      // XUartLite_SendByte(0x40600000, 'N');
      done = 1;
      switch (key1 - '0')
      {
      case 1:
        TCPFragFlood_en1 = 0;
        break;
      case 2:
        TCPFragFlood_en2 = 0;
        break;
      case 3:
        TCPFragFlood_en3 = 0;
        break;
      case 4:
        TCPFragFlood_en4 = 0;
        break;
      case 5:
        TCPFragFlood_en5 = 0;
        break;
      case 6:
        TCPFragFlood_en6 = 0;
        break;
      case 7:
        TCPFragFlood_en7 = 0;
        break;
      case 8:
        TCPFragFlood_en8 = 0;
        break;
      default:
        break;
      }
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else if (key == 13 && done == 1)
    {
      // XUartLite_SendByte(0x40600000, 'E');
      break;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
  // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
  switch (key1 - '0')
  {
  case 1:
    EnableDefender1 = (HTTPSGETFlood_en1 << 10) + (HTTPGETFlood_en1 << 9) + (UDPFragFlood_en1 << 8) + (TCPFragFlood_en1 << 7) + (IPSecFlood_en1 << 6) + (ICMPFlood_en1 << 5) + (DNSFlood_en1 << 4) + (UDPFlood_en1 << 3) + (LANDATTACK_en1 << 2) + (SYNFlood_en1 << 1) + DefenderPort_en_1;
    break;
  case 2:
    EnableDefender2 = (HTTPSGETFlood_en2 << 10) + (HTTPGETFlood_en2 << 9) + (UDPFragFlood_en2 << 8) + (TCPFragFlood_en2 << 7) + (IPSecFlood_en2 << 6) + (ICMPFlood_en2 << 5) + (DNSFlood_en2 << 4) + (UDPFlood_en2 << 3) + (LANDATTACK_en2 << 2) + (SYNFlood_en2 << 1) + DefenderPort_en_2;
    break;
  case 3:
    EnableDefender3 = (HTTPSGETFlood_en3 << 10) + (HTTPGETFlood_en3 << 9) + (UDPFragFlood_en3 << 8) + (TCPFragFlood_en3 << 7) + (IPSecFlood_en3 << 6) + (ICMPFlood_en3 << 5) + (DNSFlood_en3 << 4) + (UDPFlood_en3 << 3) + (LANDATTACK_en3 << 2) + (SYNFlood_en3 << 1) + DefenderPort_en_3;
    break;
  case 4:
    EnableDefender4 = (HTTPSGETFlood_en4 << 10) + (HTTPGETFlood_en4 << 9) + (UDPFragFlood_en4 << 8) + (TCPFragFlood_en4 << 7) + (IPSecFlood_en4 << 6) + (ICMPFlood_en4 << 5) + (DNSFlood_en4 << 4) + (UDPFlood_en4 << 3) + (LANDATTACK_en4 << 2) + (SYNFlood_en4 << 1) + DefenderPort_en_4;
    break;
  case 5:
    EnableDefender5 = (HTTPSGETFlood_en5 << 10) + (HTTPGETFlood_en5 << 9) + (UDPFragFlood_en5 << 8) + (TCPFragFlood_en5 << 7) + (IPSecFlood_en5 << 6) + (ICMPFlood_en5 << 5) + (DNSFlood_en5 << 4) + (UDPFlood_en5 << 3) + (LANDATTACK_en5 << 2) + (SYNFlood_en5 << 1) + DefenderPort_en_5;
    break;
  case 6:
    EnableDefender6 = (HTTPSGETFlood_en6 << 10) + (HTTPGETFlood_en6 << 9) + (UDPFragFlood_en6 << 8) + (TCPFragFlood_en6 << 7) + (IPSecFlood_en6 << 6) + (ICMPFlood_en6 << 5) + (DNSFlood_en6 << 4) + (UDPFlood_en6 << 3) + (LANDATTACK_en6 << 2) + (SYNFlood_en6 << 1) + DefenderPort_en_6;
    break;
  case 7:
    EnableDefender7 = (HTTPSGETFlood_en7 << 10) + (HTTPGETFlood_en7 << 9) + (UDPFragFlood_en7 << 8) + (TCPFragFlood_en7 << 7) + (IPSecFlood_en7 << 6) + (ICMPFlood_en7 << 5) + (DNSFlood_en7 << 4) + (UDPFlood_en7 << 3) + (LANDATTACK_en7 << 2) + (SYNFlood_en7 << 1) + DefenderPort_en_7;
    break;
  case 8:
    EnableDefender8 = (HTTPSGETFlood_en8 << 10) + (HTTPGETFlood_en8 << 9) + (UDPFragFlood_en8 << 8) + (TCPFragFlood_en8 << 7) + (IPSecFlood_en8 << 6) + (ICMPFlood_en8 << 5) + (DNSFlood_en8 << 4) + (UDPFlood_en8 << 3) + (LANDATTACK_en8 << 2) + (SYNFlood_en8 << 1) + DefenderPort_en_8;
    break;
  default:
    break;
  }
  ReturnMode2(key1 - '0');
}

void SetUDPFragDefender()
{
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Do you want to enable UDP Fragmentation flood protect now (Y/N)?: ");
  u8 key = 0;
  u8 done = 0;
  u8 key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 'y' || key == 'Y')
    {
      // XUartLite_SendByte(0x40600000, 'Y');
      done = 1;
      switch (key1 - '0')
      {
      case 1:
        UDPFragFlood_en1 = 1;
        break;
      case 2:
        UDPFragFlood_en2 = 1;
        break;
      case 3:
        UDPFragFlood_en3 = 1;
        break;
      case 4:
        UDPFragFlood_en4 = 1;
        break;
      case 5:
        UDPFragFlood_en5 = 1;
        break;
      case 6:
        UDPFragFlood_en6 = 1;
        break;
      case 7:
        UDPFragFlood_en7 = 1;
        break;
      case 8:
        UDPFragFlood_en8 = 1;
        break;
      default:
        break;
      }
    }
    else if (key == 'n' || key == 'N')
    {
      // XUartLite_SendByte(0x40600000, 'N');
      done = 1;
      switch (key1 - '0')
      {
      case 1:
        UDPFragFlood_en1 = 0;
        break;
      case 2:
        UDPFragFlood_en2 = 0;
        break;
      case 3:
        UDPFragFlood_en3 = 0;
        break;
      case 4:
        UDPFragFlood_en4 = 0;
        break;
      case 5:
        UDPFragFlood_en5 = 0;
        break;
      case 6:
        UDPFragFlood_en6 = 0;
        break;
      case 7:
        UDPFragFlood_en7 = 0;
        break;
      case 8:
        UDPFragFlood_en8 = 0;
        break;
      default:
        break;
      }
    }
    else if (key == 03)
    {
      // xil_printf("Loading Bitstream.........");
      Reset_System();
    }
    else if (key == 13 && done == 1)
    {
      // XUartLite_SendByte(0x40600000, 'E');
      break;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
  // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
  switch (key1 - '0')
  {
  case 1:
    EnableDefender1 = (HTTPSGETFlood_en1 << 10) + (HTTPGETFlood_en1 << 9) + (UDPFragFlood_en1 << 8) + (TCPFragFlood_en1 << 7) + (IPSecFlood_en1 << 6) + (ICMPFlood_en1 << 5) + (DNSFlood_en1 << 4) + (UDPFlood_en1 << 3) + (LANDATTACK_en1 << 2) + (SYNFlood_en1 << 1) + DefenderPort_en_1;
    break;
  case 2:
    EnableDefender2 = (HTTPSGETFlood_en2 << 10) + (HTTPGETFlood_en2 << 9) + (UDPFragFlood_en2 << 8) + (TCPFragFlood_en2 << 7) + (IPSecFlood_en2 << 6) + (ICMPFlood_en2 << 5) + (DNSFlood_en2 << 4) + (UDPFlood_en2 << 3) + (LANDATTACK_en2 << 2) + (SYNFlood_en2 << 1) + DefenderPort_en_2;
    break;
  case 3:
    EnableDefender3 = (HTTPSGETFlood_en3 << 10) + (HTTPGETFlood_en3 << 9) + (UDPFragFlood_en3 << 8) + (TCPFragFlood_en3 << 7) + (IPSecFlood_en3 << 6) + (ICMPFlood_en3 << 5) + (DNSFlood_en3 << 4) + (UDPFlood_en3 << 3) + (LANDATTACK_en3 << 2) + (SYNFlood_en3 << 1) + DefenderPort_en_3;
    break;
  case 4:
    EnableDefender4 = (HTTPSGETFlood_en4 << 10) + (HTTPGETFlood_en4 << 9) + (UDPFragFlood_en4 << 8) + (TCPFragFlood_en4 << 7) + (IPSecFlood_en4 << 6) + (ICMPFlood_en4 << 5) + (DNSFlood_en4 << 4) + (UDPFlood_en4 << 3) + (LANDATTACK_en4 << 2) + (SYNFlood_en4 << 1) + DefenderPort_en_4;
    break;
  case 5:
    EnableDefender5 = (HTTPSGETFlood_en5 << 10) + (HTTPGETFlood_en5 << 9) + (UDPFragFlood_en5 << 8) + (TCPFragFlood_en5 << 7) + (IPSecFlood_en5 << 6) + (ICMPFlood_en5 << 5) + (DNSFlood_en5 << 4) + (UDPFlood_en5 << 3) + (LANDATTACK_en5 << 2) + (SYNFlood_en5 << 1) + DefenderPort_en_5;
    break;
  case 6:
    EnableDefender6 = (HTTPSGETFlood_en6 << 10) + (HTTPGETFlood_en6 << 9) + (UDPFragFlood_en6 << 8) + (TCPFragFlood_en6 << 7) + (IPSecFlood_en6 << 6) + (ICMPFlood_en6 << 5) + (DNSFlood_en6 << 4) + (UDPFlood_en6 << 3) + (LANDATTACK_en6 << 2) + (SYNFlood_en6 << 1) + DefenderPort_en_6;
    break;
  case 7:
    EnableDefender7 = (HTTPSGETFlood_en7 << 10) + (HTTPGETFlood_en7 << 9) + (UDPFragFlood_en7 << 8) + (TCPFragFlood_en7 << 7) + (IPSecFlood_en7 << 6) + (ICMPFlood_en7 << 5) + (DNSFlood_en7 << 4) + (UDPFlood_en7 << 3) + (LANDATTACK_en7 << 2) + (SYNFlood_en7 << 1) + DefenderPort_en_7;
    break;
  case 8:
    EnableDefender8 = (HTTPSGETFlood_en8 << 10) + (HTTPGETFlood_en8 << 9) + (UDPFragFlood_en8 << 8) + (TCPFragFlood_en8 << 7) + (IPSecFlood_en8 << 6) + (ICMPFlood_en8 << 5) + (DNSFlood_en8 << 4) + (UDPFlood_en8 << 3) + (LANDATTACK_en8 << 2) + (SYNFlood_en8 << 1) + DefenderPort_en_8;
    break;
  default:
    break;
  }
  ReturnMode2(key1 - '0');
}

void SetHTTPGETDefender()
{
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Do you want to enable HTTP Flood flood protect now (Y/N)?: ");
  u8 key = 0;
  u8 done = 0;
  u8 key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 'y' || key == 'Y')
    {
      // XUartLite_SendByte(0x40600000, 'Y');
      done = 1;
      switch (key1 - '0')
      {
      case 1:
        HTTPGETFlood_en1 = 1;
        break;
      case 2:
        HTTPGETFlood_en2 = 1;
        break;
      case 3:
        HTTPGETFlood_en3 = 1;
        break;
      case 4:
        HTTPGETFlood_en4 = 1;
        break;
      case 5:
        HTTPGETFlood_en5 = 1;
        break;
      case 6:
        HTTPGETFlood_en6 = 1;
        break;
      case 7:
        HTTPGETFlood_en7 = 1;
        break;
      case 8:
        HTTPGETFlood_en8 = 1;
        break;
      default:
        break;
      }
    }
    else if (key == 'n' || key == 'N')
    {
      // XUartLite_SendByte(0x40600000, 'N');
      done = 1;
      switch (key1 - '0')
      {
      case 1:
        HTTPGETFlood_en1 = 0;
        break;
      case 2:
        HTTPGETFlood_en2 = 0;
        break;
      case 3:
        HTTPGETFlood_en3 = 0;
        break;
      case 4:
        HTTPGETFlood_en4 = 0;
        break;
      case 5:
        HTTPGETFlood_en5 = 0;
        break;
      case 6:
        HTTPGETFlood_en6 = 0;
        break;
      case 7:
        HTTPGETFlood_en7 = 0;
        break;
      case 8:
        HTTPGETFlood_en8 = 0;
        break;
      default:
        break;
      }
    }
    else if (key == 03)
    {
      // xil_printf("Loading Bitstream.........");
      Reset_System();
    }
    else if (key == 13 && done == 1)
    {
      // XUartLite_SendByte(0x40600000, 'E');
      break;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  switch (key1 - '0')
  {
  case 1:
    EnableDefender1 = (HTTPSGETFlood_en1 << 10) + (HTTPGETFlood_en1 << 9) + (UDPFragFlood_en1 << 8) + (TCPFragFlood_en1 << 7) + (IPSecFlood_en1 << 6) + (ICMPFlood_en1 << 5) + (DNSFlood_en1 << 4) + (UDPFlood_en1 << 3) + (LANDATTACK_en1 << 2) + (SYNFlood_en1 << 1) + DefenderPort_en_1;
    break;
  case 2:
    EnableDefender2 = (HTTPSGETFlood_en2 << 10) + (HTTPGETFlood_en2 << 9) + (UDPFragFlood_en2 << 8) + (TCPFragFlood_en2 << 7) + (IPSecFlood_en2 << 6) + (ICMPFlood_en2 << 5) + (DNSFlood_en2 << 4) + (UDPFlood_en2 << 3) + (LANDATTACK_en2 << 2) + (SYNFlood_en2 << 1) + DefenderPort_en_2;
    break;
  case 3:
    EnableDefender3 = (HTTPSGETFlood_en3 << 10) + (HTTPGETFlood_en3 << 9) + (UDPFragFlood_en3 << 8) + (TCPFragFlood_en3 << 7) + (IPSecFlood_en3 << 6) + (ICMPFlood_en3 << 5) + (DNSFlood_en3 << 4) + (UDPFlood_en3 << 3) + (LANDATTACK_en3 << 2) + (SYNFlood_en3 << 1) + DefenderPort_en_3;
    break;
  case 4:
    EnableDefender4 = (HTTPSGETFlood_en4 << 10) + (HTTPGETFlood_en4 << 9) + (UDPFragFlood_en4 << 8) + (TCPFragFlood_en4 << 7) + (IPSecFlood_en4 << 6) + (ICMPFlood_en4 << 5) + (DNSFlood_en4 << 4) + (UDPFlood_en4 << 3) + (LANDATTACK_en4 << 2) + (SYNFlood_en4 << 1) + DefenderPort_en_4;
    break;
  case 5:
    EnableDefender5 = (HTTPSGETFlood_en5 << 10) + (HTTPGETFlood_en5 << 9) + (UDPFragFlood_en5 << 8) + (TCPFragFlood_en5 << 7) + (IPSecFlood_en5 << 6) + (ICMPFlood_en5 << 5) + (DNSFlood_en5 << 4) + (UDPFlood_en5 << 3) + (LANDATTACK_en5 << 2) + (SYNFlood_en5 << 1) + DefenderPort_en_5;
    break;
  case 6:
    EnableDefender6 = (HTTPSGETFlood_en6 << 10) + (HTTPGETFlood_en6 << 9) + (UDPFragFlood_en6 << 8) + (TCPFragFlood_en6 << 7) + (IPSecFlood_en6 << 6) + (ICMPFlood_en6 << 5) + (DNSFlood_en6 << 4) + (UDPFlood_en6 << 3) + (LANDATTACK_en6 << 2) + (SYNFlood_en6 << 1) + DefenderPort_en_6;
    break;
  case 7:
    EnableDefender7 = (HTTPSGETFlood_en7 << 10) + (HTTPGETFlood_en7 << 9) + (UDPFragFlood_en7 << 8) + (TCPFragFlood_en7 << 7) + (IPSecFlood_en7 << 6) + (ICMPFlood_en7 << 5) + (DNSFlood_en7 << 4) + (UDPFlood_en7 << 3) + (LANDATTACK_en7 << 2) + (SYNFlood_en7 << 1) + DefenderPort_en_7;
    break;
  case 8:
    EnableDefender8 = (HTTPSGETFlood_en8 << 10) + (HTTPGETFlood_en8 << 9) + (UDPFragFlood_en8 << 8) + (TCPFragFlood_en8 << 7) + (IPSecFlood_en8 << 6) + (ICMPFlood_en8 << 5) + (DNSFlood_en8 << 4) + (UDPFlood_en8 << 3) + (LANDATTACK_en8 << 2) + (SYNFlood_en8 << 1) + DefenderPort_en_8;
    break;
  default:
    break;
  }
  ReturnMode2(key1 - '0');
}

void SetHTTPSGETDefender()
{
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Do you want to enable HTTP Flood flood protect now (Y/N)?: ");
  u8 key = 0;
  u8 done = 0;
  u8 key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 'y' || key == 'Y')
    {
      // XUartLite_SendByte(0x40600000, 'Y');
      done = 1;
      switch (key1 - '0')
      {
      case 1:
        HTTPSGETFlood_en1 = 1;
        break;
      case 2:
        HTTPSGETFlood_en2 = 1;
        break;
      case 3:
        HTTPSGETFlood_en3 = 1;
        break;
      case 4:
        HTTPSGETFlood_en4 = 1;
        break;
      case 5:
        HTTPSGETFlood_en5 = 1;
        break;
      case 6:
        HTTPSGETFlood_en6 = 1;
        break;
      case 7:
        HTTPSGETFlood_en7 = 1;
        break;
      case 8:
        HTTPSGETFlood_en8 = 1;
        break;
      default:
        break;
      }
    }
    else if (key == 'n' || key == 'N')
    {
      // XUartLite_SendByte(0x40600000, 'N');
      done = 1;
      switch (key1 - '0')
      {
      case 1:
        HTTPSGETFlood_en1 = 0;
        break;
      case 2:
        HTTPSGETFlood_en2 = 0;
        break;
      case 3:
        HTTPSGETFlood_en3 = 0;
        break;
      case 4:
        HTTPSGETFlood_en4 = 0;
        break;
      case 5:
        HTTPSGETFlood_en5 = 0;
        break;
      case 6:
        HTTPSGETFlood_en6 = 0;
        break;
      case 7:
        HTTPSGETFlood_en7 = 0;
        break;
      case 8:
        HTTPSGETFlood_en8 = 0;
        break;
      default:
        break;
      }
    }
    else if (key == 03)
    {
      // xil_printf("Loading Bitstream.........");
      Reset_System();
    }
    else if (key == 13 && done == 1)
    {
      // XUartLite_SendByte(0x40600000, 'E');
      break;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
  // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
  switch (key1 - '0')
  {
  case 1:
    EnableDefender1 = (HTTPSGETFlood_en1 << 10) + (HTTPGETFlood_en1 << 9) + (UDPFragFlood_en1 << 8) + (TCPFragFlood_en1 << 7) + (IPSecFlood_en1 << 6) + (ICMPFlood_en1 << 5) + (DNSFlood_en1 << 4) + (UDPFlood_en1 << 3) + (LANDATTACK_en1 << 2) + (SYNFlood_en1 << 1) + DefenderPort_en_1;
    break;
  case 2:
    EnableDefender2 = (HTTPSGETFlood_en2 << 10) + (HTTPGETFlood_en2 << 9) + (UDPFragFlood_en2 << 8) + (TCPFragFlood_en2 << 7) + (IPSecFlood_en2 << 6) + (ICMPFlood_en2 << 5) + (DNSFlood_en2 << 4) + (UDPFlood_en2 << 3) + (LANDATTACK_en2 << 2) + (SYNFlood_en2 << 1) + DefenderPort_en_2;
    break;
  case 3:
    EnableDefender3 = (HTTPSGETFlood_en3 << 10) + (HTTPGETFlood_en3 << 9) + (UDPFragFlood_en3 << 8) + (TCPFragFlood_en3 << 7) + (IPSecFlood_en3 << 6) + (ICMPFlood_en3 << 5) + (DNSFlood_en3 << 4) + (UDPFlood_en3 << 3) + (LANDATTACK_en3 << 2) + (SYNFlood_en3 << 1) + DefenderPort_en_3;
    break;
  case 4:
    EnableDefender4 = (HTTPSGETFlood_en4 << 10) + (HTTPGETFlood_en4 << 9) + (UDPFragFlood_en4 << 8) + (TCPFragFlood_en4 << 7) + (IPSecFlood_en4 << 6) + (ICMPFlood_en4 << 5) + (DNSFlood_en4 << 4) + (UDPFlood_en4 << 3) + (LANDATTACK_en4 << 2) + (SYNFlood_en4 << 1) + DefenderPort_en_4;
    break;
  case 5:
    EnableDefender5 = (HTTPSGETFlood_en5 << 10) + (HTTPGETFlood_en5 << 9) + (UDPFragFlood_en5 << 8) + (TCPFragFlood_en5 << 7) + (IPSecFlood_en5 << 6) + (ICMPFlood_en5 << 5) + (DNSFlood_en5 << 4) + (UDPFlood_en5 << 3) + (LANDATTACK_en5 << 2) + (SYNFlood_en5 << 1) + DefenderPort_en_5;
    break;
  case 6:
    EnableDefender6 = (HTTPSGETFlood_en6 << 10) + (HTTPGETFlood_en6 << 9) + (UDPFragFlood_en6 << 8) + (TCPFragFlood_en6 << 7) + (IPSecFlood_en6 << 6) + (ICMPFlood_en6 << 5) + (DNSFlood_en6 << 4) + (UDPFlood_en6 << 3) + (LANDATTACK_en6 << 2) + (SYNFlood_en6 << 1) + DefenderPort_en_6;
    break;
  case 7:
    EnableDefender7 = (HTTPSGETFlood_en7 << 10) + (HTTPGETFlood_en7 << 9) + (UDPFragFlood_en7 << 8) + (TCPFragFlood_en7 << 7) + (IPSecFlood_en7 << 6) + (ICMPFlood_en7 << 5) + (DNSFlood_en7 << 4) + (UDPFlood_en7 << 3) + (LANDATTACK_en7 << 2) + (SYNFlood_en7 << 1) + DefenderPort_en_7;
    break;
  case 8:
    EnableDefender8 = (HTTPSGETFlood_en8 << 10) + (HTTPGETFlood_en8 << 9) + (UDPFragFlood_en8 << 8) + (TCPFragFlood_en8 << 7) + (IPSecFlood_en8 << 6) + (ICMPFlood_en8 << 5) + (DNSFlood_en8 << 4) + (UDPFlood_en8 << 3) + (LANDATTACK_en8 << 2) + (SYNFlood_en8 << 1) + DefenderPort_en_8;
    break;
  default:
    break;
  }
  ReturnMode2(key1 - '0');
}

void SetTimeflood()
{
  u8 count;
Return:
  key = 0;
  count = 0;
  u32 timeflood_valid = 0;
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Enter attack detection time (s): ");
  u8 key1 = 0;
  while (1)
  {
    key1 = XUartLite_RecvByte(0x40600000);
    if (key1 == '1' || key1 == '2' || key1 == '3' || key1 == '4' || key1 == '5' || key1 == '6' || key1 == '7' || key1 == '8')
      break;
  }
  while (key != 13 && count < 16)
  {
    key = XUartLite_RecvByte(0x40600000);
    // if ((key <48 || key >57) && key!=13){
    // }
    // else if ( (count == 0) && (key == 48) ){
    // }
    if (key > 47 && key < 58)
    {
      // xil_printf("%c",key);
      key = key - 48;
      timeflood_valid = timeflood_valid * 10 + key;
      count++;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  if (timeflood_valid > 65535)
  {
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+========================================================================================================================================================================+");
    // xil_printf("\r\n\t\t| The value must be less than 65536.                                                                                                                                     |");
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
    // goto Return;
  }
  switch (key1 - '0')
  {
  case 1:
    TimeFlood1 = timeflood_valid;
    break;
  case 2:
    TimeFlood2 = timeflood_valid;
    break;
  case 3:
    TimeFlood3 = timeflood_valid;
    break;
  case 4:
    TimeFlood4 = timeflood_valid;
    break;
  case 5:
    TimeFlood5 = timeflood_valid;
    break;
  case 6:
    TimeFlood6 = timeflood_valid;
    break;
  case 7:
    TimeFlood7 = timeflood_valid;
    break;
  case 8:
    TimeFlood8 = timeflood_valid;
    break;
  default:
    break;
  }
  ReturnMode2(key1 - '0');
}

void SetTimeDelete()
{
  u8 count;
Return:
  key = 0;
  count = 0;
  u32 timedelete_valid = 0;
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Enter the time to delete the whitelist's information (s): ");
  while (key != 13 && count < 16)
  {
    key = XUartLite_RecvByte(0x40600000);
    // if ((key <48 || key >57) && key!=13){
    // }
    // else if ( (count == 0) && (key == 48) ){
    // }
    if (key > 47 && key < 58)
    {
      // xil_printf("%c",key);
      key = key - 48;
      timedelete_valid = timedelete_valid * 10 + key;
      count++;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  // xil_printf("\r\n\t\t|                                                                 |");
  if (timedelete_valid > 65535)
  {
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+========================================================================================================================================================================+");
    // xil_printf("\r\n\t\t| The value must be less than 65536.                                                                                                                                     |");
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
    // goto Return;
  }
  else
  {
    TimeDelete = timedelete_valid;
  }
  ReturnMode2(1);
}

void ReturnMode2(int port)
{
  // u8 key1, count = 0;
  // u8 key2 = 0;
  // HTTPGETFlood_en1 = (EnableDefender1 >> 10) % 2;
  // DefenderPort = (EnableDefender1 >> 9) % 2;
  // DefenderPort_en1 = (EnableDefender1 >> 8) % 2;
  // UDPFragFlood_en1 = (EnableDefender1 >> 7) % 2;
  // TCPFragFlood_en1 = (EnableDefender1 >> 6) % 2;
  // IPSecFlood_en1 = (EnableDefender1 >> 5) % 2;
  // ICMPFlood_en1 = (EnableDefender1 >> 4) % 2;
  // DNSFlood_en1 = (EnableDefender1 >> 3) % 2;
  // UDPFlood_en1 = (EnableDefender1 >> 2) % 2;
  // LANDATTACK_en1 = (EnableDefender1 >> 1) % 2;
  // SYNFlood_en1 = EnableDefender1 % 2;

  AssignData();
  SaveEEPROM();
  ConfigurationIPcore(port);
  // delay_0_5s();
  if ((isCompleted_AssignData == 1) && (isCompleted_ConfigurationIPcore == 1) && (isCompleted_SaveEEPROM == 1))
  {
    XUartLite_SendByte(0x40600000, 'Y');
  }
  else
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  isCompleted_AssignData = 0;
  isCompleted_ConfigurationIPcore = 0;
  isCompleted_SaveEEPROM = 0;
  // EnableDefender1 = (HTTPGETFlood_en1 << 10) + (DefenderPort << 9) + (DefenderPort_en1 << 8) + (UDPFragFlood_en1 << 7) + (TCPFragFlood_en1 << 6) + (IPSecFlood_en1 << 5) + (ICMPFlood_en1 << 4) + (DNSFlood_en1 << 3) + (UDPFlood_en1 << 2) + (LANDATTACK_en1 << 1) + SYNFlood_en1;
}

void print_process_Info()
{
  u32 SynPPS, SynBPS, Conv_SynPPS, Conv_SynBPS, Conp_SynPPS;
  u32 cnt_1s;
  u32 HttpPPS, HttpBPS;
  u32 cnt_pkt_hp, cnt_pkt_exr;
  u32 tx_pack_0, tx_pack_1, tx_byte_0, tx_byte_1, rx_pack_0, rx_pack_1, rx_byte_0, rx_byte_1;

  u32 UdpPPS, UdpBPS, Conv_UdpPPS, Conv_UdpBPS, Conp_UdpPPS;

  u32 IcmpPPS, IcmpBPS, Conv_IcmpPPS, Conv_IcmpBPS, Conp_IcmpPPS;

  u32 DnsPPS, DnsBPS, Conv_DnsPPS, Conv_DnsBPS, Conp_DnsPPS;

  u32 SynonymousPPS, SynonymousBPS, Conv_SynonymousPPS, Conv_SynonymousBPS, Conp_SynonymousPPS;

  u32 IPSecPPS, IPSecBPS, Conv_IPSecPPS, Conv_IPSecBPS, Conp_IPSecPPS;

  u32 TCPFragPPS, TCPFragBPS, Conv_TCPFragPPS, Conv_TCPFragBPS, Conp_TCPFragPPS;

  u32 UDPFragPPS, UDPFragBPS, Conv_UDPFragPPS, Conv_UDPFragBPS, Conp_UDPFragPPS;

  u8 StatusAttack;
  u8 Attacked;
  u8 StarusReg;
  u32 SynPPSThreshold;
  u32 UdpPPSThreshold;
  u32 DnsPPSThreshold;
  u32 IcmpPPSThreshold;
  u32 IPSecPPSThreshold;

  u32 HP_debug;
  u32 DT_debug;
  u32 DE_debug;
  u32 PG_debug;
  u32 ddos_wr;
  u32 ddos_rd;
  u32 ddos_fifo_empty;
  u32 fb_pg_tready;

  u32 fabric_client;
  u32 fabric_server;
  u32 client_arbiter;
  u32 server_arbiter;
  u32 fabric_client_valid;
  u32 fabric_client_rdy;

  u32 pcon_rx_client_full;
  u32 pcon_rx_server_full;
  u32 pcon_tx_client_full;
  u32 pcon_tx_server_full;

  u32 pcom_master_0_rdy;
  u32 pcom_master_1_rdy;
  u32 hp_tvalid;
  u32 ddos_fifo_nearly_full;
  u32 hp_tlast;

  u32 enable_defender;

  SynPPSThreshold = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 20);
  UdpPPSThreshold = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 28);
  DnsPPSThreshold = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 36);
  IcmpPPSThreshold = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 32);
  IPSecPPSThreshold = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 37);

  cnt_pkt_exr = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 308);
  cnt_pkt_hp = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 304);
  cnt_1s = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 316);

  SynPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 68);
  // Convert SYN PPS

  SynBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 72);
  // Convert SYN BPS

  UdpPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 76);
  // Convert UDP PPS

  UdpBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 80);
  // Convert UDP BPS

  IcmpPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 84);
  // Convert ICMP PPS

  IcmpBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 88);
  // Convert ICMP BPS

  DnsPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 92);
  // Convert DNS PPS

  DnsBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 96);
  // Convert DNS BPS

  SynonymousPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 100);
  // Convert LAND ATTACK PPS

  SynonymousBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 104);
  // Convert LAND ATTACK BPS

  IPSecPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 93);
  // Convert IPSec PPS

  IPSecBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 97);
  // Convert IPSec BPS

  TCPFragPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 105);
  // Convert TCP Fragment PPS

  TCPFragBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 106);
  // Convert TCPFragment BPS

  UDPFragPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 109);
  // Convert UDP Fragment PPS

  UDPFragBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 110);
  // Convert TCPFragment BPS

  HttpPPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 292);
  HttpBPS = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 296);

  Attacked = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 44);
  StarusReg = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 108);

  HP_debug = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 144);
  DT_debug = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 148);
  DE_debug = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 152);
  PG_debug = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 156);
  ddos_wr = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 220);
  ddos_rd = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 224);

  ddos_fifo_empty = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 160);
  fb_pg_tready = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 164);

  enable_defender = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 40);

  fabric_client = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 228);
  fabric_server = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 232);
  client_arbiter = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 236);
  server_arbiter = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 240);
  fabric_client_valid = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 244);
  fabric_client_rdy = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 248);

  pcon_rx_client_full = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 252);
  pcon_rx_server_full = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 256);
  pcon_tx_client_full = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 260);
  pcon_tx_server_full = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 264);

  pcom_master_0_rdy = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 268);
  pcom_master_1_rdy = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 272);
  hp_tvalid = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 276);
  ddos_fifo_nearly_full = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 280);
  hp_tlast = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 284);

  tx_pack_0 = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 320);
  tx_pack_1 = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 324);
  tx_byte_0 = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 328);
  tx_byte_1 = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 332);
  rx_pack_0 = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 336);
  rx_pack_1 = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 340);
  rx_byte_0 = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 344);
  rx_byte_1 = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 348);

  // xil_printf("|syn: %8d ", SynPPS);
  // xil_printf("|syn: %8d ", SynBPS);

  // xil_printf("|land: %8d ", SynonymousPPS);
  // xil_printf("|land: %8d ", SynonymousBPS);

  // xil_printf("|udp: %8d ", UdpPPS);
  // xil_printf("|udp: %8d ", UdpBPS);

  // //xil_printf("|icmp: %8d ", IcmpPPS);
  // //xil_printf("|icmp: %8d ", IcmpBPS);

  // //xil_printf("|dns: %8d ", DnsPPS);
  // //xil_printf("|dns: %8d ", DnsBPS);

  // //xil_printf("|ipsec: %8d ", IPSecPPS);
  // //xil_printf("|ipsec: %8d ", IPSecBPS);

  // //xil_printf("|tcpfraf: %8d ", TCPFragPPS);
  // //xil_printf("|tcpfrag: %8d ", TCPFragBPS);

  // //xil_printf("|udpfrag: %8d ", UDPFragPPS);
  // //xil_printf("|udpfrag: %8d ", UDPFragBPS);

  // //xil_printf("|http: %8d ", HttpPPS);
  // //xil_printf("|http: %8d ", HttpBPS);

  // xil_printf("cnt_hp: %8d ", cnt_pkt_hp);
  // xil_printf("cnt_exr: %8d ", cnt_pkt_exr);

  // xil_printf("time 1s: %8d ", cnt_1s);

  // xil_printf("tx_pack_0: %8d ",tx_pack_0 );
  // xil_printf("tx_pack_1: %8d ",tx_pack_1 );
  // xil_printf("tx_byte_0: %8d ",tx_byte_0 * 64 );
  // xil_printf("tx_byte_1: %8d ",tx_byte_1 * 64);
  // xil_printf("rx_pack_0: %8d ",rx_pack_0 );
  // xil_printf("rx_pack_1: %8d ",rx_pack_1 );
  // xil_printf("rx_byte_0: %8d ",rx_byte_0 * 64);
  // xil_printf("rx_byte_1: %8d ",rx_byte_1 * 64);

  // //xil_printf("| %8lu ", ddos_wr);
  // //xil_printf("| %8lu ", ddos_rd);
  ////xil_printf("| %8d ", ddos_fifo_empty);
  // //xil_printf("| %8d ", fb_pg_tready);

  // //xil_printf("| %8d %8d %8d ", SynPPSThreshold, UdpPPSThreshold, IcmpPPSThreshold);

  // xil_printf("| %04x ", enable_defender);

  // //xil_printf("| %08x %08x %08x %08x %lu %lu ", client_arbiter, server_arbiter, fabric_server, fabric_client, fabric_client_valid, fabric_client_rdy);

  ////xil_printf("| %lu %lu %lu %lu ", pcon_rx_client_full, pcon_rx_server_full, pcon_tx_server_full, pcon_tx_client_full);

  // //xil_printf("| %lu %lu ", pcom_master_0_rdy, pcom_master_1_rdy);
  ////xil_printf("| %lu %lu %lu ", hp_tvalid, hp_tlast, ddos_fifo_nearly_full);

  ////xil_printf("| %2lu %2lu %2lu %2lu ", HP_debug, DT_debug, DE_debug, PG_debug);
}

void set_HTTP_IP_Table()
{
  u8 key, key1;
  u8 count;
start:
  key = 0, key1 = 0;
  count = 0;
  // xil_printf("\r\n");
  // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
  // xil_printf("\t\t| Key Enter | Please choose 1 option below:                                                                              |\r\n");
  // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
  // xil_printf("\t\t|     1.    | Show Attacker's IP Table                                                                               	 |\r\n");
  // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
  // xil_printf("\t\t|     2.    | Add IP in Attacker Table                                                                              	 |\r\n");
  // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
  // xil_printf("\t\t|     3.    | Clear IP in Attacker Table                                                                                                         |\r\n");
  // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
  // xil_printf("\t\t|     4.    | Exit                                                                                                       |\r\n");
  // xil_printf("\t\t+-----------+------------------------------------------------------------------------------------------------------------+\r\n");
  // xil_printf("\t\tPlease choose mode:  ");
  while (key != 13 || count < 1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key != 13 && count == 0)
    {
      // xil_printf("%c",key);
      key1 = key;
      count = 1;
    }
  }

  if (key1 == '1')
  {
    // XUartLite_SendByte(0x40600000, '1');
    // delay_1s();
    show_Atk_IP_Table();
    goto start;
  }
  else if (key1 == '2')
  {
    // XUartLite_SendByte(0x40600000, '2');
    //  delay_1s();
    add_Atk_IP_Table();
    goto start;
  }
  else if (key1 == '3')
  {
    // XUartLite_SendByte(0x40600000, '3');
    delay_1s();
    clear_Atk_IP_Table();
    goto start;
  }
  else if (key1 == '4')
  {
  }
  else if (key1 == 03)
  {
    Reset_System();
  }
  ReturnMode2(1);
}

void show_Atk_IP_Table()
{
  u8 key1;
  u8 ip_1[4];
  //	u8 ip_2[4];
  //	u8 ip_3[4];
  //	u8 ip_4[4];

  // clear_screen();
  // xil_printf("\n");
  // xil_printf("\t\t+=====================================+\n");
  for (int i = 0; i < 50; i++)
  {
    if (*(Attacker_List + i) != 0)
    {
      // xil_printf("\t\t+\tAtk_IP [%d]: %08x \n", i+1, *(Attacker_List+i));
    }
  }

  // xil_printf("\t\t+=====================================+\n");
  // xil_printf("\t\tPress Enter to continue...\n");

  // for (int i=0; i<4; i++){
  // 	EEPROM_ReadIIC(&myDevice_EEPROM, user1_addr+i, ip_1+i, 1);
  // 	delay_1ms();
  // 	if(user1[i] == 0)
  // 		break;
  // 	i++;
  // }

  // load_HTTP_IP_EEPROM(ip_1);
  while (key1 != 13)
  {
    key1 = XUartLite_RecvByte(0x40600000);
  }
}

void load_HTTP_IP_EEPROM(u8 *ip)
{
  //  for (int i = 0; i < 4; i++)
  //  {
  //    EEPROM_ReadIIC(&myDevice_EEPROM, http_ip_1 + i, ip + i, 1);
  //    delay_1ms();
  //    // if(user1[i] == 0)
  //    // 	break;
  //    // i++;
  //  }
}

void clear_Atk_IP_Table()
{
  u8 key;
  u8 count_point = 0;
  u8 count = 0;
  u8 colect_ip[2];
  u16 IP_atk_layerA;
  u16 IP_atk_layerB;
  u16 IP_atk_layerC;
  u16 IP_atk_layerD;
  u8 next;
  u8 flag_error;
Return:
  key = 0;
  count_point = 0;
  count = 0;
  colect_ip[2] = 0;
  IP_atk_layerA = 0;
  IP_atk_layerB = 0;
  IP_atk_layerC = 0;
  IP_atk_layerD = 0;
  next = 0;
  flag_error = 0;
  // xil_printf("\r\n");
  // xil_printf("\r\n================+========================================================================================================================================================================+");
  // xil_printf("\r\n    Setting     | Enter Server IP address want to delete from table : ");
  while (count_point < 5 && flag_error != 1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if ((key < '0' || key > '9') && key != '.' && key != 13)
    {
      // No dothing;
    }
    else if ((key >= '0' && key <= '9') || key == '.' || key == 13)
    {

      if (key == '.' && count_point < 4)
      {
        count_point++;
      }
      if (key >= '0' && key <= '9')
      {
        key = key - 48;
        count = count + 1;
        colect_ip[count - 1] = key;
      }
      if (count_point == 1 && next == 0)
      {
        if (count == 1)
        {
          IP_atk_layerA = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IP_atk_layerA = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IP_atk_layerA = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count = 0;
        next = 1;
      }
      if (count_point == 2 && next == 1)
      {
        if (count == 1)
        {
          IP_atk_layerB = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IP_atk_layerB = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IP_atk_layerB = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count = 0;
        next = 2;
      }
      if (count_point == 3 && next == 2)
      {
        if (count == 1)
        {
          IP_atk_layerC = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IP_atk_layerC = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IP_atk_layerC = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count_point = 4;
        count = 0;
        next = 3;
      }
      if (key == 13 && count_point == 4)
      {
        if (count == 1)
        {
          IP_atk_layerD = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IP_atk_layerD = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IP_atk_layerD = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count = 0;
        count_point = 5;
      }
      if (IP_atk_layerA > 255 || IP_atk_layerA < 0 || IP_atk_layerB > 255 || IP_atk_layerB < 0 || IP_atk_layerC > 255 || IP_atk_layerC < 0 || IP_atk_layerD > 255 || IP_atk_layerD < 0)
      {
        flag_error = 1;
      }
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+========================================================================================================================================================================+");
    // xil_printf("\r\n\t\t| Warning: IP Server %d.%d.%d.%d invalid!", IP_atk_layerA, IP_atk_layerB, IP_atk_layerC, IP_atk_layerD);
    // xil_printf("\r\n\t\t|                                                                                                                                                                        |");
    // xil_printf("\r\n\t\t+------------------------------------------------------------------------------------------------------------------------------------------------------------------------+");
    // break;
  }
  else
  {
    IPAttacker_rm = IP_atk_layerA * 16777216 + IP_atk_layerB * 65536 + IP_atk_layerC * 256 + IP_atk_layerD;
    delay_1s();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 172, IPAttacker_rm);
    delay_1ms();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000030);
    delay_1ms();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000000);

    delay_1ms();
    XUartLite_SendByte(0x40600000, 'Y');
    // delay_0_5s();
    //  XUartLite_SendByte(0x40600000, 'I');
  }
}
void Clear_IPv4_HTTP_Table_From_File(const char *ip_str)
{
  u16 IP_atk_layerA = 0;
  u16 IP_atk_layerB = 0;
  u16 IP_atk_layerC = 0;
  u16 IP_atk_layerD = 0;
  u8 flag_error = 0;

  if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IP_atk_layerA, &IP_atk_layerB, &IP_atk_layerC, &IP_atk_layerD) != 4)
  {
    flag_error = 1;
  }

  if (flag_error == 1)
  {
    // XUartLite_SendByte(0x40600000, 'N');
    return;
  }

  IPAttacker_rm = IP_atk_layerA * 16777216 + IP_atk_layerB * 65536 + IP_atk_layerC * 256 + IP_atk_layerD;
  delay_1s();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 172, IPAttacker_rm);
  delay_1ms();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000030);
  delay_1ms();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000000);

  delay_1ms();
  // XUartLite_SendByte(0x40600000, 'Y');
}
void remove_ipv4_table_http_blacklist(const char *ip_str)
{
  u16 IP_atk_layerA = 0;
  u16 IP_atk_layerB = 0;
  u16 IP_atk_layerC = 0;
  u16 IP_atk_layerD = 0;
  u8 flag_error = 0;

  if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IP_atk_layerA, &IP_atk_layerB, &IP_atk_layerC, &IP_atk_layerD) != 4)
  {
    flag_error = 1;
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
    return;
  }

  IPAttacker_rm = IP_atk_layerA * 16777216 + IP_atk_layerB * 65536 + IP_atk_layerC * 256 + IP_atk_layerD;
  delay_1s();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 172, IPAttacker_rm);
  delay_1ms();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000030);
  delay_1ms();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000000);

  delay_1ms();
  XUartLite_SendByte(0x40600000, 'Y');
}

void remove_ipv4_table_http_whitelist(const char *ip_str)
{
  u16 IP_atk_layerA = 0;
  u16 IP_atk_layerB = 0;
  u16 IP_atk_layerC = 0;
  u16 IP_atk_layerD = 0;
  u8 flag_error = 0;

  if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IP_atk_layerA, &IP_atk_layerB, &IP_atk_layerC, &IP_atk_layerD) != 4)
  {
    flag_error = 1;
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
    return;
  }

  IPAttacker_rm = IP_atk_layerA * 16777216 + IP_atk_layerB * 65536 + IP_atk_layerC * 256 + IP_atk_layerD;
  delay_1s();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 172, IPAttacker_rm);
  delay_1ms();
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000030);
  // delay_1ms();
  // Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000000);

  delay_1ms();
  XUartLite_SendByte(0x40600000, 'Y');
}

void REMOVE_IPv4_HTTP_BLACK_LIST()
{
  char buffer_IPV4[BUFFER_SIZE_IPV4];
  ReceiveIPv4String(buffer_IPV4, BUFFER_SIZE_IPV4);
  remove_ipv4_table_http_blacklist(buffer_IPV4);
}

void REMOVE_IPv4_HTTP_WHITE_LIST()
{
  char buffer_IPV4[BUFFER_SIZE_IPV4];
  ReceiveIPv4String(buffer_IPV4, BUFFER_SIZE_IPV4);
  remove_ipv4_table_http_whitelist(buffer_IPV4);
}

void add_Atk_IP_Table()
{
  u8 key;
  u8 count_point = 0;
  u8 count = 0;
  u8 colect_ip[2];
  u16 IP_atk_layerA;
  u16 IP_atk_layerB;
  u16 IP_atk_layerC;
  u16 IP_atk_layerD;
  u8 next;
  u8 flag_error;
Return:
  key = 0;
  count_point = 0;
  count = 0;
  colect_ip[2] = 0;
  IP_atk_layerA = 0;
  IP_atk_layerB = 0;
  IP_atk_layerC = 0;
  IP_atk_layerD = 0;
  next = 0;
  flag_error = 0;
  while (count_point < 5 && flag_error != 1)
  {
    // XUartLite_SendByte(0x40600000, 'H');
    key = XUartLite_RecvByte(0x40600000);
    if (key == 03)
    {
      Reset_System();
    }
    if ((key >= '0' && key <= '9') || key == '.' || key == 13)
    {
      // xil_printf("%c", key);
      if (key == '.' && count_point < 4)
      {
        count_point++;
      }
      if (key >= '0' && key <= '9')
      {
        key = key - 48;
        count = count + 1;
        colect_ip[count - 1] = key;
      }

      if (count_point == 1 && next == 0)
      {
        if (count == 1)
        {
          IP_atk_layerA = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IP_atk_layerA = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IP_atk_layerA = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count = 0;
        next = 1;
      }
      if (count_point == 2 && next == 1)
      {
        if (count == 1)
        {
          IP_atk_layerB = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IP_atk_layerB = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IP_atk_layerB = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count = 0;
        next = 2;
      }
      if (count_point == 3 && next == 2)
      {
        if (count == 1)
        {
          IP_atk_layerC = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IP_atk_layerC = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IP_atk_layerC = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count_point = 4;
        count = 0;
        next = 3;
      }
      if (key == 13 && count_point == 4)
      {
        if (count == 1)
        {
          IP_atk_layerD = colect_ip[count - 1];
        }
        else if (count == 2)
        {
          IP_atk_layerD = colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        else if (count == 3)
        {
          IP_atk_layerD = colect_ip[count - 3] * 100 + colect_ip[count - 2] * 10 + colect_ip[count - 1];
        }
        count = 0;
        count_point = 5;
      }
      if (IP_atk_layerA > 255 || IP_atk_layerA < 0 || IP_atk_layerB > 255 || IP_atk_layerB < 0 || IP_atk_layerC > 255 || IP_atk_layerC < 0 || IP_atk_layerD > 255 || IP_atk_layerD < 0)
      {
        flag_error = 1;
      }
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  else
  {

    IPAttacker = IP_atk_layerA * 16777216 + IP_atk_layerB * 65536 + IP_atk_layerC * 256 + IP_atk_layerD;
    delay_1s();
    // delay_1s();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 172, IPAttacker);
    delay_1ms();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000010);
    if (Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 204))
    {
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000000);
      // XUartLite_SendByte(0x40600000, 'C');
    }
    delay_1ms();
    XUartLite_SendByte(0x40600000, 'Y');
    // delay_1s();
  }
}
void add_ipv4_table_http_blacklist(const char *ip_str)
{
  u16 IP_atk_layerA = 0;
  u16 IP_atk_layerB = 0;
  u16 IP_atk_layerC = 0;
  u16 IP_atk_layerD = 0;
  u8 flag_error = 0;
  if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IP_atk_layerA, &IP_atk_layerB, &IP_atk_layerC, &IP_atk_layerD) != 4)
  {
    flag_error = 1;
  }
  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
    return;
  }

  IPAttacker = IP_atk_layerA * 16777216 + IP_atk_layerB * 65536 + IP_atk_layerC * 256 + IP_atk_layerD;
  delay_1ms();
  // delay_1ms();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 172, IPAttacker);
  delay_1ms();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000010);
  if (Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 204))
  {
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000000);
    // XUartLite_SendByte(0x40600000, 'C');
  }
  delay_1ms();
  XUartLite_SendByte(0x40600000, 'Y');
}
void add_ipv4_table_http_whitelist(const char *ip_str)
{
  u16 IP_atk_layerA = 0;
  u16 IP_atk_layerB = 0;
  u16 IP_atk_layerC = 0;
  u16 IP_atk_layerD = 0;
  u8 flag_error = 0;
  if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IP_atk_layerA, &IP_atk_layerB, &IP_atk_layerC, &IP_atk_layerD) != 4)
  {
    flag_error = 1;
  }
  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'C');
    return;
  }

  IPAttacker = IP_atk_layerA * 16777216 + IP_atk_layerB * 65536 + IP_atk_layerC * 256 + IP_atk_layerD;
  delay_1ms();
  //
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 576, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 580, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 584, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 588, IPAttacker);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 592, 0x00000014);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 572, 0x00000001);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 572, 0x00000000);
  //
  delay_1ms();
  XUartLite_SendByte(0x40600000, 'Y');
}

void Add_IPv4_HTTP_Table_From_File(const char *ip_str)
{
  u16 IP_atk_layerA = 0;
  u16 IP_atk_layerB = 0;
  u16 IP_atk_layerC = 0;
  u16 IP_atk_layerD = 0;
  u8 flag_error = 0;
  if (sscanf(ip_str, "%hu.%hu.%hu.%hu", &IP_atk_layerA, &IP_atk_layerB, &IP_atk_layerC, &IP_atk_layerD) != 4)
  {
    flag_error = 1;
  }
  if (flag_error == 1)
  {
    // XUartLite_SendByte(0x40600000, 'N');
    return;
  }

  IPAttacker = IP_atk_layerA * 16777216 + IP_atk_layerB * 65536 + IP_atk_layerC * 256 + IP_atk_layerD;
  // delay_1s();
  delay_1ms();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 172, IPAttacker);
  delay_1ms();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000010);
  if (Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 204))
  {
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000000);
    // XUartLite_SendByte(0x40600000, 'C');
  }
  delay_1ms();
  // XUartLite_SendByte(0x40600000, 'Y');
}

void ADD_IPv4_HTTP_BLACK_LIST()
{
  char buffer_IPV4[BUFFER_SIZE_IPV4];
  ReceiveIPv4String(buffer_IPV4, BUFFER_SIZE_IPV4);
  add_ipv4_table_http_blacklist(buffer_IPV4);
}

void ADD_IPv4_HTTP_WHITE_LIST()
{
  char buffer_IPV4[BUFFER_SIZE_IPV4];
  ReceiveIPv4String(buffer_IPV4, BUFFER_SIZE_IPV4);
  add_ipv4_table_http_whitelist(buffer_IPV4);
}
void Clear_Atk_IP(unsigned int *Atk_List, unsigned int atk_ip)
{
  bool no_valid_IP = false;
  for (int i = 0; i < 100; i++)
  {
    if (*(Atk_List + i) == atk_ip)
    {
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 172, atk_ip);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000030);
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000000);
      *(Atk_List + i) = 0;
      // XUartLite_SendByte(0x40600000, 'Y');
      break;
    }
    else if ((*(Atk_List + i) != atk_ip) && (*(Atk_List + i) != 0))
    {
      no_valid_IP = true;
    }
  }
  if (no_valid_IP)
  {
    // xil_printf("\r\nCannot clear!, Not valid IP in Black List \r\n");
    // XUartLite_SendByte(0x40600000, 'N');
    // break;
  }
  // is_delete_http = 1;
}
void ADD_IPv6_HTTP_BLACK_LIST()
{
  int IPv6_ATTACK[4] = {0};
  u8 key;
  u16 IPv6Segment[IPV6_SEGMENTS] = {0};
  u8 next = 0;
  u8 flag_error = 0;
  char input_buffer[40] = {0};
  u8 count = 0;
  int double_colon_position = -1;

  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 13)
    {
      break;
    }

    else if (key == 03)
    {
      Reset_System();
    }
    else
    {
      input_buffer[count++] = key;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  input_buffer[count] = '\0';

  char *token = strtok(input_buffer, ":");
  while (token != NULL && next < IPV6_SEGMENTS)
  {
    if (strcmp(token, "") == 0)
    {
      if (double_colon_position == -1)
      {
        double_colon_position = next;
        token = strtok(NULL, ":");
        continue;
      }
      else
      {
        flag_error = 1;
        break;
      }
    }
    else
    {
      if (count_digits(token) > 4)
      {
        flag_error = 1;
        break;
      }
      u16 segment_value = (u16)strtol(token, NULL, 16);
      IPv6Segment[next++] = segment_value;
      // xil_printf("\r\nSegment %d set to %04x", next - 1, segment_value);
    }
    token = strtok(NULL, ":");
    // XUartLite_SendByte(0x40600000, 'H');
  }

  if (double_colon_position != -1)
  {
    int segments_to_add = IPV6_SEGMENTS - next;
    for (int i = 0; i < segments_to_add; i++)
    {
      IPv6Segment[double_colon_position + i] = 0;
    }
    for (int i = 0; token != NULL; i++)
    {
      if (next < IPV6_SEGMENTS)
      {
        u16 segment_value = (u16)strtol(token, NULL, 16);
        IPv6Segment[double_colon_position + segments_to_add + i] = segment_value;
        next++;
      }
      token = strtok(NULL, ":");
    }
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  else
  {

    IPv6_ATTACK[0] = (IPv6Segment[0] << 16) | IPv6Segment[1];
    IPv6_ATTACK[1] = (IPv6Segment[2] << 16) | IPv6Segment[3];
    IPv6_ATTACK[2] = (IPv6Segment[4] << 16) | IPv6Segment[5];
    IPv6_ATTACK[3] = (IPv6Segment[6] << 16) | IPv6Segment[7];
    //  delay_1ms();

    // xil_printf("\r\nIPv6 attack: %08x:%08x:%08x:%08x\r\n", IPv6_ATTACK[0], IPv6_ATTACK[1], IPv6_ATTACK[2], IPv6_ATTACK[3]);

    // xil_printf("\r\ncheck1");

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 424, IPV6_Version);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 408, IPv6_ATTACK[0]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 412, IPv6_ATTACK[1]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 416, IPv6_ATTACK[2]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 420, IPv6_ATTACK[3]);
    // xil_printf("\r\ncheck2");
    // delay_1ms();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 428, 0x00000001);

    //  xil_printf("\r\ncheck3");
    //  delay_1ms();
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 428, 0x00000000);
    // xil_printf("\r\nIPv6 attack: %08x:%08x:%08x:%08x\r\n", IPv6_ATTACK[0], IPv6_ATTACK[1], IPv6_ATTACK[2], IPv6_ATTACK[3]);

    XUartLite_SendByte(0x40600000, 'Y');
  }
}
// void Add_IPv6_HTTP_Table_From_File(const char *ipv6_str)
// {
//   int IPv6_ATTACK[4] = {0};
//   u16 IPv6Segment[IPV6_SEGMENTS] = {0};
//   u8 next = 0;
//   u8 flag_error = 0;
//   int double_colon_position = -1;

//   char input_buffer[40] = {0};
//   strncpy(input_buffer, ipv6_str, sizeof(input_buffer) - 1);

//   char *token = strtok(input_buffer, ":");
//   while (token != NULL && next < IPV6_SEGMENTS)
//   {
//     if (strcmp(token, "") == 0)
//     {
//       if (double_colon_position == -1)
//       {
//         double_colon_position = next;
//         token = strtok(NULL, ":");
//         continue;
//       }
//       else
//       {
//         flag_error = 1;
//         break;
//       }
//     }
//     else
//     {
//       if (count_digits(token) > 4)
//       {
//         flag_error = 1;
//         break;
//       }
//       u16 segment_value = (u16)strtol(token, NULL, 16);
//       IPv6Segment[next++] = segment_value;
//     }
//     token = strtok(NULL, ":");
//     XUartLite_SendByte(0x40600000, 'W');
//   }

//   if (double_colon_position != -1)
//   {
//     int segments_to_add = IPV6_SEGMENTS - next;
//     for (int i = IPV6_SEGMENTS - 1; i >= double_colon_position + segments_to_add; i--)
//     {
//       IPv6Segment[i] = IPv6Segment[i - segments_to_add];
//     }
//     for (int i = double_colon_position; i < double_colon_position + segments_to_add; i++)
//     {
//       IPv6Segment[i] = 0;
//     }
//   }

//   if (flag_error == 1)
//   {
//     //  XUartLite_SendByte(0x40600000, 'N');
//     return;
//     XUartLite_SendByte(0x40600000, 'L');
//   }

//   IPv6_ATTACK[0] = (IPv6Segment[0] << 16) | IPv6Segment[1];
//   IPv6_ATTACK[1] = (IPv6Segment[2] << 16) | IPv6Segment[3];
//   IPv6_ATTACK[2] = (IPv6Segment[4] << 16) | IPv6Segment[5];
//   IPv6_ATTACK[3] = (IPv6Segment[6] << 16) | IPv6Segment[7];

//   Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 424, IPV6_Version);
//   Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 408, IPv6_ATTACK[0]);
//   Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 412, IPv6_ATTACK[1]);
//   Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 416, IPv6_ATTACK[2]);
//   Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 420, IPv6_ATTACK[3]);
//   Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 428, 0x00000001);
//   Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 428, 0x00000000);

//  XUartLite_SendByte(0x40600000, 'X');
// }
void Add_IPv6_Protect_Table_From_File(const char *ipv6_str)
{
  u16 IPv6Segment[IPV6_SEGMENTS] = {0};
  const char *ptr = ipv6_str;
  char *endptr;

  for (int i = 0; i < IPV6_SEGMENTS; i++)
  {
    IPv6Segment[i] = (u16)strtol(ptr, &endptr, 16);
    if (endptr == ptr || (endptr - ptr) > 4)
    {
      return;
      // XUartLite_SendByte(0x40600000, 'p');
    }
    ptr = endptr + 1;
    // XUartLite_SendByte(0x40600000, 'K');
  }

  int IPv6_Protect_ADD[4] = {
      (IPv6Segment[0] << 16) | IPv6Segment[1],
      (IPv6Segment[2] << 16) | IPv6Segment[3],
      (IPv6Segment[4] << 16) | IPv6Segment[5],
      (IPv6Segment[6] << 16) | IPv6Segment[7]};

  delay_1ms();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 524, IPv6_Protect_ADD[0]);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 528, IPv6_Protect_ADD[1]);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 532, IPv6_Protect_ADD[2]);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 536, IPv6_Protect_ADD[3]);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 540, IPV6_Version);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 544, 1);
  // delay_1s();
  delay_1ms();
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 544, 0);
  // XUartLite_SendByte(0x40600000, 'Y');
}
void Add_IPv6_HTTP_Table_From_File(const char *ipv6_str)
{
  u16 IPv6Segment[IPV6_SEGMENTS] = {0};
  const char *ptr = ipv6_str;
  char *endptr;

  for (int i = 0; i < IPV6_SEGMENTS; i++)
  {
    IPv6Segment[i] = (u16)strtol(ptr, &endptr, 16);
    if (endptr == ptr || (endptr - ptr) > 4)
    {
      return;
      // XUartLite_SendByte(0x40600000, 'p');
    }
    ptr = endptr + 1;
    // XUartLite_SendByte(0x40600000, 'K');
  }

  int IPv6_ATTACK[4] = {
      (IPv6Segment[0] << 16) | IPv6Segment[1],
      (IPv6Segment[2] << 16) | IPv6Segment[3],
      (IPv6Segment[4] << 16) | IPv6Segment[5],
      (IPv6Segment[6] << 16) | IPv6Segment[7]};

  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 408, IPv6_ATTACK[0]);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 412, IPv6_ATTACK[1]);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 416, IPv6_ATTACK[2]);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 420, IPv6_ATTACK[3]);

  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 428, 0x00000001);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 428, 0x00000000);
  // XUartLite_SendByte(0x40600000, 'L');
}

void Clear_IPv6_HTTP_Table_From_File(const char *ipv6_str)
{
  int IPv6_ATTACK[4] = {0};
  u16 IPv6Segment[IPV6_SEGMENTS] = {0};
  u8 next = 0;
  u8 flag_error = 0;
  int double_colon_position = -1;

  char input_buffer[40] = {0};
  strncpy(input_buffer, ipv6_str, sizeof(input_buffer) - 1);

  char *token = strtok(input_buffer, ":");
  while (token != NULL && next < IPV6_SEGMENTS)
  {
    if (strcmp(token, "") == 0)
    {
      if (double_colon_position == -1)
      {
        double_colon_position = next;
        token = strtok(NULL, ":");
        continue;
      }
      else
      {
        flag_error = 1;
        break;
      }
    }
    else
    {
      if (count_digits(token) > 4)
      {
        flag_error = 1;
        break;
      }
      u16 segment_value = (u16)strtol(token, NULL, 16);
      IPv6Segment[next++] = segment_value;
    }
    token = strtok(NULL, ":");
  }

  if (double_colon_position != -1)
  {
    int segments_to_add = IPV6_SEGMENTS - next;
    for (int i = IPV6_SEGMENTS - 1; i >= double_colon_position + segments_to_add; i--)
    {
      IPv6Segment[i] = IPv6Segment[i - segments_to_add];
    }
    for (int i = double_colon_position; i < double_colon_position + segments_to_add; i++)
    {
      IPv6Segment[i] = 0;
    }
  }

  if (flag_error == 1)
  {
    //  XUartLite_SendByte(0x40600000, 'N');
    return;
  }

  IPv6_ATTACK[0] = (IPv6Segment[0] << 16) | IPv6Segment[1];
  IPv6_ATTACK[1] = (IPv6Segment[2] << 16) | IPv6Segment[3];
  IPv6_ATTACK[2] = (IPv6Segment[4] << 16) | IPv6Segment[5];
  IPv6_ATTACK[3] = (IPv6Segment[6] << 16) | IPv6Segment[7];
  delay_1ms();

  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 424, IPV6_Version);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 408, IPv6_ATTACK[0]);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 412, IPv6_ATTACK[1]);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 416, IPv6_ATTACK[2]);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 420, IPv6_ATTACK[3]);

  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 428, 0x00000001);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000020);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 428, 0x00000000);
  Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000000);

  // XUartLite_SendByte(0x40600000, 'Y');
}

void REMOVE_IPv6_HTTP_BLACK_LIST()
{
  int IPv6_ATTACK[4] = {0};
  u8 key;
  u16 IPv6Segment[IPV6_SEGMENTS] = {0};
  u8 next = 0;
  u8 flag_error = 0;
  char input_buffer[40] = {0};
  u8 count = 0;
  int double_colon_position = -1;

  // xil_printf("\r\n IPv6 ");
  while (1)
  {
    key = XUartLite_RecvByte(0x40600000);
    if (key == 13)
    {
      break;
    }
    else if (key == 03)
    {
      Reset_System();
    }
    else
    {
      // xil_printf("%c", key);
      input_buffer[count++] = key;
    }
    // XUartLite_SendByte(0x40600000, 'H');
  }
  input_buffer[count] = '\0';

  char *token = strtok(input_buffer, ":");
  while (token != NULL && next < IPV6_SEGMENTS)
  {
    if (strcmp(token, "") == 0)
    {
      if (double_colon_position == -1)
      {
        double_colon_position = next;
        token = strtok(NULL, ":");
        continue;
      }
      else
      {
        flag_error = 1;
        break;
      }
    }
    else
    {
      if (count_digits(token) > 4)
      {
        flag_error = 1;
        break;
      }
      u16 segment_value = (u16)strtol(token, NULL, 16);
      IPv6Segment[next++] = segment_value;
      // xil_printf("\r\nSegment %d set to %04x", next - 1, segment_value);
    }
    token = strtok(NULL, ":");
    // XUartLite_SendByte(0x40600000, 'H');
  }

  if (double_colon_position != -1)
  {
    int segments_to_add = IPV6_SEGMENTS - next;
    for (int i = 0; i < segments_to_add; i++)
    {
      IPv6Segment[double_colon_position + i] = 0;
    }
    for (int i = 0; token != NULL; i++)
    {
      if (next < IPV6_SEGMENTS)
      {
        u16 segment_value = (u16)strtol(token, NULL, 16);
        IPv6Segment[double_colon_position + segments_to_add + i] = segment_value;
        next++;
      }
      token = strtok(NULL, ":");
    }
  }

  if (flag_error == 1)
  {
    XUartLite_SendByte(0x40600000, 'N');
  }
  else
  {
    //   xil_printf("\r\n ip  ");
    //   for (int i = 0; i < IPV6_SEGMENTS; i++)
    //   {
    //     xil_printf("%04x", IPv6Segment[i]);
    //     if (i < IPV6_SEGMENTS - 1)
    //     {
    //       xil_printf(":");
    //     }
    //   }
    // }
    IPv6_ATTACK[0] = (IPv6Segment[0] << 16) | IPv6Segment[1];
    IPv6_ATTACK[1] = (IPv6Segment[2] << 16) | IPv6Segment[3];
    IPv6_ATTACK[2] = (IPv6Segment[4] << 16) | IPv6Segment[5];
    IPv6_ATTACK[3] = (IPv6Segment[6] << 16) | IPv6Segment[7];
    delay_1ms();

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 424, IPV6_Version);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 408, IPv6_ATTACK[0]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 412, IPv6_ATTACK[1]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 416, IPv6_ATTACK[2]);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 420, IPv6_ATTACK[3]);

    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 428, 0x00000001);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000020);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 428, 0x00000000);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000000);

    XUartLite_SendByte(0x40600000, 'Y');
  }
}

void init_http_resource()
{
  // unsigned int *Attacker_List;
  Attacker_List = (unsigned int *)malloc(100 * sizeof(unsigned int));
  // if (Attacker_List == NULL)
  //     xil_printf("\r\nAttacker List malloc failed\r\n");
  // else //xil_printf("Attacker List malloc successfully\r\n");

  IP_Conn_Table = (struct IP_Connection *)malloc(Num_of_IP * sizeof(struct IP_Connection));
  // if (IP_Conn_Table == NULL)
  //     xil_printf("IP malloc failed\r\n");
  // else //xil_printf("IP malloc successfully\r\n");

  ////xil_printf("size of IP Table: %d (bytes)\r\n", sizeof(*IP_Conn_Table));
  // //xil_printf("size of IP Conn: %d (bytes)\r\n", sizeof(struct IP_Connection));
  ////xil_printf("IP Conn Addr: %08x\r\n", IP_Conn_Table);
  ////xil_printf("Size IP Pointer: %d\r\n", malloc_usable_size(IP_Conn_Table));

  URL_Conn_Table = (struct URL_Connection *)malloc(Num_of_URL * sizeof(struct URL_Connection));
  // if (URL_Conn_Table == NULL)
  //     xil_printf("URL malloc failed\r\n");
  // else //xil_printf("URL malloc successfully\r\n");

  ////xil_printf("size of URL Conn: %d (bytes)\r\n", sizeof(struct URL_Connection));
  ////xil_printf("Size URL Pointer: %d\r\n", malloc_usable_size(URL_Conn_Table));

  for (int i = 0; i < Num_of_URL; i++)
  {
    (URL_Conn_Table + i)->PAL = (struct Potential_IP_List *)malloc(Num_in_PAL * sizeof(struct Potential_IP_List));
    // if ((URL_Conn_Table+i)->PAL == NULL)
    //     xil_printf("PAL malloc failed\r\n");
    //  else xil_printf("PAL malloc successfully\r\n");
  }
  ////xil_printf("size of PAL Conn: %d (bytes)\r\n", sizeof(struct Potential_IP_List));
}

void cleanup_http_resource()
{
  // free(current_config);
  free(IP_Conn_Table);
  free(URL_Conn_Table);
  // free(PAL);
}

// int get_HTTP_Data (struct IP_Connection *IP_Conn_Table, struct URL_Connection *URL_Conn_Table, int *numURL, int *numIP, int *numPAL, unsigned int *Atk_List)
int get_HTTP_Data()
{
  int http_fifo_empty;
  int export_rdy;
  unsigned int packet_timer;
  unsigned int src_ip;
  unsigned int hash_url;

  http_fifo_empty = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 188);
  export_rdy = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 208);

  // //xil_printf("| Empty: %d ", http_fifo_empty);
  // //xil_printf("| Ready: %d ", export_rdy);
  ////xil_printf("| WR: %lu ", Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 192));
  // //xil_printf("| RD: %lu ", Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 196));

  // //xil_printf("| Drop: %5lu", Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 212));
  // //xil_printf("| Bypass: %5lu", Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 216));

  if ((!http_fifo_empty) & export_rdy)
  {
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000001);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000000);

    packet_timer = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 176);
    src_ip = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 180);
    hash_url = Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 184);

    // //xil_printf("| Timer: %x ", packet_timer);
    ////xil_printf("| IP: %08x ", src_ip);
    // //xil_printf("| URL_hash: %08x ", hash_url);

    // //xil_printf("| just_IP: %08x ", Atk_HTTP.just_detected_IP);
    // //xil_printf("| Drop: %5d", ANTI_DDOS_10GB_mReadReg(ptr + offset, HTTP_DROP_PKT));
    // //xil_printf("| Bypass: %5d", ANTI_DDOS_10GB_mReadReg(ptr + offset, HTTP_BYPASS_PKT));

    // if (Atk_HTTP.just_detected_IP == src_ip)
    // {
    //     if (ANTI_DDOS_10GB_mReadReg(ptr + offset, HTTP_TIME_INTERVAL))
    //     {
    //         // //xil_printf("\nChecking timeout\r\n");
    //         // getchar();
    //         // reset_HTTP_Table(IP_Conn_Table, URL_Conn_Table, numURL, numIP, numPAL);
    //         ANTI_DDOS_10GB_mWriteReg (ptr + offset, HTTP_RD_EN, 0x00000100);    // next interval
    //         ANTI_DDOS_10GB_mWriteReg (ptr + offset, HTTP_RD_EN, 0x00000000);
    //     }
    //     return Atk_HTTP.connected_IP;
    // }

    // Atk_HTTP.connected_IP = IP_Management(IP_Conn_Table, src_ip, packet_timer, numIP);
    Atk_HTTP.connected_IP = IP_Management(IP_Conn_Table, src_ip, packet_timer, &Num_of_IP);
    // //xil_printf("| Connected IP: %d ", Atk_HTTP.connected_IP);
    // Atk_HTTP.attacked_URL = URL_Management(URL_Conn_Table, hash_url, Atk_HTTP.connected_IP, numURL);
    Atk_HTTP.attacked_URL = URL_Management(URL_Conn_Table, hash_url, Atk_HTTP.connected_IP, &Num_of_URL);
  }

  // can xem xet dua cac ham ben duoi vao if neu khong khi empty cac ham nay van thuc hien la sai
  if (Atk_HTTP.attacked_URL > 0)
  {
    // //xil_printf("| Target_URL: %08x ", Atk_HTTP.attacked_URL);
  }

  // //xil_printf("| Connected IP: %d ", Atk_HTTP.connected_IP);

  // //xil_printf("| URL_hash: %08x ", hash_url);
  if (Atk_HTTP.attacked_URL > 0 && Atk_HTTP.attacked_URL == hash_url)
  {
    // //xil_printf("| Dection ");
    // Attacker_Detection(URL_Conn_Table, src_ip, numPAL, Atk_List);
    Attacker_Detection(URL_Conn_Table, src_ip, &Num_in_PAL, Attacker_List);
    // getchar();
  }

  // for (int i=0; i<50; i++)
  // {
  //     if (*(Attacker_List+i) != 0)
  //     {
  //         //xil_printf("| Atk_IP: %08x ", *(Attacker_List+i));
  //     }
  // }

  // //xil_printf("| Time Interval: %d", ANTI_DDOS_10GB_mReadReg(ptr + offset, HTTP_TIME_INTERVAL));
  if (Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 200))
  {
    // //xil_printf("\nChecking timeout\r\n");
    // getchar();
    // reset_HTTP_Table(IP_Conn_Table, URL_Conn_Table, numURL, numIP, numPAL);
    reset_HTTP_Table(IP_Conn_Table, URL_Conn_Table, &Num_of_URL, &Num_of_IP, &Num_in_PAL);
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000100); // next interval
    Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000000);
  }

  return Atk_HTTP.connected_IP;
}

int URL_Management(struct URL_Connection *URL_Conn_Table, unsigned int url, unsigned int IP_cnt, int *numURL)
{
  int URL_Atk_Threshold;
  int attacked_URL;
  int temp_counter;
  int index;
  int numURLs = 0;

  URL_Atk_Threshold = IP_cnt * 15 * IP_TIMEOUT;
  // xil_printf("| Nth1: %d ", URL_Atk_Threshold);

  for (int i = 0; i < *numURL; i++)
  {
    if (!(URL_Conn_Table + i)->b_flag)
    {
      (URL_Conn_Table + i)->b_flag = true;
      (URL_Conn_Table + i)->hash_url = url;
      (URL_Conn_Table + i)->request_counter++;
      break;
    }
    else if ((URL_Conn_Table + i)->b_flag)
    {
      if ((URL_Conn_Table + i)->hash_url == url)
      {
        (URL_Conn_Table + i)->request_counter++;
        break;
      }
      else
        ; // neu khong cung URL va khong con vung nho trong trong khoi nho cua URL_Table thi cap phat them
    }
  }

  index = 0;
  temp_counter = 0;
  for (int i = 0; i < *numURL; i++)
  {
    if ((URL_Conn_Table + i)->b_flag && ((URL_Conn_Table + i)->request_counter > Atk_URL.max_URL_cnt))
    {
      // //xil_printf("| URL %d Max: %d ", i, (URL_Conn_Table+i)->request_counter);
      Atk_URL.max_URL_cnt = (URL_Conn_Table + i)->request_counter;
      index = i;
    }
    if ((URL_Conn_Table + i)->b_flag)
    {
      // //xil_printf("| URL %d: %d ", i, (URL_Conn_Table+i)->request_counter);
      // Atk_URL.num_URLs++;
      numURLs++;
    }
  }

  // //xil_printf("| Index: %d ", index);
  // xil_printf("| Mcnt: %d ", Atk_URL.max_URL_cnt);
  // xil_printf("| NumURL: %d ", numURLs);
  if (Atk_URL.max_URL_cnt > URL_Atk_Threshold)
  {
    Atk_URL.index = index;
    Atk_URL.url = (URL_Conn_Table + index)->hash_url;
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

int IP_Management(struct IP_Connection *IP_Conn_Table, unsigned int ip, int timer, int *numIP)
{
  int connected_IP;
  int next_size;

  for (int i = 0; i < *numIP; i++)
  {
    if (!(IP_Conn_Table + i)->b_flag)
    {
      (IP_Conn_Table + i)->b_flag = true;
      (IP_Conn_Table + i)->src_ip = ip;
      (IP_Conn_Table + i)->timer = timer;
      break;
    }
    else if ((IP_Conn_Table + i)->b_flag)
    {
      if ((IP_Conn_Table + i)->src_ip == ip)
      {
        if ((timer - (IP_Conn_Table + i)->timer) > (IP_TIMEOUT * 1000)) // vi timestamp_counter duoi core tinh bang ms nen TIMEMOUT*1000
        {
          // //xil_printf("| ID:%d ", i);
          (IP_Conn_Table + i)->b_flag = false;
          (IP_Conn_Table + i)->src_ip = 0;
          (IP_Conn_Table + i)->timer = 0;
          break;
        }
        else
        {
          (IP_Conn_Table + i)->timer = timer;
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

  // //xil_printf("| *NumIP: %d ", *numIP);
  connected_IP = 0;
  for (int i = 0; i < *numIP; i++)
  {
    if ((IP_Conn_Table + i)->b_flag)
    {
      // //xil_printf("| Index: %d ", i);
      // //xil_printf("| Flag: %d ", (IP_Conn_Table+i)->b_flag);
      // //xil_printf("| Timer: %x ", (IP_Conn_Table+i)->timer);
      // //xil_printf("| IP: %8x ", (IP_Conn_Table+i)->src_ip);
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

int Blacklist(unsigned int atk_ip, unsigned int *Atk_List)
{
  // xil_printf("| Up BL ");
  for (int i = 0; i < 100; i++)
  {
    if (*(Atk_List + i) == atk_ip)
    {
      break;
    }
    else if (*(Atk_List + i) == 0)
    {
      *(Atk_List + i) = atk_ip;
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 172, atk_ip); // cho den khi xay dung t/hieu clear atk ip
      // ANTI_DDOS_10GB_mWriteReg (ptr + offset, HTTP_ATK_IP, 0x00000000);        // cho den khi xay dung t/hieu clear atk ip
      Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000010);
      if (Xil_In32(XPAR_DDOS_DEFENDER_0_BASEADDR + 204))
      {
        Xil_Out32(XPAR_DDOS_DEFENDER_0_BASEADDR + 168, 0x00000000);
      }
      break;
    }
    else
      ;
  }
  return 0;
}

int Attacker_Detection(struct URL_Connection *URL_Conn_Table, unsigned int ip, int *numPAL, unsigned int *Atk_List)
{
  int PAL_Thres;
  PAL_Thres = 15 * IP_TIMEOUT;
  unsigned int target_IP;
  // bool atk_flag = false;

  for (int i = 0; i < *numPAL; i++)
  {
    if (!((URL_Conn_Table + Atk_URL.index)->PAL + i)->b_flag)
    {
      ((URL_Conn_Table + Atk_URL.index)->PAL + i)->b_flag = true;
      ((URL_Conn_Table + Atk_URL.index)->PAL + i)->src_ip_1 = ip;
      ((URL_Conn_Table + Atk_URL.index)->PAL + i)->pkt_counter_1++;
      break;
    }
    else if (((URL_Conn_Table + Atk_URL.index)->PAL + i)->b_flag)
    {
      if (((URL_Conn_Table + Atk_URL.index)->PAL + i)->src_ip_1 == ip)
      {
        ((URL_Conn_Table + Atk_URL.index)->PAL + i)->pkt_counter_1++;
        break;
      }
      else
        ;
    }
  }

  // xil_printf("| Atk_URL.index: %d ", Atk_URL.index);
  for (int i = 0; i < *numPAL; i++)
  {
    if (((URL_Conn_Table + Atk_URL.index)->PAL + i)->b_flag)
    {
      // //xil_printf("| Atk IP: %08x ", ((URL_Conn_Table + Atk_URL.index)->PAL+i)->src_ip_1);
      // //xil_printf("| Cnt: %d ", ((URL_Conn_Table + Atk_URL.index)->PAL+i)->pkt_counter_1);
      if (((URL_Conn_Table + Atk_URL.index)->PAL + i)->pkt_counter_1 > PAL_Thres)
      {
        // xil_printf("| PAL %d: %d ", i, ((URL_Conn_Table + Atk_URL.index)->PAL+i)->pkt_counter_1);
        Blacklist(((URL_Conn_Table + Atk_URL.index)->PAL + i)->src_ip_1, Atk_List);
        Atk_HTTP.just_detected_IP = ((URL_Conn_Table + Atk_URL.index)->PAL + i)->src_ip_1;
        // atk_flag = true;
      }
    }
  }

  return 0;
}

void reset_HTTP_Table(struct IP_Connection *IP_Conn_Table, struct URL_Connection *URL_Conn_Table, int *numURL, int *numIP, int *numPAL)
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

  for (int i = 0; i < *numURL; i++)
  {
    (URL_Conn_Table + i)->b_flag = false;
    (URL_Conn_Table + i)->index = 0;
    (URL_Conn_Table + i)->hash_url = 0;
    (URL_Conn_Table + i)->request_counter = 0;
    for (int j = 0; j < *numPAL; j++)
    {
      ((URL_Conn_Table + i)->PAL + j)->b_flag = false;
      ((URL_Conn_Table + i)->PAL + j)->src_ip_1 = 0;
      ((URL_Conn_Table + i)->PAL + j)->pkt_counter_1 = 0;
    }
  }

  for (int i = 0; i < *numIP; i++)
  {
    (IP_Conn_Table + i)->b_flag = false;
    (IP_Conn_Table + i)->src_ip = 0;
    (IP_Conn_Table + i)->timer = 0;
  }

  Atk_HTTP.connected_IP = 0;
  Atk_HTTP.attacked_URL = 0;

  Atk_URL.index = 0;
  Atk_URL.url = 0;
  Atk_URL.max_URL_cnt = 0;
}
// main.c 31-3
