/*
* PacketDispatcher.h
*
* Created: 14.04.2015 23:24:41
* Author: helge
*/


#ifndef __PACKETDISPATCHER_H__
#define __PACKETDISPATCHER_H__

#include "../networking/Packets.h"
#include "ResponseHandler.h"
#include "CommandHandler.h"
#include "DiscoveryService.h"
#include "SubscriptionService.h"

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
		PacketDispatcher(Layer3* networking, HardwareInterface* hwinterface) {
			this->networking = networking;
			this->commandHandler.setHardwareInterface(hwinterface);

			#ifdef ENABLE_DISCOVERY_SERVICE
			this->discoveryService.setHardwareInterface(hwinterface);
			#endif

			#ifdef ENABLE_SUBSCRIPTION_SERVICE
			this->subscriptionService.setHardwareInterface(hwinterface);
			this->subscriptionService.setCommandHandler(&commandHandler);
			this->subscriptionService.setNetworking(networking);
			#endif
		}

		~PacketDispatcher() {

		}

		/**
		 * this loop periodically queries the network for new packets
		 * by now, we only handle numbered packets.
		 */
		void loop() {
			//#ifdef DEBUG_HANDLER_ENABLE
				//Serial.print(millis());
				//Serial.println(F(": PacketDispatcher::loop()"));
				//Serial.flush();
			//#endif

			//networking
			while(networking->receiveQueueSize() > 0) {
				Layer3::packet_t packet;
				networking->receiveQueuePop(&packet);

				#ifdef DEBUG_HANDLER_ENABLE
					Serial.print(millis());
					Serial.print(F(":\treceived packetType="));
					Serial.println(packet.data.type);
					Serial.flush();
				#endif

				switch(packet.data.type) {
					case PACKET_ACK:
						continue;
					case PACKET_NUMBERED:
						handleNumberedFromNetwork(packet);
						break;
					case PACKET_UNNUMBERED:
						handleUnNumberedFromNetwork(packet);
						break;
					case PACKET_BEACON:
					default:
						continue;
				}
			}

			#ifdef ENABLE_SUBSCRIPTION_SERVICE
			////subscriptions
			//actual timed subscriptions
			subscriptionService.executeSubscriptions();
			//event detection
				#ifdef ENABLE_EVENTS
				subscriptionService.doPollingForSubscriptions();
				#endif
			#endif
		}

		/**
		 * gather necessary network information from packet and pass it to the actual handling method
		 * we presume having a numbered packet here.
		 * @param l3 network packet
		 * @return success
		 */
		boolean handleNumberedFromNetwork( Layer3::packet_t packet ) {
			packet_numbered_t* numbered = (packet_numbered_t*) packet.data.payload;
			packet_application_numbered_cmd_t* appPacket = (packet_application_numbered_cmd_t*) numbered->payload;

			packet_type_application_t type = (packet_type_application_t) appPacket->packetType;
			seq_t seq = numbered->seqNumber;
			l3_address_t remote = packet.data.source;

			return handleNumbered(seq, type, remote, appPacket);
		}

		/**
		 * gather necessary network information from packet and pass it to the actual handling method
		 * we presume having a numbered packet here.
		 * @param l3 network packet
		 * @return success
		 */
		boolean handleUnNumberedFromNetwork( Layer3::packet_t packet ) {
			packet_unnumbered_t* unnumbered = (packet_unnumbered_t*) packet.data.payload;
			packet_application_unnumbered_cmd_t* appPacket = (packet_application_unnumbered_cmd_t*) unnumbered->payload;

			packet_type_application_t type = (packet_type_application_t) appPacket->packetType;
			l3_address_t remote = packet.data.source;

			return handleUnNumbered(type, remote, appPacket);
		}

		/**
		 * handle a numbered packet.
		 * we by now only support hardware reads, writes and acks.
		 * @param sequence
		 * @param application packet type
		 * @param remote address
		 * @param actual application packet
		 * @return success
		 */
		boolean handleNumbered( const seq_t seq, const packet_type_application_t type, const l3_address_t remote, packet_application_numbered_cmd_t* appPacket) {
			#ifdef DEBUG_HANDLER_ENABLE
				Serial.print(millis());
				Serial.print(F(": PacketDispatcher::handleNumbered() type="));
				Serial.println(type);
				Serial.flush();
			#endif
			switch(type) {
				case HARDWARE_COMMAND_WRITE:
				case HARDWARE_COMMAND_READ:
					return commandHandler.handleNumbered(networking->getCallbackInterface(), seq, type, remote, appPacket);

			#ifdef ENABLE_DISCOVERY_SERVICE
				case HARDWARE_DISCOVERY_REQ:
					return discoveryService.handleInfoRequest(networking->getCallbackInterface(), seq, type, remote, appPacket);
				case HARDWARE_DISCOVERY_RES:
					return responseHandler.handleReponseNumbered(seq, type, remote, appPacket);
			#endif

			#ifdef ENABLE_SUBSCRIPTION_SERVICE
				case HARDWARE_SUBSCRIPTION_SET:
				case HARDWARE_SUBSCRIPTION_INFO:
					return subscriptionService.handleRequest(networking->getCallbackInterface(), seq, type, remote, appPacket);
			#endif

				case ACK:
				case NACK:
					return responseHandler.handleReponseNumbered(seq, type, remote, appPacket);

				default:
					return false;
			}
		}

		/**
		 * handle a UNnumbered packet.
		 * TODO!
		 * @param application packet type
		 * @param remote address
		 * @param actual application packet
		 * @return success
		 */
		boolean handleUnNumbered( const packet_type_application_t type, const l3_address_t remote, packet_application_unnumbered_cmd_t* appPacket) const {
			switch(type) {
				default:
					return false;
			}
		}

		#ifdef ENABLE_SUBSCRIPTION_SERVICE
		/**
		 * @return service
		 */
		SubscriptionService* getSubscriptionsService() {
			return &subscriptionService;
		}
		#endif

		#ifdef ENABLE_DISCOVERY_SERVICE
		/**
		 * @return service
		 */
		DiscoveryService* getDiscoveryService() {
			return &discoveryService;
		}
		#endif
}; //PacketDispatcher

#endif //__PACKETDISPATCHER_H__
