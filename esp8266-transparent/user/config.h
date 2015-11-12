#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <gpio.h>

#define DEFAULT_AP_SSID_PRE	  "SensorX_"

#define MSG_OK				"OK\r\n"
#define MSG_ERROR			"ERROR\r\n"
#define MSG_INVALID_CMD		"UNKNOWN COMMAND\r\n"

#define MAX_ARGS		12
#define MSG_BUF_LEN		128


typedef struct config_cmds {
	char *command;
	void(*function)(server_conn_data *conn, uint8_t argc, char *argv[]);
} config_commands_t;


void config_parse(server_conn_data *conn, char *buf, int len);
void config_ap(void);
void config_gpio(void);

#endif /* __CONFIG_H__ */
