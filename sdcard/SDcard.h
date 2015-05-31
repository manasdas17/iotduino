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

#define PIN_SD_SS 4
const char* fileNameNodeInfo = {"NODEINFO.TXT"};

class SDcard {
	private:
	File myFile;
	enum FileMode {READ = FILE_READ, WRITE = FILE_WRITE};


	public:
	#define NODE_INFO_SIZE (0xf)

	SDcard() {
	}

	boolean init() {
		if (!SD.begin(PIN_SD_SS)) {
			#ifdef DEBUG_SD_ENABLE
				Serial.println(F("init failed"));
			#endif
			return false;
		}
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
		myFile = SD.open(fileNameNodeInfo, READ);
		if(!myFile.seek(((uint32_t) (nodeId-1)) * NODE_INFO_SIZE)) {
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

#endif /* SDCARD_H_ */