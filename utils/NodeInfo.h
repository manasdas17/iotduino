/*
* NodeInfo.h
*
* Created: 31.08.2015 20:26:08
* Author: helge
*/


#ifndef __NODEINFO_H__
#define __NODEINFO_H__

#include <Arduino.h>
#include <networking/Packets.h>
#include <ramManager.h>


extern SPIRamManager ram;

#define NODE_INFO_SIZE (0x32)

class NodeInfo
{
	public:
		/** maximum number of nodeids */
		const static uint8_t NUM_NODES = 255;
		typedef struct SD_nodeInfoTableEntryStruct {
			uint8_t nodeId;
			char nodeStr[NODE_INFO_SIZE];
			uint32_t lastDiscoveryRequest;
		} NodeInfoTableEntry_t;

		/**  */
		typedef struct SD_nodeDiscoveryInfoTableEntryStruct {
			uint8_t hardwareAddress;
			uint8_t hardwareType;
			uint32_t rtcTimestamp;
		} NodeDiscoveryInfoTableEntry_t;

		/**  */
		typedef struct SD_subscriptionInfoTableEntryStruct {
			uint8_t hardwareAddress;
			uint8_t hardwareType;
			uint32_t delay;
			uint8_t subscription_event_type_t;
			seq_t sequence;
			uint32_t rtcLastkeepalive;
			uint32_t rtcLastRequest;
			uint8_t active;
		} NodeSubscriptionInfoTableEntry_t;


		uint8_t memRegionId;

		/**
		 */
		NodeInfo() {
		}

		void init() {
			Serial.print(millis());
			Serial.print(F(": creating memregion for nodeInfo"));
			memRegionId = ram.createRegion(sizeof(NodeInfoTableEntry_t), NUM_NODES);

			#ifdef SDCARD_ENABLE
			readInfoFromSDCard();
			#endif
		}

		/**
		 * get node info entry
		 * @pram id
		 * @pram elem
		 * @return bytes read
		 */
		uint8_t getNodeInfo(l3_address_t address, NodeInfoTableEntry_t* elem) {
			boolean ret = ram.readElementIntoVar(memRegionId, address, elem);

			if(!ret) {
				memset(elem, 0, sizeof(NodeInfoTableEntry_t));
			}

			return ret;
		}

		/**
		 * get an iterator for node infos
		 * @param iterator
		 * @return success
		 */
		boolean getIterator(SPIRamManager::iterator* it) {
			return it->init(&ram, memRegionId);
		}

		boolean writeInfoToSDCard() {
			#ifdef SDCARD_ENABLE
			#endif

			return false;
		}

		boolean readInfoFromSDCard() {
			#ifdef SDCARD_ENABLE
			#endif

			return false;
		}
}; //NodeInfo

#endif //__NODEINFO_H__
