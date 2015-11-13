/*
 * File	: flash_param.h
 * Head file of esp8266 flash_param. Some pamameters stored in flash.
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

#ifndef __FLASH_PARAM_H__
#define __FLASH_PARAM_H__

#include "ip_addr.h"

typedef struct flash_param{
	uint32_t baud; 
	uint16_t port;
	uint16_t remote_port;
	uint32_t remote_ip;
	uint8_t uartconf0; // UART_CONF0 register register : stop_bit_num 	 [5:4], bit_num [3:2] ,parity_en [1],parity	[0] 
	char padding[1]; // set array index so that the flash area is readable as data with aligned 4-byte reads.
} flash_param_t; 

flash_param_t *flash_param_get(void);
int flash_param_set(void);

#endif /* __FLASH_PARAM_H__ */
