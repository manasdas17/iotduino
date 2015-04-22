//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : AnalogIOGeneric.cpp
//  @ Date : 20.10.2014
//  @ Author : 
//
//


#include "../../interfaces/AnalogIOGeneric.h"

uint8_t AnalogIOGeneric::read() {
	uint8_t pin = getPIN();
	pinMode(pin, INPUT);
	
	return analogRead(pin);
}
	
void AnalogIOGeneric::write(uint8_t val) {
	if(val > 0xff) val = 0xff;
	
	uint8_t pin = getPIN();
	pinMode(pin, OUTPUT);
	analogWrite(pin, val);
}

int16_t AnalogIOGeneric::readCalibrated() {
	uint8_t read = this->read();
	return (int16_t) read - zeroVal;
}

boolean AnalogIOGeneric::calibrateZero() {
	uint16_t measurements = 0;
	for(uint8_t i = 0; i < CALIBRATION_MEASUREMENTS_NUM; i++) {
		measurements += this->read();
	}
	
	this->zeroVal = measurements / CALIBRATION_MEASUREMENTS_NUM;
	
	return true;
}

boolean AnalogIOGeneric::implementsInterface( HardwareTypeIdentifier type ) {
	if(type == HWType_ANALOG)
		return true;
	return false;
}

boolean AnalogIOGeneric::readVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
	if(implementsInterface(type)) {
		result->setFloatListNum(1);
		result->getFloatList()[0] = readCalibrated();
		return true;
	}
	
	return false;
}

boolean AnalogIOGeneric::writeVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
	if(type == HWType_ANALOG && result != NULL) {
		uint8_t val = result->getUintList()[0];
		write(val);
		
		return true;
	}
	
	return false;
}
