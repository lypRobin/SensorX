/*
JSON protocal:
GET:
	GET={"timestamp":"time(string)",
		 "cmd":"DATA",
		 "id": cmd id			
		}

	+++GET={ "timestamp":"time(string)",
			 "cmd":"DATA",
			 "id": cmd id,
			 "TEMP": {
					 "pin1": float,
					 "pin2": float,
			 }
			 "DUST": {
					 "pin1": float,
					 "pin2": float,
			 }
	
	}

	GET={"timestamp":"time(string)",
		 "cmd":"SET",
		 "id": cmd id,
		  "LED": {
					 "pin1": True,
					 "pin2": False,
			 }				
	}
	+++GET={"timestamp":"time(string)",
		 "cmd":"SET",
		 "id": cmd id
	}

	GET={"timestamp":"time(string)",
		 "cmd":"ADD",
		 "id": cmd id,
	 	 "TEMP": pin,
		 "DUST": pin		
		}

	+++GET={ "timestamp":"time(string)",
			 "cmd":"ADD",
			 "id": cmd id,
			}

	GET={"timestamp":"time(string)",
		 "cmd":"DELET",
		 "id": cmd id,
	 	 "TEMP": pin,
		 "DUST": pin		
		}

	+++GET={ "timestamp":"time(string)",
			 "cmd":"DELET",
			 "id": cmd id,
			}

POST:
	POST={"timestamp":"time(string)",
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
			"timestamp":"time(string)",
			"ID": id
			}
*/


#include <stdlib.h>
#include "SensorX.h"

SensorX::SensorX(Serial *com){
	_sQueue = NULL
	_sensorNum = 0;
	_com = com;
	int i = ;
	for (;i < MAX_POST; i++)
		_post[i] = 0;
	memset(_time, 0, 18);
}

SensorX::~SensorX(){
	if(_sNodeRoot != NULL)
		free(_sNodeRoot);

	if(_aJson != NULL)
		free(_aJson);	
}


bool SensorX::deletePost(int id){
	if (id < MAX_POST)
		if(_post[id] == 1)
			_post[id] = 0;
		else
			return false;
	else
		return false;
	return true;
}

bool SensorX::addPost(int id){
	if (id > MAX_POST)
		return false;

	int i = 0;
	for(; i < MAX_POST; i++){
		if(_post[id] == 0){
			_post[id] = 1;      
			break;
		}
	}
	if (i == MAX_POST)  // no space for another post
		return false;

	return true;
}

int SensorX::setTime(char* timestring){
	if(timestring != NULL)
		return strcpy(_time, timestring);
}

char* getTime(){
	return _time;
}


int SensorX::parseCmd(char *str, int opt){
	aJsonObject* root = aJson.parse(str);
	aJsonObject* timestamp = aJson.getObjectItem(root, "timestamp");
	setTime(timestamp->valuestring);
	if(opt == PARSE_GET){
		aJsonObject* cmd = aJson.getObjectItem(root, "CMD");
		if (cmd == NULL)
			return CMD_PARSE_ERROR;
		if(!strcmp(cmd->valuestring, "DATA"))
			return CMD_SEND_DATA;

		if(!strcmp(cmd->valuestring, "SET"))
			return CMD_SET_DATA;

		if(!strcmp(cmd->valuestring, "ADD"))
			return CMD_ADD_SENSOR;

		if(!strcmp(cmd->valuestring, "DELET"))
			return CMD_DELET_SENSOR;
	}


	if(opt == PARSE_POST){
		aJsonObject* post_id = aJson.getObjectItem(root, "ID");
		if (deletePost(post_id->valueint))
			return CMD_POST_OK;
		else
			return 0;
	}

	return CMD_NULL;
}


int SensorX::comAvalaible(){
	return _com->available();
}

int SensorX::readFromHost(char *inbuf, char* cmd){
	int ret = _com->readBytes(inbuf, MAX_RX_BUFFER);

	if (ret < MAX_RX_BUFFER - 1)
		inbuf[ret] = '\0';
	else 
		return CMD_ERROR;

	char *str = inbuf;
	if(!strncmp(str, "GET", 3)){
		cmd = str+4;
		return parseCmd(str += 4, PARSE_GET);
	}

	if(!strncmp(str, "+++POST", 7)){
		cmd = str+7;
		return parseCmd(str+7, PARSE_POST))
	}

	return CMD_NULL;
}


int SensorX::writeToHost(char* send_str){
	if(send_str == NULL)
		return 0;

	_com->write(send_str, strlen(send_str));
	return strlen(send_str);
}

int SensorX::addSensor(SensorItem *sensor){
	if(sensor == NULL)
		return -1;
	struct SensorItem *s = _sQueue;
	if(_sQueue == NULL)
		_sQueue = sensor;
	while(s)
		s = s->next;
	s = sensor;
}



