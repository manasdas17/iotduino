/*
* HardwareInterface.cpp
*
* Created: 13.02.2015 09:01:57
* Author: helge
*/


#include "HardwareInterface.h"

//#include "interfaces/input/Accelerometer.h"
//#include "interfaces/input/Button.h"
//#include "interfaces/input/Gyroscope.h"
//#include "interfaces/input/Humidity.h"
//#include "interfaces/input/IRreceive.h"
//#include "interfaces/input/KeyPad.h"
//#include "interfaces/input/MagneticField.h"
//#include "interfaces/input/Methane.h"
//#include "interfaces/input/MotionDetector.h"
//#include "interfaces/input/Pressure.h"
////#include "interfaces/input/RTC.h"
//#include "interfaces/input/Sonar.h"
//#include "interfaces/input/Switch.h"
//#include "interfaces/input/Temperature.h"
//#include "interfaces/input/Tochpad.h"
//
//#include "interfaces/output/Relay.h"
//#include "interfaces/output/LED.h"
//#include "interfaces/output/RCSwitchTevionFSI07.h"

#include "drivers/digitalio/DHT11.h"

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

uint8_t HardwareInterface::getFreeResultIndex() const {
	for(uint8_t i = 0; i < resultSetSize; i++) {
		if(!resultSetInUse[i])
			return i;
	}

	return 255;
}

HardwareCommandResult* HardwareInterface::readHardware(HardwareDriver* driver, HardwareCommandResult* cmd) {
	#ifdef DEBUG_HARDWARE_ENABLE
		Serial.print(millis());
		Serial.println(F(": HardwareInterface::readHardware()"));
	#endif

	HardwareCommandResult* res = getFreeHardwareCommandResultEntry();

	if(res == NULL)
		return NULL;

	driver->readVal(cmd->getHardwareType(), res);

	res->setAddress(cmd->getAddress());
	res->setHardwareType(cmd->getHardwareType());
	res->setReadRequest(cmd->isReadRequest());

	return res;
}

HardwareCommandResult* HardwareInterface::writeHardware(HardwareDriver* driver, HardwareCommandResult* cmd) {
	HardwareCommandResult* res = getFreeHardwareCommandResultEntry();

	if(res == NULL)
		return NULL;

	driver->writeVal(cmd->getHardwareType(), res);
	res->setAddress(cmd->getAddress());
	res->setHardwareType(cmd->getHardwareType());

	return res;
}

HardwareInterface::~HardwareInterface() {

}

boolean HardwareInterface::releaseHardwareCommandResultEntry(HardwareCommandResult* ptr) {
	//get id
	for(uint8_t i = 0; i < resultSetSize; i++) {
		if(ptr == &resultset[i] && resultSetInUse[i] == true) {

			#ifdef DEBUG_HARDWARE_ENABLE
				Serial.print(millis());
				Serial.print(F(": HardwareInterface::releaseHardwareCommandResultEntry() index="));
				Serial.println(i);
				Serial.flush();
			#endif

			resultset[i].reset();
			resultSetInUse[i] = false;

			return true;
		}
	}

	return false;
}

HardwareCommandResult* HardwareInterface::getFreeHardwareCommandResultEntry() {
	uint8_t freeIndex = getFreeResultIndex();

	if(freeIndex == 0xff)
		return NULL;

	resultSetInUse[freeIndex] = true;
	return &resultset[freeIndex];
}
