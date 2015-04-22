
/*
 * Commands.h
 *
 * Created: 18.01.2015 22:06:46
 *  Author: helge
 */ 

#if !defined(_HARDWARERESULT_H)
#define _HARDWARERESULT_H

#include <Arduino.h>
#include "networking/Packets.h"
#include <HardwareID.h>

#define sizeFloatList 1
#define sizeUInt8List 4


typedef struct hwCommand {
	HardwareTypeIdentifier type;
	uint8_t isRead;
	uint8_t address;
	uint8_t uintlistnum;
	uint8_t floatlistnum;
	uint8_t uint8list[sizeUInt8List];
	float floatlist[sizeFloatList];
} command_t; //Maximum length CONFIG_L3_PACKET_NUMBERED_MAX_LEN

#if (CONFIG_L3_PACKET_NUMBERED_MAX_LEN < (4 + sizeUInt8List*1 + sizeFloatList*4))
	#error maximum payload len exceeded. 
#endif

class HardwareCommandResult {
	//const static uint8_t sizeFloatList = 4;
	//const static uint8_t sizeUInt8List = 8;
	
	uint8_t isRead;
	
	uint8_t uint8List[sizeUInt8List];
	float floatList[sizeFloatList];
	
	uint8_t floatlistnum;
	uint8_t uintlistnum;
	
	HardwareTypeIdentifier type;
	uint8_t address;

	public:
		void reset() {
			floatlistnum = 0;
			uintlistnum = 0;
			
			memset(uint8List, 0, sizeof(uint8List));
			memset(floatList, 0, sizeof(floatList));
			
			type = HWType_UNKNOWN;
			address = 0xff;
		}
	
		HardwareCommandResult() {
			reset();
		}

		virtual ~HardwareCommandResult() {
		}
		
		uint8_t getUintListNum() {
			return uintlistnum;
		}

		uint8_t getFloatListNum() {
			return uintlistnum;
		}
		
		uint8_t* getUintList() {
			return uint8List;
		}
		
		float* getFloatList() {
			return floatList;
		}
		
		void setUintListNum(uint8_t num) {
			this->uintlistnum = num;
		}

		void setFloatListNum(uint8_t num) {
			this->floatlistnum = num;
		}
		
		void setHardwareType(HardwareTypeIdentifier type) {
			this->type = type;
		}
		
		HardwareTypeIdentifier getHardwareType() {
			return this->type;
		}
		
		void setAddress(uint8_t address) {
			this->address = address;
		}
		
		uint8_t getAddress() {
			return address;
		}
		
		uint8_t isReadRequest() {
			return isRead;
		}

		void setReadRequest(uint8_t val) {
			isRead = val;
		}
		
		/**
		 */
		void serialize(command_t* hwresult) {
			hwresult->address = address;
			hwresult->type = type;
			hwresult->uintlistnum = uintlistnum;
			hwresult->floatlistnum = floatlistnum;
			hwresult->isRead = isRead;
			memcpy(hwresult->uint8list, uint8List, sizeof(uint8List));
			memcpy(hwresult->floatlist, floatList, sizeof(floatList));
		}
		
		void deSerialize(command_t* hwresult) {
			reset();
			
			this->address = hwresult->address;
			this->type = hwresult->type;
			this->isRead = hwresult->isRead;
			this->uintlistnum = hwresult->uintlistnum;
			this->floatlistnum = hwresult->floatlistnum;
			
			memcpy(this->uint8List, hwresult->uint8list, hwresult->uintlistnum);
			memcpy(this->floatList, hwresult->floatlist, hwresult->floatlistnum * sizeof(float));
		}
};

#endif  //_HARDWARERESULT_H
