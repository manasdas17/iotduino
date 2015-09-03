/*
* PacketDispatcher.h
*
* Created: 14.04.2015 23:24:41
* Author: helge
*/


#ifndef __PACKETDISPATCHER_H__
#define __PACKETDISPATCHER_H__

#include <networking/Packets.h>
#include <dispatcher/HardwareInterface.h>
#include <dispatcher/ResponseHandler.h>
#include <dispatcher/CommandHandler.h>
#include <dispatcher/DiscoveryService.h>
#include <dispatcher/SubscriptionService.h>

class PacketDispatcher {
	//variables
	public:
	protected:
	private:
		//modules for handling different packets
		CommandHandler commandHandler;
		ResponseHandler responseHandler;

		#ifdef ENABLE_DISCOVERY_SERVICE
			DiscoveryService discoveryService;
		#endif

		#ifdef ENABLE_SUBSCRIPTION_SERVICE
			SubscriptionService subscriptionService;
		#endif

		//network
		Layer3* networking;

	//functions
	public:
		void init() {
			commandHandler.init();
			responseHandler.init();
			subscriptionService.init();
			discoveryService.init();
		}

		/**
		 * initialise this class
		 * @param networking
		 * @param hwinterface
		 */
		void init(Layer3* networking, HardwareInterface* hwinterface);

		ResponseHandler* getResponseHandler() {
			return &responseHandler;
		}

		/**
		 * this loop periodically queries the network for new packets
		 * by now, we only handle numbered packets.
		 */
		void loop();

		/**
		 * gather necessary network information from packet and pass it to the actual handling method
		 * we presume having a numbered packet here.
		 * @param l3 network packet
		 * @return success
		 */
		boolean handleNumberedFromNetwork( Layer3::packet_t packet );

		/**
		 * gather necessary network information from packet and pass it to the actual handling method
		 * we presume having a numbered packet here.
		 * @param l3 network packet
		 * @return success
		 */
		boolean handleUnNumberedFromNetwork( Layer3::packet_t packet );

		/**
		 * handle a numbered packet.
		 * we by now only support hardware reads, writes and acks.
		 * @param sequence
		 * @param application packet type
		 * @param remote address
		 * @param actual application packet
		 * @return success
		 */
		boolean handleNumbered( const seq_t seq, const packet_type_application_t type, const l3_address_t remote, packet_application_numbered_cmd_t* appPacket);

		/**
		 * handle a UNnumbered packet.
		 * TODO!
		 * @param application packet type
		 * @param remote address
		 * @param actual application packet
		 * @return success
		 */
		boolean handleUnNumbered( const packet_type_application_t type, const l3_address_t remote, packet_application_unnumbered_cmd_t* appPacket) const;

		#ifdef ENABLE_SUBSCRIPTION_SERVICE
		/**
		 * @return service
		 */
		SubscriptionService* getSubscriptionsService();
		#endif

		#ifdef ENABLE_DISCOVERY_SERVICE
		/**
		 * @return service
		 */
		DiscoveryService* getDiscoveryService();
		#endif
}; //PacketDispatcher

#endif //__PACKETDISPATCHER_H__
