//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : RCSwitch.cpp
//  @ Date : 20.10.2014
//  @ Author : 
//
//


#include "../../interfaces/output/RCSwitchTevionFSI07.h"

const void RCSwitchTevionFSI07::sendByte(char i, uint8_t pin) {
	switch(i) {
		case '1':
			digitalWrite(pin, HIGH);
			delayMicroseconds(lengthForTwoThird);
			digitalWrite(pin, LOW);
			delayMicroseconds(lengthForOneTHird);
			return;
		case '0':
			digitalWrite(pin, HIGH);
			delayMicroseconds(lengthForOneTHird);
			digitalWrite(pin, LOW);
			delayMicroseconds(lengthForTwoThird);
			return;
		case 'x':
			digitalWrite(pin, HIGH);
			delayMicroseconds(lengthEndHigh);
			digitalWrite(pin, LOW);
			delayMicroseconds(lengthEndLow);
			return;
	}
}

const void RCSwitchTevionFSI07::sendCode(uint32_t val) {
	uint8_t pin = getPIN();
	for(uint8_t z = 0; z < repreatNum; z++) {
		for(int8_t i = 23; i >= 0; i--) {
			sendByte((((val >> i) & 0b1) == 1) ? '1' : '0', pin);
		}
		sendByte('x', pin);
	}
}

const void RCSwitchTevionFSI07::turnOnA() {
	sendCode(this->onA);
}
const void RCSwitchTevionFSI07::turnOnB() {
	sendCode(this->onB);
}
const void RCSwitchTevionFSI07::turnOnC() {
	sendCode(this->onC);
}
const void RCSwitchTevionFSI07::turnOnD() {
	sendCode(this->onD);
}
const void RCSwitchTevionFSI07::turnOnALL() {
	sendCode(this->onALL);
}

const void RCSwitchTevionFSI07::turnOffA() {
	sendCode(this->offA);
}
const void RCSwitchTevionFSI07::turnOffB() {
	sendCode(this->offB);
}
const void RCSwitchTevionFSI07::turnOffC() {
	sendCode(this->offC);
}
const void RCSwitchTevionFSI07::turnOffD() {
	sendCode(this->offD);
}
const void RCSwitchTevionFSI07::turnOffALL() {
	sendCode(this->offALL);
}

boolean RCSwitchTevionFSI07::implementsInterface( HardwareTypeIdentifier type ) {
	if(type == HWType_rcswitch)
		return true;
	return false;
}

boolean RCSwitchTevionFSI07::writeVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
	if(implementsInterface(type) && result != NULL && result->getUintListNum() > 1) {
		
		uint8_t switchNumber = result->getUintList()[0];
		uint8_t action = result->getUintList()[1];
		
		switch(switchNumber) {
			case 1:
				if(action == 1)
					turnOnA();
				else
					turnOffA();
				break;
			case 2:
				if(action == 1)
					turnOnB();
				else
					turnOffB();
				break;
			case 3:
				if(action == 1)
					turnOnC();
				else
					turnOffC();
				break;
			case 4:
				if(action == 1)
					turnOnD();
				else
					turnOffD();
				break;
			case 5:
				if(action == 1)
					turnOnALL();
				else
					turnOffALL();
				break;
			default:
				return false;
		}

		return true;
	}

	return false;
}

HardwareTypeIdentifier* RCSwitchTevionFSI07::getImplementedInterfaces(HardwareTypeIdentifier* arr, uint8_t maxLen) {
	this->getImplementedInterfaces(arr, maxLen);
	return this->addImplementedInterface(arr, maxLen, HWType_rcswitch);
}
