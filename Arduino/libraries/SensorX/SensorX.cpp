
#include "Arduino.h"
#include "SensorX.h"

SensorItem::SensorItem(uint8_t pin, uint8_t type, uint8_t val_type){
	_pin = pin;
	_type = type;
	_val_type = val_type;
	_value.vfloat = 0;
	_next = NULL
	_max_val = 32767
	_min_val = -32768;
	_alarm = false;
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
	_sQueue = NULL;
	_aJson = NULL;
}

uint8_t SensorX::addPost(){
	int i = 0;
	for(; i < MAX_IDS; i++){
		if(_post[i] == 0){
			_post[i] = 1;
			return i;
		}
	}
	_post[MAX_IDS-1] = 1;
	return MAX_IDS-1;
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

int SensorX::saveSensorData(uint8_t pin, int valueint){
	if(_sQueue == NULL)
		return 0;
	SensorItem* s = _sQueue;
	while(s){
		if(s->getPin() == pin && s->getIOType() == OUTPUT){
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
		if(s->getPin() == pin && s->getIOType() == OUTPUT){
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
		if(s->getPin() == pin && s->getIOType() == OUTPUT){
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
		if(!aJson.getObjectItem(root, "PIN"))
			return CMD_PARSE_ERROR;
		*command = CMD_DELETE_SENSOR;
		return CMD_OK;
	}

	return CMD_NULL;
}

void SensorX::addSensorDataToJson(aJsonObject *ajson){
	aJson.addNumberToObject(ajson, "STATUS", CMD_OK);
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
		aJson.addItemToArray(pin_list, pin);
		aJson.addItemToArray(type_list, type);
		aJson.addItemToArray(value_list, value);
	}
}

bool SensorX::deleteUnusedSensor(uint8_t pin){
	SensorItem *s = _sQueue;
	while(s){
		if(s->getPin() == pin)
			return deleteSensor(s); 
		s = s->next;
	}
	return false;
}	

bool SensorX::addNewSensor(uint8_t pin, uint8_t type, uint8_t val_type){
	SensorItem *s = _sQueue;
	while(s){
		if(s->getPin() == pin){
			if(s->getType() == type)
				return true;
			s->setType(type);
			s->setValueType(val_type);
			return true;
		}
		s = s->next;
	}
	if (s == NULL){
		s = newSensor(pin, type, val_type);
		if(s != NULL)
			return addSensor(s);
	}
	
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
		aJson.addStringToObject(_aJson, "CMD", "DATA");
		aJson.addNumberToObject(_aJson,"ID", id);
		if(ret <= 0){
			aJson.addNumberToObject(_aJson, "STATUS", ret);
			return;
		}
		addSensorDataToJson(_aJson)
	}
	else if(command == CMD_SET_DATA){
		aJson.addStringToObject(_aJson, "CMD", "SET");
		aJson.addNumberToObject(_aJson,"ID", id);
		if(ret <= 0){
			aJson.addNumberToObject(_aJson, "STATUS", ret);
			return;
		}
		aJsonObject* pin_list = aJson.getObjectItem(str, "PIN");
		aJsonObject* type_list = aJson.getObjectItem(str, "TYPE");
		aJsonObject* value_list = aJson.getObjectItem(str, "VALUE");
		unsigned char c1 = aJson.getArraySize(pin_list);
		unsigned char c2 = aJson.getArraySize(type_list);
		unsigned char c3 = aJson.getArraySize(value_list);
		if(c1 != c2 || c1 != c3 || c2 != c3){
			aJson.addNumberToObject(_aJson,"STATUS", CMD_PARSE_ERROR);
			return;
		}
		int i = 0;
		for(; i < (int)c1; i++){
			aJsonObject *pin = aJson.getArrayItem(pin_list, i);
			aJsonObject *type = aJson.getArrayItem(type_list, i);
			aJsonObject *value = aJson.getArrayItem(value_list, i);
			if(type->type == aJson_Boolean){
				if(!saveSensorData(pin->valueint, value->valuebool)){
					aJson.addNumberToObject(_aJson,"STATUS", CMD_ERROR);
					return;
				}
			}
			else if(type->type == aJson_Int){
				if(!saveSensorData(pin->valueint, value->valueint)){
					aJson.addNumberToObject(_aJson,"STATUS", CMD_ERROR);
					return;
				}
			}
			else if(type->type == aJson_Float){
				if(!saveSensorData(pin->valueint, value->valuefloat)){
					aJson.addNumberToObject(_aJson,"STATUS", CMD_ERROR);
					return;
				}
			}
		} // end for
		aJson.addNumberToObject(_aJson,"STATUS", CMD_OK);
	}
	else if(command == CMD_ADD_SENSOR){
		aJson.addStringToObject(_aJson, "CMD", "ADD");
		aJson.addNumberToObject(_aJson,"ID", id);
		if(ret <= 0){
			aJson.addNumberToObject(_aJson, "STATUS", ret);
			return;
		}

		aJsonObject *pin_list = aJson.getObjectItem(str, "PIN");
		aJsonObject *type_list = aJson.getObjectItem(str, "TYPE");
		unsigned char c1 = aJson.getArraySize(pin_list);
		unsigned char c2 = aJson.getArraySize(type_list);
		if(c1 != c2){
			aJson.addNumberToObject(_aJson,"STATUS", CMD_PARSE_ERROR);
			return;
		}
		int i = 0;
		for(; i < c1; i++){
			aJsonObject *pin = aJson.getArrayItem(pin_list, i);
			aJsonObject *type = aJson.getArrayItem(type_list, i);
			if(!addNewSensor(pin->valueint, type->valueint)){
				aJson.addNumberToObject(_aJson,"STATUS", CMD_ERROR);
				return;
			}
		}
		aJson.addNumberToObject(_aJson,"STATUS", OK);	
	}
	else if(command == CMD_DELETE_SENSOR){
		aJson.addStringToObject(_aJson, "CMD", "DELETE");
		aJson.addNumberToObject(_aJson,"ID", id);
		if(ret <= 0){
			aJson.addNumberToObject(_aJson, "STATUS", ret);
			return;
		}
		aJsonObject *pin_list = aJson.getObjectItem(str, "PIN");
		unsigned char c1 = aJson.getArraySize(pin_list);
		int i = 0;
		for(; i < c1; i++){
			aJsonObject *pin = aJson.getArrayItem(pin_list, i);
			if(!deleteUnusedSensor(pin->valueint){
				aJson.addNumberToObject(_aJson, "STATUS", CMD_ERROR);
				return;
			}
		}
		aJson.addNumberToObject(_aJson, "STATUS", CMD_OK);
	}
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

bool SensorX::writeToHost(){
	if(_aJson == NULL)
		return false;
	char *send_str = send_str = (char*)malloc(MAX_TX_BUFFER);
	if(send_str == NULL)
		return false;
	char *json_str = aJson.print(_aJson);
	aJsonObject *cmd = aJson.getObjectItem(_aJson, "CMD");
	if(!strcmp(cmd->valuestring, "POST")){
		strcpy(send_str, "POST=");
		strcpy(send_str+5, json_str);
	}

	strcpy(send_str, "+++GET=");
	strcpy(send_str+7, json_str);
	_com.println(send_str);

	free(send_str);
	aJson.deleteItem(_aJson);
	_aJson = NULL;
	return true
}

SensorItem* SensorX::newSensor(uint8_t pin, uint8_t type, uint8_t val_type){
	SensorItem *s = new SensorItem(pin, type, val_type);
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

void SensorX::setup(uint32_t baudrate){
	_com.begin(baudrate);
	if(_sQueue == NULL)
		return;
	SensorItem *s = _sQueue;
	while(s){
		s->setup();
		s = s->next;
	}
}

bool SensorX::checkAlarm(SensorItem *s){
	if(s == NULL)
		return false;
	int max = 0, min = 0;
	s->getThresholdValue(&max, &min);
	switch(s->getValueType()){
		case VALUE_INT:
			int val = (int)s->getValue();
			if(val >= max || val <= min)
				return true;
		case VALUE_FLOAT:
			double val = (double)s->getValue();
			if(val >= max || val <= min)
				return = true;
		case VALUE_BOOL:
			char val = (char)s->getValue();
			if(!val)
				return = true;
		default:
			break;
	}
	return false;
}

void SensorX::processSensor(){
	if(_sQueue == NULL)
		return;
	// only post 8 sensor once
	uint8_t pin[8];
	uint8_t type[8];
	double value[8];
	memset(pin, 0, 8);
	memset(type, 0, 8);
	memset(value, 0, 8);
	SensorItem *s = _sQueue;
	int i = 0;
	while(s){
		if(s->getType() >= 100)
			s->write();
		if(s->getIOType() < 100){
			s->read();
			if (checkAlarm(s)){
				pin[i] = s->getPin();
				type[i] = s->getType();
				switch(s->getValueType()){
				case VALUE_INT:
					value[i] = (int)s->getValue();
					break;
				case VALUE_FLOAT:
					value[i] = (double)s->getValue();
					break;
				case VALUE_BOOL:
					value[i] = (char)s->getValue();
					break;
				default:
					break;
				}
				i++;
			}
		}
		s = s->next;
	}
	// format json to post
	if(!type[0]){
		if(_aJson != NULL){
			aJson.deleteItem(_aJson);
			_aJson = NULL;
		}
		_aJson = aJson.createObject();
		aJson.addStringToObject(_aJson, "CMD", "POST");
		aJson.addNumberToObject(_aJson, "ID", addPost());
		aJsonObject *pin_list = aJson.createArray();
		aJsonObject *type_list = aJson.createArray();
		aJsonObject *value_list = aJson.createArray();
		aJson.addItemToObject(_aJson, "PIN", pin_list);
		aJson.addItemToObject(_aJson, "TYPE", type_list);
		aJson.addItemToObject(_aJson, "VALUE", value_list);
		i = 0;
		while(type[i]){
			aJson.addItemToArray(pin_list, aJson.createItem(pin[i]));
			aJson.addItemToArray(type_list, aJson.createItem(type[i]));
			aJson.addItemToArray(type_list, aJson.createItem(value[i]));
			i++;
		}
	}
}



