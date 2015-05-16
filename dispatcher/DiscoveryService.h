/*
* DiscoveryService.h
*
* Created: 24.04.2015 00:57:50
* Author: helge
*/


#ifndef __DISCOVERYSERVICE_H__
#define __DISCOVERYSERVICE_H__

#include <networking/Packets.h>
#include <dispatcher/EventCallbackInterface.h>
#include <dispatcher/HardwareInterface.h>

#ifdef ENABLE_DISCOVERY_SERVICE

class DiscoveryService {
	//variables
	public:
	protected:
	private:
		HardwareInterface* hardwareInterface;

	//functions
	public:
		DiscoveryService() {}
		~DiscoveryService() {}

		/**
		 * set hardware interface - needed for local device hardware lookup
		 * @param interface
		 */
		void setHardwareInterface(HardwareInterface* hwinterface);

		/**
		 * handle a request - does hardware lookup and gives result to callback function
		 * @param callback function
		 * @param sequence
		 * @param packet type (should be hardware discovery request)
		 * @param l3 remote address
		 * @param applayer packet (will be reused, i.e. modified)
		 */
		boolean handleInfoRequest(EventCallbackInterface* callback, seq_t seq, packet_type_application_t type, l3_address_t remote, packet_application_numbered_cmd_t* appPacket);

	protected:
		/**
		 * @param application packet
		 */
		uint8_t getDriverInterfacesAll(packet_application_numbered_discovery_info_t* info);

	private:

}; //DiscoveryService

#endif //ENABLE_DISCOVERY_SERVICE
#endif //__DISCOVERYSERVICE_H__
