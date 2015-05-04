//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : MotionDetector.cpp
//  @ Date : 20.10.2014
//  @ Author :
//
//


#include "../../interfaces/input/MotionDetector.h"

boolean MotionDetector::implementsInterface( HardwareTypeIdentifier type ) {
	if(type == HWType_motion)
		return true;
	return false;
}

boolean MotionDetector::readVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
	if(implementsInterface(type)) {
		result->setUintListNum(1);
		boolean val = read();
		result->getUintList()[0] = val;

		return true;
	}

	return false;
}

boolean MotionDetector::canDetectEvents() {
	return true;
}

uint32_t MotionDetector::checkForEvent(subscription_event_type_t type) {
	//get old data
	HardwareCommandResult* last = this->getLastResult();
	const uint32_t lastT = this->getLastResultTimestamp();

	//setup temporary new values
	HardwareCommandResult newReading;
	newReading.setAddress(getAddress());
	newReading.setHardwareType(HWType_motion);

	//read data
	if(!readVal(HWType_motion, &newReading))
		return 0;

	//detect event
	boolean eventDetected = false;
	switch(type) {
		case EVENT_TYPE_EDGE_FALLING:
			if(last->getUintList()[0] > newReading.getUintList()[0]) {
				eventDetected = true;
			}
			break;

		case EVENT_TYPE_EDGE_RISING:
			if(last->getUintList()[0] < newReading.getUintList()[0]) {
				eventDetected = true;
			}
			break;

		case EVENT_TYPE_CHANGE:
		default:
			if(last->getUintList()[0] != newReading.getUintList()[0]) {
				eventDetected = true;
			}
	}

	if(eventDetected) {
		this->updateResult(&newReading);
		return lastT;
	}

	//do not update data nor timestamp
	return 0;
}
