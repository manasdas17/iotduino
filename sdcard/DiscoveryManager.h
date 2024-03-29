/*
* SubscriptionManager.h
*
* Created: 31.05.2015 23:24:59
* Author: helge
*/


#ifndef __SUBSCRIPTIONMANAGER_H__
#define __SUBSCRIPTIONMANAGER_H__

#include <Arduino.h>
#include <Configuration.h>
#include <avr/wdt.h>
#include <dispatcher/EventCallbackInterface.h>
#include <dispatcher/PacketDispatcher.h>
#include <dispatcher/PacketFactory.h>
#include <networking/Layer3.h>
#include <webserver/DiscoveryListener.h>
#include <SpiRAM.h>
#include <utils/NodeInfo.h>
#include <sdcard/SDcard.h>
#include <SD.h>
#include <Time.h>

extern PacketDispatcher dispatcher;
extern PacketFactory pf;
extern Layer3 l3;
extern SPIRamManager ram;
extern NodeInfo nodeInfo;

class DiscoveryManager {
	/** static dicoverylistener object */
	discoveryListener listenerDiscovery;

	/** saved node information */
	uint8_t memRegionDiscoveryInfo;

	/** last discovery request that has been sent */
	uint32_t lastDiscoveryRequest;
	/** last full iteration for discovery */
	uint32_t lastDiscoveryRequestFullRun;
	/** next neighbour id to process */
	uint8_t nextDiscoveryRequestNeighbourIndex;

	uint32_t lastSDWrite;

	static const char* filenameDiscovery;

	public:
		/**  */
		typedef struct SD_nodeDiscoveryInfoTableEntryStruct {
			uint8_t hardwareAddress;
			uint8_t hardwareType;
			uint32_t rtcTimestamp;
		} Discovery_nodeDiscoveryInfoTableEntry_t;

		/**
		 * send a discovery request to a node
		 * @param node
		 * @return success
		 */
		boolean sendDiscoveryRequest(l3_address_t node);

		/**
		 * delete node info
		 * @param node
		 * @return success
		 */
		boolean deleteInfo(l3_address_t node) {
			boolean result = true;
			for(uint8_t i = 0; i < NUM_INFOS_PER_NODE; i++) {
				result &= ram.memsetElement(memRegionDiscoveryInfo, node * NUM_INFOS_PER_NODE, 0);
			}
			
			if(!result)
				return false;
			
			////write changes to SD.
			//File f = SD.open(filenameDiscovery, FILE_WRITE);
			//if(!f) {
				//return false;
			//}
			//uint32_t pos = node * sizeof(Discovery_nodeDiscoveryInfoTableEntry_t);
			//if(!f.seek(pos)) {
				//return false;
			//}
			//Discovery_nodeDiscoveryInfoTableEntry_t dummy;
			//memset(&dummy, 0, sizeof(dummy));
			//if(f.write((uint8_t*) &dummy, sizeof(Discovery_nodeDiscoveryInfoTableEntry_t)) == 0) {
				//return false;
			//}
			//f.flush();
			
			return true;
		}

		/**
		 * delete one info for node
		 * @param node
		 * @param hwAddress
		 * @param hwType
		 */
		boolean deleteInfo(l3_address_t node, uint8_t hwAddress, uint8_t hwType) {
			Discovery_nodeDiscoveryInfoTableEntry_t tmp;
			uint16_t index;
			for(uint8_t i = 0; i < NUM_INFOS_PER_NODE; i++) {
				index = node * NUM_INFOS_PER_NODE + i;
				ram.readElementIntoVar(memRegionDiscoveryInfo, index, &tmp);
				
				if(tmp.hardwareAddress == hwAddress && tmp.hardwareType == hwType) {
					return ram.memsetElement(memRegionDiscoveryInfo, index, 0);
				}
			}
			
			return false;
		}

		/**
		 * read am element into buffer wrapper
		 * @param elem buffer
		 * @param remote node
		 * @praram i index
		 */
		void readElemIntoVar(Discovery_nodeDiscoveryInfoTableEntry_t* elem, l3_address_t remote, uint8_t i);

		/**
		 * initialise memory.
		 */
		void init();

		/**
		 * maintenance loop.
		 */
		void loop();

		/**
		 * update listeners - this especially processes answers
		 */
		void maintainListeners();

		/**
		 * trigger discovery.
		 */
		void maintainDiscoveries();


		/**
		 * get an iterator for node infos
		 * @param iterator
		 * @return success
		 */
		boolean getIteratorDiscovery(SPIRamManager::iterator* it);

		boolean readDataFromSDCard();

		/**
		 * store disovery info to file.
		 */
		boolean writeDataToSDCard();

		/**
		 * register active subscription and update timestamp
		 * @param destination
		 * @param hwAddress
		 * @param hwType
		 * @return success
		 */
		//boolean createSubscription(l3_address_t address, uint8_t hardwareAddress, HardwareTypeIdentifier hardwareType) {
			//#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				//Serial.print(millis());
				//Serial.print(F(": SubscriptionManager::createSubscription()"));
				//Serial.flush();
			//#endif
//
			////return in case we already have a subscription
			//int8_t index = getSubscriptionSlot(address, hardwareAddress, hardwareType);
			//if(index != -1) {
				//#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
					//Serial.print(F(" already exists@index="));
					//Serial.println(index);
				//#endif
				//return false;
			//}
//
			////get new slot
			//index = getSubscriptionSlot();
//
			////failed?
			//if(index == -1) {
				//#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
					//Serial.println(F(" list full."));
				//#endif
				//return false;
			//}
//
			////save subscription
			////seq_t seq = random(1, 0xffff);
			////lastRefresh[index] = 0;
			////activeSubscriptionNodeIDs[index] = address;
////
			////activeSubscriptions[index].address = l3.localAddress;
			////activeSubscriptions[index].hardwareAddress = hardwareAddress;
			////activeSubscriptions[index].hardwareType = hardwareType;
			////activeSubscriptions[index].millisecondsDelay = SUBSCRIPTION_DEFAULT_PERIOD_MILLIS;
			////activeSubscriptions[index].onEventType = EVENT_TYPE_CHANGE;
			////activeSubscriptions[index].sequence = seq;
//
			//#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				//Serial.print(F(" saved in slot="));
				//Serial.println(index);
				//Serial.print(F("\tforAddress="));
				//Serial.print(l3.localAddress);
				//Serial.print(F(" remote="));
				//Serial.print(address);
				//Serial.print(F(" hwAddress="));
				//Serial.print(hardwareAddress);
				//Serial.print(F(" hwType="));
				//Serial.print(hardwareType);
				//Serial.print(F(" delay="));
				//Serial.print(SUBSCRIPTION_DEFAULT_PERIOD_MILLIS);
				//Serial.print(F(" event="));
				//Serial.print(EVENT_TYPE_CHANGE);
				//Serial.print(F(" seq="));
				//Serial.println();
				//Serial.flush();
			//#endif
//
			//return true;
		//}

}; //SubscriptionManager

#endif //__SUBSCRIPTIONMANAGER_H__
