/*
JSON protocal:
GET:
	GET={
		 "cmd":"DATA",
		 "id": cmd id			
		}

	+++GET={ 
			 "cmd":"DATA",
			 "id": cmd id,
			 "TEMP": {
					 "pin": [pin1, pin2, pin3],
					 "value": [value1, value2, value2],
			 }	
			 "DUST": {
					 "pin": [pin1, pin2, pin3],
					 "value": [value1, value2, value2],
			 }
	
	}

	GET={
		 "cmd":"SET",
		 "id": cmd id,
		  "LED": {
					 "pin": [pin1, pin2, pin3],
					 "value": [value1, value2, value2],
			 }				
	}
	+++GET={
		 "cmd":"SET",
		 "id": cmd id,
		 "STATUS":"OK/ERROR"
	}

	GET={
		 "cmd":"ADD",
		 "id": cmd id,
	 	 "TEMP": [pin],
		 "DUST": pin		
		}

	+++GET={ 
			 "cmd":"ADD",
			 "id": cmd id,
			 "STATUS":"OK/ERROR"
			}

	GET={
		 "cmd":"DELET",
		 "id": cmd id,
	 	 "TEMP": pin,
		 "DUST": pin		
		}

	+++GET={ 
			 "cmd":"DELET",
			 "id": cmd id,
			 "STATUS":"OK/ERROR"
			}

POST:
	POST={
			"CMD": "POST"
			"ID": id,
			"FIRE": {
					"pin": true,
					"pin": false
				},
			"HUMAN": {
					"pin": true,
					"pin": false
				}
	}
	+++POST={
			"CMD": "POST"
			"ID": id
			}
*/


#include <stdlib.h>
#include "SensorX.h"

SensorX::SensorX(Serial *com){
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
		free(_sQueue);

	if(_aJson != NULL)
		aJson.deletItem(_aJson);	
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

bool SensorX::deletPost(uint8_t id){
	if (id < MAX_IDS)
		if(_post[id] == 1)
			_post[id] = 0;
		else
			return false;
	else
		return false;
	return true;
}

bool SensorX::deletGet(uint8_t id){
	if (id < MAX_IDS)
		if(_get[id] == 1)
			_get[id] = 0;
		else
			return false;
	else
		return false;
	return true;
}

int SensorX::parseCmd(char *str){
	aJsonObject* root = aJson.parse(str);
	aJsonObject* cmd = aJson.getObjectItem(root, "CMD");
	if (cmd == NULL)
		return CMD_PARSE_ERROR;
	if(!strcmp(cmd->valuestring, "POST")){
		aJsonObject* post_id = aJson.getObjectItem(root, "ID");
		if (deletePost(post_id->valueint))
			return CMD_POST_OK;
		else
			return CMD_PARSE_ERROR;
	}

	aJsonObject* id = aJson.getObjectItem(root, "ID");
	if(!addGet(id->valueint))
		return CMD_PARSE_ERROR;

	if(!strcmp(cmd->valuestring, "DATA"))
		return CMD_SEND_DATA;

	if(!strcmp(cmd->valuestring, "SET"))
		return CMD_SET_DATA;

	if(!strcmp(cmd->valuestring, "ADD"))
		return CMD_ADD_SENSOR;

	if(!strcmp(cmd->valuestring, "DELET"))
		return CMD_DELET_SENSOR;	

	return CMD_NULL;
}

int SensorX::saveSensorData(uint8_t pin, uint8_t type, int valueint){
	if(_sQueue == NULL)
		return 0;
	SensorItem* s = _sQueue;
	while(s){
		if(s->pin == pin && s->type == type){
			s->valueint = valueint;
			return -1;
		}
	}
	return 0;
}

int SensorX::saveSensorData(uint8_t pin, uint8_t type, double valuefloat){
	if(_sQueue == NULL)
		return 0;
	SensorItem* s = _sQueue;
	while(s){
		if(s->pin == pin && s->type == type){
			s->valuefloat = valuefloat;
			return -1;
		}
	}
	return 0;
}

int SensorX::saveSensorData(uint8_t pin, uint8_t type, char valuebool){
	if(_sQueue == NULL)
		return 0;
	SensorItem* s = _sQueue;
	while(s){
		if(s->pin == pin && s->type == type){
			s->valuebool = valuebool;
			return -1;
		}
	}
	return 0;
}


int SensorX::addSensorDataToJson(){
	SensorItem* s = _sQueue;
	aJsonObject* fmt;
	while(s){

		switch(s->type){
			case: SENSOR_TEMP:
				aJson.addItemToObject(_aJson, "TEMP", fmt = aJson.createObject());
				s->pin
				break;	
			case: SENSOR_HUMI:
				break;	
			case: SENSOR_DUST:
				break;	
			case: SENSOR_LIGHT:
				break;
			case: SENSOR_HCHO:
				break;	
			case: SENSOR_CO2:
				break;					
			case: SENSOR_GAS:
				break;					
			case: SENSOR_HUMAN:
				break;
			case: SENSOR_FIRE:
				break;	
			case: SENSOR_LED:
				break;	
		}
	}
}

int SensorX::processCmd(int cmd, uint8_t id){
	aJsonObject* fmt;
	switch(cmd){
		case CMD_NULL:
		case CMD_ERROR:
		case CMD_PARSE_ERROR:
			return 0;
		case CMD_SEND_DATA:
			if(_aJson != NULL){
				aJson.deletItem(_aJson);
				_aJson = NULL;
			}
			_aJson = aJson.createObject();
			aJson.addItemToObject(_aJson, "CMD", "DATA");
			aJson.addItemToObject(_aJson,"ID", id);


		case CMD_SET_DATA:

		case CMD_ADD_SENSOR:

		case CMD_DELET_SENSOR:

		case CMD_POST_OK:

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
		strncpy(inbuf_json, inbuf+8)
		return -1;
	}

	return 0;
}

int SensorX::formatData(aJsonObject* ajson, uint8_t id){
	if(_sQueue == NULL)
		return NULL;
	aJsonObject* root = aJson.createObject();
	aJson.addItemToObject(root, "CMD", cmd);
	aJson.addItemToObject(root, "ID", id);
	switch()


}

int SensorX::writeToHost(char* send_str){
	if(send_str == NULL)
		return 0;

	_com->write(send_str, strlen(send_str));
	return strlen(send_str);
}

SensorItem* SensorX::newSensor(uint8_t pin, uint8_t type, int value){
	SensorItem *s = (SensorItem*)malloc(sizeof(SensorItem));
	s->pin = pin;
	s->type = type;
	s->valueint = valueint;
	s->next = NULL;
	return s;
}

SensorItem* SensorX::newSensor(uint8_t pin, uint8_t type, double valuefloat){
	SensorItem *s = (SensorItem*)malloc(sizeof(SensorItem));
	s->pin = pin;
	s->type = type;
	s->valueint = valueint;
	s->next = NULL;
	return s;
}

SensorItem* SensorX::newSensor(uint8_t pin, uint8_t type, char valuebool){
	SensorItem *s = (SensorItem*)malloc(sizeof(SensorItem));
	s->pin = pin;
	s->type = type;
	s->valueint = valueint;
	s->next = NULL;
	return s;
}

// add new Sensor and put together the Sensors with the same type
bool SensorX::addSensor(SensorItem *sensor){
	if(sensor == NULL)
		return false;

	// the list is empty and put the sensor at the first one
	if(_sQueue == NULL){
		_sQueue = sensor;
		_sensorNum++
		return true;
	}

	struct SensorItem *s = _sQueue;
	struct SensorItem *sn = NULL;
	while(s){
		if(s->type == sensor->type){
			sn = s;   // sign the position of the same type Sensor
			if(s->pin == sensor->pin)  // find whether there is the same pin Sensor in the list
				return false;
			s = s->next;
			continue;
		}
		s = s->next;
	}

	// put the sensor in the list with the same type around it
	if(sn != NULL){
		sensor->next = sn->next;
		sn->next = sensor;
		_sensorNum++;
		return true;
	}

	// not find the same type sensor and put it at the end of the list
	if(s == NULL){
		s = sensor;
		_sensorNum++;
		return true;
	}

	return false;
}

bool SensorX::deletSensor(uint8_t type, uint8_t pin){
	if(_sQueue == NULL)
		return false;
	struct SensorItem *s = _sQueue;
	if(_sQueue->type == type && _sQueue->pin == pin){
		_sQueue = _sQueue->next;
		free(s);
		_sensorNum--;
		return true;
	}

	struct SensorItem *sn = s->next; 
	while(sn){
		if(sn->type == type && sn->pin == pin){
			s->next = sn->next;
			free(sn);
			_sensorNum--;
			return true;
		}
	}
	return false;
}

bool SensorX::deletAllSensors(){
	struct SensorItem *s = _sQueue;
	while(_sQueue){
		_sQueue = _sQueue->next;
		free(s);
		s = _sQueue;
		_sensorNum--;
	}
	if(_sensorNum == 0)
		return true;
	else
		return false;
}



