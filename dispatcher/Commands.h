
/*
 * Commands.h
 *
 * Created: 18.01.2015 22:06:46
 *  Author: helge
 */

#if !defined(_HARDWARERESULT_H)
#define _HARDWARERESULT_H

#include <Configuration.h>
#include <networking/Packets.h> //used for sanity check
#include <drivers/HardwareID.h> //types are referenced

typedef struct hwCommand {
	HardwareTypeIdentifier type;
	uint8_t isRead;
	uint8_t address;

	uint8_t isEventType;

	uint8_t numUint8;
	uint8_t numUint16;
	uint8_t numInt8;
	uint8_t numInt16;

	union {
		uint8_t uint8list[sizeUInt8List];
		uint16_t uint16list[(sizeUInt8List / 2)];
		int8_t int8list[sizeUInt8List];
		int16_t int16list[(sizeUInt8List / 2)];
	};
} command_t; //Maximum length CONFIG_L3_PACKET_NUMBERED_MAX_LEN


/**
 * this class is a representation of a hardware read/write command.
 * it may hold up to <code>sizeUint8List</code> bytes.
 * they may be accessed as u/int8/16 - the actual data buffer is the same.
 */
class HardwareCommandResult {
	uint8_t isRead;

	uint8_t uint8List[sizeUInt8List];

	uint8_t numUint8;
	uint8_t numUint16;
	uint8_t numInt8;
	uint8_t numInt16;

	HardwareTypeIdentifier type;
	uint8_t address;

	subscription_event_type_t eventType;

	public:
		void reset();

		HardwareCommandResult();

		~HardwareCommandResult() {
		}

		inline uint8_t getUint8ListNum() {
			return numUint8;
		}
		inline uint16_t getUint16ListNum() {
			return numUint16;
		}
		inline int8_t getInt8ListNum() {
			return numInt8;
		}
		inline int16_t getInt16ListNum() {
			return numInt16;
		}

		inline uint8_t* getUint8List() {
			return uint8List;
		}
		inline uint16_t* getUint16List() {
			return (uint16_t*) uint8List;
		}
		inline int8_t* getInt8List() {
			return (int8_t*) uint8List;
		}
		inline int16_t* getInt16List() {
			return (int16_t*) uint8List;
		}

		inline void setUint8ListNum(uint8_t num) {
			this->numUint8 = num;
		}
		inline void setUint16ListNum(uint8_t num) {
			this->numUint16 = num;
		}
		inline void setInt8ListNum(uint8_t num) {
			this->numInt8 = num;
		}
		inline void setInt16ListNum(uint8_t num) {
			this->numInt16 = num;
		}

		inline void setHardwareType(HardwareTypeIdentifier type) {
			this->type = type;
		}

		inline HardwareTypeIdentifier getHardwareType() {
			return this->type;
		}

		inline void setAddress(uint8_t address) {
			this->address = address;
		}

		inline uint8_t getAddress() {
			return address;
		}

		inline uint8_t isReadRequest() {
			return isRead;
		}

		inline void setReadRequest(uint8_t val) {
			isRead = val;
		}

		inline void setEventType(subscription_event_type_t t) {
			eventType = t;
		}

		inline subscription_event_type_t getEventType() {
			return eventType;
		}
		/**
		 * put data into the command struct - copies complete size of uint8list
		 * @param hwresult
		 * @return true
		 */
		boolean serialize(command_t* hwresult);

		/**
		 * read data from struct and copy it into this object.
		 * @param hwresult
		 * @return true
		 */
		boolean deSerialize(command_t* hwresult);
};

#endif  //_HARDWARERESULT_H
