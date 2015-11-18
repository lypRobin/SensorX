/*
 * File	: config.c
 * Implementation file of esp8266 configuerations including +++AT commands. 
 * Copyright (C) 2015 - 2016, Yanpeng Li <lyp40293@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.	If not, see <http://www.gnu.org/licenses/>.
 */

#include <ets_sys.h>
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"
#include "driver/uart.h"

#include "flash_param.h"
#include "server.h"
#include "config.h"

void config_cmd_reset(server_conn_data *conn, uint8_t argc, char *argv[]);
void config_cmd_gpio2(server_conn_data *conn, uint8_t argc, char *argv[]);
void config_cmd_baud(server_conn_data *conn, uint8_t argc, char *argv[]);
void config_cmd_port(server_conn_data *conn, uint8_t argc, char *argv[]);
void config_cmd_mode(server_conn_data *conn, uint8_t argc, char *argv[]);
void config_cmd_sta(server_conn_data *conn, uint8_t argc, char *argv[]); 
void config_cmd_ap(server_conn_data *conn, uint8_t argc, char *argv[]);
void config_cmd_status(server_conn_data *conn, uint8_t argc, char *argv[]);
void config_cmd_sta_ip(server_conn_data *conn, uint8_t argc, char *argv[]);
void config_cmd_sta_hostname(server_conn_data *conn, uint8_t argc, char *argv[]);
void config_cmd_remote_server(server_conn_data *conn, uint8_t argc, char *argv[]);
void config_cmd_restore(server_conn_data *conn, uint8_t argc, char *argv[]);

void config_default_ap(bool restore);
void config_default_sta_hostname(bool restore);
void config_default_flash_param(bool restore);
int check_ip_validation(const char *str);

const config_commands_t config_commands[] = {
		{ "RESET", &config_cmd_reset },
		{ "PORT", &config_cmd_port },
		{ "REMOTE", &config_cmd_remote_server}, // config remote server ip and port when esp8266 as a client
		{ "MODE", &config_cmd_mode },
		{ "STA", &config_cmd_sta },   // 
		{ "AP", &config_cmd_ap },
		{ "GPIO2", &config_cmd_gpio2 },
		{ "RESTORE", &config_cmd_restore},
		{ "STAHOSTNAME", &config_cmd_sta_hostname},
		{ "STAIP", &config_cmd_sta_ip }, // set sta ip address
		{ "STATUS", &config_cmd_status }, // get current status including baud, port, sta ip, sta ssid, ap ip, ap ssid and mode.
		{ NULL, NULL }
	};

bool doflash = true;

void config_gpio(void) {
	// Initialize the GPIO subsystem.
	gpio_init();
	//Set GPIO2 to output mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	//Set GPIO2 high
	gpio_output_set(BIT2, 0, BIT2, 0);
}

void config_cmd_reset(server_conn_data *conn, uint8_t argc, char *argv[]) {
	espbuff_send_string(conn, MSG_OK);
	system_restart();
}


void config_cmd_gpio2(server_conn_data *conn, uint8_t argc, char *argv[]) {
	if (argc == 0)
		espbuff_send_printf(conn, "Args: 0=low, 1=high, 2 <delay in ms>=reset (delay optional).\r\n");
	else {
		uint32_t gpiodelay = 100;
		if (argc == 2) {
			gpiodelay = atoi(argv[2]);
		}
		uint8_t gpio = atoi(argv[1]);
		if (gpio < 3) {
			if (gpio == 0) {
				gpio_output_set(0, BIT2, BIT2, 0);
				espbuff_send_string(conn, "LOW\r\n");
			}
			if (gpio == 1) {
				gpio_output_set(BIT2, 0, BIT2, 0);
				espbuff_send_string(conn, "HIGH\r\n");
			}
			if (gpio == 2) {
				gpio_output_set(0, BIT2, BIT2, 0);
				os_delay_us(gpiodelay*1000);
				gpio_output_set(BIT2, 0, BIT2, 0);
				espbuff_send_printf(conn, "RESET %d ms\r\n",gpiodelay);
			}
		} else {
			espbuff_send_string(conn, MSG_ERROR);
		}
	}
}

void config_cmd_baud(server_conn_data *conn, uint8_t argc, char *argv[]) {
	flash_param_t *flash_param = flash_param_get();
	if (argc == 0)
		espbuff_send_printf(conn, "BAUD=%d %d %s %s\r\n"MSG_OK, flash_param->baud);
	else if(argc == 1){
		uint32_t baud = atoi(argv[1]);
		if ((baud > (UART_CLK_FREQ / 16)) || baud == 0) {
			espbuff_send_string(conn, MSG_ERROR);
			return;
		}
		
		flash_param->baud = baud;
		if (doflash) {
			if (flash_param_set())
				espbuff_send_string(conn, MSG_OK);
			else
				espbuff_send_string(conn, MSG_ERROR);
		}
		else
			espbuff_send_string(conn, MSG_OK);
	}
	else
		espbuff_send_string(conn, "+++AT BAUD Invalid arguments.\r\n"MSG_ERROR);
}


void config_cmd_port(server_conn_data *conn, uint8_t argc, char *argv[]) {
	flash_param_t *flash_param = flash_param_get();

	if (argc == 0)
		espbuff_send_printf(conn, "PORT=%d\r\n"MSG_OK, flash_param->port);
	else if (argc != 1)
		espbuff_send_string(conn, MSG_ERROR);
	else {
		uint32_t port = atoi(argv[1]);
		if ((port == 0)||(port>65535)) {
			espbuff_send_string(conn, MSG_ERROR);
		} else {
			if (port != flash_param->port) {
				flash_param->port = port;
				if (flash_param_set())
					espbuff_send_string(conn, MSG_OK);
				else
					espbuff_send_string(conn, MSG_ERROR);
				os_delay_us(10000);
				system_restart();
			} else {
				espbuff_send_string(conn, MSG_OK);
			}
		}
	}
}

void config_cmd_mode(server_conn_data *conn, uint8_t argc, char *argv[]) {
	uint8_t mode;

	if (argc == 0) {
		espbuff_send_printf(conn, "MODE=%d\r\n"MSG_OK, wifi_get_opmode());
	} else if (argc != 1) {
		espbuff_send_string(conn, MSG_ERROR);
	} else {
		mode = atoi(argv[1]);
		if (mode >= 1 && mode <= 3) {
			if (wifi_get_opmode() != mode) {
				ETS_UART_INTR_DISABLE();
				wifi_set_opmode(mode);
				ETS_UART_INTR_ENABLE();
				espbuff_send_string(conn, MSG_OK);
				os_delay_us(10000);
				system_restart();
			} else {
				espbuff_send_string(conn, MSG_OK);
			}
		} else {
			espbuff_send_string(conn, MSG_ERROR);
		}
	}
}

// spaces are not supported in the ssid or password
void config_cmd_sta(server_conn_data *conn, uint8_t argc, char *argv[]) {
	char *ssid = argv[1], *password = argv[2];
	struct station_config sta_conf;

	os_bzero(&sta_conf, sizeof(struct station_config));
	wifi_station_get_config(&sta_conf);

	if (argc == 0)
		espbuff_send_printf(conn, "SSID=%s PASSWORD=%s\r\n"MSG_OK, sta_conf.ssid, sta_conf.password);
	 else if (argc != 2)
		espbuff_send_string(conn, MSG_ERROR);
	else {
		os_strncpy(sta_conf.ssid, ssid, sizeof(sta_conf.ssid));
		os_strncpy(sta_conf.password, password, sizeof(sta_conf.password));
		espbuff_send_string(conn, MSG_OK);
		wifi_station_disconnect();
		ETS_UART_INTR_DISABLE();
		wifi_station_set_config(&sta_conf);
		ETS_UART_INTR_ENABLE();
		wifi_station_connect();
	}
}

// spaces are not supported in the ssid or password
void config_cmd_ap(server_conn_data *conn, uint8_t argc, char *argv[]) {
	char *ssid = argv[1], *password = argv[2];
	struct softap_config ap_conf;
	os_bzero(&ap_conf, sizeof(struct softap_config));
	wifi_softap_get_config(&ap_conf);
	if (argc == 0)
		espbuff_send_printf(conn, "SSID=%s PASSWORD=%s AUTHMODE=%d CHANNEL=%d\r\n"MSG_OK, ap_conf.ssid, ap_conf.password, ap_conf.authmode, ap_conf.channel);
	else if (argc > 4)
		espbuff_send_string(conn, MSG_ERROR);
	else { //argc > 0
		os_strncpy(ap_conf.ssid, ssid, sizeof(ap_conf.ssid));
		ap_conf.ssid_len = strlen(ssid); //without set ssid_len, no connection to AP is possible
		if (argc == 1) { //  no password
			os_bzero(ap_conf.password, sizeof(ap_conf.password));
			ap_conf.authmode = AUTH_OPEN;
		} else { // with password
			os_strncpy(ap_conf.password, password, sizeof(ap_conf.password));
			if (argc > 2) { // authmode
				int amode = atoi(argv[3]);
				if ((amode < 1) || (amode>4)) {
					espbuff_send_string(conn, MSG_ERROR);
					return;
				}
				ap_conf.authmode = amode;
			}
			if (argc > 3) { //channel
				int chan = atoi(argv[4]);
				if ((chan < 1) || (chan>13)){
					espbuff_send_string(conn, MSG_ERROR);
					return;
				}
				ap_conf.channel = chan;
			}
		}
		espbuff_send_string(conn, MSG_OK);
		ETS_UART_INTR_DISABLE();
		wifi_softap_set_config(&ap_conf);
		ETS_UART_INTR_ENABLE();
	}
}


void config_cmd_status(server_conn_data *conn, uint8_t argc, char *argv[]){
	if(argc == 0){
		espbuff_send_printf(conn, "MODE=%d\r\n", wifi_get_opmode()); // get mode

		struct ip_info sta_ip, ap_ip;
		wifi_get_ip_info(SOFTAP_IF, &ap_ip);
		wifi_get_ip_info(STATION_IF, &sta_ip);
		char tmp[64];
		if(sta_ip.ip.addr == 0){
		    espbuff_send_string(conn, "SensorX is not connected to AP\r\n");
		    //return;
		}else{
			os_sprintf(tmp, "\"%d.%d.%d.%d\"", IP2STR(&sta_ip.ip));
			espbuff_send_printf(conn, "STA IP=%s\r\n", tmp);  // get sta ip
			struct station_config sta_conf;
			wifi_station_get_config(&sta_conf);
			espbuff_send_printf(conn, "STA SSID=%s\r\n", sta_conf.ssid); // get sta ssid
		}
		struct softap_config ap_conf;
		wifi_softap_get_config(&ap_conf);
		os_sprintf(tmp, "\"%d.%d.%d.%d\"", IP2STR(&ap_ip.ip));
		espbuff_send_printf(conn, "AP IP=%s\r\n", tmp);  // get ap ip

		flash_param_t *flash_param  = flash_param_get();
		UartBitsNum4Char data_bits = GETUART_DATABITS(flash_param->uartconf0);
		UartParityMode parity = GETUART_PARITYMODE(flash_param->uartconf0);
		UartStopBitsNum stop_bits = GETUART_STOPBITS(flash_param->uartconf0);
		const char *stopbits[4] = { "?", "1", "1.5", "2" };
		const char *paritymodes[4] = { "E", "O", "N", "?" };
		espbuff_send_printf(conn, "REMOTE IP ADDRESS=%d.%d.%d.%d, PORT=%d\r\n", IP2STR(&(flash_param->remote_ip)), flash_param->remote_port); // get remote ip and port
		espbuff_send_printf(conn, "Seiral Info=%d %d %s %s\r\n", flash_param->baud, data_bits + 5, paritymodes[parity], stopbits[stop_bits]); // get baud rate
		espbuff_send_string(conn, MSG_OK);
	}
	else{
		espbuff_send_string(conn, "+++AT STATUS Invalid arguments.\r\n");
		return;
	}
}


int check_ip_validation(const char *str){
	if(str == NULL)
		return -1;

	int len = os_strlen(str);
	char s[20];
	os_strcpy(s, str);
	s[len] = '\0';
	int i,j, cnt = 0, n = 0;
	for(i = 0, j = 0; i < len+1; i++){
		if(s[i] == '.' || s[i] == '\0'){
			if(s[i] == '.')
				cnt++;   // cnt number of '.'

			if(i == j) // first or last char is '.'
				return -1;

			if(s[j] == '0' && (i-j) > 1) // first number char is '0', like 012.01.03.4. But 0.xx.0.xx is valid
				return -1;
			for(; j < i; j++)
				n = n * 10 + (s[j] - '0');

			if(n > 255)
				return -1;
			j++;  // j jump to the next char before i
			n = 0;
		}
		else if(s[i] < '0' || s[i] > '9')
			return -1;
	}
	if(cnt != 3)
		return -1;

	return 0;
}

void config_cmd_sta_hostname(server_conn_data *conn, uint8_t argc, char *argv[]){
	if(argc == 0){
		char sta_hostname[32];
		os_strcpy(sta_hostname, wifi_station_get_hostname());
		espbuff_send_printf(conn, "STATION HOSTNAME=%s\r\n"MSG_OK, sta_hostname);
	}
	else if(argc > 1){
		espbuff_send_string(conn, "+++AT STAIP Invalid IP Address.\t\n");
		espbuff_send_string(conn, MSG_ERROR);	
		return;
	}
	else{ // argc == 1
		if(os_strlen(argv[1]) > 32){
			espbuff_send_string(conn, "STATION HOSTNAME TOO LONG.\r\n"MSG_ERROR);
			return;
		}
		else{
			if(wifi_station_set_hostname(argv[1]))
				espbuff_send_printf(conn, "SET STATION HOSTNAME=%s\r\n"MSG_OK, argv[1]);
			else
				espbuff_send_string(conn, MSG_ERROR);
		}

	}
}

void config_cmd_sta_ip(server_conn_data *conn, uint8_t argc, char *argv[]){
	struct ip_info sta_ip;
	char tmp[64];
	wifi_get_ip_info(STATION_IF, &sta_ip);
	
	if(argc == 0){	
		os_sprintf(tmp, "\"%d.%d.%d.%d\"", IP2STR(&sta_ip.ip));
		espbuff_send_printf(conn, "STATION IP ADDRESS=%s\r\n"MSG_OK, tmp);
	}
	else if(argc == 1){
		if(check_ip_validation(argv[1]) < 0){
			espbuff_send_string(conn, "+++AT STAIP Invalid IP Address.\t\n");
			espbuff_send_string(conn, MSG_ERROR);
			return;
		}
		else{
			wifi_station_dhcpc_stop();
			sta_ip.ip.addr = ipaddr_addr(argv[1]);
			if(wifi_set_ip_info(STATION_IF, &sta_ip))
				espbuff_send_string(conn, MSG_OK);
			else{
				espbuff_send_string(conn, MSG_ERROR);
				wifi_station_dhcpc_start();
			}
		}
	}
	else{
		espbuff_send_string(conn, "+++AT STAIP Invalid arguments");
		espbuff_send_string(conn, MSG_ERROR);
	}
}



void config_cmd_remote_server(server_conn_data *conn, uint8_t argc, char *argv[]){
	flash_param_t *flash_param  = flash_param_get();
	if(argc == 0)
		espbuff_send_printf(conn, "REMOTE IP ADDRESS=%d.%d.%d.%d, PORT=%d, LOCAL_PORT=%d\r\n"MSG_OK, IP2STR(&(flash_param->remote_ip)), flash_param->remote_port, espconn_port());

	if(argc > 2){
		espbuff_send_string(conn, MSG_ERROR);
		espbuff_send_string(conn, "+++AT REMOTE Invalid arguments.\t\n");
		return;
	}
	else{
		if(check_ip_validation(argv[1]) < 0){
			espbuff_send_string(conn, MSG_ERROR);
			espbuff_send_string(conn, "+++AT REMOTE Invalid IP Address.\t\n");
			return;
		}

		if(argc == 1){
			uint32 addr = ipaddr_addr(argv[1]);
			flash_param->remote_ip = addr;

			if (flash_param_set())
				espbuff_send_printf(conn, "REMOTE IP ADDRESS=%s\r\n"MSG_OK, argv[1]);
			else
				espbuff_send_string(conn, "SET REMOTE IP "MSG_ERROR);
		}	

		if(argc == 2){
			uint32_t port = atoi(argv[2]);
			if(port > 65535 || port == 0){
				espbuff_send_printf(conn, "+++AT REMOTE Invalid remote port: %s\r\n"MSG_ERROR, argv[2]);
				return;
			}
			else{
				flash_param->remote_port = port;
				if (flash_param_set())
					espbuff_send_printf(conn, "REMOTE IP ADDRESS=%s, PORT=%d\r\n"MSG_OK, argv[1], argv[2]);
				else
					espbuff_send_string(conn, "SET REMOTE IP "MSG_ERROR);
			}
		}
		os_delay_us(10000);
		system_restart();
	} 

}

void get_ap_mac_addr(char *mac_addr){
	if(mac_addr == NULL)
		return;

	const char mac_char[] = "0123456789ABCDEF";
	char ap_mac_str[12];
	uint8 ap_mac[6];
	wifi_get_macaddr(SOFTAP_IF, ap_mac);

	uint8_t i;
	for(i = 0; i < 12; i++)  // convert mac to string
		if(i%2 == 0)
			ap_mac_str[i] = mac_char[ ap_mac[i/2] / 16 ];  // convert first 0x bit    
		else
			ap_mac_str[i] = mac_char[ ap_mac[i/2] % 16 ];	// convert second 0x bit
	ap_mac_str[i] = '\0';
	os_strcpy(mac_addr, ap_mac_str);
}

void config_default_sta_hostname(bool restore){
	char sta_hostname[32];
	os_strcpy(sta_hostname, wifi_station_get_hostname());
	if(!os_strncmp(sta_hostname, "ESP", 3) || !os_strncmp(sta_hostname, "esp", 3) || restore){
		char ap_mac_str[32];
		get_ap_mac_addr(ap_mac_str);
		os_strcpy(sta_hostname, DEFAULT_NAME_PRE);
		os_strcat(sta_hostname, ap_mac_str);
		wifi_station_set_hostname(sta_hostname);
	}
}

void config_default_ap(bool restore){
	struct softap_config ap_conf;
	os_bzero(&ap_conf, sizeof(struct softap_config));
	wifi_softap_get_config(&ap_conf);

	wifi_set_opmode(STATIONAP_MODE);
	// get self ap mac address string
	char ap_mac_str[32];
	get_ap_mac_addr(ap_mac_str);

	if(!os_strncmp(ap_conf.ssid, "ESP", 3) || !os_strncmp(ap_conf.ssid, "esp", 3) || restore){
		os_bzero(ap_conf.ssid, sizeof(ap_conf.ssid));
		os_strcpy(ap_conf.ssid, DEFAULT_NAME_PRE);
		os_strcat(ap_conf.ssid, ap_mac_str);  // joint ap mac string

		// set ap ssid
		ap_conf.ssid_len = os_strlen(DEFAULT_NAME_PRE) + os_strlen(ap_mac_str); //without set ssid_len, no connection to AP is possible
		os_bzero(ap_conf.password, sizeof(ap_conf.password));
		ap_conf.authmode = 0;
		ap_conf.channel = 1;
		ETS_UART_INTR_DISABLE();
		wifi_softap_set_config(&ap_conf);  // set SensorX_mac ssid
		ETS_UART_INTR_ENABLE();
	}

	// set ap ip to 192.168.8.1
	struct ip_info ap_ip;
	wifi_softap_dhcps_stop();
	IP4_ADDR(&ap_ip.ip, 192, 168, 8, 1);
	IP4_ADDR(&ap_ip.gw, 192, 168, 8, 1);
	IP4_ADDR(&ap_ip.netmask, 255, 255, 255, 0);
	wifi_set_ip_info(SOFTAP_IF, &ap_ip);
	wifi_softap_dhcps_start();
}

void config_default_flash_param(bool restore){
	struct softap_config ap_conf;
	wifi_softap_get_config(&ap_conf);

	if(!os_strncmp(ap_conf.ssid, "ESP", 3) || !os_strncmp(ap_conf.ssid, "esp", 3) || restore){
		flash_param_t *flash_param = flash_param_get();
		flash_param->baud = 115200;
		flash_param->port = 23;
		flash_param->remote_port = 11311;
		flash_param->remote_ip = IPADDR_NONE;
		flash_param->uartconf0 = CALC_UARTMODE(EIGHT_BITS, NONE_BITS, ONE_STOP_BIT);
		flash_param_set();
		os_printf("config default_flash_param: done!\n");
	}
}


void config_cmd_restore(server_conn_data *conn, uint8_t argc, char *argv[]){
	bool restore = true;
	config_default_ap(restore);
	config_default_sta_hostname(restore);
	config_default_flash_param(restore);

	espbuff_send_string(conn, MSG_OK);
	os_delay_us(10000);
	system_restart();
}


void config_sensorx(void){
	bool restore = false;
	config_default_ap(restore);
	config_default_sta_hostname(restore);
	config_default_flash_param(restore);
}

char *string_save(char *str) {
	if(str == NULL)
		return NULL;

	size_t len;
	char *copy;

	len = strlen(str) + 1;
	if (!(copy = (char*)os_malloc((u_int)len)))
		return (NULL);
	os_memcpy(copy, str, len);
	return (copy);
}

char **config_parse_args(char *buf, uint8_t *argc) {
	if(buf == NULL || argc == NULL)
		return NULL;

	const char delim[] = " \t";
	char *save, *tok;
	char **argv = (char **)os_malloc(sizeof(char *) * MAX_ARGS);	// note fixed length
	os_memset(argv, 0, sizeof(char *) * MAX_ARGS);

	*argc = 0;
	for (; *buf == ' ' || *buf == '\t'; ++buf); // absorb leading spaces
	for (tok = strtok_r(buf, delim, &save); tok; tok = strtok_r(NULL, delim, &save)) {
		argv[*argc] = string_save(tok);
		(*argc)++;
		if (*argc == MAX_ARGS) {
			break;
		}
	}
	return argv;
}

void config_parse_args_free(uint8_t argc, char *argv[]) {
	if(argv == NULL)
		return;

	uint8_t i;
	for (i = 0; i <= argc; ++i) {
		if (argv[i])
			os_free(argv[i]);
	}
	os_free(argv);
}

void config_parse(server_conn_data *conn, char *buf, int len) {
	if(conn == NULL || buf == NULL)
		return;

	char *lbuf = (char *)os_malloc(len + 1), **argv;
	uint8_t i, argc;
	// we need a '\0' end of the string
	os_memcpy(lbuf, buf, len);
	lbuf[len] = '\0';

	// remove any CR / LF
	for (i = 0; i < len; ++i)
		if (lbuf[i] == '\n' || lbuf[i] == '\r')
			lbuf[i] = '\0';

	// verify the command prefix
	if (os_strncmp(lbuf, "+++AT", 5) != 0) {
		return;
	}
	// parse out buffer into arguments
	argv = config_parse_args(&lbuf[5], &argc);
	if(argv == NULL)
		return;

	if (argc == 0) {
		espbuff_send_string(conn, MSG_OK);
	} else {
		argc--;	// to mimic C main() argc argv
		for (i = 0; config_commands[i].command; ++i) {
			if (os_strncmp(argv[0], config_commands[i].command, strlen(argv[0])) == 0) {
				config_commands[i].function(conn, argc, argv);
				break;
			}
		}
		if (!config_commands[i].command)
			espbuff_send_string(conn, MSG_INVALID_CMD);
	}
	config_parse_args_free(argc, argv);
	os_free(lbuf);
}

