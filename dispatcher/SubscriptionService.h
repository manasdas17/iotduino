/*
* SubscriptionService.h
*
* Created: 30.04.2015 23:43:59
* Author: helge
*/


#ifndef __SUBSCRIPTIONSERVICE_H__
#define __SUBSCRIPTIONSERVICE_H__

#include <Configuration.h>
#include <networking/Layer3.h>
#include <dispatcher/CommandHandler.h>
#include <dispatcher/HardwareInterface.h>
#include <ramManager.h>

extern SPIRamManager ram;

class SubscriptionService {
	//variables
	public:
	protected:
	private:
		#ifdef ENABLE_EXTERNAL_RAM
			uint8_t memRegionSubscriptions;
			uint8_t memRegionSubscriptionLastExecutions;
			uint8_t memRegionSubscriptionsTmpBuffer;
		#else
			/** internal list for subscriptions */
			subscription_helper_t subscriptions[numSubscriptionList];

			/** list for storing last subscription execution times */
			uint32_t subscriptionsLastExecution[numSubscriptionList];
		#endif

		/**  */
		uint32_t lastSubscriptionCheckTimestamp;

		#ifdef ENABLE_EVENTS
		uint32_t lastSubscriptionPollingCheckTimestamp;
		#endif

		/** */
		HardwareInterface* hwinterface;

		/** used for subscription execution */
		CommandHandler* commandHandler;

		/** used for networking callback */
		Layer3* networking;

	//functions
	public:
		#ifdef ENABLE_EVENTS
		/**
		 * get all event subscriptions & poll them
		 */
		void doPollingForSubscriptions();
		#endif

		void init() {
			//memset(&subscriptions, 0,  (size_t) numSubscriptionList * sizeof(subscription_helper_t));
			//memset(&subscriptionsLastExecution, 0, (size_t) numSubscriptionList * sizeof(uint32_t));

			#ifdef ENABLE_EXTERNAL_RAM
			memRegionSubscriptionLastExecutions = ram.createRegion(sizeof(uint32_t), numSubscriptionList);
			memRegionSubscriptions = ram.createRegion(sizeof(subscription_helper_t), numSubscriptionList);
			memRegionSubscriptionsTmpBuffer = ram.createRegion(sizeof(subscription_helper_t), numSubscriptionList);
			#else
			for(uint8_t i = 0; i < numSubscriptionList; i++) {
				subscriptions[i].address = 0;
				subscriptionsLastExecution[i] = 0;
			}
			#endif

			lastSubscriptionCheckTimestamp = 0;

			#ifdef ENABLE_EVENTS
			lastSubscriptionPollingCheckTimestamp = 0;
			#endif
		}

		/**
		 * handles a new subscriptio request
		 * @param callback
		 * @param sequence
		 * @param request type
		 * @param remote address
		 * @param application layer packet
		 */
		boolean handleRequest(EventCallbackInterface* callback, const seq_t seq, const packet_type_application_t type, const l3_address_t remote, packet_application_numbered_cmd_t* appPacket);

		/**
		 * set reference for hardware drivers
		 * @param hwinterface
		 */
		inline void setHardwareInterface(HardwareInterface* hwinterface) {
			this->hwinterface = hwinterface;
		}

		/**
		 * set reference for command handler
		 * @param handler
		 */
		inline void setCommandHandler(CommandHandler* handler) {
			this->commandHandler = handler;
		}

		/**
		 * set reference for networking - used for networking callback.
		 * @param networking
		 */
		inline void setNetworking(Layer3* networking) {
			this->networking = networking;
		}

		/**
		 * constructor.
		 * basically sets the subcription list to 0
		 */
		//__attribute__((optimize("O1")))
		SubscriptionService();

		/**
		 * get size of internal list
		 * @return size
		 */
		inline const static uint8_t getSubscriptionListSize() {
			return numSubscriptionList;
		}

		/**
		 * check all subscriptions and execute if timed out
		 */
		void executeSubscriptions();

	protected:
		/**
		 * get subscription info
		 * @param filter for remote address; 0 returns all subscriptions
		 * @param buffer
		 * @param buffer len
		 * @return num subscriptions
		 */
		uint8_t getSubscriptionInfos(l3_address_t forAddress, subscription_helper_struct* buffer, uint8_t buffer_len) const;

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
		boolean setSubscription(subscription_helper_t* s);

		/**
		 * delete a subscription
		 * @param subscription info
		 * @return success
		 */
		boolean deleteSubscription(subscription_helper_t* s);

		/**
		 * get index to a subscription
		 * @param subscription info
		 * @return index, 0xff is none found
		 */
		uint8_t getSubscriptionIndex(subscription_helper_t* s);

		/**
		 * get a free subscription index
		 * @return index, 0xff if list is full.
		 */
		uint8_t getFreeSubscriptionIndex();

		/**
		 * handle INFO request
		 * @param callback
		 * @param seq
		 * @param type
		 * @param remote
		 * @param applayer packet (is being changed)
		 * @return success
		 */
		boolean handleSubscriptionInfoRequest(EventCallbackInterface* callback, seq_t seq, packet_type_application_t type, l3_address_t remote, packet_application_numbered_cmd_t* appPacket) const;

		/**
		 * handle SET request
		 * @param callback
		 * @param seq
		 * @param type
		 * @param remote
		 * @param applayer packet (is being changed)
		 * @return success
		 */
		boolean handleSubscriptionRequest(EventCallbackInterface* callback, const seq_t seq, const packet_type_application_t type, const l3_address_t remote, packet_application_numbered_cmd_t* appPacket);

		/**
		 * execute a subscription
		 * setup packet and send it into system as if it were send from remote
		 * @param parameters
		 * @param success
		 */
		boolean executeSubscription(const subscription_helper_t* subscription, subscription_event_type_t eventType);


	private:

}; //SubscriptionService

#endif //__SUBSCRIPTIONSERVICE_H__
