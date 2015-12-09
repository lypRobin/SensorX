#ifndef _SENSORX_H_
#define _SENSORX_H_
/*
JSON format:
GET:
	GET={
		 "CMD":"DATA",
		 "ID": id			
		}

	+++GET={ 
			 "CMD":"DATA",
			 "ID": id,
			 "STATUS":-1/-2/0/100        // state is int number
			"PIN": [pin1, pin2, pin3],
			"TYPE": [type1, type2, type3],
			 "value": [value1, value2, value2]
	}

	GET={
		 "CMD":"SET",
		 "ID": id,
		 "PIN": [pin1, pin2, pin3],
		 "TYPE": [type1, type2, type3],
		 "value": [value1, value2, value2],			
	}
	+++GET={
		 "CMD":"SET",
		 "ID": id,
		 "STATUS":-1/-2/0/100
	}

	GET={
		 "CMD":"ADD",
		 "ID": id,
	 	 "PIN": [pin1, pin2],
		 "TYPE": [type1, type2],
		 "VALUETYPE": [type1, type2]
		}

	+++GET={ 
			 "CMD":"ADD",
			 "ID": id,
			 "STATUS":-1/-2/0/100
			}

	GET={
		 "CMD":"DELETE",
		 "ID": id,
	 	 "PIN": [pin1, pin2],	
		}

	+++GET={ 
			 "CMD":"DELETE",
			 "ID": id,
			 "STATUS":-1/-2/0/100
			}

POST:
	POST={
			"CMD": "POST"
			"ID": ID,
			"PIN": [pin1, pin2],
			"VALUE": [value1, value2]
		}
	+++POST={
			"CMD": "POST"
			"ID": ID
			}
*/

#include "../aJson/aJSON.h"

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
	SensorItem(uint8_t pin, uint8_t _type, uint8_t _val_type);

	virtual void setup() {}
	virtual void read() {}
	virtual void write() {}

	double getValue() { return _value.vfloat; }
	void setValue (int value) {  _value.vint = value; }
	void setValue(char value) { _value.vbool = value; }
	void setValue(double value) { _value.vfloat = value; }
	void setPin(uint8_t pin) { _pin = pin;}
	uint8_t getPin() { return _pin; }
	void setType(uint8_t type) { _type = type; }
	uint8_t getType() { return _type;}
	void setValueType(uint8_t val_type) { _val_type = val_type; }
	uint8_t getValueType() { return _val_type; }
	void setThresholdValue(int max, int min) { _max_val = max; _min_val = min; }
	void getThresholdValue(int *max, int *min) { *max = _max_val; *min = _min_val; }

	SensorItem *next;

private:
	uint8_t _pin;	// pin number
	// char name[16];
	uint8_t _type;   // sensor type code
	uint8_t _val_type;  // sensor value type
	union value{
		char vbool; 
		int vint; 
		double vfloat;
	}_value;
	int _max_val;
	int _min_val;
};


class SensorX{
public:
	SensorX(HardwareSerial *com);
	// ~SensorX();

	int readFromHost(char *inbuf_json);
	bool writeToHost();
	void setup(uint32_t baudrate);
	int comAvailable() { return _com->available(); }
	int processCmd(char *str);
	uint8_t getSensorNum(){ return _sensorNum; }	
private:
	SensorItem* newSensor(uint8_t pin, uint8_t type, uint8_t val_type);
	bool addSensor(SensorItem *sensor);
	SensorItem* deleteSensor(uint8_t pin);

	bool deleteAllSensors();

	int saveSensorData(uint8_t pin, int valueint);
	int saveSensorData(uint8_t pin, double valuefloat);
	int saveSensorData(uint8_t pin, char valuebool);
	void addSensorDataToJson(aJsonObject *ajson);
	// SensorItem * deleteUnusedSensor(uint8_t pin);
	bool addNewSensor(uint8_t pin, uint8_t type, uint8_t val_type);
	void processSensor();
	bool checkAlarm(SensorItem *s);
	uint8_t addPost();
	bool deletePost(uint8_t id);
	int parseCmd(char *str, int *command, int *id);
	SensorItem *_sQueue;
	uint8_t _sensorNum;
	HardwareSerial *_com;
	uint8_t _post[MAX_IDS];
	aJsonObject *_aJson;
};

#endif