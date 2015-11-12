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
 		bool readytosend; //true, if txbuffer can send by espconn_sent
};

void ICACHE_FLASH_ATTR server_init(int port);
sint8  ICACHE_FLASH_ATTR espbuff_send(server_conn_data *conn, const char *data, uint16 len);
sint8  ICACHE_FLASH_ATTR espbuff_send_string(server_conn_data *conn, const char *data);
sint8  ICACHE_FLASH_ATTR espbuff_send_printf(server_conn_data *conn, const char *format, ...);

#endif /* __SERVER_H__ */
