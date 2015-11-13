/*
 * File	: server.h
 * Head of esp8266 module as a server.
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

#ifndef __SERVER_H__
#define __SERVER_H__

#include <ip_addr.h>
#include <c_types.h>
#include <espconn.h>

#define MAX_CONN 5
#define SERVER_TIMEOUT 28799

//Max send buffer len
#define MAX_TXBUFFER 1024

typedef struct server_conn_data server_conn_data;

struct server_conn_data {
        struct espconn *conn;
		char *txbuffer; //the buffer for the data to send
		uint16  txbufferlen; //the length  of data in txbuffer
 		bool readytosend; //true, if txbuffer can send by espconn_send
};

void ICACHE_FLASH_ATTR server_init(int port);
sint8  ICACHE_FLASH_ATTR espbuff_send(server_conn_data *conn, const char *data, uint16 len);
sint8  ICACHE_FLASH_ATTR espbuff_send_string(server_conn_data *conn, const char *data);
sint8  ICACHE_FLASH_ATTR espbuff_send_printf(server_conn_data *conn, const char *format, ...);

#endif /* __SERVER_H__ */
