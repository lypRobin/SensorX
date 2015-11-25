/*
 * File	: user_main.c
 * main file of the esp8266 transparent module.
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
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "driver/uart.h"
#include "task.h"

#include "server.h"
#include "client.h"
#include "config.h"
#include "flash_param.h"

os_event_t	recv_task_queue[RECEIVE_TASK_QUEUE_LEN];
extern  server_conn_data server_conn[MAX_CONN];
extern client_conn_data client_conn;

#define MAX_UARTBUFFER (MAX_TXBUFFER/4)
static uint8 uartbuffer[MAX_UARTBUFFER];

static void ICACHE_FLASH_ATTR recv_task(os_event_t *events)
{
	uint8_t i;	 
	int ret;

	switch(events->sig){
		case SIG_CLIENT_DISCONN:
			// client_conn_data* conn = client_get_conn();
			ret = espconn_disconnect(client_conn.conn);
			if(!ret)
				DEBUG_SEND_STRING("disconnect server OK\n");
			else
				DEBUG_SEND_STRING("disconnect server failed\n");
			break;
		default:
			DEBUG_SEND_STRING("UNKNOWN SYSTEM EVENTS\n");
			break;
	}

	while (READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S))
	{
		WRITE_PERI_REG(0X60000914, 0x73); //WTD
		uint16 length = 0;
		while ((READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S)) && (length<MAX_UARTBUFFER))
			uartbuffer[length++] = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;


		if(length >= 4 && uartbuffer[0] == 'P' && uartbuffer[1] == 'O' && uartbuffer[2] == 'S' && uartbuffer[3] == 'T'){
			client_connect(uartbuffer);
		}
		
		for (i = 0; i < MAX_CONN; ++i)
			if (server_conn[i].conn) 
				espbuff_send(&server_conn[i], uartbuffer, length);		
	}

	if(UART_RXFIFO_FULL_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST))
	{
		WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);
	}
	else if(UART_RXFIFO_TOUT_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_TOUT_INT_ST))
	{
		WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_TOUT_INT_CLR);
	}
	ETS_UART_INTR_ENABLE();
}


// UartDev is defined and initialized in rom code.
extern UartDevice    UartDev;

void user_init(void)
{
	// set flash and uart
	config_sensorx();
	flash_param_t *flash_param;
	flash_param = flash_param_get();
	UartDev.data_bits = EIGHT_BITS;
	UartDev.parity = NONE_BITS;
	UartDev.stop_bits = ONE_STOP_BIT;
	uart_init(flash_param->baud, BIT_RATE_115200);

	client_init();
	server_init(23);
	config_gpio();
	

	uint8_t i = 0;
	for (i = 0; i < 16; ++i)
		uart0_send_string("\r\n");

	system_os_task(recv_task, RECEIVE_TASK_PRIO, recv_task_queue, RECEIVE_TASK_QUEUE_LEN);
}
