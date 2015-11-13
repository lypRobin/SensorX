/*
 * File	: server.c
 * implementation file of esp8266 server.
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

#include "server.h"
#include "config.h"

static struct espconn server_conn;
static esp_tcp serverTcp;

//Connection pool
static char txbuffer[MAX_CONN][MAX_TXBUFFER];
server_conn_data connect_data[MAX_CONN];



static server_conn_data ICACHE_FLASH_ATTR *server_find_conn_data(void *arg) {
	int i;
	for (i=0; i<MAX_CONN; i++) {
		if (connect_data[i].conn==(struct espconn *)arg) 
			return &connect_data[i];
	}
	//os_printf("FindConnData: Huh? Couldn't find connection for %p\n", arg);
	return NULL; //WtF?
}


//send all data in conn->txbuffer
//returns result from espconn_send if data in buffer or ESPCONN_OK (0)
//only internal used by espbuff_send, server_send_cb
static sint8  ICACHE_FLASH_ATTR send_txbuffer(server_conn_data *conn) {
	sint8 result = ESPCONN_OK;
	if (conn->txbufferlen != 0)	{
		conn->readytosend = false;
		result= espconn_send(conn->conn, (uint8_t*)conn->txbuffer, conn->txbufferlen);
		conn->txbufferlen = 0;	
		if (result != ESPCONN_OK)
			os_printf("send_txbuffer: espconn_send error %d on conn %p\n", result, conn);
	}
	return result;
}

//send formatted output to transmit buffer and call send_txbuffer, if ready (previous espconn_send is completed)
sint8 ICACHE_FLASH_ATTR  espbuff_send_printf(server_conn_data *conn, const char *format, ...) {
	uint16 len;
	va_list al;
	va_start(al, format);
	len = ets_vsnprintf(conn->txbuffer + conn->txbufferlen, MAX_TXBUFFER - conn->txbufferlen - 1, format, al);
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


//send string through espbuff_send
sint8 ICACHE_FLASH_ATTR espbuff_send_string(server_conn_data *conn, const char *data) {
	return espbuff_send(conn, data, strlen(data));
}

//use espbuff_send instead of espconn_send
//It solve problem: the next espconn_send must after espconn_send_callback of the pre-packet.
//Add data to the send buffer and send if previous send was completed it call send_txbuffer and  espconn_send
//Returns ESPCONN_OK (0) for success, -128 if buffer is full or error from  espconn_send
sint8 ICACHE_FLASH_ATTR espbuff_send(server_conn_data *conn, const char *data, uint16 len) {
	if (conn->txbufferlen + len > MAX_TXBUFFER) {
		os_printf("espbuff_send: txbuffer full on conn %p\n", conn);
		return -128;
	}
	os_memcpy(conn->txbuffer + conn->txbufferlen, data, len);
	conn->txbufferlen += len;
	if (conn->readytosend) 
		return send_txbuffer(conn);
	return ESPCONN_OK;
}

//callback after the data are sent
static void ICACHE_FLASH_ATTR server_send_cb(void *arg) {
	server_conn_data *conn = server_find_conn_data(arg);
	//os_printf("Sent callback on conn %p\n", conn);
	if (conn==NULL) return;
	conn->readytosend = true;
	send_txbuffer(conn); // send possible new data in txbuffer
}

static void ICACHE_FLASH_ATTR server_recv_cb(void *arg, char *data, unsigned short len) {
	int x;
	char *p, *e;
	server_conn_data *conn = server_find_conn_data(arg);
	//os_printf("Receive callback on conn %p\n", conn);
	if (conn == NULL) return;

	if (len >= 5 && data[0] == '+' && data[1] == '+' && data[2] == '+' && data[3] =='A' && data[4] == 'T') 
		config_parse(conn, data, len);
	else
		uart0_tx_buffer(data, len);
}

static void ICACHE_FLASH_ATTR server_reconn_cb(void *arg, sint8 err) {
	server_conn_data *conn=server_find_conn_data(arg);
	if (conn==NULL) return;
	//Yeah... No idea what to do here. ToDo: figure something out.
}

static void ICACHE_FLASH_ATTR server_disconn_cb(void *arg) {
	//Just look at all the sockets and kill the slot if needed.
	int i;
	for (i=0; i<MAX_CONN; i++) {
		if (connect_data[i].conn!=NULL) {
			if (connect_data[i].conn->state==ESPCONN_NONE || connect_data[i].conn->state==ESPCONN_CLOSE) {
				connect_data[i].conn=NULL;
			}
		}
	}
}

static void ICACHE_FLASH_ATTR server_connect_cb(void *arg) {
	struct espconn *conn = arg;
	int i;
	//Find empty conndata in pool
	for (i=0; i<MAX_CONN; i++) if (connect_data[i].conn==NULL) break;
	os_printf("Con req, conn=%p, pool slot %d\n", conn, i);

	if (i==MAX_CONN) {
		os_printf("Aiee, conn pool overflow!\n");
		espconn_disconnect(conn);
		return;
	}
	connect_data[i].conn=conn;
	connect_data[i].txbufferlen = 0;
	connect_data[i].readytosend = true;

	espconn_regist_recvcb(conn, server_recv_cb);
	espconn_regist_reconcb(conn, server_reconn_cb);
	espconn_regist_disconcb(conn, server_disconn_cb);
	espconn_regist_sentcb(conn, server_send_cb);
}

void ICACHE_FLASH_ATTR server_init(int port) {
	int i;
	for (i = 0; i < MAX_CONN; i++) {
		connect_data[i].conn = NULL;
		connect_data[i].txbuffer = txbuffer[i];
		connect_data[i].txbufferlen = 0;
		connect_data[i].readytosend = true;
	}
	server_conn.type=ESPCONN_TCP;
	server_conn.state=ESPCONN_NONE;
	serverTcp.local_port=port;
	server_conn.proto.tcp=&serverTcp;

	espconn_regist_connectcb(&server_conn, server_connect_cb);
	espconn_accept(&server_conn);
	espconn_regist_time(&server_conn, SERVER_TIMEOUT, 0);
}
