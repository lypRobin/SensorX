/*
 * File	: flash_param.c
 * Implementation file of esp8266 flash_param. 
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

#include "osapi.h"
#include "user_interface.h"
#include "spi_flash.h"
#include "ets_sys.h"
#include "c_types.h"
#include "driver/uart.h"
#include "flash_param.h"

#define FLASH_PARAM_START_SECTOR 0x3C
#define FLASH_PARAM_SECTOR (FLASH_PARAM_START_SECTOR + 0)
#define FLASH_PARAM_ADDR (SPI_FLASH_SEC_SIZE * FLASH_PARAM_SECTOR)

static int flash_param_loaded = 0;
static flash_param_t flash_param;

void ICACHE_FLASH_ATTR flash_param_read(flash_param_t *flash_param) {
	spi_flash_read(FLASH_PARAM_ADDR, (uint32 *)flash_param, sizeof(flash_param_t));
}

void ICACHE_FLASH_ATTR flash_param_write(flash_param_t *flash_param) {
	ETS_UART_INTR_DISABLE();
	spi_flash_erase_sector(FLASH_PARAM_SECTOR);
	spi_flash_write(FLASH_PARAM_ADDR, (uint32 *) flash_param, sizeof(flash_param_t));
	ETS_UART_INTR_ENABLE();
}

flash_param_t *ICACHE_FLASH_ATTR flash_param_get(void) {
	if (!flash_param_loaded) {
		flash_param_read(&flash_param);
		flash_param_loaded = 1;
	}
	return &flash_param;
}

int ICACHE_FLASH_ATTR flash_param_set(void) {
	flash_param_write(&flash_param);
	flash_param_t tmp;
	flash_param_read(&tmp);
	if (memcmp(&tmp, &flash_param, sizeof(flash_param_t)) != 0) {
		return 0;
	}
	return 1;
}

void ICACHE_FLASH_ATTR flash_param_init_defaults(void) {	
	flash_param_t *flash_param = flash_param_get();
	flash_param->baud = 115200;
	flash_param->port = 23;
	flash_param->remote_port = 11311;
	flash_param->remote_ip = IP_IPADDR_NONE;
	flash_param->uartconf0 = CALC_UARTMODE(EIGHT_BITS, NONE_BITS, ONE_STOP_BIT);
	flash_param_set();
}

flash_param_t* ICACHE_FLASH_ATTR flash_param_init(void) {
	flash_param_t *flash_param = flash_param_get();
	flash_param_init_defaults();

	return flash_param;
}
