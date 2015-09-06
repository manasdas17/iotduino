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
#include <sdcard/SDcard.h>
#include <SD.h>
#include <avr/wdt.h>


extern SPIRamManager ram;

#define NODE_INFO_SIZE 32
#define NODE_INFO_MAX 255

class NodeInfo
{
	public:
		/** string node information */
		const char* fileNameNodeInfo = {"NODEINFO.TXT"};

		/** maximum number of nodeids */
		typedef struct SD_nodeInfoTableEntryStruct {
			uint8_t nodeId;
			char nodeStr[NODE_INFO_SIZE];
			uint32_t lastDiscoveryRequest;
		} NodeInfoTableEntry_t;

		uint8_t memRegionId;

		/**
		 */
		NodeInfo() {
		}

		void init();

		/**
		 * delete node info
		 * @param nodeid
		 * @return success
		 */
		boolean deleteInfo(l3_address_t nodeId) {
			return ram.memsetElement(memRegionId, nodeId, 0);
		}

		/**
		 * get node info entry
		 * @pram id
		 * @pram elem
		 * @return bytes read
		 */
		uint8_t getNodeInfo(l3_address_t address, NodeInfoTableEntry_t* elem);

		/**
		 * get an iterator for node infos
		 * @param iterator
		 * @return success
		 */
		boolean getIterator(SPIRamManager::iterator* it);

		/**
		 * writes info back to sd card (only update existing items)
		 * @return success
		 */
		boolean writeInfoToSDCard();

		/**
		 * reads all info from sd card
		 * @return success
		 */
		boolean readInfoFromSDCard();

		/**
		 * update an info string
		 * @return success
		 */
		boolean updateString(l3_address_t id, byte* buf, uint8_t buflen);

		/**
		 * update last discovery timestamp
		 * @return success
		 */
		boolean updateDiscoveryTime(l3_address_t id, uint32_t timestamp);

}; //NodeInfo

#endif //__NODEINFO_H__
