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
#include "driver/uart.h"

#include "server.h"
#include "config.h"

#define LED4_BLINK_TIME 200   // in ms

static struct espconn server_espconn;
static esp_tcp serverTcp;

// just one connection for each sensorx
static char txbuffer[MAX_TXBUFFER];
server_conn_data server_conn;

os_timer_t connect_timer;

//send all data in conn->txbuffer
//returns result from espconn_send if data in buffer or ESPCONN_OK (0)
//only internal used by server_send, server_send_cb
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
sint8 ICACHE_FLASH_ATTR  server_send_printf(server_conn_data *conn, const char *format, ...) {
	uint16 len;
	va_list al;
	va_start(al, format);
	len = ets_vsnprintf(conn->txbuffer + conn->txbufferlen, MAX_TXBUFFER - conn->txbufferlen - 1, format, al);
	va_end(al);
	if (len <0)  {
		os_printf("server_send_printf: txbuffer full on conn %p\n", conn);
		return len;
	}
	conn->txbufferlen += len;
	if (conn->readytosend)
		return send_txbuffer(conn);
	return ESPCONN_OK;
}


//send string through server_send
sint8 ICACHE_FLASH_ATTR server_send_string(server_conn_data *conn, const char *data) {
	return server_send(conn, data, strlen(data));
}

//use server_send instead of espconn_send
//It solve problem: the next espconn_send must after espconn_send_callback of the pre-packet.
//Add data to the send buffer and send if previous send was completed it call send_txbuffer and  espconn_send
//Returns ESPCONN_OK (0) for success, -128 if buffer is full or error from  espconn_send
sint8 ICACHE_FLASH_ATTR server_send(server_conn_data *conn, const char *data, uint16 len) {
	if (conn->txbufferlen + len > MAX_TXBUFFER) {
		os_printf("server_send: txbuffer full on conn %p\n", conn);
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
	server_conn.conn = (struct espconn*)arg;
	if (server_conn.conn == NULL) 
		return;
	server_conn.readytosend = true;
	send_txbuffer(&server_conn); // send possible new data in txbuffer
}

static void ICACHE_FLASH_ATTR server_recv_cb(void *arg, char *data, unsigned short len) {
	server_conn.conn = (struct espconn*)arg;
	if (server_conn.conn==NULL) 
		return;

	if (len >= 5 && data[0] == '+' && data[1] == '+' && data[2] == '+' && data[3] =='A' && data[4] == 'T') 
		config_parse(&server_conn, data, len);
	else 
		uart0_tx_buffer(data, len);  // DO NOT CHANGE if want to uploader program wirelessly
}

static void ICACHE_FLASH_ATTR server_reconn_cb(void *arg, sint8 err) {
	// if (server_conn.conn != (struct espconn*)arg) 
	// 	return;
	//ToDo: figure something out.
}

static void ICACHE_FLASH_ATTR server_disconn_cb(void *arg) {
	//Just look at all the sockets and kill the slot if needed.
	server_conn.conn = (struct espconn*)arg;
	if (server_conn.conn!=NULL) {
		if (server_conn.conn->state==ESPCONN_NONE || server_conn.conn->state==ESPCONN_CLOSE) 
			server_conn.conn = NULL;
	}

	os_timer_disarm(&connect_timer);  // disable led4 blink
}

void server_connect_welcome(server_conn_data *conn){
	const char wel[] = "\n\n\
/************  Welcome to SensorX  *************/\n\
/****  Usage:                                ***/\n\
/****  +++AT COMMAND [PARAMETERS]            ***/\n\
/****  COMMAND: PORT BAUD REMOTE STA STAIP   ***/\n\
/****           REMOTE RESTORE STATUS GPIO2  ***/\n\
/****           STAHOSTNAME AP RESET         ***/\n\
/******************   ENJOY!   *****************/\n\n\n";

	server_send_string(conn, wel);
}

// led blink if sensorx is connected
bool led4_flag = FALSE;
static void ICACHE_FLASH_ATTR led4_blink(void *arg){
	os_timer_disarm(&connect_timer);
	if(led4_flag){
		led4_flag = FALSE;
		SET_GPIO(BIT4, FALSE);
	}
	else{
		led4_flag = TRUE;
		SET_GPIO(BIT4, TRUE);
	}
	os_timer_arm(&connect_timer, LED4_BLINK_TIME, 0);
}

static void ICACHE_FLASH_ATTR server_connect_cb(void *arg) {
	struct espconn *conn = arg;

	server_conn.conn = conn;
	server_conn.txbufferlen = 0;
	server_conn.readytosend = true;

	espconn_regist_recvcb(conn, server_recv_cb);
	espconn_regist_reconcb(conn, server_reconn_cb);
	espconn_regist_disconcb(conn, server_disconn_cb);
	espconn_regist_sentcb(conn, server_send_cb);

	server_connect_welcome(&server_conn);

	os_timer_disarm(&connect_timer);
    os_timer_setfn(&connect_timer, (os_timer_func_t *)led4_blink, NULL);
    os_timer_arm(&connect_timer, LED4_BLINK_TIME, 0);
}

void ICACHE_FLASH_ATTR server_init(int port) {

	server_conn.conn = NULL;
	os_bzero(txbuffer, sizeof(txbuffer));
	server_conn.txbuffer = txbuffer;
	server_conn.txbufferlen = 0;
	server_conn.readytosend = true;

	server_espconn.type=ESPCONN_TCP;
	server_espconn.state=ESPCONN_NONE;
	serverTcp.local_port=port;
	server_espconn.proto.tcp=&serverTcp;

	espconn_regist_connectcb(&server_espconn, server_connect_cb);
	espconn_accept(&server_espconn);
	espconn_regist_time(&server_espconn, SERVER_TIMEOUT, 0);
}
