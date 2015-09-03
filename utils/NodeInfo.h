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

		void init() {
			Serial.print(millis());
			Serial.print(F(": creating memregion for nodeInfo"));
			memRegionId = ram.createRegion(sizeof(NodeInfoTableEntry_t), NODE_INFO_MAX);

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
				SD.remove((char*) fileNameNodeInfo);
				File fd = SD.open(fileNameNodeInfo, FILE_WRITE);
				fd.seek(0);

				SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionId);
				NodeInfoTableEntry_t* currentItem = NULL;
				while(it.hasNext()) {
					currentItem = (NodeInfoTableEntry_t*) it.next();
					fd.write((const uint8_t*) currentItem, sizeof(NodeInfoTableEntry_t));
					wdt_reset();
				}
				fd.flush();
				fd.close();

				return true;
			#endif

			return false;
		}

		boolean readInfoFromSDCard() {
			#ifdef SDCARD_ENABLE
				File fd = SD.open(fileNameNodeInfo, FILE_WRITE);
				fd.seek(0);
				NodeInfoTableEntry_t elem;
				for(uint16_t i = 0; i < NODE_INFO_MAX; i++) {
					if(fd.read(&elem, sizeof(elem)) > 0) {
						ram.writeElementToRam(memRegionId, i, &elem);
						wdt_reset();
					} else {
						fd.close();
						break;
					}
				}
				fd.close();
			#endif

			return false;
		}

		boolean updateString(l3_address_t id, byte* buf, uint8_t buflen) {
			//read current info
			NodeInfoTableEntry_t elem;
			ram.readElementIntoVar(memRegionId, id, &elem);

			//update local string
			buf[NODE_INFO_SIZE-1] = '\0';
			uint8_t len = strlen((const char*) buf)+1;
			memcpy(elem.nodeStr, buf, len);

			if(!ram.writeElementToRam(memRegionId, id, &elem)) {
				return false;
			}

			return writeInfoToSDCard();
		}

		boolean updateDiscoveryTime(l3_address_t id, uint32_t timestamp) {
			NodeInfoTableEntry_t elem;

			ram.readElementIntoVar(memRegionId, id, &elem);
			elem.lastDiscoveryRequest = timestamp;

			if(!ram.writeElementToRam(memRegionId, id, &elem)) {
				return false;
			}
		}

}; //NodeInfo

#endif //__NODEINFO_H__
