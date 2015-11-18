/*
 * File	: client.h
 * Head of esp8266 module as a client.
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

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <ip_addr.h>
#include <c_types.h>
#include <espconn.h>

#define CLIENT_TIMEOUT 28799

//Max send buffer len
#define MAX_CLIENT_TXBUFFER 1024

typedef struct client_conn_data client_conn_data;

struct client_conn_data {
        struct espconn *conn;
		char *txbuffer; //the buffer for the data to send
		uint16  txbufferlen; //the length  of data in txbuffer
 		bool readytosend; //true, if txbuffer can send by espconn_send
};

void ICACHE_FLASH_ATTR client_init();
void client_config(char *addr, uint32_t port);
sint8  ICACHE_FLASH_ATTR espbuff_client_send(client_conn_data *conn, const char *data, uint16 len);
sint8  ICACHE_FLASH_ATTR espbuff_client_send_string(client_conn_data *conn, const char *data);
sint8  ICACHE_FLASH_ATTR espbuff_client_send_printf(client_conn_data *conn, const char *format, ...);


#endif /* __CLIENT_H__ */