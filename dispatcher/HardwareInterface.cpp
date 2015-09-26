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
		Serial.print(F(": HWIntf::regDrv() hwadr="));
		Serial.println(mydriver->getAddress());
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
		Serial.print(F(": HWIntf::getHWDrv() hwT="));
		Serial.println(type);
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
		Serial.print(F(": HWIntf::getHWDrv() hwT="));
		Serial.print(type);
		Serial.print(F(" adr="));
		Serial.println(address);
	#endif

	for(uint8_t i = 0; i < driverPointerListSize; i++) {
		if(driver[i] != NULL && driver[i]->getAddress() == address && driver[i]->implementsInterface(type)) {
			#ifdef DEBUG_HARDWARE_ENABLE
				Serial.print(F("\t&drv="));
				Serial.println((uint16_t) &driver[i], HEX);
			#endif

			return driver[i];
		}
	}

	return NULL;
}

boolean HardwareInterface::executeCommand(HardwareCommandResult* cmd) {
	if(cmd == NULL)
		return false;

	#ifdef DEBUG_HARDWARE_ENABLE
		Serial.print(millis());
		Serial.println(F(": HWIntf::excCmd()"));
		Serial.print(F("\tcmd=[hwAdr="));
		Serial.print(cmd->getAddress());
		Serial.print(F(" hwT="));
		Serial.print(cmd->getHardwareType());
		Serial.print(F(" isR="));
		Serial.print(cmd->isReadRequest());
		Serial.println(F("]"));
	#endif

	HardwareDriver* driver = getHardwareDriver(cmd->getHardwareType(), cmd->getAddress());
	if(driver == NULL)
		driver = getHardwareDriver(cmd->getHardwareType());

	if(driver == NULL)
		return false;

	if(cmd->isReadRequest()) {
		#ifdef DEBUG_HARDWARE_ENABLE
			Serial.print(millis());
			Serial.println(F(": read"));
		#endif
		return readHardware(driver, cmd);
	} else {
		#ifdef DEBUG_HARDWARE_ENABLE
			Serial.print(millis());
			Serial.println(F(": write"));
		#endif
		return writeHardware(driver, cmd);
	}
}

boolean HardwareInterface::readHardware(HardwareDriver* driver, HardwareCommandResult* cmd) {
	#ifdef DEBUG_HARDWARE_ENABLE
		Serial.print(millis());
		Serial.println(F(": HWIntf::readHW()"));
	#endif

	if(cmd == NULL)
		return false;

	return driver->readVal(cmd->getHardwareType(), cmd);
}

boolean HardwareInterface::writeHardware(HardwareDriver* driver, HardwareCommandResult* cmd) {
	if(cmd == NULL)
		return false;

	return driver->writeVal(cmd->getHardwareType(), cmd);
}