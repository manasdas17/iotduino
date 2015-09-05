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
				//SD.remove((char*) fileNameNodeInfo);
				File fd = SD.open(fileNameNodeInfo, FILE_WRITE);

				if(!SDcard::fillFile(&fd, 0, sizeof(NodeInfoTableEntry_t) * NODE_INFO_MAX))
					return false;


				SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionId);
				NodeInfoTableEntry_t* currentItem = NULL;
				while(it.hasNext()) {
					currentItem = (NodeInfoTableEntry_t*) it.next();

					if(currentItem->nodeId > 0) {
						uint32_t pos = (it.getIteratorIndex()-1) * sizeof(NodeInfoTableEntry_t);
						if(!fd.seek(pos)) {
							#ifdef DEBUG
								Serial.print(millis());
								Serial.print(F(": NodeInfo::writeInfoToSDCard() seek to pos="));
								Serial.print(pos);
								Serial.print(F("failed for index="));
								Serial.println(it.getIteratorIndex()-1);
								Serial.flush();
							#endif
							continue;
						}
						if(!fd.write((const uint8_t*) currentItem, sizeof(NodeInfoTableEntry_t))) {
							#ifdef DEBUG
								Serial.print(millis());
								Serial.print(F(": NodeInfo::writeInfoToSDCard() write at pos="));
								Serial.print(pos);
								Serial.print(F("failed for index="));
								Serial.println(it.getIteratorIndex()-1);
								Serial.flush();
							#endif
						}
					}

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
						if(elem.nodeId != 0) {
							#ifdef DEBUG
								Serial.print(millis());
								Serial.print(F(": NodeInfo::readFromSDCard() index="));
								Serial.print(i);
								Serial.print(F(" info=[id="));
								Serial.print(elem.nodeId);
								Serial.print(F(" lastDiscovery="));
								Serial.print(elem.lastDiscoveryRequest);
								Serial.print(F(" str="));
								Serial.print(elem.nodeStr);
								Serial.println(F("]"));
							#endif
							ram.writeElementToRam(memRegionId, i, &elem);
						} else {
							#ifdef DEBUG
								Serial.print(F("\tindex="));
								Serial.print(i);
								Serial.println(F(" empty."));
							#endif
						}
						wdt_reset();
					} else {
						#ifdef DEBUG
							Serial.print(millis());
							Serial.print(F(": NodeInfo::readInfoFromSDCard() read failed for index="));
							Serial.print(i);
							Serial.flush();
						#endif
						break;
					}
				}
				fd.close();
				return true;
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

			elem.nodeId = id;

			if(!ram.writeElementToRam(memRegionId, id, &elem)) {
				return false;
			}

			return writeInfoToSDCard();
		}

		boolean updateDiscoveryTime(l3_address_t id, uint32_t timestamp) {
			NodeInfoTableEntry_t elem;

			ram.readElementIntoVar(memRegionId, id, &elem);
			elem.lastDiscoveryRequest = timestamp;

			elem.nodeId = id;

			if(!ram.writeElementToRam(memRegionId, id, &elem)) {
				return false;
			}
		}

}; //NodeInfo

#endif //__NODEINFO_H__
