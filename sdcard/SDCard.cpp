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

const char* fileNameNodeInfo = {"NODEINFO.TXT"};

boolean SDcard::openFile(char* fileName, FileMode mode) {
	#ifdef DEBUG_SD_ENABLE
	Serial.print(F("openeing "));
	Serial.print(fileName);
	Serial.print(F(" mode="));
	Serial.println(mode);
	#endif
	myFile = SD.open(fileName, mode);

	return myFile;
}

uint8_t SDcard::getNodeInfo(uint8_t nodeId, uint8_t* buf, uint8_t bufSize) {
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

boolean SDcard::appendToFile(char* fileName, uint8_t* buf, uint8_t bufSize) {
	openFile(fileName, WRITE);
	boolean success = myFile.write(buf, bufSize);
	myFile.close();

	return success;
}

boolean SDcard::init() {
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

 SDcard::SDcard() {

}