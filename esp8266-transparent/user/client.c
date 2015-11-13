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

#include "client.h"
#include "flash_param.h"

static clientEspconn client_conn;
static esp_tcp client_tcp;

sint8  ICACHE_FLASH_ATTR espbuff_client_send(clientEspconn *conn, const char *data, uint16 len){

}


sint8  ICACHE_FLASH_ATTR espbuff_client_send_string(clientEspconn *conn, const char *data){


}

sint8  ICACHE_FLASH_ATTR espbuff_client_send_printf(clientEspconn *conn, const char *format, ...){


}

static void ICACHE_FLASH_ATTR client_connect_cb(void *arg)

void client_config(char *addr, int port){
	if(addr == NULL || port > 65535 || port < 0)
		return;

	flash_param_t *flash_param = flash_param_get();
	if(flash_param->remote_ip == ipaddr_addr(addr) && flash_param->remote_ip != IP_IPADDR_NONE && flash_param->remote_port == port) // same
		return;
	else{
		flash_param->remote_ip = ipaddr_addr(addr);
		flash_param->remote_port = port;
		flash_param_set();
	}

}

void ICACHE_FLASH_ATTR client_init() {
	flash_param_t *flash_param = flash_param_get();
	uint8 ip[4];


	client_conn.type = ESPCONN_TCP;
	client_conn.state = ESPCONN_NONE;
	client_tcp.remote_port = (int)flash_param->remote_port;
	client_tcp.remote_ip[0] = (uint8)ip4_addr1_16(flash_param->remote_ip);
	client_tcp.remote_ip[1] = (uint8)ip4_addr2_16(flash_param->remote_ip);
	client_tcp.remote_ip[2] = (uint8)ip4_addr3_16(flash_param->remote_ip);
	client_tcp.remote_ip[3] = (uint8)ip4_addr4_16(flash_param->remote_ip);
	client_conn.proto.tcp=&client_tcp;

	espconn_regist_connectcb(&client_conn, client_connect_cb);
	espconn_regist_time(&client_conn, SERVER_TIMEOUT, 0);
}

