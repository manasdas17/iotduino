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


#include <drivers/AnalogIOGeneric.h>

int16_t AnalogIOGeneric::read() {
	uint8_t pin = getPIN();
	pinMode(pin, INPUT);

	return analogRead(pin);
}

void AnalogIOGeneric::write(int8_t val) {
	uint8_t pin = getPIN();
	pinMode(pin, OUTPUT);
	analogWrite(pin, val);
}

int16_t AnalogIOGeneric::readCalibrated() {
	int16_t read = this->read();
	return (int16_t) read - zeroVal;
}

void AnalogIOGeneric::readCalibrated(HardwareCommandResult* hwresult) {
	hwresult->setInt16ListNum(1);
	hwresult->getUint16List()[0] = readCalibrated();
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
		readCalibrated(result);
		return true;
	}

	return false;
}

boolean AnalogIOGeneric::writeVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
	if(type == HWType_ANALOG && result != NULL && result->getInt16ListNum() > 0) {
		int16_t val = result->getInt16List()[0];
		write(val);

		return true;
	}

	return false;
}
