/*
* SDHardwareResponseListener.cpp
*
* Created: 31.05.2015 21:25:20
* Author: helge
*/


#include "SDHardwareResponseListener.h"

#define SD_LOGGER_BINARY

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
	char bytes[40];
	uint8_t offset = 0;
	uint32_t now = rtc.read();

	#ifdef SD_LOGGER_BINARY
		memcpy(&bytes[0], &now, 4);
		memcpy(&bytes[4], cmd->uint8list, sizeUInt8List);
	#else
		//if(cmd->numUint8 > 0) {
			//sprintf(bytes, "%10lu", now);
			//offset += 10;
			//for(uint8_t i = 0; i < cmd->numUint8; i++) {
				//sprintf(bytes+offset, "%3u", cmd->uint8list[i]);
				//offset += 3;
			//}
		//} else if (cmd->numUint16 > 0) {
		//} else if(cmd->numInt16 > 0) {
		//} else {
			////no data.
			return;
		//}
	#endif

	//write if possible.
	sdcard.appendToFile(filename, (uint8_t*) bytes, offset);
	#undef size
}
