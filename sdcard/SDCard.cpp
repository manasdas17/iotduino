//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : SDCard.cpp
//  @ Date : 20.10.2014
//  @ Author :
//
//
#include <sdcard/SDcard.h>

boolean SDcard::openFile(const char* fileName, FileMode mode) {
	#ifdef DEBUG_SD_ENABLE
		Serial.print(millis());
		Serial.print(F(": SDcard::openFile() opening "));
		Serial.print(fileName);
		Serial.print(F(" mode="));
		Serial.println(mode);
		Serial.flush();
	#endif

	if(!strcmp(fileName, fileNameDiscoveryInfo)) {
		myFileDiscovery = SD.open(fileNameDiscoveryInfo, WRITE);
		return myFileDiscovery;
	} else if(!strcmp(fileName, fileNameNodeInfo)) {
		myFileInfo = SD.open(fileNameNodeInfo, WRITE);
		return myFileInfo;
	}

	return false;
}

uint8_t SDcard::getNodeInfoString(uint8_t nodeId, uint8_t* buf, uint8_t bufSize) {
	if(bufSize < NODE_INFO_SIZE)
		return false;

	uint32_t pos = nodeId * NODE_INFO_SIZE;
	if(!myFileInfo.seek(pos)) {
		return false;
	}

	return myFileInfo.readBytes(buf, NODE_INFO_SIZE);
}

boolean SDcard::initSD() {
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

	prepareDiscoveryFile();

	openFile(fileNameDiscoveryInfo, WRITE);
	openFile(fileNameNodeInfo, WRITE);

	return true;
}

void SDcard::prepareDiscoveryFile() {
	#ifdef DEBUG_SD_ENABLE
		Serial.print(millis());
		Serial.print(F(": SDcard::prepareDiscoveryFile() fileName="));
		Serial.println(fileNameDiscoveryInfo);
	#endif

	//prepare discovery swap
	if(myFileDiscovery) {
		uint16_t bytes = 0;
		uint8_t val = 0;
		while(myFileDiscovery.size() < SD_DISCOVERY_FILESIZE) {
			bytes += myFileDiscovery.write(&val, 1);
		}
		myFileDiscovery.flush();

		#ifdef DEBUG_SD_ENABLE
			if(bytes > 0) {
				Serial.print(F("\textented by "));
				Serial.print(bytes);
				Serial.println(F(" bytes"));
				Serial.flush();
			}
		#endif
	}
}

boolean SDcard::appendToFile(const char* fileName, uint8_t* buf, uint8_t bufSize) {
	File tmp = SD.open(fileName, WRITE);
	if(!tmp)
		return false;

	boolean success = tmp.write(buf, bufSize);
	tmp.flush();

	return success;
}