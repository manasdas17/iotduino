/*
* SDHardwareResponseListener.cpp
*
* Created: 31.05.2015 21:25:20
* Author: helge
*/


#include "SDHardwareResponseListener.h"
#include <interfaces/input/RTC.h>
#include <SD.h>

extern RTC rtc;

#define SD_LOGGER_BINARY

void SDHardwareRequestListener::doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq) {
	//abort for uninteresting packets
	if(appLayerPacket->packetType != HARDWARE_COMMAND_RES)
		return;

	//command
	command_t* cmd = (command_t*) appLayerPacket->payload;

	#ifdef DEBUG_SD_ENABLE
		Serial.print(millis());
		Serial.println(F(": SDHardwareRequestListener::doCallback received HARDWARE_COMMAND_RES"));
		Serial.print(F("\tfrom="));
		Serial.print(address);
		Serial.print(F("\tisRead="));
		Serial.print(cmd->isRead);
		Serial.print(F(" hwType="));
		Serial.print(cmd->type);
		Serial.print(F(" hwAddress="));
		Serial.println(cmd->address);
		Serial.flush();
	#endif

	if(!cmd->isRead) {
		//discard.
		return;
	}

	//filename according to "{HexRemoteAddress}_{HexHardwareType}_{HexHardwareAddress}.LOG"
	//e.g. A7_08_14.LOG
	char filename[13];
	snprintf(filename, 13, "%02X_%02X_%02X.LOG", address, cmd->type, cmd->address);

	//content to write: +--------------+------------+
	//                  | 4b timestamp | 4byte info |
	//                  +--------------+------------+
	char bytes[40];
	uint8_t offset = 0;

	#ifdef RTC_ENABLE
		uint32_t now = rtc.read();
	#else
		uint32_t now = millis();
		#ifdef SDCARD_ENABLE
			#warning RTC_ENABLE NOT SET, USING INTERNAL MILLIS FOR TIMESTAMP.
		#endif
	#endif

	#ifdef SD_LOGGER_BINARY
		memcpy(&bytes[0], &now, 4);
		offset += 4;
		memcpy(&bytes[4], cmd->uint8list, sizeUInt8List);
		offset += sizeUInt8List;
		#ifdef DEBUG_SD_ENABLE
			Serial.print(millis());
			Serial.println(F(": sensor data listener storing binary."));
		#endif
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
			#ifdef DEBUG_SD_ENABLE
				Serial.print(millis());
				Serial.print(F(": sensor data listener not binary, nothing to write."));
			#endif
			return;
		//}
	#endif

	//write if possible.
	File tmp = SD.open(filename, FILE_WRITE);
	if(!tmp)
		return;

	tmp.write(bytes, offset);
	tmp.flush();
}
