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

boolean SDcard::deleteSubscriptionInfo() {
	boolean success = true;
	for(uint8_t i = 0; i < SD_DISCOVERY_NUM_SUBSCRIPTIONS_PER_NODE; i++) {
		success &= deleteSubscriptionInfo(i);
	}

	return success;
}

boolean SDcard::deleteSubscriptionInfo(uint8_t nodeId) {
	if(nodeId >= SD_DISCOVERY_NUM_NODES)
		return false;

		boolean success = true;
		for(uint8_t i = 0; i < SD_DISCOVERY_NUM_SUBSCRIPTIONS_PER_NODE; i++) {
			success &= deleteSubscriptionInfo(nodeId, i);
		}

		return success;
}

boolean SDcard::deleteSubscriptionInfo(uint8_t nodeId, uint8_t index) {
	if(nodeId >= SD_DISCOVERY_NUM_NODES || index >= SD_DISCOVERY_NUM_INFOS_PER_NODE)
		return false;

	uint8_t tmp[sizeof(SD_subscriptionInfoTableEntry_t)];
	memset(tmp, 0, sizeof(tmp));

	uint32_t pos = getSubscriptionInfosAddressForNodeById(nodeId, index);
	return discoveryWriteToSD(pos, tmp, sizeof(SD_subscriptionInfoTableEntry_t));
}

boolean SDcard::saveSubscriptionInfo(uint8_t nodeId, uint8_t index, SD_subscriptionInfoTableEntry_t* info) {
	if(nodeId >= SD_DISCOVERY_NUM_NODES || index >= SD_DISCOVERY_NUM_INFOS_PER_NODE)
		return false;

	uint32_t pos = getSubscriptionInfosAddressForNodeById(nodeId, index);
	return discoveryWriteToSD(pos, (uint8_t*) info, sizeof(SD_subscriptionInfoTableEntry_t));
}

boolean SDcard::saveSubscriptionsInfos(uint8_t nodeId, SD_subscriptionInfoTableEntry_t* info, uint8_t numInfos) {
	if(numInfos > SD_DISCOVERY_NUM_INFOS_PER_NODE)
		return false;

	//prepare data
	SD_subscriptionInfoTableEntry_t data[SD_DISCOVERY_NUM_SUBSCRIPTIONS_PER_NODE];
	memset(&data[numInfos], 0, sizeof(data) - numInfos * sizeof(SD_subscriptionInfoTableEntry_t));
	memcpy(data, info, numInfos * sizeof(SD_subscriptionInfoTableEntry_t));

	//write to sd
	uint32_t pos = getSubscriptionsInfosAddressForNode(nodeId);
	uint16_t len = sizeof(data);
	return discoveryWriteToSD(pos, (uint8_t*) data, len);
}

boolean SDcard::deleteDiscoveryInfosAll() {
	boolean success = true;
	for(uint8_t i = 0; i < SD_DISCOVERY_NUM_NODES; i++) {
		success &= deleteDiscoveryInfos(i);
	}
	return success;
}

boolean SDcard::deleteDiscoveryInfos(uint8_t nodeId) {
	if(nodeId >= SD_DISCOVERY_NUM_NODES)
		return false;
	uint8_t tmp[SD_DISCOVERY_NUM_INFOS_PER_NODE * sizeof(SD_nodeDiscoveryInfoTableEntry_t)];
	memset(tmp, 0, sizeof(tmp));

	//write to sd
	uint32_t pos = getDiscoveryInfosAddressForNode(nodeId);
	return discoveryWriteToSD(pos, tmp, sizeof(tmp));
}

boolean SDcard::saveDiscoveryInfos(uint8_t nodeId, SD_nodeDiscoveryInfoTableEntry_t* info, uint8_t numInfos, uint8_t startIndex) {
	if(nodeId >= SD_DISCOVERY_NUM_NODES || numInfos > SD_DISCOVERY_NUM_INFOS_PER_NODE - startIndex || info == NULL)
		return false;
		uint32_t pos = getDiscoveryInfosAddressForNode(nodeId) + startIndex * sizeof(SD_nodeDiscoveryInfoTableEntry_t);
		uint16_t len = numInfos * sizeof(SD_nodeDiscoveryInfoTableEntry_t);
		return discoveryWriteToSD(pos, (uint8_t*) info, len);
}

boolean SDcard::saveDiscoveryInfos(uint8_t nodeId, SD_nodeDiscoveryInfoTableEntry_t* info, uint8_t numInfos) {
	if(nodeId >= SD_DISCOVERY_NUM_NODES || numInfos > SD_DISCOVERY_NUM_INFOS_PER_NODE || info == NULL)
		return false;

	//prepare data
	SD_nodeDiscoveryInfoTableEntry_t data[SD_DISCOVERY_NUM_INFOS_PER_NODE];
	memset(&data[numInfos], 0, (SD_DISCOVERY_NUM_INFOS_PER_NODE - numInfos) * sizeof(SD_nodeDiscoveryInfoTableEntry_t));
	memcpy(data, info, numInfos * sizeof(SD_nodeDiscoveryInfoTableEntry_t));

	//write to sd
	uint32_t pos = getDiscoveryInfosAddressForNode(nodeId);
	uint16_t len = sizeof(data);
	return discoveryWriteToSD(pos, (uint8_t*) data, len);
}

boolean SDcard::discoveryWriteToSD(uint32_t pos, void* buf, size_t len) {
	if(seekDiscovery(pos)) {
		uint32_t bytes = myFileDiscovery.write((uint8_t*) buf, len);
		myFileDiscovery.flush();

		#ifdef DEBUG_SD_ENABLE
			Serial.print(millis());
			Serial.print(F(": SDcard::discoveryWriteToSD() pos="));
			Serial.print(pos);
			Serial.print(F(" len="));
			Serial.print(len);
			Serial.print(F(" written="));
			Serial.println(bytes);
		#endif

		return bytes;
	}
	return false;
}

boolean SDcard::saveNodeInfo(uint8_t nodeId, SD_nodeInfoTableEntry_t* info) {
	if(nodeId >= SD_DISCOVERY_NUM_NODES || info == NULL)
		return false;
	uint32_t pos = nodeId * sizeof(SD_nodeInfoTableEntry_t);
	uint16_t len = sizeof(SD_nodeInfoTableEntry_t);
	return discoveryWriteToSD(pos, (uint8_t*) info, len);
}

boolean SDcard::getSubscriptionInfosForNode(uint8_t nodeId, SD_subscriptionInfoTableEntry_t* buf, uint8_t numEntries) {
	if(nodeId >= SD_DISCOVERY_NUM_NODES || numEntries < SD_DISCOVERY_NUM_SUBSCRIPTIONS_PER_NODE || buf == NULL)
		return false;
	//read
	seekDiscovery(getSubscriptionsInfosAddressForNode(nodeId));
	return myFileDiscovery.readBytes((uint8_t*) buf, numEntries * SD_DISCOVERY_NUM_SUBSCRIPTIONS_PER_NODE * sizeof(SD_subscriptionInfoTableEntry_t));
}

uint32_t SDcard::getSubscriptionsInfosAddressForNode(uint8_t nodeId) {
	return subscriptionInfoStart + nodeId * sizeof(SD_subscriptionInfoTableEntry_t);
}

boolean SDcard::getDiscoveryInfosForNode(uint8_t nodeId, SD_nodeDiscoveryInfoTableEntry_t* buf, uint8_t numEntries) {
	if(nodeId >= SD_DISCOVERY_NUM_NODES || numEntries < SD_DISCOVERY_NUM_INFOS_PER_NODE || buf == NULL)
		return false;
	//read
	if(!seekDiscovery(getDiscoveryInfosAddressForNode(nodeId)))
		return false;

	return myFileDiscovery.readBytes((uint8_t*) buf, SD_DISCOVERY_NUM_INFOS_PER_NODE * sizeof(SD_nodeDiscoveryInfoTableEntry_t));
}

uint32_t SDcard::getDiscoveryInfosAddressForNode(uint8_t nodeId) {
	return discoveryInfoStart + nodeId * SD_DISCOVERY_NUM_INFOS_PER_NODE * sizeof(SD_nodeDiscoveryInfoTableEntry_t);
}

boolean SDcard::getDiscoveryNodeInfo(uint8_t nodeId, SD_nodeInfoTableEntry_t* buf) {
	if(nodeId >= SD_DISCOVERY_NUM_NODES || buf == NULL)
		return false;
	//read
	if(!seekDiscovery(getNodeDiscoveryInfoAddress(nodeId))) {
		#ifdef DEBUG_SD_ENABLE
			Serial.print(millis());
			Serial.print(F(" getDiscoveryNodeInfo() seek for node="));
			Serial.print(nodeId);
			Serial.println(F(" failed."));
		#endif
		return false;
	}

	boolean ret = false;
	ret = myFileDiscovery.readBytes((uint8_t*) buf, sizeof(SD_nodeInfoTableEntry_t));
	return ret;
}

uint32_t SDcard::getNodeDiscoveryInfoAddress(uint8_t nodeId) {
	return nodeId * sizeof(SD_nodeInfoTableEntry_t);
}

boolean SDcard::saveNodeInfoString(uint8_t nodeId, uint8_t* buf, uint8_t bufSize) {
	if(bufSize < NODE_INFO_SIZE)
		return false;
	buf[NODE_INFO_SIZE-1] = '\0';
	uint32_t pos = nodeId * NODE_INFO_SIZE;
	if(!myFileInfo.seek(pos)) {
		return false;
	}
	boolean success = myFileInfo.write(buf, NODE_INFO_SIZE);
	myFileInfo.flush();
	return success;
}

boolean SDcard::seekDiscovery(uint32_t pos) {
	if(!myFileDiscovery)
		return false;
	return myFileDiscovery.seek(pos);
}

boolean SDcard::seekInfo(uint32_t pos) {
	if(!myFileInfo)
		return false;
	return myFileInfo.seek(pos);
}

uint32_t SDcard::getSubscriptionInfosAddressForNodeById(uint8_t nodeId, uint8_t index) {
	return subscriptionInfoStart + (nodeId * SD_DISCOVERY_NUM_SUBSCRIPTIONS_PER_NODE * index) * sizeof(SD_subscriptionInfoTableEntry_t);
}
