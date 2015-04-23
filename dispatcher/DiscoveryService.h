/*
* DiscoveryService.h
*
* Created: 24.04.2015 00:57:50
* Author: helge
*/


#ifndef __DISCOVERYSERVICE_H__
#define __DISCOVERYSERVICE_H__


class DiscoveryService {
	//variables
	public:
	protected:
	private:

	//functions
	public:
		DiscoveryService() {};
		~DiscoveryService() {};

		void doRequestFullToNode(l3_address_t remoteAddress) {};
		void doRequestFullToCompleteNetwork() {};
		void doRequestHardwareTypeToNode(l3_address_t remoteAddress, HardwareTypeIdentifier hwtype) {};
		void doRequestHardwareTypeToCompleteNetwork(HardwareTypeIdentifier hwtype) {};
	protected:
	private:

}; //DiscoveryService

#endif //__DISCOVERYSERVICE_H__
