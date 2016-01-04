#include "Arduino.h"
#include "SensorX.h"
#include "Temperature.h"

int tPin = 5;
Temperature temp(tPin, SENSOR_TEMP, VALUE_INT);
SensorX sx(&Serial);

void setup(){
	println(sx._sensorNum);
	sx.addSensor((SensorItem*) &temp);
	sx.setup(115200);
	println(sx._sensorNum);
}

void loop(){

}
















