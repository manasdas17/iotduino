/*
* DiscoveryService.h
*
* Created: 24.04.2015 00:57:50
* Author: helge
*/


#ifndef __DISCOVERYSERVICE_H__
#define __DISCOVERYSERVICE_H__

#include <networking/Packets.h>

class DiscoveryService {
	//variables
	public:
	protected:
	private:
		HardwareInterface* hardwareInterface;

	//functions
	public:
		DiscoveryService() {};
		~DiscoveryService() {};

		void setHardwareInterface(HardwareInterface* hwinterface) {
			this->hardwareInterface = hwinterface;
		}


		boolean handleInfoRequest(packet_application_numbered_cmd_t* appPacket, EventCallbackInterface* callback, l3_address_t remote, seq_t seq) {
			#ifdef DEBUG_HANDLER_ENABLE
				Serial.print(millis());
				Serial.println(F(": DiscoveryService::handleInfoRequest()"));
				Serial.flush();
			#endif

			//create response - add info in place.
			//info
			packet_application_numbered_discovery_info_t* info = (packet_application_numbered_discovery_info_t*) appPacket->payload;
			memset(&info, 0, sizeof(info));
			uint8_t num = getDriverInterfacesWithAddresses(info);
			info->numSensors = num;

			//packet
			packet_application_numbered_cmd_t appLayerPacket;
			appLayerPacket.packetType = HARDWARE_DISCOVERY_RES;
			memcpy(appLayerPacket.payload, &info, sizeof(info));
			callback->doCallback(&appLayerPacket, remote, seq);

			return true;
		}

		/**
		 * @param application packet
		 *
		 */
		uint8_t getDriverInterfacesWithAddresses(packet_application_numbered_discovery_info_t* info) {
			HardwareDriver** drivers = hardwareInterface->getHardwareDrivers();
			uint8_t numDrivers = 0;

			#define tempArraySize 5
			HardwareTypeIdentifier tempArray[tempArraySize];
			for(uint8_t i = 0; i < hardwareInterface->getHardwareDriversListSize(); i++) {
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


		void doRequestFullToNode(l3_address_t remoteAddress) {};
		void doRequestFullToCompleteNetwork() {};
		void doRequestHardwareTypeToNode(l3_address_t remoteAddress, HardwareTypeIdentifier hwtype) {};
		void doRequestHardwareTypeToCompleteNetwork(HardwareTypeIdentifier hwtype) {};
	protected:
	private:

}; //DiscoveryService

#endif //__DISCOVERYSERVICE_H__
