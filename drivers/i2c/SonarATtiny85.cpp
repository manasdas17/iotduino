//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : Sonar.cpp
//  @ Date : 20.10.2014
//  @ Author :
//
//


#include "../../interfaces/input/Sonar.h"

uint8_t Sonar::read() {
	Wire.requestFrom(this->address, 1);
	uint8_t val = Wire.read();
	return (val != 0xff) ? val : 0;
}

boolean Sonar::readVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
	if(implementsInterface(type)) {
		result->setUint8ListNum(1);
		result->getUint8List()[0] = read();

		return true;
	}

	return false;
}

boolean Sonar::implementsInterface( HardwareTypeIdentifier type ) {
	if(type == HWType_sonar)
		return true;
	return false;
}

