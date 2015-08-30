/*
* ramManager.h
*
* Created: 29.08.2015 09:31:29
* Author: helge
*/


#ifndef __RAMMANAGER_H__
#define __RAMMANAGER_H__

#include <Arduino.h>
#include <DebugConfig.h>
#include "SpiRAM.h"

class SPIRamManager {
	public:
		typedef struct memRegion_struct {
			uint16_t elementSize;
			uint16_t numElements;
			uint32_t ramStartAddress;
			uint8_t id;
		} memRegion_t;

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
			SPIRamManager* mgr;
			uint8_t regionId;
			memRegion_t region;
			uint16_t iteratorIndex;

			public:
			/**
			 * set data & region region info
			 */
			iterator(SPIRamManager* mgr, uint8_t regionId);

			iterator() {

			}

			boolean init(SPIRamManager* mgr, uint8_t regionId) {
				this->mgr = mgr;
				this->regionId = regionId;
				this->iteratorIndex = 0;

				return mgr->getRegionInfo(&region, regionId);
			}


			/**
			 * @return true if the end has not been reached
			 */
			boolean hasNext();

			/**
			 * return pinter to next element
			 * only reads new data from spi ram when necessary.
			 */
			void* next();

			/**
			 * @return number if elements in mgr buffer
			 */
			uint8_t getNumElemPerBufferInstance();

			uint16_t getIteratorIndex();

			void remove();

			/**
			 * write the element from the mgr buffer back into ram.
			 * change must be done in place and the user must ensure that it is not getting dirty.
			 */
			boolean writeBack();

			void reset();
		};


		/**
		 * initializer.
		 */
		SPIRamManager(uint8_t SS, SpiRAM::addressLengthEnum adrLen, uint32_t size);

		/**
		 * create a memory region. searches next free.
		 * @param elementSize
		 * @param numElements
		 */
		uint8_t createRegion(uint16_t elementSize, uint16_t numElements);

		/**
		 * print region information.
		 */
		void printRegionInfo(uint8_t region);

		/**
		 * store an element in the ram buffer
		 * @param region
		 * @pram index of element
		 */
		void* readElementIntoBuffer(uint8_t regionId, uint16_t index);

		uint16_t readElementIntoVar(uint8_t regionId, uint16_t index, void* buf);

		/**
		 * helper to store element in ram
		 */
		boolean writeElementToRam(uint8_t regionId, uint16_t index, void* elem);

		/**
		 * get mem region information
		 * @param region
		 * @param regionId
		 */
		boolean getRegionInfo(memRegion_t* region, uint8_t regionId);

		/**
		 * write value to ram - pagewise.
		 * @param address
		 * @param value
		 * @param len
		 */
		void memset_R(uint32_t address, int value, uint32_t len);

		/**
		 * copy from system mem to spi ram
		 * @param destination
		 * @param source
		 * @param len
		 */
		void memcpy_R(uint32_t destination, void* source, uint32_t len);

		/**
		 * copy from spi to system ram
		 * @param destination
		 * @param source
		 * @param len
		 */
		void* memcpy_R(void* destination, uint32_t source, uint32_t len);


		void printRam(uint8_t regionId);

		void printRam(uint32_t from, uint32_t to);
}; //ramManager


#endif //__RAMMANAGER_H__
