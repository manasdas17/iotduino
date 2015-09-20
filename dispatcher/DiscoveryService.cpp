/*
* DiscoveryService.cpp
*
* Created: 24.04.2015 00:57:50
* Author: helge
*/


#include "DiscoveryService.h"

#ifdef ENABLE_DISCOVERY_SERVICE

uint8_t DiscoveryService::getDriverInterfacesAll(packet_application_numbered_discovery_info_helper_t* info, uint8_t bufSize) {
	if(info == NULL)
		return 0;

	HardwareDriver** drivers = hardwareInterface->getHardwareDrivers();
	uint8_t numDrivers = 0;

	HardwareTypeIdentifier tempArray[MAX_INTERFACES_PER_DRIVER];
	uint8_t driverListSize = hardwareInterface->getHardwareDriversListSize();
	for(uint8_t i = 0; i < driverListSize; i++) {
		if(drivers[i] != NULL) {
			//this is a driver.
			memset(tempArray, 0, sizeof(tempArray));
			void* result = drivers[i]->getImplementedInterfaces(tempArray, MAX_INTERFACES_PER_DRIVER);

			//did we get a valid result?
			if(result != NULL) {
				//iterate interfaces.
				for(uint8_t j = 0; j < MAX_INTERFACES_PER_DRIVER && numDrivers < bufSize; j++) {
					if(tempArray[j] != 0) {
						//this is an interface.
						info[numDrivers].hardwareAddress = drivers[i]->getAddress();
						info[numDrivers].hardwareType = tempArray[j];

						#ifdef ENABLE_EVENTS
							info[numDrivers].canDetectEvents = drivers[i]->canDetectEvents();
						#else
							info[numDrivers].canDetectEvents = 0;
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
	#ifdef DEBUG_HANDLER_DISCOVERY_ENABLE
		Serial.print(millis());
		Serial.println(F(": DiscoveryService::handleInfoRequest()"));
		Serial.flush();
	#endif

	if(callback == NULL || appPacket == NULL)
		return false;

	//get driver infos
	packet_application_numbered_discovery_info_helper_t interfaces[INTERFACES_BUF_SIZE];
	memset(interfaces, 0, sizeof(interfaces));
	uint8_t num = getDriverInterfacesAll(interfaces, INTERFACES_BUF_SIZE);

	//number of packets to send
	uint8_t numPerPacket = PACKET_APP_NUMBERED_DISCOVERY_DRIVERS_NUM;
	uint8_t packets = ceil((float) num / numPerPacket);

	//send packets
	for(uint8_t i = 0; i < packets; i++) {
		packet_application_numbered_cmd_t appLayerPacket;
		memset(&appLayerPacket, 0, sizeof(appLayerPacket));
		appLayerPacket.packetType = HARDWARE_DISCOVERY_RES;
		packet_application_numbered_discovery_info_t* discoveryInfo = (packet_application_numbered_discovery_info_t*) appLayerPacket.payload;

		discoveryInfo->numTotalSensors = num;

		if(i == packets -1) {
			//copy part
			discoveryInfo->numSensors = num - numPerPacket * i;
			memcpy(discoveryInfo->infos, &interfaces[i * numPerPacket], discoveryInfo->numSensors * sizeof(packet_application_numbered_discovery_info_helper_t));
		} else {
			//copy full
			memcpy(discoveryInfo->infos, &interfaces[i * numPerPacket], numPerPacket * sizeof(packet_application_numbered_discovery_info_helper_t));
			discoveryInfo->numSensors = numPerPacket;
		}

		callback->doCallback(&appLayerPacket, remote, seq++);
	}

	return true;
}

void DiscoveryService::setHardwareInterface(HardwareInterface* hwinterface) {
	this->hardwareInterface = hwinterface;
}

#endif