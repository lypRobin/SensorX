/*
 * File	: client.c
 * Implementation of esp8266 module as a client.
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
#include <stdarg.h>
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"
#include "driver/uart.h"

#include "client.h"
#include "flash_param.h"

#define CONNECT_TIME  4000    // connect to wifi ap per time, in ms

client_conn_data client_conn;
static char client_txbuffer[MAX_CLIENT_TXBUFFER];
os_timer_t timer;

sint8  ICACHE_FLASH_ATTR client_send(client_conn_data *conn, const char *data, uint16 len){
	if(conn->conn == NULL || conn == NULL)
		return ESPCONN_ARG;
	if(conn->txbufferlen != 0){
		conn->readytosend = false;
		sint8 result = espconn_send(conn->conn, (uint8_t*)data, len);
		conn->txbufferlen = 0;
		if (result != ESPCONN_OK){
			DEBUG_SEND_PRINTF("send_txbuffer: espconn_send error, result %d.\n", result);
			return ESPCONN_ARG;
		}
	}

	return ESPCONN_OK;
}


sint8  ICACHE_FLASH_ATTR client_send_string(client_conn_data *conn, const char *data){
	if(conn == NULL || conn->conn == NULL)
		return ESPCONN_ARG;
	return client_send(conn, data, os_strlen(data));
}

sint8  ICACHE_FLASH_ATTR client_send_printf(client_conn_data *conn, const char *format, ...){
	uint16 len;
	va_list al;
	va_start(al, format);
	len = ets_vsnprintf(conn->txbuffer, MAX_CLIENT_TXBUFFER-1, format, al);
	va_end(al);
	if (len <0)  {
		DEBUG_SEND_STRING("client_send_printf: txbuffer full\n");
		return ESPCONN_ARG;
	}
	conn->txbufferlen = len;
	if (conn->readytosend)
		return client_send(conn, conn->txbuffer, conn->txbufferlen);

	return ESPCONN_OK;
}

// callbacks 
//callback after the data are sent
static void ICACHE_FLASH_ATTR client_send_cb(void *arg) {
	client_conn_data *conn;
	conn->conn = (struct espconn*)arg;
	if (conn->conn == NULL) 
		return;
	conn->readytosend = true;
}

// client receive data callbacks
static void ICACHE_FLASH_ATTR client_recv_cb(void *arg, char *data, unsigned short len) {
	client_conn.conn = (struct espconn*)arg;
	if (len >= 7 && data[0] == '+' && data[1] == '+' && data[2] == '+' && data[3] =='P' && data[4] == 'O' && data[5] == 'S' && data[6] == 'T') 
	{
		DEBUG_SEND_PRINTF("receive server send back data: %s\n", data);
		uart0_send_string("POST=OK\n");
		
		system_os_post(USER_TASK_PRIO_0, SIG_CLIENT_DISCONN, 'd');
	}
		
}

static void ICACHE_FLASH_ATTR client_disconn_cb(void *arg){
	DEBUG_SEND_STRING("disconnect ok\n");
	client_conn.conn = (struct espconn*)arg;
	if(client_conn.conn != NULL)
		if(client_conn.conn->state == ESPCONN_NONE || client_conn.conn->state == ESPCONN_CLOSE){
			os_free(client_conn.conn->proto.tcp);   // free esp_tcp 
			os_free(client_conn.conn);    // free espconn
			client_conn.conn == NULL;
			os_bzero(client_txbuffer, sizeof(client_txbuffer));  // free send buffer
			client_conn.txbuffer = NULL;
			client_conn.txbufferlen = 0;
			client_conn.readytosend = true;
		}
}

static void ICACHE_FLASH_ATTR client_connect_cb(void *arg){
	DEBUG_SEND_STRING("Entry in connect cb\n");
	struct espconn *conn = (struct espconn *)arg;
	client_conn.conn = conn;
	espconn_regist_recvcb(conn, client_recv_cb);
	espconn_regist_disconcb(conn, client_disconn_cb);
	espconn_regist_sentcb(conn, client_send_cb);

	int cnt = 0;
	int ret;
	for(; cnt < 3; cnt++){
		ret = client_send(&client_conn, (const char *)client_conn.txbuffer, client_conn.txbufferlen);
		if(ret != ESPCONN_OK){
			uart0_send_string("+++POST=ERROR\n");
			os_delay_us(10000);
		}
		else
			break;
	}
}

static int connect_time = 0;
static void ICACHE_FLASH_ATTR client_reconn_cb(void *arg, sint8 err){
	client_conn.conn = (struct espconn *)arg;
	DEBUG_SEND_STRING("entry in reconnect\n");
	if(connect_time > 2){
		os_timer_disarm(&timer);
		connect_time = 0;
		return;
	}

	os_timer_arm(&timer, CONNECT_TIME, 0);
}

static void ICACHE_FLASH_ATTR client_connected_check(void *arg){
	DEBUG_SEND_PRINTF("client connect %d times.\n", connect_time);
	os_timer_disarm(&timer);
	
	int status = wifi_station_get_connect_status();
	if(status == STATION_GOT_IP){
		connect_time = 0;
		flash_param_t *flash_param = flash_param_get();
		struct espconn *client_espconn = (struct espconn *)os_zalloc(sizeof(struct espconn));
		if(client_espconn == NULL){
			uart0_send_string("+++POST=ERROR\n");
			DEBUG_SEND_STRING("Alloc failed.\n");
			return;
		}
		client_conn.conn = client_espconn;
		client_conn.txbuffer = client_txbuffer;
		client_conn.txbufferlen = os_strlen(client_txbuffer);
		client_conn.conn->type = ESPCONN_TCP;
		client_conn.conn->state = ESPCONN_NONE;

		client_conn.conn->proto.tcp = (esp_tcp*)os_zalloc(sizeof(esp_tcp));
		client_conn.conn->proto.tcp->remote_port = 11311;//(int)flash_param->remote_port;
		client_conn.conn->proto.tcp->local_port = espconn_port();

		uint32_t ip = flash_param->remote_ip;
		os_memcpy(client_conn.conn->proto.tcp->remote_ip, &ip, 4);
		client_conn.readytosend = true;

		espconn_regist_connectcb(client_conn.conn, client_connect_cb);
		espconn_regist_reconcb(client_conn.conn, client_reconn_cb);
		espconn_regist_time(client_conn.conn, CLIENT_TIMEOUT, 0);

		int ret = espconn_connect(client_conn.conn);
		if(!ret){	
			DEBUG_SEND_STRING("Client connect OK.\n");
		}
		else{
			DEBUG_SEND_STRING("Client connect failed.\n");

			switch(ret){
				case ESPCONN_RTE: 
					DEBUG_SEND_STRING("Routing problem.\n");
					break;
				case ESPCONN_MEM:
					DEBUG_SEND_STRING("Out of Memery.\n");
					break;
				case ESPCONN_ISCONN: 
					DEBUG_SEND_STRING("Already connected.\n");
					break;
				case ESPCONN_ARG:
					DEBUG_SEND_STRING("Cannot find TCP connection.\n");
					break;
				default:
					DEBUG_SEND_STRING("Unknown error.\n");
					break;
			}
		}

		return;
	}
	else{  // just print wifi status 
		switch(status){
			case STATION_IDLE:
				DEBUG_SEND_STRING("station idle\n");
				break;
			case STATION_CONNECTING:
				DEBUG_SEND_STRING("station connecting\n");
				break;
			case STATION_WRONG_PASSWORD:
				DEBUG_SEND_STRING("station wrong password\n");
				break;
			case STATION_NO_AP_FOUND:
				DEBUG_SEND_STRING("station no ap found\n");
				break;
			case STATION_CONNECT_FAIL:
				DEBUG_SEND_STRING("station connect fail\n");
				break;
			default:
				DEBUG_SEND_STRING("unknown status");
				break;
		}
	}
	
	connect_time++;
	// try 3 times connection
	if(connect_time > 2){
		uart0_send_string("+++POST=ERROR\n");
		wifi_station_disconnect();
		connect_time = 0;
		return;
	}
	wifi_station_connect();
	os_timer_arm(&timer, CONNECT_TIME, 0);

}

void client_connect(char *data){
	if (data == NULL)
		return;
	os_strncpy(client_txbuffer, data, os_strlen(data));

	wifi_station_disconnect();
	struct station_config sta_conf;
	wifi_station_get_config(&sta_conf);
	sta_conf.bssid_set = 0;
	wifi_station_set_config(&sta_conf);
	wifi_station_connect();

	os_timer_disarm(&timer);
    os_timer_setfn(&timer, (os_timer_func_t *)client_connected_check, NULL);
    os_timer_arm(&timer, CONNECT_TIME, 0);
}

// void client_config(char *addr, uint32_t port){
// 	if(addr == NULL || port > 65535 || port < 0)
// 		return;

// 	flash_param_t *flash_param = flash_param_get();
// 	if(flash_param->remote_ip == ipaddr_addr(addr) && flash_param->remote_ip != IPADDR_NONE && flash_param->remote_port == port) // same
// 		return;
// 	else{
// 		flash_param->remote_ip = ipaddr_addr("192.168.1.102");
// 		flash_param->remote_port = 11311;//port;
// 		flash_param_set();
// 	}
// }


void ICACHE_FLASH_ATTR client_init() {
	// client_config("192.168.1.102", 11311);
	uart0_send_string("\n\n\n\n");

	client_conn.conn = NULL;
	client_conn.txbuffer = NULL;
	os_bzero(client_txbuffer, sizeof(client_txbuffer));
	client_conn.txbufferlen = 0;
	client_conn.readytosend = true;

	struct station_config sta_conf;
	wifi_station_get_config(&sta_conf);
	sta_conf.bssid_set = 0;
	wifi_station_set_config(&sta_conf);

}

