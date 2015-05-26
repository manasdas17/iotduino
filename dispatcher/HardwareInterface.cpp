/*
* HardwareInterface.cpp
*
* Created: 13.02.2015 09:01:57
* Author: helge
*/


#include "HardwareInterface.h"

boolean HardwareInterface::registerDriver( HardwareDriver* mydriver ) {
	if(mydriver == NULL)
		return false;

	#ifdef DEBUG_HARDWARE_ENABLE
		Serial.print(millis());
		Serial.println(F(": HardwareInterface::registerDriver()"));
		Serial.flush();
	#endif

	for(uint8_t i = 0; i < driverPointerListSize; i++) {
		if(driver[i] == NULL) {
			driver[i] = mydriver;
			return true;
		}
	}

	return false;
}

boolean HardwareInterface::hasHardwareDriver( const HardwareTypeIdentifier type ) const {
	return getHardwareDriver(type) != NULL;
}

boolean HardwareInterface::hasHardwareDriver( const HardwareTypeIdentifier type, const uint8_t address ) const {
	return getHardwareDriver(type, address) != NULL;
}

HardwareDriver* HardwareInterface::getHardwareDriver( HardwareTypeIdentifier type ) const {
	#ifdef DEBUG_HARDWARE_ENABLE
		Serial.print(millis());
		Serial.print(F(": HardwareInterface::getHardwareDriver() hwType="));
		Serial.println(type);
		Serial.flush();
	#endif

	for(uint8_t i = 0; i < driverPointerListSize; i++) {
		if(driver[i] != NULL && driver[i]->implementsInterface(type)) {
			#ifdef DEBUG_HARDWARE_ENABLE
			Serial.print(F("\tfound! &drv="));
			Serial.println((uint16_t) &driver[i], HEX);
			#endif

			return driver[i];
		}
	}

	return NULL;
}

HardwareDriver* HardwareInterface::getHardwareDriver( const HardwareTypeIdentifier type, const uint8_t address ) const {
	#ifdef DEBUG_HARDWARE_ENABLE
		Serial.print(millis());
		Serial.print(F(": HardwareInterface::getHardwareDriver() hwType="));
		Serial.print(type);
		Serial.print(F(" address="));
		Serial.println(address);
		Serial.flush();
	#endif

	for(uint8_t i = 0; i < driverPointerListSize; i++) {
		if(driver[i] != NULL && driver[i]->getAddress() == address && driver[i]->implementsInterface(type)) {
			#ifdef DEBUG_HARDWARE_ENABLE
				Serial.print(F("\tfound! &drv="));
				Serial.println((uint16_t) &driver[i], HEX);
			#endif

			return driver[i];
		}
	}

	return NULL;
}

HardwareCommandResult* HardwareInterface::executeCommand(HardwareCommandResult* cmd) {
	if(cmd == NULL)
		return NULL;

	#ifdef DEBUG_HARDWARE_ENABLE
		Serial.print(millis());
		Serial.println(F(": HardwareInterface::executeCommand()"));
		Serial.print(F("\tcmd=[hwAddress="));
		Serial.print(cmd->getAddress());
		Serial.print(F(" hwType="));
		Serial.print(cmd->getHardwareType());
		Serial.print(F(" isRead="));
		Serial.print(cmd->isReadRequest());
		Serial.println(F("]"));
		Serial.flush();
	#endif

	HardwareDriver* driver = getHardwareDriver(cmd->getHardwareType(), cmd->getAddress());
	if(driver == NULL)
		driver = getHardwareDriver(cmd->getHardwareType());

	if(driver == NULL)
		return NULL;

	if(cmd->isReadRequest()) {
		#ifdef DEBUG_HARDWARE_ENABLE
			Serial.print(millis());
			Serial.println(F(": trigger read"));
		#endif
		return readHardware(driver, cmd);
	} else {
		#ifdef DEBUG_HARDWARE_ENABLE
			Serial.print(millis());
			Serial.println(F(": trigger write"));
		#endif
		return writeHardware(driver, cmd);
	}
}

HardwareCommandResult* HardwareInterface::readHardware(HardwareDriver* driver, HardwareCommandResult* cmd) {
	#ifdef DEBUG_HARDWARE_ENABLE
		Serial.print(millis());
		Serial.println(F(": HardwareInterface::readHardware()"));
	#endif

	if(cmd == NULL)
		return NULL;

	if(!driver->readVal(cmd->getHardwareType(), cmd)) {
		return NULL;
	}

	return cmd;
}

HardwareCommandResult* HardwareInterface::writeHardware(HardwareDriver* driver, HardwareCommandResult* cmd) {
	if(cmd == NULL)
		return NULL;

	if(!driver->writeVal(cmd->getHardwareType(), cmd)) {
		return NULL;
	}

	return cmd;
}

HardwareInterface::~HardwareInterface() {

}

HardwareInterface::HardwareInterface() {
	memset(driver, 0, sizeof(driver));
}
