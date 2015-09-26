/*
* ramManager.cpp
*
* Created: 29.08.2015 09:31:29
* Author: helge
*/


#include "ramManager.h"
#include <avr/wdt.h>

void SPIRamManager::printRam(uint32_t from, uint32_t to) {
	#ifdef DEBUG_RAM_ENABLE
		Serial.print(millis());
		Serial.print(F(": print() from=0x"));
		Serial.print(from, HEX);
		Serial.print(F(" to=0x"));
		Serial.println(to, HEX);

		uint16_t num = 0;
		while(from < to) {
			if(num % 16 == 0) {
				if(num > 0) Serial.println();
				Serial.print(F("0x"));
				Serial.print(from, HEX);
				Serial.print(F(":\t"));
			}
			uint8_t tmp = spiram.read_byte(from);
			if(tmp < 0x10)
				Serial.print('0');
			Serial.print(tmp, HEX);
			Serial.print(F(" "));
			from++;
			num++;
			Serial.flush();
		}
		Serial.println();
	#endif
}

void SPIRamManager::printRam(uint8_t regionId) {
	#ifdef DEBUG_RAM_ENABLE
		Serial.print(millis());
		printRegionInfo(regionId);

		memRegion_t region;
		getRegionInfo(&region, regionId);
		printRam(region.ramStartAddress, region.ramStartAddress + region.numElements * region.elementSize);
	#endif
}

void* SPIRamManager::memcpy_R(void* destination, uint32_t source, uint32_t len) {
	spiram.read_stream(source, (char*) destination, len);
	return destination;
}

void SPIRamManager::memcpy_R(uint32_t destination, void* source, uint32_t len) {
	spiram.write_stream(destination, (char*) source, len);
}

void SPIRamManager::memset_R(uint32_t address, int value, uint32_t len) {
	memset(buffer, value, RAM_MGR_BUF_SIZE);

	//full buffers to write
	uint16_t fullPagesToFill = len / RAM_MGR_BUF_SIZE;
	for(uint16_t i = 0; i < fullPagesToFill; i++) {
		spiram.write_stream(address + i * RAM_MGR_BUF_SIZE, (char*) buffer, RAM_MGR_BUF_SIZE);
		wdt_reset();
	}

	//remaining bytes
	uint8_t bytesToFill = len % RAM_MGR_BUF_SIZE;
	for(uint8_t i = 0; i < bytesToFill; i++) {
		spiram.write_byte(address + fullPagesToFill * RAM_MGR_BUF_SIZE + i, value);
	}
}

boolean SPIRamManager::getRegionInfo(memRegion_t* region, uint8_t regionId) {
	memcpy_R(region, regionId * sizeof(memRegion_t), sizeof(memRegion_t));
	if(region->numElements == 0)
		return false;
	return true;
}

boolean SPIRamManager::writeElementToRam(uint8_t regionId, uint16_t index, void* elem) {
	#ifdef DEBUG_RAM_ENABLE
		Serial.print(millis());
		Serial.print(F(": wrElmntToRam() id="));
		Serial.print(index);
		Serial.print(F(" "));
		printRegionInfo(regionId);
	#endif

	memRegion_t region;

	//region not found
	if(!getRegionInfo(&region, regionId) || index >= region.numElements) {
		#ifdef DEBUG_RAM_ENABLE
			Serial.println(F("\treg unavail"));
			while(1);
		#endif
		return false;
	}

	memcpy_R(region.ramStartAddress + region.elementSize * index, elem, region.elementSize);
	return true;
}

void* SPIRamManager::readElementIntoBuffer(uint8_t regionId, uint16_t index) {
	memRegion_t region;
	getRegionInfo(&region, regionId);

	#ifdef DEBUG_RAM_ENABLE
		Serial.print(millis());
		Serial.print(F(": putElmtToBuf() idx="));
		Serial.print(index);
		Serial.print(F(" "));
		printRegionInfo(region.id);
	#endif

	//index not part of region or region empty
	if(index >= region.numElements || region.numElements == 0) {
		//#ifdef DEBUG_RAM_ENABLE
			//Serial.println(F("\tindex unavail"));
		//#endif
		return NULL;
	}

	memcpy_R(buffer, region.ramStartAddress + index * region.elementSize, region.elementSize);
	return buffer;
}

void SPIRamManager::printRegionInfo(uint8_t region) {
	#ifdef DEBUG_RAM_ENABLE
		memRegion_t r;
		getRegionInfo(&r, region);
		Serial.print(F("reg="));
		Serial.print(r.id);
		Serial.print(F(" startAdr=0x"));
		Serial.print(r.ramStartAddress, HEX);
		Serial.print(F(" elmSize="));
		Serial.print(r.elementSize);
		Serial.print(F(" #Elms="));
		Serial.println(r.numElements);
		Serial.flush();
	#endif
}

uint8_t SPIRamManager::createRegion(uint16_t elementSize, uint16_t numElements) {
	#ifdef DEBUG_RAM_ENABLE
		Serial.print(millis());
		Serial.print(F(": createRegion() elmSize="));
		Serial.print(elementSize);
		Serial.print(F(" elm#="));
		Serial.println(numElements);
		Serial.flush();
	#endif

	//buffer suitable for elment?
	if(RAM_MGR_BUF_SIZE < elementSize) {
		#ifdef DEBUG_RAM_ENABLE
			Serial.print(millis());
			Serial.println(F(": BUF too small"));
		#endif
		return 0;
	}

	//search next free region
	memRegion_t region;
	uint32_t nextFreeAddress = usableRamStartAddress;
	for(uint8_t i = 1; i < RAM_MGR_MAX_REGIONS; i++) {
		if(!getRegionInfo(&region, i)) {
			//region is empty, use it.
			region.elementSize = elementSize;
			region.id = i;
			region.numElements = numElements;
			region.ramStartAddress = nextFreeAddress;

			//adjust address to page.
			//if(nextFreeAddress % pageSize != 0) {
				//nextFreeAddress = nextFreeAddress / pageSize + pageSize;
//
				//#ifdef DEBUG_RAM_ENABLE
					//Serial.println("\tskipping bytes in favor of page alignment.");
				//#endif
			//}

			//ram full - unlikely.
			if(nextFreeAddress + elementSize * numElements > size) {
				//no sufficient space.
				#ifdef DEBUG_RAM_ENABLE
					Serial.println(F("\tRAM full"));
					while(1);
				#endif
				return 0;
			}

			//clean
			memset_R(nextFreeAddress, 0, region.elementSize * region.numElements);
			//store region info
			memcpy_R(i * sizeof(memRegion_t), &region, sizeof(memRegion_t));
			#ifdef DEBUG_RAM_ENABLE
				Serial.print(F("\t"));
				printRegionInfo(region.id);
			#endif
			return i;
		} else {
			//region is in use.
			nextFreeAddress = region.ramStartAddress + region.numElements * region.elementSize;

			#ifdef DEBUG_RAM_ENABLE
				Serial.print(F("\tregion="));
				Serial.print(i);
				Serial.print(F(" in use, nxtFreeAdr=0x"));
				Serial.println(nextFreeAddress, HEX);
				Serial.flush();
			#endif
		}
	}

	#ifdef DEBUG_RAM_ENABLE
		Serial.println(F("\tno free region."));
		while(1);
	#endif
	return 0;
}

 void SPIRamManager::init(uint8_t SS, SpiRAM::addressLengthEnum adrLen, uint32_t size) {
	this->spiram = SpiRAM(0, SS, adrLen);
	this->size = size;

	//init regions.
	memset_R(0, 0, RAM_MGR_MAX_REGIONS * sizeof(memRegion_t));
	memRegion_t region0;
	region0.id = 0;
	region0.elementSize = sizeof(memRegion_t);
	region0.numElements = RAM_MGR_MAX_REGIONS;
	region0.ramStartAddress = 0;

	#ifdef DEBUG_RAM_ENABLE
	Serial.print(millis());
	Serial.print(F(": creating Reg0, id="));
	Serial.print(region0.id);
	Serial.print(F(" elS="));
	Serial.print(region0.elementSize);
	Serial.print(F(" #elm="));
	Serial.print(region0.numElements);
	Serial.print(F(" adr=0x"));
	Serial.println(region0.ramStartAddress, HEX);
	#endif

	memcpy_R(0, &region0, sizeof(memRegion_t));

	//#ifdef DEBUG_RAM_ENABLE
		//Serial.print(millis());
		//Serial.print(F(": initialize ram SS="));
		//Serial.print(SS);
		//Serial.print(F(" adrLen="));
		//Serial.print(adrLen);
		//Serial.print(F(" size="));
		//Serial.println(size);
	//#endif
}

uint16_t SPIRamManager::readElementIntoVar(uint8_t regionId, uint16_t index, void* buf) {
	memRegion_t region;
	if(!getRegionInfo(&region, regionId))
		return 0;

	if(index >= region.numElements)
		return 0;

	memcpy_R(buf, region.ramStartAddress + index * region.elementSize, region.elementSize);

	return region.elementSize;
}

boolean SPIRamManager::memsetElement(uint8_t regionId, uint16_t index, uint8_t value) {
	memRegion_t region;
	getRegionInfo(&region, regionId);

	if(index >= region.numElements)
		return false;

	memset_R(region.ramStartAddress + region.elementSize * index, value, region.elementSize);

	return true;
}

void SPIRamManager::iterator::reset() {
	iteratorIndex = 0;
}

boolean SPIRamManager::iterator::writeBack() {
	if(iteratorIndex == 0)
		return false;

	//the iteratorindex has already been increased due to first read.
	uint8_t lastElementIndex = iteratorIndex - 1;

	uint8_t indexInBuffer = (lastElementIndex % getNumElemPerBufferInstance()) * region.elementSize;
	mgr->writeElementToRam(regionId, lastElementIndex, &mgr->buffer[indexInBuffer]);

	return true;
}

uint16_t SPIRamManager::iterator::getIteratorIndex() {
	return iteratorIndex;
}

uint8_t SPIRamManager::iterator::getNumElemPerBufferInstance() {
	return RAM_MGR_BUF_SIZE / region.elementSize;
}

void* SPIRamManager::iterator::next() {
	if(iteratorIndex >= region.numElements) {
		return NULL;
	}

	//do we have to read again?
	uint8_t numElemPerBufferInstance = getNumElemPerBufferInstance();
	if(iteratorIndex % numElemPerBufferInstance == 0) {
		//maybe, we do not need to read a full buffer.
		uint16_t len = min(region.numElements - iteratorIndex, numElemPerBufferInstance) * region.elementSize;
		mgr->memcpy_R(mgr->buffer, region.ramStartAddress + (uint32_t) iteratorIndex * region.elementSize, len);
	}

	uint8_t indexInBuffer = iteratorIndex % numElemPerBufferInstance;

	iteratorIndex++;

	return &mgr->buffer[indexInBuffer * region.elementSize];
}

boolean SPIRamManager::iterator::hasNext() {
	return iteratorIndex < region.numElements;
}

 SPIRamManager::iterator::iterator(SPIRamManager* mgr, uint8_t regionId) {
	init(mgr, regionId);
}

void SPIRamManager::iterator::remove() {
	mgr->memset_R(region.ramStartAddress + (iteratorIndex-1) * region.elementSize, 0, region.elementSize);
}
