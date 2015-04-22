//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : Relay.cpp
//  @ Date : 20.10.2014
//  @ Author : 
//
//


#include "../../interfaces/output/Relay.h"

void Relay::turnOn() {
	this->write((highIsOn) ? true : false);
}

void Relay::turnOff() {
	this->write((highIsOn) ? false : true);
}

boolean Relay::outputRead() {
	boolean val = this->DigitalIOGeneric::outputRead();
	return (highIsOn) ?  val : !val;
}

boolean Relay::implementsInterface( HardwareTypeIdentifier type ) {
	if(type == HWType_relay)
		return true;
	return false;
}

boolean Relay::readVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
	if(type == HWType_relay && result != NULL) {
		result->setUintListNum(1);
		result->getUintList()[0] = outputRead();
		
		return true;
	}
	
	return false;
}

boolean Relay::read() {
	return outputRead();
}

void Relay::write( int boolean ) {
}

void Relay::toggle() {
}


boolean Relay::writeVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
	if(type == HWType_relay && result != NULL) {
		uint8_t action = result->getUintList()[0];
		
		switch(action) {
			case 1:
				turnOn();
				return true;
			case 0:
				turnOff();
				return true;
			case 2:
				toggle();
				return true;
			default:
				return false;
		}
		
		return true;
	}
	
	return false;
}
