/*
 * Humidity.h
 *
 * Created: 11.11.2014 22:11:44
 *  Author: helge
 */


#ifndef HUMIDITY_H_
#define HUMIDITY_H_


#include <Arduino.h>
#include <drivers/HardwareID.h>
#include <dispatcher/Commands.h>

class Humidity {
	public:
		virtual int8_t readHumidity()=0;
		virtual void readHumidity(HardwareCommandResult* hwresult)=0;
};



#endif /* HUMIDITY_H_ */