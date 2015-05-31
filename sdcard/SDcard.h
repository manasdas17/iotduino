/*
 * SDcard.h
 *
 * Created: 31.05.2015 10:32:51
 *  Author: helge
 */


#ifndef SDCARD_H_
#define SDCARD_H_

#include <Arduino.h>
#include <DebugConfig.h>
#include <SD/SD.h>
#include <dispatcher/EventCallbackInterface.h>
#include <dispatcher/Commands.h>

#define PIN_SD_SS 4
const char* fileNameNodeInfo = {"NODEINFO.TXT"};

class SDcard {
	private:
		File myFile;
		enum FileMode {READ = FILE_READ, WRITE = FILE_WRITE};

	public:
		#define NODE_INFO_SIZE (0x10)

		SDcard() {
		}

		boolean init() {
			if (!SD.begin(PIN_SD_SS)) {
				#ifdef DEBUG_SD_ENABLE
					Serial.print(millis());
					Serial.println(F(": init failed"));
				#endif
				return false;
			}
			#ifdef DEBUG_SD_ENABLE
				Serial.print(millis());
				Serial.println(F(": sd init done"));
			#endif

			return true;
		}

		boolean appendToFile(char* fileName, uint8_t* buf, uint8_t bufSize) {
			openFile(fileName, WRITE);
			boolean success = myFile.write(buf, bufSize);
			myFile.close();

			return success;
		}

		/**
		 * get node info from file.
		 * structure:
		 * Node 0:   0x00000000..0x00000000f [unused]
		 * Node 1:   0x00000010..0x00000001f
		 *  ...
		 * Node 255: 0x00000ff0..0x000000fff[broadcast]
		 */
		uint8_t getNodeInfo(uint8_t nodeId, uint8_t* buf, uint8_t bufSize) {
			if(bufSize < NODE_INFO_SIZE)
				return false;

			myFile.close();

			if(!SD.exists((char*) fileNameNodeInfo)) {
				Serial.print(millis());
				Serial.println(F(": file does not exist."));
				return 0;
			}

			myFile = SD.open(fileNameNodeInfo, READ);
			uint32_t pos = ((uint32_t) (nodeId-1)) * NODE_INFO_SIZE;
			if(!myFile.seek(pos)) {
				return false;
			}

			return myFile.readBytes(buf, NODE_INFO_SIZE);
		}

	protected:
		boolean openFile(char* fileName, FileMode mode) {
			#ifdef DEBUG_SD_ENABLE
			Serial.print(F("openeing "));
			Serial.print(fileName);
			Serial.print(F(" mode="));
			Serial.println(mode);
			#endif
			myFile = SD.open(fileName, mode);

			return myFile;
		}
};


#ifdef RTC_ENABLE
extern RTC rtc;
extern SDcard sdcard;

class SDHardwareRequestListener : public EventCallbackInterface {
	void doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq) {
		if(appLayerPacket->packetType != HARDWARE_COMMAND_RES)
		return;
		command_t* cmd = (command_t*) appLayerPacket->payload;

		char filename[13];
		snprintf(filename, 13, "%X_%X_%X.LOG", address, cmd->type, cmd->address);

		#define size (sizeUInt8List + 4)
		uint8_t bytes[size]; //timestamp
		uint32_t now = rtc.read();
		memcpy(&bytes[0], &now, 4);
		memcpy(&bytes[4], cmd->uint8list, sizeUInt8List);

		sdcard.appendToFile(filename, bytes, size);
		#undef size
	}

	void fail(seq_t seq, l3_address_t remote) {
	}
};
#endif
#endif /* SDCARD_H_ */