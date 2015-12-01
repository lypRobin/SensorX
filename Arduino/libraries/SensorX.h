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

#define MAX_POST	16

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


class SensorX{
public:
virtual SensorX(Serial *com);
virtual ~SensorX();

virtual	int readFromHost();
virtual	int writeToHost();
virtual	int comAvailable();
virtual	char* getTime();
	int newSensor();
	int addSensor(SensorItem *sensor);
	int deletSensor(uint8_t type, uint8_t id);
	int deletAllSensors();
	uint8_t getSensorNum(){return _sensorNum};

private:
virtual	int setTime(char *time);
	bool deletPost(int id);
	int parseCmd(char *str, int opt);
	bool addPost();
	SensorItem *_sQueue;
	uint8_t _sensorNum;
	Serial *_com;
	uint8_t _post[MAX_POST];
	char _time[18];
};

#endif