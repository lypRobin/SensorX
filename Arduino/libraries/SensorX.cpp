/*
JSON format:
GET:
	GET={
		 "cmd":"DATA",
		 "id": cmd id			
		}

	+++GET={ 
			 "cmd":"DATA",
			 "id": cmd id,
			 "status":-1/-2/0/100        // state is int number
			"pin": [pin1, pin2, pin3],
			"type": [type1, type2, type3],
			 "value": [value1, value2, value2]
	}

	GET={
		 "cmd":"SET",
		 "id": cmd id,
		 "pin": [pin1, pin2, pin3],
		 "type": [type1, type2, type3],
		 "value": [value1, value2, value2],			
	}
	+++GET={
		 "cmd":"SET",
		 "id": cmd id,
		 "STATUS":-1/-2/0/100
	}

	GET={
		 "cmd":"ADD",
		 "id": cmd id,
	 	 "PIN": [pin1, pin2],
		 "TYPE": [type1, type2]		
		}

	+++GET={ 
			 "cmd":"ADD",
			 "id": cmd id,
			 "STATUS":-1/-2/0/100
			}

	GET={
		 "cmd":"DELETE",
		 "id": cmd id,
	 	 "PIN": [pin1, pin2],	
		}

	+++GET={ 
			 "cmd":"DELETE",
			 "id": cmd id,
			 "STATUS":-1/-2/0/100
			}

POST:
	POST={
			"CMD": "POST"
			"ID": id,
			"PIN": [pin1, pin2],
			"TYPE": [type1, type2]
	}
	+++POST={
			"CMD": "POST"
			"ID": id
			}
*/


#include <stdlib.h>
#include "SensorX.h"

SensorItem::SensorItem(uint8_t pin, uint8_t type, uint8_t io_type, uint8_t val_type){
	_pin = pin;
	_type = type;
	_io_type = io_type;
	_val_type = val_type;
	_value.vfloat = 0;
	_next = NULL
}


// SensorX
SensorX::SensorX(HardwareSerial *com){
	_sQueue = NULL
	_sensorNum = 0;
	_com = com;
	_aJson = NULL;
	int i = ;
	for (;i < MAX_IDS; i++){
		_post[i] = 0;
	}

}

SensorX::~SensorX(){
	if(_sQueue != NULL)
		deleteAllSensors(_sQueue);

	if(_aJson != NULL)
		aJson.deleteItem(_aJson);	
}

bool SensorX::addPost(uint8_t id){
	if (id > MAX_IDS)
		return false;

	int i = 0;
	for(; i < MAX_IDS; i++){
		if(_post[id] == 0){
			_post[id] = 1;      
			break;
		}
	}
	if (i == MAX_IDS)  // no space for another post
		return false;

	return true;
}

bool SensorX::addGet(uint8_t id){
	if (id > MAX_IDS)
		return false;

	int i = 0;
	for(; i < MAX_IDS; i++){
		if(_get[id] == 0){
			_get[id] = 1;      
			break;
		}
	}
	if (i == MAX_IDS)  // no space for another get
		return false;

	return true;
}

bool SensorX::deletePost(uint8_t id){
	if (id < MAX_IDS)
		if(_post[id] == 1)
			_post[id] = 0;
		else
			return false;
	else
		return false;
	return true;
}

bool SensorX::deleteGet(uint8_t id){
	if (id < MAX_IDS)
		if(_get[id] == 1)
			_get[id] = 0;
		else
			return false;
	else
		return false;
	return true;
}


int SensorX::saveSensorData(uint8_t pin, int valueint){
	if(_sQueue == NULL)
		return 0;
	SensorItem* s = _sQueue;
	while(s){
		if(s->getPin() == pin){
			s->setValue(valueint);
			return -1;
		}
	}
	return 0;
}

int SensorX::saveSensorData(uint8_t pin, double valuefloat){
	if(_sQueue == NULL)
		return 0;
	SensorItem* s = _sQueue;
	while(s){
		if(s->getPin() == pin){
			s->setValue(valuefloat);
			return -1;
		}
	}
	return 0;
}

int SensorX::saveSensorData(uint8_t pin, char valuebool){
	if(_sQueue == NULL)
		return 0;
	SensorItem* s = _sQueue;
	while(s){
		if(s->getPin() == pin){
			s->setValue(valuebool);
			return -1;
		}
	}
	return 0;
}


int SensorX::parseCmd(char *str, int *command){
	aJsonObject* root = aJson.parse(str);
	aJsonObject* cmd = aJson.getObjectItem(root, "CMD");
	if (cmd == NULL)
		return CMD_PARSE_ERROR;
	if(!strcmp(cmd->valuestring, "POST")){
		aJsonObject* post_id = aJson.getObjectItem(root, "ID");
		if (deleteePost(post_id->valueint))
			return CMD_OK;
	}

	aJsonObject* id = aJson.getObjectItem(root, "ID");
	if(!addGet(id->valueint))
		return CMD_PARSE_ERROR;

	if(!strcmp(cmd->valuestring, "DATA")){
		*command = CMD_SEND_DATA;
		return CMD_OK;
	}

	if(!strcmp(cmd->valuestring, "SET"))
		*command = CMD_SET_DATA;
		if(!aJson.getObjectItem(root, "PIN") || !aJson.getObjectItem(root, "TYPE") || !aJson.getObjectItem(root, "VALUE"))
			return CMD_PARSE_ERROR;
		return CMD_OK;
	}

	if(!strcmp(cmd->valuestring, "ADD"))
		*command = CMD_ADD_SENSOR;
		if(!aJson.getObjectItem(root, "PIN") || !aJson.getObjectItem(root, "TYPE"))
			return CMD_PARSE_ERROR;
		return CMD_OK;
	}

	if(!strcmp(cmd->valuestring, "DELETE"))
		if(!aJson.getObjectItem(root, "PIN") || !aJson.getObjectItem(root, "TYPE"))
			return CMD_PARSE_ERROR;
		*command = CMD_DELETE_SENSOR;
		return CMD_OK;
	}

	return CMD_NULL;
}

void SensorX::addSensorDataToJson(aJsonObject *ajson){
	aJson.addItemToObject(ajson, "STATUS", CMD_OK);
	SensorItem* s = _sQueue;
	aJsonObject *pin_list, *value_list, *type_list;
	aJson.addItemToObject(ajson, "PIN", pin_list = aJson.createArray());
	aJson.addItemToObject(ajson, "TYPE", type_list = aJson.createArray());
	aJson.addItemToObject(ajson, "VALUE", value_list = aJson.createArray());
	if(_sQueue == NULL)
		return;
	aJsonObject *pin, *value, *type;
	while(s){
		pin = aJson.createItem((int)s->getPin());
		type = aJson.createItem((int)s->getType());
		if(s->getValueType() == VALUE_INT)
			value = aJson.createItem((int)s->getValue());
		if(s->getValueType() == VALUE_FLOAT)
			value = aJson.createItem((double)s->getValue());
		if(s->getValueType() == VALUE_BOOL)
			value = aJson.createItem((char)s->getValue());
		aJson.addItemToObject(pin_list, pin);
		aJson.addItemToObject(type_list, type);
		aJson.addItemToObject(value_list, value);
	}
}

bool SensorX::deleteUnusedSensor(uint8_t pin){
	if(_sQueue == NULL)
		return false;
	SensorItem *s = _sQueue;
	while(s){
		if(s->getPin() == pin){
			s = deleteSensor(s);
			SensorItem *sn = newSensor(pin, type, io_type, val_type);
			if(sn != NULL)
				addSensor(sn);
		}
	}
}	

bool SensorX::addNewSensor(uint8_t pin, uint8_t type){
	SensorItem *s = _sQueue;
	uint8_t io_type = 0;
	uint8_t val_type = 0;
	switch(type){
		case SENSOR_TEMP:
		case SENSOR_HUMI:
		case SENSOR_DUST:
			io_type = 1;
			val_type = VALUE_FLOAT;
			break;
		case SENSOR_LIGHT:
		case SENSOR_HCHO:
		case SENSOR_CO2:
		case SENSOR_GAS:
			io_type = 1;
			val_type = VALUE_INT;
			break;
		case SENSOR_FIRE:
		case SENSOR_HUMAN:
			io_type = 1;
			val_type = VALUE_BOOL;
			break;
		case SENSOR_LED:
		case SENSOR_PUMP:
			io_type = 0;
			val_type = VALUE_BOOL;
			break;
		default:
			io_type = 1;
			val_type = VALUE_FLOAT;
			break;
	}
	while(s){
		if(s->getPin() == pin){
			s = deleteSensor(s);
			SensorItem *sn = newSensor(pin, type, io_type, val_type);
			if(sn != NULL)
				addSensor(sn);
		}
		s = s->next;
	}
	SensorItem *sn = newSensor(pin, type, io_type, val_type);
	if(sn != NULL)
		return addSensor(sn);
	else
		return false
}

void SensorX::processCmd(char *str){
	int command = 0;
	int ret = parseCmd(str, &command);

	if(_aJson != NULL){
		aJson.deleteItem(_aJson);
		_aJson = NULL;
	}

	_aJson = aJson.createObject();
	if(command == CMD_SEND_DATA){
		aJson.addItemToObject(_aJson, "CMD", "DATA");
		aJson.addItemToObject(_aJson,"ID", id);
		if(ret <= 0){
			aJson.addItemToObject(_aJson, "STATUS", ret);
			return;
		}
		addSensorDataToJson(_aJson)
	}
	else if(command == CMD_SET_DATA){
		aJson.addItemToObject(_aJson, "CMD", "SET");
		aJson.addItemToObject(_aJson,"ID", id);
		if(ret <= 0){
			aJson.addItemToObject(_aJson, "STATUS", ret);
			return;
		}
		aJsonObject* pin_list = aJson.getObjectItem(str, "PIN");
		aJsonObject* type_list = aJson.getObjectItem(str, "TYPE");
		aJsonObject* value_list = aJson.getObjectItem(str, "VALUE");
		unsigned char c1 = aJson.getArraySize(pin_list);
		unsigned char c2 = aJson.getArraySize(type_list);
		unsigned char c3 = aJson.getArraySize(value_list);
		if(c1 != c2 || c1 != c3 || c2 != c3){
			aJson.addItemToObject(_aJson,"STATUS", CMD_PARSE_ERROR);
			return;
		}
		int i = 0;
		for(; i < (int)c1; i++){
			aJsonObject *pin = aJson.getArrayItem(pin_list, i);
			aJsonObject *type = aJson.getArrayItem(type_list, i);
			aJsonObject *value = aJson.getArrayItem(value_list, i);
			if(type->type == aJson_Boolean){
				if(!saveSensorData(pin->valueint, value->valuebool)){
					aJson.addItemToObject(_aJson,"STATUS", CMD_ERROR);
					return;
				}
			}
			else if(type->type == aJson_Int){
				if(!saveSensorData(pin->valueint, value->valueint)){
					aJson.addItemToObject(_aJson,"STATUS", CMD_ERROR);
					return;
				}
			}
			else if(type->type == aJson_Float){
				if(!saveSensorData(pin->valueint, value->valuefloat)){
					aJson.addItemToObject(_aJson,"STATUS", CMD_ERROR);
					return;
				}
			}
		} // end for
		aJson.addItemToObject(_aJson,"STATUS", CMD_OK);
	}
	else if(command == CMD_ADD_SENSOR){
		aJson.addItemToObject(_aJson, "CMD", "ADD");
		aJson.addItemToObject(_aJson,"ID", id);
		if(ret <= 0){
			aJson.addItemToObject(_aJson, "STATUS", ret);
			return;
		}

		aJsonObject *pin_list = aJson.getObjectItem(str, "PIN");
		aJsonObject *type_list = aJson.getObjectItem(str, "TYPE");
		unsigned char c1 = aJson.getArraySize(pin_list);
		unsigned char c2 = aJson.getArraySize(type_list);
		if(c1 != c2){
			aJson.addItemToObject(_aJson,"STATUS", CMD_PARSE_ERROR);
			return;
		}
		int i = 0;
		for(; i < c1; i++){
			aJsonObject *pin = aJson.getArrayItem(pin_list, i);
			aJsonObject *type = aJson.getArrayItem(type_list, i);
			if(!addNewSensor(pin->valueint, type->valueint)){
				aJson.addItemToObject(_aJson,"STATUS", CMD_ERROR);
				return;
			}

		}
		aJson.addItemToObject(_aJson,"STATUS", OK);	
	}
	else if(command == CMD_DELETE_SENSOR){
		aJson.addItemToObject(_aJson, "CMD", "DELETE");
		aJson.addItemToObject(_aJson,"ID", id);
		if(ret <= 0){
			aJson.addItemToObject(_aJson, "STATUS", ret);
			return;
		}
		aJsonObject *pin_list = aJson.getObjectItem(str, "PIN");
		aJsonObject *type_list = aJson.getObjectItem(str, "TYPE");
		unsigned char c1 = aJson.getArraySize(pin_list);
		unsigned char c2 = aJson.getArraySize(type_list);

	}

}


int SensorX::comAvalaible(){
	return _com->available();
}

int SensorX::readFromHost(char *inbuf_json){
	char inbuf[256];
	int ret = _com->readBytes(inbuf, MAX_RX_BUFFER);

	if (ret < MAX_RX_BUFFER - 1)
		inbuf[ret] = '\0';
	else 
		return 0;

	if(!strncmp(inbuf, "GET", 3)){
		strncpy(inbuf_json, inbuf+4)
		return -1;
	}

	if(!strncmp(str, "+++POST", 7)){
		if(!strncmp(str+8, "ERROR", 5))
			return 0; 		// send from esp8266 to host error
		strncpy(inbuf_json, inbuf+8)
		return -1;
	}

	return 0;
}

int SensorX::writeToHost(char* send_str){
	if(send_str == NULL)
		return 0;

	_com->write(send_str, strlen(send_str));
	return strlen(send_str);
}

SensorItem* newSensor(uint8_t pin, uint8_t type, uint8_t io_type, uint8_t val_type){
	SensorItem *s = new SensorItem(pin, type, io_type, val_type);
	return s;
}

// add new Sensor and put together the Sensors with the same type
bool SensorX::addSensor(SensorItem *sensor){
	SensorItem *s = _sQueue;
	while(s){
		if(s->getPin() == sensor->getPin())
			return false;
		s = s->next;
	}
	s = sensor;
	_sensorNum++;

	return true;
}

SensorItem* SensorX::deleteSensor(uint8_t pin){
	if(_sQueue == NULL)
		return NULL;
	SensorItem *s = _sQueue;
	if(_sQueue->getPin() == pin){
		_sQueue = _sQueue->next;
		delete s;
		_sensorNum--;
		return _sQueue;
	}

	SensorItem *sn = s->next; 
	while(sn){
		if(sn->getPin() == pin){
			s->next = sn->next;
			delete sn;
			_sensorNum--;
			return s->next;
		}
	}
	return NULL;
}

bool SensorX::deleteAllSensors(){
	if(_sQueue == NULL)
		return true;
	SensorItem *s = _sQueue;
	while(_sQueue){
		_sQueue = _sQueue->next;
		delete s;
		s = _sQueue;
		_sensorNum--;
	}
	if(_sensorNum == 0)
		return true;
	else
		return false;
}

void SensorX::begin(uint32_t baudrate){
	_com.begin(baudrate);
	if(_sQueue == NULL)
		return;
	SensorItem *s = _sQueue;
	while(s){
		s->setup();
		s = s->next;
	}
}



