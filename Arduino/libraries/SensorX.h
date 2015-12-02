#ifndef _SENSORX_H_
#define _SENSORX_H_

#include <aJSON.h>

#define SENSOR_NULL  	0
#define SENSOR_TEMP		1
#define SENSOR_HUMI		2
#define SENSOR_DUST		3
#define SENSOR_LIGHT	4
#define SENSOR_HCHO		5
#define SENSOR_CO2		6
#define SENSOR_GAS		7
#define SENSOR_HUMAN	8
#define SENSOR_FIRE		9
#define SENSOR_LED		10

#define CMD_ERROR		-1
#define CMD_PARSE_ERROR	-2
#define CMD_NULL		0
#define CMD_SEND_DATA	1
#define CMD_SET_DATA	2
#define CMD_ADD_SENSOR	3
#define CMD_DELET_SENSOR	4
#define CMD_POST_OK		100

#define PARSR_GET		0
#define PARSE_POST		1

#define MAX_RX_BUFFER		256
#define MAX_TX_BUFFER		256

#define MAX_IDS		16

typedef struct SensorItem{
	uint8_t pin;
	uint8_t type;
	union {
		char valuebool; 
		int valueint; 
		double valuefloat;
	};
	SensorItem *next;
}SensorItem;

typedef struct CmdInfo{
	uint8_t type;  // cmd type
	uint8_t id;    // cmd id
}CmdInfo;

class SensorX{
public:
virtual SensorX(Serial *com);
virtual ~SensorX();

virtual	int readFromHost(char *inbuf_cmd);
virtual	int writeToHost();
virtual	int comAvailable();
	int newSensor(uint8_t pin, uint8_t type, int valueint);
	int newSensor(uint8_t pin, uint8_t type, double valuefloat);
	int newSensor(uint8_t pin, uint8_t type, char valuebool);
	bool addSensor(SensorItem *sensor);
	bool deletSensor(uint8_t type, uint8_t pin);
	bool deletAllSensors();
	int processCmd(char *str);
	uint8_t getSensorNum(){return _sensorNum};
	int saveSensorData(uint8_t pin, uint8_t type, int valueint);
	int saveSensorData(uint8_t pin, uint8_t type, double valuefloat);
	int saveSensorData(uint8_t pin, uint8_t type, char valuebool);
	bool addPost(uint8_t id);
	bool deletPost(uint8_t id);

private:
	int parseCmd(char *str);
	SensorItem *_sQueue;
	uint8_t _sensorNum;
	Serial *_com;
	uint8_t _post[MAX_IDS];
	aJsonObject *_aJson;
};

#endif