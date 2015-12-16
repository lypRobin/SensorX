#ifndef _SENSORX_H_
#define _SENSORX_H_
/*
JSON format that communicating between SensorX and host:
GET command:
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

POST command:
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
	/*
	* description: construction function. Initialize the pin, type and value type of the sensor
	* parameters: pin: sensor pin
	*             type: sensor type
	*             val_type: the value type of the sensor
	* return: return sensor value in double
	*/
	SensorItem(uint8_t pin, uint8_t type, uint8_t val_type);

	/*
	* description: sensor initilization such as pin mode and some internal values.
	*/
	virtual void setup() {}

	/*
	* description: read data from the certain sensor.
	*/
	virtual void read() {}

	/*
	* description: write data to the certain sensor.
	*/
	virtual void write() {}

	/*
	* description: get sensor value
	* parameters: NONE
	* return: return sensor value in double
	*/
	double getValue() { return _value.vfloat; }

	/*
	* description: set sensor value
	* parameters: value: set sensor value, most are control values
	* return: NONE
	*/
	void setValue (int value) {  _value.vint = value; }
	void setValue(char value) { _value.vbool = value; }
	void setValue(double value) { _value.vfloat = value; }

	/*
	* description: set sensor pin on SensorX
	* parameters: pin: set sensor pin.
	* return: NONE
	*/
	void setPin(uint8_t pin) { _pin = pin;}

	/*
	* description: return the pin number of the sensor plugin the SensorX
	* parameters: NONE
	* return: pin number
	*/
	uint8_t getPin() { return _pin; }

	/*
	* description: set the sensor type
	* parameters: type: sensor type
	* return: NONE
	*/
	void setType(uint8_t type) { _type = type; }

	/*
	* description: return the type of the sensor
	* parameters: NONE
	* return: sensor type
	*/
	uint8_t getType() { return _type;}

	/*
	* description: set value type of the sensor. there are five types of value: VALUE_NULL, VALUE_BOOL, VALUE_INT, 
	*				VALUE_FLOAT, VALUE_STRING
	* parameters: val_type: the type of values
	* return: NONE
	*/
	void setValueType(uint8_t val_type) { _val_type = val_type; }

	/*
	* description: get the type of the sensor value
	* parameters: NONE
	* return: the ype of the sensor value
	*/
	uint8_t getValueType() { return _val_type; }

	/*
	* description: set the sensor threshold value. If the value of the sensor got exceeds the threshold value, a POST 
	*				request will be triggerd.
	* parameters: max
	* return: the ype of the sensor value
	*/
	void setThresholdValue(int max, int min) { _max_val = max; _min_val = min; }

	/*
	* description: get the sensor threshold value. 
	* parameters: max: the maximum threshold.
	*			  min: the minimum threshold.
	* return: the ype of the sensor value
	*/
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
	/*
	* description: Construction function. Initialize the SensorX with serial com.
	* parameters: com: serial com
	* return: NONE
	*/
	SensorX(HardwareSerial *com);
	// ~SensorX();

	/*
	* description: Setup the SensorX to initialize all the sensor items pluged in the SensorX.
	* parameters: baudrate: the working baudrate of the SensorX communicating with the esp8266 wifi module.
	* return: NONE
	*/
	void setup(uint32_t baudrate);

	/*
	* description: If there is data in the RX com buffer, then it is available for SensorX to read from buffer.
	* parameters: NONE.
	* return: Serial.available()
	*/
	int comAvailable() { return _com->available(); }

	/*
	* description: Read data from the host command which stored in the RX buffer.
	* parameters: inbuf_json: read from the RX buffer to inbuf_json.
	* return: 0: failed
    *         none-0: success
	*/
	int readFromHost(char *inbuf_json);

	/*
	* description: Write data to TX buffer and send to host.
	* parameters: NONE.
	* return: false: failed
    *         true: success
	*/
	bool writeToHost();
	
	/*
	* description: Parse and process the cmd received from the host.
	* parameters: str: valid json data string.
	* return: CMD_OK: success
    *         others: failed
	*/
	int processCmd(char *str);

	/*
	* description: Get the number of the sensor pluggin in the SensorX.
	* parameters: NONE.
	* return: The number of sensors
	*/
	uint8_t getSensorNum(){ return _sensorNum; }	

private:

	/*
	* description: Create a new sensor item and initialize it.
	* parameters: pin: sensor pin
	*             type: sensor type
	*             val_type: the value type of the sensor
	* return: the pointer of the new sensor item
	*/	
	SensorItem* newSensor(uint8_t pin, uint8_t type, uint8_t val_type);

	/*
	* description: Add the sensor to the sensor list.
	* parameters: sensor: the pointer to the added sensor.       
	* return: true: success
	*         false: failed
	*/	
	bool addSensor(SensorItem *sensor);

	/*
	* description: Delete the sensor plug in the "pin".
	* parameters: pin: pin number
	* return: Pointer to the next of the deleted sensor in the list. 
	*/	
	SensorItem* deleteSensor(uint8_t pin);

	/*
	* description: Delete all the sensor on the SensorX.
	* parameters: NONE
	* return: true: success
	*         false: failed
	*/		
	bool deleteAllSensors();

	/*
	* description: Save the new data to the certain data.
	* parameters: pin: pin number which sensor plugged in.
	*             valueint: saved value in int type.
	* return: 0: failed
    *         none-0: success
	*/
	int saveSensorData(uint8_t pin, int valueint);
	int saveSensorData(uint8_t pin, double valuefloat);
	int saveSensorData(uint8_t pin, char valuebool);

	/*
	* description: Add sensor data to the json string.
	* parameters: ajson: The json string which the data added in.
	* return: NONE
	*/
	void addSensorDataToJson(aJsonObject *ajson);
	// SensorItem * deleteUnusedSensor(uint8_t pin);

	/*
	* description: Add new sensor if the host sends the ADD sensor command.
	* parameters: ajson: The json string which the data added in.
	* return: true: success
	*         false: failed
	*/	
	bool addNewSensor(uint8_t pin, uint8_t type, uint8_t val_type);

	/*
	* description: Process all sensors which needed to read or write.
	* parameters: NONE.
	* return: NONE
	*/	
	void processSensor();

	/*
	* description: Check if the sensor value exceeds the threshold.
	* parameters: NONE.
	* return: true: exceed the threshold
	*         false: safe
	*/	
	bool checkAlarm(SensorItem *s);

	/*
	* description: Add a post flag in the post list.
	* parameters: NONE.
	* return: The post flag index
	*/
	uint8_t addPost();

	/*
	* description: Delete the post flag in the post list.
	* parameters: The post flag index.
	* return: true: success
	*         false: failed
	*/
	bool deletePost(uint8_t id);

	SensorItem *_sQueue;
	uint8_t _sensorNum;
	HardwareSerial *_com;
	uint8_t _post[MAX_IDS];
	aJsonObject *_aJson;
};

#endif