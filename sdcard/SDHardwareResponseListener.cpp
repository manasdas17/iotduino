/*
* SDHardwareResponseListener.cpp
*
* Created: 31.05.2015 21:25:20
* Author: helge
*/


#include "SDHardwareResponseListener.h"

void SDHardwareRequestListener::doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq) {
	//abort for uninteresting packets
	if(appLayerPacket->packetType != HARDWARE_COMMAND_RES)
		return;

	//command
	command_t* cmd = (command_t*) appLayerPacket->payload;

	//filename according to "{HexRemoteAddress}_{HexHardwareType}_{HexHardwareAddress}.LOG"
	//e.g. A7_08_14.LOG
	char filename[13];
	snprintf(filename, 13, "%02X_%02X_%02X.LOG", address, cmd->type, cmd->address);

	//content to write: +--------------+------------+
	//                  | 4b timestamp | 4byte info |
	//                  +--------------+------------+
	#define size (sizeUInt8List + 4)
	uint8_t bytes[size]; //timestamp
	uint32_t now = rtc.read();
	memcpy(&bytes[0], &now, 4);
	memcpy(&bytes[4], cmd->uint8list, sizeUInt8List);

	//write
	sdcard.appendToFile(filename, bytes, size);
	#undef size
}
