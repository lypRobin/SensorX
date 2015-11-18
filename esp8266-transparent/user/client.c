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

#include "client.h"
#include "flash_param.h"

// static struct espconn client_espconn;
// static esp_tcp client_tcp;
client_conn_data client_conn;
static char client_txbuffer[MAX_CLIENT_TXBUFFER];

static sint8  ICACHE_FLASH_ATTR send_txbuffer(client_conn_data *conn) {
	if(conn->conn == NULL)
		return ESPCONN_ARG;
	if(conn->txbufferlen != 0){
		conn->readytosend = false;
		sint8 result = espconn_send(conn->conn, (uint8_t*)conn->txbuffer, conn->txbufferlen);
		conn->txbufferlen = 0;	
		if (result != ESPCONN_OK)
			os_printf("send_txbuffer: espconn_send error %d on conn %p\n", result, conn);
	}

	return ESPCONN_OK;
}

sint8  ICACHE_FLASH_ATTR espbuff_client_send(client_conn_data *conn, const char *data, uint16 len){
	if(conn == NULL)
		return ESPCONN_ARG;

	if(conn->txbufferlen + len > MAX_CLIENT_TXBUFFER)
		return ESPCONN_ARG;

	os_memcpy(conn->txbuffer + conn->txbufferlen, data, len);
	conn->txbufferlen += len;
	if (conn->readytosend) 
		return send_txbuffer(conn);
		
	return ESPCONN_OK;
}


sint8  ICACHE_FLASH_ATTR espbuff_client_send_string(client_conn_data *conn, const char *data){
	if(conn == NULL)
		return ESPCONN_ARG;
	return espbuff_send(conn, data, os_strlen(data));
}

sint8  ICACHE_FLASH_ATTR espbuff_client_send_printf(client_conn_data *conn, const char *format, ...){
	uint16 len;
	va_list al;
	va_start(al, format);
	len = ets_vsnprintf(conn->txbuffer + conn->txbufferlen, MAX_CLIENT_TXBUFFER - conn->txbufferlen - 1, format, al);
	va_end(al);
	if (len <0)  {
		os_printf("espbuff_send_printf: txbuffer full on conn %p\n", conn);
		return len;
	}
	conn->txbufferlen += len;
	if (conn->readytosend)
		return send_txbuffer(conn);
	return ESPCONN_OK;
}

// callbacks 
//callback after the data are sent
static void ICACHE_FLASH_ATTR client_send_cb(void *arg) {
	client_conn_data *conn;
	conn->conn = arg;
	if (conn->conn == NULL) 
		return;
	conn->readytosend = true;
}

// client receive data callbacks
static void ICACHE_FLASH_ATTR client_recv_cb(void *arg, char *data, unsigned short len) {
	if (len >= 6 && data[0] == '+' && data[1] == '+' && data[2] == 'P' && data[3] =='O' && data[4] == 'S' && data[5] == 'T') 
	// 	config_parse(conn, data, len);
	// else
		uart0_tx_buffer(data, len);
}

static void ICACHE_FLASH_ATTR client_disconn_cb(void *arg){
	if(client_conn.conn != NULL)
		if(client_conn.conn->state == ESPCONN_NONE || client_conn.conn->state == ESPCONN_CLOSE){
			client_conn.conn == NULL;
			client_conn.txbuffer = NULL;
			client_conn.txbufferlen = 0;
			client_conn.readytosend = true;
		}
}

static void ICACHE_FLASH_ATTR client_connect_cb(void *arg){
	uart0_sendStr("Entry in connect cb\n");
	struct espconn *conn = (struct espconn *)arg;
	client_conn.conn = conn;
	espconn_regist_recvcb(conn, client_recv_cb);
	espconn_regist_disconcb(conn, client_disconn_cb);
	espconn_regist_sentcb(conn, client_send_cb);
}


static void ICACHE_FLASH_ATTR client_reconn_cb(void *arg, sint8 err){
	client_conn.conn = (struct espconn *)arg;
	uart0_sendStr("entry in reconnect\n");
}



void client_config(char *addr, uint32_t port){
	if(addr == NULL || port > 65535 || port < 0)
		return;

	flash_param_t *flash_param = flash_param_get();
	if(flash_param->remote_ip == ipaddr_addr(addr) && flash_param->remote_ip != IPADDR_NONE && flash_param->remote_port == port) // same
		return;
	else{
		flash_param->remote_ip = ipaddr_addr("192.168.1.102");
		flash_param->remote_port = 11311;//port;
		flash_param_set();
	}
}


void ICACHE_FLASH_ATTR client_init() {
	flash_param_t *flash_param = flash_param_get();

	// client_config("192.168.1.102", 11311);
	uart0_sendStr("\n\n\n\n");

	client_conn.conn = NULL;
	client_conn.txbuffer = client_txbuffer;
	client_conn.txbufferlen = 0;
	client_conn.readytosend = true;

	struct espconn *client_espconn = (struct espconn *)os_zalloc(sizeof(struct espconn));
	if(client_espconn == NULL)
		uart0_sendStr("Alloc failed.\n");
	client_espconn->type = ESPCONN_TCP;
	client_espconn->state = ESPCONN_NONE;

	client_espconn->proto.tcp = (esp_tcp*)os_zalloc(sizeof(esp_tcp));
	client_espconn->proto.tcp->remote_port = 11311;//(int)flash_param->remote_port;
	client_espconn->proto.tcp->local_port = espconn_port();

	uint32_t ip = flash_param->remote_ip;
	// client_tcp->remote_ip[0] = (uint8)ip4_addr1_16(&ip);
	// client_tcp->remote_ip[1] = (uint8)ip4_addr2_16(&ip);
	// client_tcp->remote_ip[2] = (uint8)ip4_addr3_16(&ip);
	// client_tcp->remote_ip[3] = (uint8)ip4_addr4_16(&ip);
	os_memcpy(client_espconn->proto.tcp->remote_ip, &ip, 4);
	
	char tmp[32];
	os_sprintf(tmp, "%d.%d.%d.%d\r\n", IP2STR(&ip));
	uart0_sendStr(tmp);

	espconn_regist_connectcb(client_espconn, client_connect_cb);
	espconn_regist_reconcb(client_espconn, client_reconn_cb);
	espconn_regist_time(client_espconn, CLIENT_TIMEOUT, 0);


	int ret = espconn_connect(client_espconn);
	if(!ret){	
		uart0_sendStr("Client connect OK.\n");
	}
	else
		uart0_sendStr("Client connect failed.\n");

	switch(ret){
		case ESPCONN_RTE: 
			uart0_sendStr("Routing problem.\n");
			break;
		case ESPCONN_MEM:
			uart0_sendStr("Out of Memery.\n");
			break;
		case ESPCONN_ISCONN: 
			uart0_sendStr("Already connected.\n");
			break;
		case ESPCONN_ARG:
			uart0_sendStr("Cannot find TCP connection.\n");
			break;
		default:
			uart0_sendStr("Unknown error.\n");
			break;
	}

	// os_free(client_espconn);


}

