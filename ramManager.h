/*
* ramManager.h
*
* Created: 29.08.2015 09:31:29
* Author: helge
*/


#ifndef __RAMMANAGER_H__
#define __RAMMANAGER_H__

#include <Arduino.h>
#include "SpiRAM.h"

class ramManager {
	typedef struct memRegion_struct {
		uint16_t elementSize;
		uint16_t numElements;
		uint32_t ramStartAddress;
		uint8_t id;
	} memRegion_t;

	public:
		static const uint8_t pageSize = 32;
		static const uint8_t bufferSize = 2 * pageSize; //buffersize is page size.
		static const uint8_t maxRegions = 32; //reservation for memRegion structs.
		static const uint32_t usableRamStartAddress = sizeof(memRegion_t) * maxRegions;
		SpiRAM ram;
		uint32_t size;

		byte buffer[bufferSize];

		/**
		 * iterate a data region using the ramManager buffer
		 * the user must ensure that the buffer does not get dirty while iterating.
		 */
		class iterator {
			ramManager* mgr;
			uint8_t regionId;
			memRegion_t region;
			uint16_t iteratorIndex;

			public:
			/**
			 * set data & region region info
			 */
			iterator(ramManager* mgr, uint8_t regionId) {
				this->mgr = mgr;
				this->regionId = regionId;
				this->iteratorIndex = 0;

				mgr->getRegionInfo(&region, regionId);
			}

			/**
			 * @return true if the end has not been reached
			 */
			boolean hasNext() {
				return iteratorIndex < region.numElements;
			}

			/**
			 * return pinter to next element
			 * only reads new data from spi ram when necessary.
			 */
			void* next() {
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

			/**
			 * @return number if elements in mgr buffer
			 */
			uint8_t inline getNumElemPerBufferInstance() {
				return mgr->bufferSize / region.elementSize;
			}

			uint16_t getIteratorIndex() {
				return iteratorIndex;
			}

			/**
			 * write the element from the mgr buffer back into ram.
			 * change must be done in place and the user must ensure that it is not getting dirty.
			 */
			boolean writeBack() {
				if(iteratorIndex == 0)
					return false;

				//the iteratorindex has already been increased due to first read.
				uint8_t lastElementIndex = iteratorIndex - 1;

				uint8_t indexInBuffer = (lastElementIndex % getNumElemPerBufferInstance()) * region.elementSize;
				mgr->writeElementToRam(regionId, lastElementIndex, &mgr->buffer[indexInBuffer]);

				return true;
			}

			void reset() {
				iteratorIndex = 0;
			}
		};


		/**
		 * initializer.
		 */
		ramManager(uint8_t SS, SpiRAM::addressLengthEnum adrLen, uint32_t size) {
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

		/**
		 * create a memory region. searches next free.
		 * @param elementSize
		 * @param numElements
		 */
		uint8_t createRegion(uint16_t elementSize, uint16_t numElements) {
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

		/**
		 * print region information.
		 */
		#ifdef DEBUG_RAM_ENABLE
		void printRegionInfo(uint8_t region) {
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
		}
		#endif

		/**
		 * store an element in the ram buffer
		 * @param region
		 * @pram index of element
		 */
		void* readElementIntoBuffer(uint8_t regionId, uint16_t index) {
			memRegion_t region;
			getRegionInfo(&region, regionId);

			#ifdef DEBUG_RAM_ENABLE
				Serial.print(millis());
				Serial.print(F(": putElementInfoBuffer() "));
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

		/**
		 * helper to store element in ram
		 */
		boolean writeElementToRam(uint8_t regionId, uint16_t index, void* elem) {
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

		/**
		 * get mem region information
		 * @param region
		 * @param regionId
		 */
		boolean getRegionInfo(memRegion_t* region, uint8_t regionId) {
			memcpy_R(region, regionId * sizeof(memRegion_t), sizeof(memRegion_t));
			if(region->numElements == 0)
				return false;
			return true;
		}

		/**
		 * write value to ram - pagewise.
		 * @param address
		 * @param value
		 * @param len
		 */
		void memset_R(uint32_t address, int value, uint32_t len) {
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

		/**
		 * copy from system mem to spi ram
		 * @param destination
		 * @param source
		 * @param len
		 */
		void memcpy_R(uint32_t destination, void* source, uint32_t len) {
			ram.write_stream(destination, (char*) source, len);
		}

		/**
		 * copy from spi to system ram
		 * @param destination
		 * @param source
		 * @param len
		 */
		void* memcpy_R(void* destination, uint32_t source, uint32_t len) {
			ram.read_stream(source, (char*) destination, len);
			return destination;
		}


		void printRam(uint8_t regionId) {
			#ifdef DEBUG_RAM_ENABLE
			Serial.print(millis());
			Serial.print(F(": printRam() for region. "));
			printRegionInfo(regionId);

			memRegion_t region;
			getRegionInfo(&region, regionId);
			printRam(region.ramStartAddress, region.ramStartAddress + region.numElements * region.elementSize);
			#endif
		}

		void printRam(uint32_t from, uint32_t to) {
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
}; //ramManager

#endif //__RAMMANAGER_H__
