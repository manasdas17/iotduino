#include "DiscoveryManager.h"


const char* DiscoveryManager::filenameDiscovery = {DISCOVERY_SD_FILENAME};



boolean DiscoveryManager::sendDiscoveryRequest(l3_address_t node) {
	#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
			Serial.print(millis());
			Serial.print(F(": SubscriptionManager::sendDiscoveryRequest() node="));
			Serial.println(node);
			Serial.flush();
		#endif

		Layer3::packet_t p;
		memset(&p, 0, sizeof(p));
		//seq_t seq = pf.generateDiscoveryInfoRequest(&p, node);
		pf.generateDiscoveryInfoRequest(&p, node);

		return l3.sendPacket(p);
}

void DiscoveryManager::readElemIntoVar(Discovery_nodeDiscoveryInfoTableEntry_t* elem, l3_address_t remote, uint8_t i) {
	ram.readElementIntoVar(memRegionDiscoveryInfo, NUM_INFOS_PER_NODE * remote + i, elem);
}

void DiscoveryManager::init() {
	#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
	Serial.print(millis());
	Serial.println(F(": SubscriptionManager::init() node="));
	Serial.flush();
	#endif
	listenerDiscovery.init(0, webserverListener::START);

	lastSDWrite = 0;

	lastDiscoveryRequest = 0;
	lastDiscoveryRequestFullRun = 0;
	nextDiscoveryRequestNeighbourIndex = 0;

	memRegionDiscoveryInfo = ram.createRegion(sizeof(Discovery_nodeDiscoveryInfoTableEntry_t), NUM_KNOWN_NODES * NUM_INFOS_PER_NODE);

	readDataFromSDCard();
}

void DiscoveryManager::loop() {
	//maintainSubscriptions();
	maintainDiscoveries();
	maintainListeners();

	uint32_t timeNow = millis();
	if(timeNow > DISCOVERY_SD_WRITE_PERIOD_MILLIS && lastSDWrite < timeNow - DISCOVERY_SD_WRITE_PERIOD_MILLIS) {
		//write
		lastSDWrite = timeNow;
		writeDataToSDCard();
		nodeInfo.writeInfoToSDCard();
	}
}

void DiscoveryManager::maintainListeners() {
	if(listenerDiscovery.state == webserverListener::FINISHED) {
			//init buffer for sd storage
			Discovery_nodeDiscoveryInfoTableEntry_t buf[NUM_INFOS_PER_NODE];
			l3_address_t currentNodeID = listenerDiscovery.remote;

			for(uint8_t i = 0; i < NUM_INFOS_PER_NODE; i++) {
				ram.readElementIntoVar(memRegionDiscoveryInfo, currentNodeID * NUM_INFOS_PER_NODE + i, &buf[i]);
			}

			uint32_t timeNow = now();

			//put data into buffer and store to ram if data is new.
			uint8_t currentHwAddress = 0;
			uint8_t currentHwType = 0;
			//iterate over all gotten infos
			for(uint8_t i = 0; i < listenerDiscovery.gottenInfos; i++) {
				currentHwAddress = listenerDiscovery.sensorInfos[i].hardwareAddress;
				currentHwType = listenerDiscovery.sensorInfos[i].hardwareType;

				//search existing, free or oldest index to update.
				uint8_t firstFreeIndex = 0xff;
				uint8_t oldestIndex = 0xff;
				uint32_t oldestTime = -1;
				uint8_t toBeChangedIndex = 0xff;
				for(uint8_t j = 0; j < NUM_INFOS_PER_NODE; j++) {
					Discovery_nodeDiscoveryInfoTableEntry_t* currentRamItem = (Discovery_nodeDiscoveryInfoTableEntry_t*) &buf[j];

					//this one is free and we have not found a free index yet
					if(buf[j].hardwareAddress == 0 && buf[j].hardwareType == 0 && firstFreeIndex < 0xff) {
						firstFreeIndex = j;
						continue;
					}

					//we are still searching a suitable index
					if(oldestTime > currentRamItem->rtcTimestamp) {
						oldestTime = currentRamItem->rtcTimestamp;
						oldestIndex = j;
					}

					//exact match, update this one
					if((buf[j].hardwareAddress == currentHwAddress && buf[j].hardwareType == currentHwType)) {
						toBeChangedIndex = j;
						break;
					}
				}//search complete.

				//determine index to update.
				if(toBeChangedIndex == 0xff) {
					if(firstFreeIndex == 0xff) {
						//no exact match, nothing free, update oldest.
						toBeChangedIndex = oldestIndex;
					} else {
						//no exact match, but a free index available
						toBeChangedIndex = firstFreeIndex;
					}
					buf[toBeChangedIndex].hardwareAddress = currentHwAddress;
					buf[toBeChangedIndex].hardwareType = currentHwType;
				}

				buf[toBeChangedIndex].rtcTimestamp = timeNow;

				//write to ram
				ram.writeElementToRam(memRegionDiscoveryInfo, currentNodeID * NUM_INFOS_PER_NODE + toBeChangedIndex, &buf[toBeChangedIndex]);
			}

			//store node info
			nodeInfo.updateDiscoveryTime(listenerDiscovery.remote, timeNow);

			//reset listener
			listenerDiscovery.init(0, webserverListener::START);
			dispatcher.getResponseHandler()->unregisterListener(&listenerDiscovery);
		} else if(listenerDiscovery.state == webserverListener::FAILED) {
				#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				Serial.print(millis());
				Serial.print(F(": discovery failed for node="));
				Serial.println(listenerDiscovery.remote);
				#endif
			//reset listener
			listenerDiscovery.init(0, webserverListener::START);
			dispatcher.getResponseHandler()->unregisterListener(&listenerDiscovery);
		}
}

void DiscoveryManager::maintainDiscoveries() {
	uint32_t now = millis();

		//delay the sending.
		if(	listenerDiscovery.state == webserverListener::START && now > DISCOVERY_REQUEST_DELAY_MILLIS && now - DISCOVERY_REQUEST_DELAY_MILLIS > lastDiscoveryRequest
			&& now > DISCOVERY_REQUEST_PERIOD_MILLIS && now - DISCOVERY_REQUEST_PERIOD_MILLIS > lastDiscoveryRequestFullRun) {

			#ifdef ENABLE_EXTERNAL_RAM
				ram.readElementIntoBuffer(l3.getNeighbourManager()->memRegionId, nextDiscoveryRequestNeighbourIndex);
				l3_address_t remote = ((NeighbourManager::neighbourData_t*) ram.buffer)->nodeId;
			#else
				l3_address_t remote = l3.getNeighbourManager()->neighbours[nextDiscoveryRequestNeighbourIndex].nodeId;
			#endif

			//loopback?
			if(nextDiscoveryRequestNeighbourIndex == CONFIG_L3_NUM_NEIGHBOURS) {
				remote = l3.localAddress;
			}

			//we have a node, send request.
			if(remote > 0) {
				#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
					Serial.print(millis());
					Serial.println(F(": SubscriptionManager::maintainDiscoveries()"));

					Serial.print(F("\tnow="));
					Serial.print(now);
					Serial.print(F(" lastDicoveryRequest="));
					Serial.print(lastDiscoveryRequest);
					Serial.print(F(" lastDiscoveryRequestFullRun="));
					Serial.println(lastDiscoveryRequestFullRun);

					Serial.print(F("\tsending discovery request to neighbourIndex="));
					Serial.print(nextDiscoveryRequestNeighbourIndex);
					Serial.print(F(" nodeId="));
					Serial.println(remote);
					Serial.flush();
				#endif

				//register listener by type.
				listenerDiscovery.init(remote, webserverListener::AWAITING_ANSWER);
				dispatcher.getResponseHandler()->registerListenerByPacketType(millis() + DISCOVERY_TIMEOUT, HARDWARE_DISCOVERY_RES, remote, &listenerDiscovery);

				//send request
				sendDiscoveryRequest(remote);
				lastDiscoveryRequest = now;
			}

			//iterate
			nextDiscoveryRequestNeighbourIndex++;
			if(nextDiscoveryRequestNeighbourIndex > CONFIG_L3_NUM_NEIGHBOURS) {
				nextDiscoveryRequestNeighbourIndex = 0;
				lastDiscoveryRequestFullRun = now;
			}
		}
}

boolean DiscoveryManager::readDataFromSDCard() {
	#ifdef SDCARD_ENABLE
	#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
	Serial.print(millis());
	Serial.println(F(": SubscriptionManager::readDataFromSDCard()"));
	Serial.flush();
	#endif

	File f = SD.open(filenameDiscovery, FILE_READ);

	if(!f) {
		return false;
	}

	Discovery_nodeDiscoveryInfoTableEntry_t elem;
	uint16_t i = 0;
	while(f.read(&elem, sizeof(Discovery_nodeDiscoveryInfoTableEntry_t)) == sizeof(Discovery_nodeDiscoveryInfoTableEntry_t)) {
		if(elem.hardwareAddress != 0 && elem.hardwareType != 0) {
			ram.writeElementToRam(memRegionDiscoveryInfo, i, &elem);
		}

		#ifdef SDCARD_ENABLE
		if(i % 100 == 0) {
			Serial.print(i);
			Serial.print('/');
			Serial.println(1UL * NUM_INFOS_PER_NODE * NUM_KNOWN_NODES);
		}
		#endif

		wdt_reset();
		i++;
	}

	f.close();
	return true;
	#endif
	return false;
}

boolean DiscoveryManager::getIteratorDiscovery(SPIRamManager::iterator* it) {
	return it->init(&ram, memRegionDiscoveryInfo);
}

boolean DiscoveryManager::writeDataToSDCard() {
	#ifdef SDCARD_ENABLE
		#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
		Serial.print(millis());
		Serial.println(F(": SubscriptionManager::writeDataToSDCard()"));
		Serial.flush();
		#endif

		SPIRamManager::iterator it;
		getIteratorDiscovery(&it);

		//delete old info
		SD.remove((char*) filenameDiscovery);

		//reopen file for write
		File f = SD.open(filenameDiscovery, FILE_WRITE);

		if(!f) {
			return false;
		}

		if(!SDcard::fillFile(&f, 0, sizeof(Discovery_nodeDiscoveryInfoTableEntry_t) * NUM_INFOS_PER_NODE * NUM_KNOWN_NODES))
			return false;

		Discovery_nodeDiscoveryInfoTableEntry_t* elem = NULL;
		while(it.hasNext()) {
			elem = (Discovery_nodeDiscoveryInfoTableEntry_t*) it.next();

			if(elem->hardwareAddress != 0 && elem->hardwareType != 0) {
				uint32_t pos = (it.getIteratorIndex()-1) * sizeof(Discovery_nodeDiscoveryInfoTableEntry_t);
				if(!f.seek(pos)) {
					#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
						Serial.print(millis());
						Serial.print(F(": NodeInfo::writeDataToSDCard() seek to pos="));
						Serial.print(pos);
						Serial.print(F("failed for index="));
						Serial.println(it.getIteratorIndex()-1);
						Serial.flush();
					#endif
					continue;
				}

				if(f.write((uint8_t*) elem, sizeof(Discovery_nodeDiscoveryInfoTableEntry_t)) == 0) {
					#ifdef DEBUG
						Serial.print(millis());
						Serial.print(F(": NodeInfo::writeDataToSDCard() write at pos="));
						Serial.print(pos);
						Serial.print(F("failed for index="));
						Serial.println(it.getIteratorIndex()-1);
						Serial.flush();
					#endif
				}
			}
			wdt_reset();
		}
		//SPIRamManager::memRegion_t region;
		//ram.getRegionInfo(&region, memRegionDiscoveryInfo);
		//uint8_t numElemsPerBufferPage = ram.bufferSize / sizeof(Discovery_nodeDiscoveryInfoTableEntry_t);
		//uint16_t remaining = NUM_KNOWN_NODES * NUM_INFOS_PER_NODE;
		//uint32_t currentAddress = region.ramStartAddress;
		//while(remaining > 0) {
			//uint16_t remainingForThisBufferPage = min(numElemsPerBufferPage, remaining);
			//ram.memcpy_R(ram.buffer, currentAddress,  remainingForThisBufferPage * region.elementSize);
			//f.write(ram.buffer, remainingForThisBufferPage * region.elementSize);
			//
			//remaining -= remainingForThisBufferPage;
		//}


		f.flush();
		f.close();
		#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
		Serial.print(millis());
		Serial.println(F(": SubscriptionManager::writeDataToSDCard() finished."));
		Serial.flush();
		#endif
		return true;
		#endif
		return false;
}
