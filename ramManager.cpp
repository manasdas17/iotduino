/*
* ramManager.cpp
*
* Created: 29.08.2015 09:31:29
* Author: helge
*/


#include "ramManager.h"

void SPIRamManager::printRam(uint32_t from, uint32_t to) {
	#ifdef DEBUG_RAM_ENABLE
		Serial.print(millis());
		Serial.print(F(": printRam() from=0x"));
		Serial.print(from, HEX);
		Serial.print(F(" to=0x"));
		Serial.println(to, HEX);
		Serial.println(F("\t0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F"));
		Serial.flush();

		uint16_t num = 0;
		while(from < to) {
			if(num % 16 == 0) {
				if(num > 0) Serial.println();
				Serial.print(F("0x"));
				Serial.print(from, HEX);
				Serial.print(F(":\t"));
				Serial.flush();
			}
			uint8_t tmp = ram.read_byte(from);
			if(tmp < 0x10)
				Serial.print('0');
			Serial.print(tmp, HEX);
			Serial.print(F(" "));
			Serial.flush();
			from++;
			num++;
		}
		Serial.println();
		Serial.flush();
	#endif
}

void SPIRamManager::printRam(uint8_t regionId) {
	#ifdef DEBUG_RAM_ENABLE
		Serial.print(millis());
		Serial.print(F(": printRam() for region. "));
		printRegionInfo(regionId);

		memRegion_t region;
		getRegionInfo(&region, regionId);
		printRam(region.ramStartAddress, region.ramStartAddress + region.numElements * region.elementSize);
		#endif
	}

void* SPIRamManager::memcpy_R(void* destination, uint32_t source, uint32_t len) {
	ram.read_stream(source, (char*) destination, len);
	return destination;
}

void SPIRamManager::memcpy_R(uint32_t destination, void* source, uint32_t len) {
	ram.write_stream(destination, (char*) source, len);
}

void SPIRamManager::memset_R(uint32_t address, int value, uint32_t len) {
	memset(buffer, value, bufferSize);

	uint16_t fullPagesToFill = len / pageSize;
	for(uint16_t i = 0; i < fullPagesToFill; i++) {
		ram.write_page(address + i * pageSize, (char*) buffer);
	}

	uint8_t bytesToFill = len % pageSize;
	for(uint8_t i = 0; i < bytesToFill; i++) {
		ram.write_byte(address + fullPagesToFill * pageSize + i, value);
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
		Serial.print(F(": writeElementToRam() index="));
		Serial.print(index);
		Serial.print(F(" "));
		printRegionInfo(regionId);
		Serial.flush();
	#endif

	memRegion_t region;

	//region not found
	if(!getRegionInfo(&region, regionId)) {
		#ifdef DEBUG_RAM_ENABLE
			Serial.println(F("\tregion not present"));
			Serial.flush();
		#endif
		return false;
	}

	//index is not part of this region
	if(region.numElements <= index) {
		#ifdef DEBUG_RAM_ENABLE
			Serial.println(F("\tindex not present."));
			Serial.flush();
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
		Serial.print(F(": putElementInfoBuffer() index="));
		Serial.print(index);
		Serial.print(F(" "));
		printRegionInfo(region.id);
	#endif

	//index not part of region or region empty
	if(index >= region.numElements || region.numElements == 0) {
		#ifdef DEBUG_RAM_ENABLE
			Serial.println(F("\tindex not present."));
			Serial.flush();
		#endif
		return NULL;
	}

	memcpy_R(buffer, region.ramStartAddress + index * region.elementSize, region.elementSize);
	return buffer;
}

void SPIRamManager::printRegionInfo(uint8_t region) {
	#ifdef DEBUG_RAM_ENABLE
		memRegion_t r;
		getRegionInfo(&r, region);
		Serial.print(F("region="));
		Serial.print(r.id);
		Serial.print(F(" startAddress=0x"));
		Serial.print(r.ramStartAddress, HEX);
		Serial.print(F(" elemSize="));
		Serial.print(r.elementSize);
		Serial.print(F(" numElems="));
		Serial.println(r.numElements);
		Serial.flush();
	#endif
}

uint8_t SPIRamManager::createRegion(uint16_t elementSize, uint16_t numElements) {
	#ifdef DEBUG_RAM_ENABLE
		Serial.print(millis());
		Serial.print(F(": createRegion() elemSize="));
		Serial.print(elementSize);
		Serial.print(F(" elemNum="));
		Serial.println(numElements);
		Serial.flush();
	#endif

	//buffer suitable for elment?
	if(bufferSize < elementSize) {
		#ifdef DEBUG_RAM_ENABLE
			Serial.print(millis());
			Serial.println(F(": no sufficient space."));
			Serial.flush();
		#endif
		return 0;
	}

	//search next free region
	memRegion_t region;
	uint32_t nextFreeAddress = 0;
	for(uint8_t i = 0; i < maxRegions; i++) {
		if(!getRegionInfo(&region, i)) {
			//region is empty, use it.
			region.elementSize = elementSize;
			region.id = i;
			region.numElements = numElements;
			region.ramStartAddress = nextFreeAddress;

			//ram full - unlikely.
			if(nextFreeAddress + elementSize * numElements > size) {
				//no sufficient space.
				#ifdef DEBUG_RAM_ENABLE
					Serial.println(F("\tno sufficient space."));
				Serial.flush();
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
				Serial.flush();
			#endif
			return i;
		} else {
			//region is in use.
			nextFreeAddress = region.ramStartAddress + region.numElements * region.elementSize;

			#ifdef DEBUG_RAM_ENABLE
				Serial.print(F("\tregion="));
				Serial.print(i);
				Serial.print(F(" in use, nextFreeAddress=0x"));
				Serial.println(nextFreeAddress, HEX);
				Serial.flush();
			#endif
		}
	}

	#ifdef DEBUG_RAM_ENABLE
		Serial.println(F("\tno free region."));
		Serial.flush();
	#endif
	return 0;
}

 SPIRamManager::SPIRamManager(uint8_t SS, SpiRAM::addressLengthEnum adrLen, uint32_t size) {
	ram = SpiRAM(0, SS, adrLen);
	this->size = size;

	//init regions.
	memset_R(0, 0, maxRegions * sizeof(memRegion_t));
	memRegion_t region0;
	region0.id = 0;
	region0.elementSize = sizeof(memRegion_t);
	region0.numElements = maxRegions;
	region0.ramStartAddress = 0;

	memcpy_R(0, &region0, sizeof(memRegion_t));

	#ifdef DEBUG_RAM_ENABLE
		Serial.print(millis());
		Serial.print(F(": initialize ram SS="));
		Serial.print(SS);
		Serial.print(F(" adrLen="));
		switch(adrLen) {
			case SpiRAM::l16bit:
				Serial.print(F("16bit"));
				break;
			case SpiRAM::l24bit:
				Serial.print(F("24bit"));
				break;
			default:
				Serial.print(F("unknown"));
		}
		Serial.print(F(" size="));
		Serial.println(size);
		Serial.flush();
	#endif
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
	return mgr->bufferSize / region.elementSize;
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
		mgr->memcpy_R(mgr->buffer, region.ramStartAddress + iteratorIndex * region.elementSize, len);
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
