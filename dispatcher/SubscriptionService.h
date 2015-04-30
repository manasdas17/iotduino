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

#define numSubscriptionList 10

class SubscriptionService {
	//variables
	public:
	protected:
	private:
		subscriptopn_helper_t subscriptions[numSubscriptionList];

	//functions
	public:
		SubscriptionService() {
			memset(subscriptions, 0, sizeof(subscriptions));
		}

		~SubscriptionService() {}

		/**
		 * get size of internal list
		 * @return size
		 */
		inline static uint8_t getSubscriptionListeSize() {
			return numSubscriptionList;
		}

		/**
		 *
		 */
		boolean handleRequest(EventCallbackInterface* callback, seq_t seq, packet_type_application type, l3_address_t remote, packet_application_numbered_cmd_t* appPacket) {
			switch(type) {
				case HARDWARE_SUBSCRIPTION_INFO:
					return handleSubscriptionInfoRequest(callback, seq, type, remote, appPacket);
				case HARDWARE_SUBSCRIPTION_SET:
					return handleSubscriptionRequest(callback, seq, type, remote, appPacket);
				default:
					return false;
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
		uint8_t getSubscriptionInfos(l3_address_t remote, subscription_helper_struct* buffer, uint8_t buffer_len) {
			//sanity check.
			if(buffer_len < sizeof(subscriptions)) {
				return 0;
			}

			//iterate
			uint8_t numFound = 0;
			for(uint8_t i = 0; i < numSubscriptionList; i++) {
				if(remote != 0 && subscriptions[i].address == remote) {
					memcpy(&buffer, &subscriptions[i], sizeof(subscriptopn_helper_t));
					numFound++;
				}
			}

			return numFound;
		}

		boolean setSubscription(subscriptopn_helper_t* s) {
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
				memcpy(&subscriptions[index], s, sizeof(subscriptopn_helper_t));
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
		boolean deleteSubscription(subscriptopn_helper_t* s) {
			uint8_t index = getSubscriptionIndex(s);

			if(index == 0xff)
				return false;

			//delete
			memset(&subscriptions[index], 0, sizeof(subscriptopn_helper_t));
			return true;
		}

		/**
		 * get index to a subscription
		 * @param subscription info
		 * @return index, 0xff is none found
		 */
		uint8_t getSubscriptionIndex(subscriptopn_helper_t* s) {
			for(uint8_t i = 0; i < numSubscriptionList; i++) {
				//do we have this subscription?
				if(subscriptions[i].address == s->address && subscriptions[i].hardwareAddress == s->hardwareAddress && subscriptions[i].hardwareType == s->hardwareType) {
					return i;
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
		boolean handleSubscriptionInfoRequest(EventCallbackInterface* callback, seq_t seq, packet_type_application type, l3_address_t remote, packet_application_numbered_cmd_t* appPacket) {
			subscription_info_t* subscriptionInfo = (subscription_info_t*) appPacket->payload;

			subscriptopn_helper_t buffer[getSubscriptionListeSize()];
			memset(buffer, 0, sizeof(buffer));

			uint8_t num = getSubscriptionInfos(subscriptionInfo->forAddress, buffer, sizeof(buffer));

			if(num == 0) {
				callback->fail(seq, remote);
				return true;
			}

			packet_application_numbered_cmd_t newPkt;
			memset(&newPkt, 0, sizeof(newPkt));
			newPkt.packetType = HARDWARE_SUBSCRIPTION_INFO;
			subscription_info_t* info = (subscription_info_t*) newPkt.payload;
			info->forAddress = subscriptionInfo->forAddress; //not yet changed, everything is fine.

			for(; num > 0; num--) {
				info->numInfosFollowing = num - 1;
				memcpy(&info->info, &buffer[num - 1], sizeof(subscriptopn_helper_t));

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
		boolean handleSubscriptionRequest(EventCallbackInterface* callback, seq_t seq, packet_type_application type, l3_address_t remote, packet_application_numbered_cmd_t* appPacket) {
			subscription_set_struct* subscriptionSet = (subscription_set_struct*) appPacket->payload;

			boolean result = setSubscription(&subscriptionSet->info);

			if(!result) {
				callback->fail(seq, remote);
				return false;
			}

			appPacket->packetType = ACK;
			callback->doCallback(appPacket, remote, seq);
			return true;
		}

	private:

}; //SubscriptionService

#endif //__SUBSCRIPTIONSERVICE_H__
