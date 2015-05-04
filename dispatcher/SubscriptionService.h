/*
* SubscriptionService.h
*
* Created: 30.04.2015 23:43:59
* Author: helge
*/


#ifndef __SUBSCRIPTIONSERVICE_H__
#define __SUBSCRIPTIONSERVICE_H__

#include <networking/Packets.h>
#include "EventCallbackInterface.h"
#include "CommandHandler.h"
#include "HardwareInterface.h"
#include "../networking/Layer3.h"

#define numSubscriptionList 10

class SubscriptionService {
	//variables
	public:
	protected:
	private:
		/** internal list for subscriptions */
		subscription_helper_t subscriptions[numSubscriptionList];

		/** list for storing last subscription execution times */
		uint32_t subscriptionsLastExecution[numSubscriptionList];

		/** */
		HardwareInterface* hwinterface;

		/** used for subscription execution */
		CommandHandler* commandHandler;

		/** used for networking callback */
		Layer3* networking;

	//functions
	public:
		/**
		 * get all event subscriptions & poll them
		 */
		void doPollingForSubscriptions() {
			for(uint8_t i = 0; i < getSubscriptionListSize(); i++) {
				//is this an event subscription?
				if(subscriptions[i].onEvent == 1) {
					//get driver
					HardwareDriver* drv = hwinterface->getHardwareDriver((HardwareTypeIdentifier) subscriptions[i].hardwareType, subscriptions[i].hardwareAddress);

					//does it support events?
					if(drv == NULL || !drv->canDetectEvents())
						continue;

					//check for new event
					uint32_t t = drv->checkForEvent(subscriptions[i].onEventType);

					//did we detect an event? - execute subscription
					if(t > 0 && t - subscriptions[i].onEventBlackout > subscriptionsLastExecution[i]) {
						executeSubscription(&subscriptions[i]);
					}
				}
			}
		}

		/**
		 * handles a new subscriptio request
		 * @param callback
		 * @param sequence
		 * @param request type
		 * @param remote address
		 * @param application layer packet
		 */
		boolean handleRequest(EventCallbackInterface* callback, const seq_t seq, const packet_type_application_t type, const l3_address_t remote, packet_application_numbered_cmd_t* appPacket) {
			if(callback == NULL || appPacket == NULL)
				return false;

			switch(type) {
				case HARDWARE_SUBSCRIPTION_SET:
					return handleSubscriptionRequest(callback, seq, type, remote, appPacket);
				case HARDWARE_SUBSCRIPTION_INFO:
					return handleSubscriptionInfoRequest(callback, seq, type, remote, appPacket);
				default:
					return false;
			}
		}

		/**
		 * set reference for hardware drivers
		 * @param hwinterface
		 */
		void setHardwareInterface(HardwareInterface* hwinterface) {
			this->hwinterface = hwinterface;
		}

		/**
		 * set reference for command handler
		 * @param handler
		 */
		void setCommandHandler(CommandHandler* handler) {
			this->commandHandler = handler;
		}

		/**
		 * set reference for networking - used for networking callback.
		 * @param networking
		 */
		void setNetworking(Layer3* networking) {
			this->networking = networking;
		}

		/**
		 * constructor.
		 * basically sets the subcription list to 0
		 */
		SubscriptionService() {
			memset(subscriptions, 0, sizeof(subscriptions));
			memset(subscriptionsLastExecution, 0, sizeof(subscriptionsLastExecution));
		}

		/** */
		~SubscriptionService() {}

		/**
		 * get size of internal list
		 * @return size
		 */
		const static uint8_t getSubscriptionListSize() {
			return numSubscriptionList;
		}

		/**
		 * check all subscriptions and execute if timed out
		 */
		void executeSubscriptions() {
			uint32_t now = millis();

			for(uint8_t i = 0; i < numSubscriptionList; i++) {
				if(subscriptions[i].address != 0 && now - subscriptionsLastExecution[i] > subscriptions[i].millisecondsDelay) {
				//if(subscriptions[i].address != 0 && subscriptions[i].onEvent == 0 && now - subscriptionsLastExecution[i] > subscriptions[i].millisecondsDelay) {
					subscriptionsLastExecution[i] = now;
					executeSubscription(&subscriptions[i]);
				}
			}
		}

	protected:
		/**
		 * get subscription info
		 * @param filter for remote address; 0 returns all subscriptions
		 * @param buffer
		 * @param buffer len
		 * @return num subscriptions
		 */
		uint8_t getSubscriptionInfos(l3_address_t forAddress, subscription_helper_struct* buffer, uint8_t buffer_len) const {
			//sanity check.
			if(buffer == NULL || buffer_len < sizeof(subscriptions)) {
				return 0;
			}

			//iterate
			uint8_t numFound = 0;
			for(uint8_t i = 0; i < numSubscriptionList; i++) {
				if(subscriptions[i].address != 0 && (forAddress == 0 || subscriptions[i].address == forAddress)) {
					memcpy(&buffer[numFound], &subscriptions[i], sizeof(subscription_helper_t));
					numFound++;
				}
			}

			return numFound;
		}

		/**
		 * add, update or delete a subscription
		 * a subscription is determined by:
		 * - remote node
		 * - hardware type
		 * - hardware address
		 * in case no subscription exists, it is added
		 * in case a subsciption exists, it is updated
		 * a deletion is performaned in case of onError=0 (no trigger) and a delay of 0ms
		 * @param s
		 * @return success
		 */
		boolean setSubscription(subscription_helper_t* s) {
			if(s == NULL)
				return false;

			//do we know this hardware?
			if(!hwinterface->hasHardwareDriver((HardwareTypeIdentifier) s->hardwareType, s->hardwareAddress))
				return false;

			//we do not want any update and this subscription has no event trigger
			if(s->millisecondsDelay == 0 && s->onEvent == false) {
				return deleteSubscription(s);
			}

			//get current subscription index in case it exists
			uint8_t index = getSubscriptionIndex(s);

			//do we already have a subscription?
			if(index == 0xff) {
				//find free slot
				index = getFreeSubscriptionIndex();
			}

			//update
			if(index != 0xff) {
				memcpy(&subscriptions[index], s, sizeof(subscription_helper_t));
				subscriptionsLastExecution[index] = 0;
				return true;
			}

			//something went wrong or list is full.
			return false;
		}

		/**
		 * delete a subscription
		 * @param subscription info
		 * @return success
		 */
		boolean deleteSubscription(subscription_helper_t* s) {
			if(s == NULL)
				return true; //not present, it is "deleted"

			uint8_t index = getSubscriptionIndex(s);

			if(index == 0xff)
				return false;

			//delete
			memset(&subscriptions[index], 0, sizeof(subscription_helper_t));
			subscriptionsLastExecution[index] = 0;
			return true;
		}

		/**
		 * get index to a subscription
		 * @param subscription info
		 * @return index, 0xff is none found
		 */
		uint8_t getSubscriptionIndex(subscription_helper_t* s) {
			if(s != NULL) {
				for(uint8_t i = 0; i < numSubscriptionList; i++) {
					//do we have this subscription?
					if(subscriptions[i].address == s->address && subscriptions[i].hardwareAddress == s->hardwareAddress && subscriptions[i].hardwareType == s->hardwareType) {
						return i;
					}
				}
			}
			return 0xff;
		}

		/**
		 * get a free subscription index
		 * @return index, 0xff if list is full.
		 */
		uint8_t getFreeSubscriptionIndex() {
			for(uint8_t i = 0; i < numSubscriptionList; i++) {
				//do we have this subscription?
				if(subscriptions[i].address == 0)
					return i;
			}
			return 0xff;
		}

		/**
		 * handle INFO request
		 * @param callback
		 * @param seq
		 * @param type
		 * @param remote
		 * @param applayer packet (is being changed)
		 * @return success
		 */
		boolean handleSubscriptionInfoRequest(EventCallbackInterface* callback, seq_t seq, packet_type_application_t type, l3_address_t remote, packet_application_numbered_cmd_t* appPacket) const {
			if(appPacket == NULL || callback == NULL)
				return false;

			subscription_info_t* subscriptionInfo = (subscription_info_t*) appPacket->payload;

			//create buffer for info
			subscription_helper_t buffer[getSubscriptionListSize()];
			memset(buffer, 0, sizeof(buffer));

			uint8_t num = getSubscriptionInfos(subscriptionInfo->forAddress, buffer, sizeof(buffer));

			//we have no subscriptions
			if(num == 0) {
				callback->fail(seq, remote);
				return true;
			}

			//return answer
			packet_application_numbered_cmd_t newPkt;
			memset(&newPkt, 0, sizeof(newPkt));
			newPkt.packetType = HARDWARE_SUBSCRIPTION_INFO;
			subscription_info_t* info = (subscription_info_t*) newPkt.payload;
			info->forAddress = subscriptionInfo->forAddress; //not yet changed, everything is fine.

			//send one packet for each entry - currently on same sequence, should be changed. TODO!
			for(; num > 0; num--) {
				info->numInfosFollowing = num - 1;
				memcpy(&info->info, &buffer[num - 1], sizeof(subscription_helper_t));

				callback->doCallback(&newPkt, remote, seq);
			}
			return true;
		}

		/**
		 * handle SET request
		 * @param callback
		 * @param seq
		 * @param type
		 * @param remote
		 * @param applayer packet (is being changed)
		 * @return success
		 */
		boolean handleSubscriptionRequest(EventCallbackInterface* callback, const seq_t seq, const packet_type_application_t type, const l3_address_t remote, packet_application_numbered_cmd_t* appPacket) {
			if(appPacket == NULL || callback == NULL)
				return false;

			subscription_set_struct* subscriptionSet = (subscription_set_struct*) appPacket->payload;

			//set.
			boolean result = setSubscription(&subscriptionSet->info);

			//no failed
			if(!result) {
				callback->fail(seq, remote);
				return false;
			}

			//success
			appPacket->packetType = ACK;
			callback->doCallback(appPacket, remote, seq);
			return true;
		}

		/**
		 * execute a subscription
		 * setup packet and send it into system as if it were send from remote
		 * @param parameters
		 * @param success
		 */
		boolean executeSubscription(const subscription_helper_t* subscription) {
			if(networking == NULL || subscription == NULL || commandHandler == NULL)
				return false;

			//setup application packet data
			packet_application_numbered_cmd_t pan;
			memset(&pan, 0, sizeof(pan));
			//this is a hardware read
			pan.packetType = HARDWARE_COMMAND_READ;

			//the actual command gets the hardware type and address
			command_t* cmd = (command_t*) pan.payload;
			cmd->address = subscription->hardwareAddress;
			cmd->type = (HardwareTypeIdentifier) subscription->hardwareType;
			cmd->isRead = 1;

			//fire into the system - with networking callback.
			return commandHandler->handleHardwareCommand(&pan, networking->getCallbackInterface(), subscription->address, subscription->sequence);
		}


	private:

}; //SubscriptionService

#endif //__SUBSCRIPTIONSERVICE_H__
