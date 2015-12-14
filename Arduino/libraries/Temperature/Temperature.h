#ifndef __TEMPERATURE_H__
#define __TEMPERATURE_H__

#include "../SensorX/SensorX.h"

#define HOST_START_TIME		20  // ms
#define HOST_WAIT_TIME 		40
#define PRE_DATA_TIME			50  // us
#define TIME_OUT					200

class Temperature : public SensorItem{
public:
	Temperature(uint8_t pin, uint8_t type, uint8_t val_type) : SensorItem(pin, type, val_type) {}

public:
	void setup();
	void read();
};

#endif