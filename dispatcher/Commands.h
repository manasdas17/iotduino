
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
#include <drivers/HardwareID.h>

#define sizeUInt8List 8 //enough size for 4 16bit vars.


typedef struct hwCommand {
	HardwareTypeIdentifier type;
	uint8_t isRead;
	uint8_t address;

	uint8_t numUint8 : 4;
	uint8_t numUint16 : 4;
	uint8_t numInt8 : 4;
	uint8_t numInt16 : 4;

	union {
		uint8_t uint8list[sizeUInt8List];
		uint16_t uint16list[(sizeUInt8List / 2)];
		int8_t int8list[sizeUInt8List];
		int16_t int16list[(sizeUInt8List / 2)];
	};
} command_t; //Maximum length CONFIG_L3_PACKET_NUMBERED_MAX_LEN

#if (CONFIG_L3_PACKET_NUMBERED_MAX_LEN < (4 + sizeUInt8List*1 + sizeFloatList*4))
	#error maximum payload len exceeded.
#endif

class HardwareCommandResult {
	//const static uint8_t sizeFloatList = 4;
	//const static uint8_t sizeUInt8List = 8;

	uint8_t isRead;

	uint8_t uint8List[sizeUInt8List];

	uint8_t numUint8;
	uint8_t numUint16;
	uint8_t numInt8;
	uint8_t numInt16;

	HardwareTypeIdentifier type;
	uint8_t address;

	public:
		void reset() {
			numUint16 = 0;
			numUint8 = 0;
			numInt16 = 0;
			numInt8 = 0;

			memset(uint8List, 0, sizeof(uint8List));

			type = HWType_UNKNOWN;
			address = 0xff;
		}

		HardwareCommandResult() {
			reset();
		}

		virtual ~HardwareCommandResult() {
		}

		uint8_t getUint8ListNum() {
			return numUint8;
		}
		uint16_t getUint16ListNum() {
			return numUint16;
		}
		int8_t getInt8ListNum() {
			return numInt8;
		}
		int16_t getInt16ListNum() {
			return numInt16;
		}

		uint8_t* getUint8List() {
			return uint8List;
		}
		uint16_t* getUint16List() {
			return (uint16_t*) uint8List;
		}
		int8_t* getInt8List() {
			return (int8_t*) uint8List;
		}
		int16_t* getInt16List() {
			return (int16_t*) uint8List;
		}

		void setUint8ListNum(uint8_t num) {
			this->numUint8 = num;
		}
		void setUint16ListNum(uint8_t num) {
			this->numUint16 = num;
		}
		void setInt8ListNum(uint8_t num) {
			this->numInt8 = num;
		}
		void setInt16ListNum(uint8_t num) {
			this->numInt16 = num;
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
		boolean serialize(command_t* hwresult) {
			hwresult->address = address;
			hwresult->type = type;
			hwresult->numUint8 = numUint8;
			hwresult->numUint16 = numUint16;
			hwresult->numInt8 = numInt8;
			hwresult->numInt16 = numInt16;
			hwresult->isRead = isRead;

			memcpy(hwresult->uint8list, uint8List, sizeof(uint8List));

			return true;
		}

		boolean deSerialize(command_t* hwresult) {
			reset();

			this->address = hwresult->address;
			this->type = hwresult->type;
			this->isRead = hwresult->isRead;
			this->numUint8 = hwresult->numUint8;
			this->numUint16 = hwresult->numUint16;
			this->numInt8 = hwresult->numInt8;
			this->numInt16 = hwresult->numInt16;

			memcpy(this->uint8List, hwresult->uint8list, sizeof(uint8List));

			return true;
		}
};

#endif  //_HARDWARERESULT_H
