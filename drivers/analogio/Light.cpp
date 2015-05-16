/*
 * Light.cpp
 *
 * Created: 14.05.2015 15:18:44
 *  Author: helge
 */

#include "../../interfaces/input/Light.h"

boolean Light::implementsInterface( HardwareTypeIdentifier type ) {
	if(type == HWType_light)
		return true;
	return false;
}

boolean Light::readVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
	if(type == HWType_light && result != NULL)
		return AnalogIOGeneric::readVal(type, result);
	return false;
}

#ifdef ENABLE_EVENTS
boolean Light::canDetectEvents() {
	return true;
}

subscription_event_type_t Light::eventLoop() {
	//get old data
	HardwareCommandResult* last = this->getLastResult();

	//setup temporary new values
	HardwareCommandResult newReading;

	//read data
	if(!readVal(HWType_light, &newReading))
		return EVENT_TYPE_DISABLED;

	//detect event
	subscription_event_type_t eventDetected = AnalogIO::checkForEvent(&newReading, last->getInt16List()[0], newReading.getInt16List()[0], (int16_t) EVENT_MIN_DIFF);

	#ifdef DEBUG_HARDWARE_ENABLE
	if(eventDetected != EVENT_TYPE_DISABLED) {
		Serial.print(millis());
		Serial.print(F(": Light::eventLoop() has found event type="));
		Serial.println(eventDetected);
		Serial.flush();
	}
	#endif

	return eventDetected;
}
#endif