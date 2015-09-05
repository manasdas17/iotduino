/*
* NodeInfo.cpp
*
* Created: 31.08.2015 20:26:08
* Author: helge
*/


#include "NodeInfo.h"

void NodeInfo::init() {
	Serial.print(millis());
	Serial.print(F(": creating memregion for nodeInfo"));
	memRegionId = ram.createRegion(sizeof(NodeInfoTableEntry_t), NODE_INFO_MAX);

	#ifdef SDCARD_ENABLE
	readInfoFromSDCard();
	#endif
}

uint8_t NodeInfo::getNodeInfo(l3_address_t address, NodeInfoTableEntry_t* elem) {
	boolean ret = ram.readElementIntoVar(memRegionId, address, elem);

	if(!ret) {
		memset(elem, 0, sizeof(NodeInfoTableEntry_t));
	}

	return ret;
}

boolean NodeInfo::getIterator(SPIRamManager::iterator* it) {
	return it->init(&ram, memRegionId);
}

boolean NodeInfo::updateDiscoveryTime(l3_address_t id, uint32_t timestamp) {
	NodeInfoTableEntry_t elem;

	ram.readElementIntoVar(memRegionId, id, &elem);
	elem.lastDiscoveryRequest = timestamp;

	elem.nodeId = id;

	if(!ram.writeElementToRam(memRegionId, id, &elem)) {
		return false;
	}

	return true;
}

boolean NodeInfo::updateString(l3_address_t id, byte* buf, uint8_t buflen) {
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

boolean NodeInfo::readInfoFromSDCard() {
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

boolean NodeInfo::writeInfoToSDCard() {
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
