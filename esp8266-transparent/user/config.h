/*
 * File	: config.c
 * Head file of esp8266 configuerations including +++AT commands. 
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <gpio.h>

#define DEFAULT_NAME_PRE	  "SensorX_"

#define MSG_OK				"OK\r\n"
#define MSG_ERROR			"ERROR\r\n"
#define MSG_INVALID_CMD		"UNKNOWN COMMAND\r\n"

#define MAX_ARGS		12
#define MSG_BUF_LEN		128

#define HIGH  1
#define LOW   0
#define SET_GPIO(BIT, STATE) ( STATE ? gpio_output_set(BIT, 0, BIT, 0) : gpio_output_set(0, BIT, BIT, 0) )

typedef struct config_cmds {
	char *command;
	void(*function)(server_conn_data *conn, uint8_t argc, char *argv[]);
} config_commands_t;


void config_parse(server_conn_data *conn, char *buf, int len);
void config_sensorx(void);
void config_gpio(void);

#endif /* __CONFIG_H__ */
