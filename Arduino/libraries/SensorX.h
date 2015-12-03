#ifndef _SENSORX_H_
#define _SENSORX_H_

#include <aJSON.h>

#define SENSOR_NULL  	0
// input sensor
#define SENSOR_TEMP		1
#define SENSOR_HUMI		2
#define SENSOR_DUST		3
#define SENSOR_LIGHT	4
#define SENSOR_HCHO		5
#define SENSOR_CO2		6
#define SENSOR_GAS		7
#define SENSOR_HUMAN	8
#define SENSOR_FIRE		9
// output sensor
#define SENSOR_LED		100
#define SENSOR_PUMP		101

#define CMD_ERROR		-1
#define CMD_PARSE_ERROR	-2
#define CMD_NULL		0
#define CMD_SEND_DATA	1
#define CMD_SET_DATA	2
#define CMD_ADD_SENSOR	3
#define CMD_DELETE_SENSOR	4
#define CMD_OK		100

#define VALUE_NULL		0
#define VALUE_BOOL		1	
#define VALUE_INT		2
#define VALUE_FLOAT		3
#define VALUE_STRING	4

#define MAX_RX_BUFFER		256
#define MAX_TX_BUFFER		256

#define MAX_IDS		16

class SensorItem{
public:
	SensorItem(uint8_t pin, uint8_t _type, uint8_t _io_type, uint8_t _val_type);

	virtual setup() {}
	virtual read() {}
	virtual write() {}

	double getValue() { return _value.vfloat; }
	void setValue (int value) {  _value.vint = value; }
	void setValue(char value) { _value.vbool = value; }
	void setValue(double value) { _value.vfloat = value; }
	uint8_t getPin() { return _pin; }
	uint8_t getType() { return _type;}
	uint8_t getIOType() { return _io_type; }
	uint8_t getValueType() { return _val_type; }

	SensorItem *next;

private:
	uint8_t _pin;	// pin number
	// char name[16];
	uint8_t _type;   // sensor type code
	bool _io_type;   // I/O type
	uint8_t _val_type;  // sensor value type
	union value{
		char vbool; 
		int vint; 
		double vfloat;
	}_value;
};


class SensorX{
public:
virtual SensorX(HardwareSerial *com);
virtual ~SensorX();

virtual	int readFromHost(char *inbuf_cmd);
virtual	int writeToHost();
virtual	int comAvailable();
	SensorItem* newSensor(uint8_t pin, uint8_t type, int valueint);
	SensorItem* newSensor(uint8_t pin, uint8_t type, double valuefloat);
	SensorItem* newSensor(uint8_t pin, uint8_t type, char valuebool);
	bool addSensor(SensorItem *sensor);
	bool deleteSensor(uint8_t pin);
	bool deleteAllSensors();
	int processCmd(char *str);
	uint8_t getSensorNum(){return _sensorNum};
	int saveSensorData(uint8_t pin, int valueint);
	int saveSensorData(uint8_t pin, double valuefloat);
	int saveSensorData(uint8_t pin, char valuebool);
	bool addPost(uint8_t id);
	bool deletePost(uint8_t id);

private:
	int parseCmd(char *str);
	SensorItem *_sQueue;
	uint8_t _sensorNum;
	Serial *_com;
	uint8_t _post[MAX_IDS];
	aJsonObject *_aJson;
};

#endif