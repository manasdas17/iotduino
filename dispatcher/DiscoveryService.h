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

#define INTERFACES_BUF_SIZE 16

class DiscoveryService {
	//variables
	public:
	protected:
	private:
		HardwareInterface* hardwareInterface;

	//functions
	public:
		void init() {}

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
		 * @param info list
		 * @param bufSize
		 * @return num interfaces
		 */
		uint8_t getDriverInterfacesAll(packet_application_numbered_discovery_info_helper_t* info, uint8_t bufSize);

	private:

}; //DiscoveryService

#endif //__DISCOVERYSERVICE_H__
