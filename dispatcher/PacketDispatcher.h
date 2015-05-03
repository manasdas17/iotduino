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
		DiscoveryService discoveryService;
		SubscriptionService subscriptionService;

		//network
		Layer3* networking;

	//functions
	public:
		PacketDispatcher(Layer3* networking, HardwareInterface* hwinterface) {
			this->networking = networking;
			this->commandHandler.setHardwareInterface(hwinterface);
			this->discoveryService.setHardwareInterface(hwinterface);
			this->subscriptionService.setHardwareInterface(hwinterface);
			this->subscriptionService.setCommandHandler(&commandHandler);
			this->subscriptionService.setNetworking(networking);
		}

		~PacketDispatcher() {

		}

		/**
		 * this loop periodically queries the network for new packets
		 * by now, we only handle numbered packets.
		 */
		void loop() {
			#ifdef DEBUG_HANDLER_ENABLE
				Serial.print(millis());
				Serial.println(F("PacketDispatcher::loop()"));
				Serial.flush();
			#endif

			//networking
			while(l3->receiveQueueSize() > 0) {
				Layer3::packet_t packet;
				l3->receiveQueuePop(&packet);

				#ifdef DEBUG_HANDLER_ENABLE
					Serial.print(millis());
					Serial.print(F("\treceived packetType="));
					Serial.println(packet.data.type);
					Serial.flush();
				#endif

				switch(packet.data.type) {
					case PACKET_NUMBERED:
							handleNumberedFromNetwork(packet);
						break;
					case PACKET_UNNUMBERED:
							handleUnNumberedFromNetwork(packet);
						break;
					case PACKET_ACK:
					case PACKET_BEACON:
					default:
						continue;
				}
			}

			//subscriptions
			subscriptionService.executeSubscriptions();
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
		boolean handleUnNumberedFromNetwork( Layer3::packet_t packet ) const {
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
			switch(type) {
				case HARDWARE_COMMAND_READ:
				case HARDWARE_COMMAND_WRITE:
					return commandHandler.handleNumbered(l3->getCallbackInterface(), seq, type, remote, appPacket);

				case ACK:
				case NACK:
					return responseHandler.handleReponseNumbered(seq, type, remote, appPacket);

				case HARDWARE_SUBSCRIPTION_SET:
				case HARDWARE_SUBSCRIPTION_INFO:
					return subscriptionService.handleRequest(l3->getCallbackInterface(), seq, type, remote, appPacket);

				case HARDWARE_DISCOVERY_REQ:
					return discoveryService.handleInfoRequest(l3->getCallbackInterface(), seq, type, remote, appPacket);
				case HARDWARE_DISCOVERY_RES:

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

		/**
		 * @return service
		 */
		SubscriptionService* getSubscriptionsService() {
			return &subscriptionService;
		}

		/**
		 * @return service
		 */
		DiscoveryService* getDiscoveryService() {
			return &discoveryService;
		}
}; //PacketDispatcher

#endif //__PACKETDISPATCHER_H__
