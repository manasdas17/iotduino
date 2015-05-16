#include "Commands.h"

boolean HardwareCommandResult::deSerialize(command_t* hwresult) {
	reset();

	this->address = hwresult->address;
	this->type = hwresult->type;
	this->isRead = hwresult->isRead;
	this->numUint8 = hwresult->numUint8;
	this->numUint16 = hwresult->numUint16;
	this->numInt8 = hwresult->numInt8;
	this->numInt16 = hwresult->numInt16;

	memcpy(this->uint8List, hwresult->uint8list, sizeof(hwresult->uint8list));

	return true;
}

boolean HardwareCommandResult::serialize(command_t* hwresult) {
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

 HardwareCommandResult::HardwareCommandResult() {
	reset();
}

void HardwareCommandResult::reset() {
	numUint16 = 0;
	numUint8 = 0;
	numInt16 = 0;
	numInt8 = 0;

	memset(uint8List, 0, sizeof(uint8List));

	type = HWType_UNKNOWN;
	address = 0;
}
