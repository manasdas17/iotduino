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

boolean MotionDetector::canDetectEvents() {
	return true;
}

subscription_event_type_t MotionDetector::eventLoop() {
	//get old data
	HardwareCommandResult* last = this->getLastResult();

	//setup temporary new values
	HardwareCommandResult newReading;

	//read data
	if(!readVal(HWType_motion, &newReading))
		return EVENT_TYPE_DISABLED;

	//detect event
	subscription_event_type_t eventDetected = EventDetector::checkForEvent(&newReading, last->getUintList()[0], newReading.getUintList()[0]);

	#ifdef DEBUG_HARDWARE_ENABLE
		if(eventDetected != EVENT_TYPE_DISABLED) {
			Serial.print(millis());
			Serial.print(F(": MotionDetector::eventLoop() has found event type="));
			Serial.println(eventDetected);
			Serial.flush();
		}
	#endif

	return eventDetected;
}
