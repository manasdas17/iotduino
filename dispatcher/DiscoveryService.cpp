/*
* DiscoveryService.cpp
*
* Created: 24.04.2015 00:57:50
* Author: helge
*/


#include "DiscoveryService.h"

#ifdef ENABLE_DISCOVERY_SERVICE

uint8_t DiscoveryService::getDriverInterfacesAll(packet_application_numbered_discovery_info_t* info) {
	if(info == NULL)
		return 0;

	HardwareDriver** drivers = hardwareInterface->getHardwareDrivers();
	uint8_t numDrivers = 0;

	#define tempArraySize 5
	HardwareTypeIdentifier tempArray[tempArraySize];
	uint8_t driverListSize = hardwareInterface->getHardwareDriversListSize();
	for(uint8_t i = 0; i < driverListSize; i++) {
		if(drivers[i] != NULL) {
			//this is a driver.
			memset(tempArray, 0, sizeof(tempArray));
			void* result = drivers[i]->getImplementedInterfaces(tempArray, tempArraySize);

			//did we get a valid result?
			if(result != NULL) {
				//iterate interfaces.
				for(uint8_t j = 0; j < tempArraySize; j++) {
					if(tempArray[j] != 0) {
						//this is an interface.
						info->infos[numDrivers].hardwareAddress = drivers[i]->getAddress();
						info->infos[numDrivers].hardwareType = tempArray[j];

						#ifdef ENABLE_EVENTS
							info->infos[numDrivers].canDetectEvents = drivers[i]->canDetectEvents();
						#else
							info->infos[numDrivers].canDetectEvents = 0;
						#endif

						numDrivers++;
					} else {
						break;
					}
				}
			}
		}
	}
	return numDrivers;
}

boolean DiscoveryService::handleInfoRequest(EventCallbackInterface* callback, seq_t seq, packet_type_application_t type, l3_address_t remote, packet_application_numbered_cmd_t* appPacket) {
	#ifdef DEBUG_HANDLER_ENABLE
		Serial.print(millis());
		Serial.println(F(": DiscoveryService::handleInfoRequest()"));
		Serial.flush();
	#endif

	if(callback == NULL || appPacket == NULL)
		return false;

	//create response - add info in place.
	//info
	packet_application_numbered_discovery_info_t* info = (packet_application_numbered_discovery_info_t*) appPacket->payload;
	//memset(info, 0, sizeof(info)); //should not be necessary
	uint8_t num = getDriverInterfacesAll(info);
	info->numSensors = num;

	//packet
	packet_application_numbered_cmd_t appLayerPacket;
	appLayerPacket.packetType = HARDWARE_DISCOVERY_RES;
	memcpy(appLayerPacket.payload, &info, sizeof(info));
	callback->doCallback(&appLayerPacket, remote, seq);

	return true;
}

void DiscoveryService::setHardwareInterface(HardwareInterface* hwinterface) {
	this->hardwareInterface = hwinterface;
}

#endif