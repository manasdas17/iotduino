#include "ResponseHandler.h"

boolean ResponseHandler::handleReponseNumbered(const seq_t seq, const packet_type_application_t type, const l3_address_t remote, packet_application_numbered_cmd_t* appPacket) {
	#ifdef DEBUG_HANDLER_RESPONSE_ENABLE
		Serial.print(millis());
		Serial.println(F(": ResponseHandler::handleResponseNumbered()"));
		Serial.print(F("\tseq="));
		Serial.print(seq);
		Serial.print(F(" type="));
		switch(type) {
			case HARDWARE_COMMAND_RES:
			Serial.print(F("HARDWARE_COMMAND_RES"));
			break;
			case HARDWARE_DISCOVERY_RES:
			Serial.print(F("HARDWARE_DISCOVERY_RES"));
			break;
			case HARDWARE_SUBSCRIPTION_SET_RES:
			Serial.print(F("HARDWARE_SUBSCRIPTION_SET_RES"));
			break;
			case HARDWARE_SUBSCRIPTION_INFO_RES:
			Serial.print(F("HARDWARE_SUBSCRIPTION_INFO_RES"));
			break;
			case ACK:
			Serial.print(F("ACK"));
			break;
			case NACK:
			Serial.print(F("NACK"));
			break;
			default:
			Serial.print(F("UNKNOWN"));
		}
		Serial.print(F(" remote="));
		Serial.println(remote);
	#endif

	//search for seq listener
	EventCallbackInterface* listenersList[LISTENER_NUM];
	uint8_t listenersFound = getListenerCallbacks(listenersList, type, seq, remote);

	if(listenersFound == 0)
		return false;

	//iterate
	for(uint8_t i = 0; i < listenersFound; i++) {
		if(type == NACK) {
			listenersList[i]->fail(seq, remote);
		} else {
			listenersList[i]->doCallback(appPacket, remote, seq);
		}
	}
	return true;
}

boolean ResponseHandler::registerListenerBySeq(const uint32_t timeout, const seq_t seqNumber, const l3_address_t remoteAddress, EventCallbackInterface* callbackObject) {
	return registerListener(timeout, seqNumber, (packet_type_application_t) 0xff, remoteAddress, callbackObject);
}
boolean ResponseHandler::registerListenerByPacketType(const uint32_t timeout, packet_type_application_t type, const l3_address_t remoteAddress, EventCallbackInterface* callbackObject) {
	return registerListener(timeout, 0, type, remoteAddress, callbackObject);
}

boolean ResponseHandler::registerListener(const uint32_t timeout, const seq_t seqNumber, const packet_type_application_t type, const l3_address_t remoteAddress, EventCallbackInterface* callbackObject) {
	#ifdef DEBUG_HANDLER_RESPONSE_ENABLE
		Serial.print(millis());
		Serial.print(F(": ResponseHandler::registerListener() remote="));
		Serial.print(remoteAddress);
		Serial.print(F(" seq="));
		Serial.print(seqNumber);
		Serial.print(F(" type="));
		Serial.println(type);
	#endif

	if(callbackObject == NULL)
		return false;


	#ifdef ENABLE_EXTERNAL_RAM
		SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionId);
		responseListener_t* currentItem = NULL;
		boolean handled = false;
		while(it.hasNext()) {
			currentItem = (responseListener_t*) it.next();
			if(currentItem->callbackObj == NULL) {
				currentItem->timestamp = timeout;
				currentItem->callbackObj = callbackObject;
				currentItem->remote = remoteAddress;
				currentItem->seqNumber = seqNumber;
				currentItem->packetType = type;

				it.writeBack();
				handled = true;
				break;
			}
		}

		if(!handled)
			return false;
	#else
		uint8_t index = 0xff;
		for(uint8_t i = 0; i < LISTENER_NUM; i++) {
			if(listeners[i].callbackObj == NULL) {
				index= i;
				break;
			}
		}

		if(index == 0xff) {
			return false;
		}

		listeners[index].timestamp = timeout;
		listeners[index].callbackObj = callbackObject;
		listeners[index].remote = remoteAddress;
		listeners[index].seqNumber = seqNumber;
		listeners[index].packetType = type;
	#endif

	activeListenersNum++;

	return true;
}

void ResponseHandler::maintainListeners() {
	if(activeListenersNum == 0)
		return;

	uint32_t now = millis();
	if(now > MAINTENANCE_PERIOD_MILLIS && now - lastCheckedTimestampMillis > MAINTENANCE_PERIOD_MILLIS) {
		lastCheckedTimestampMillis = now;


		#ifdef DEBUG_HANDLER_RESPONSE_ENABLE
		Serial.print(millis());
		Serial.println(F(": ResponseHandler::maintainListeners()"));
		#endif


		responseListener_t* currentItem = NULL;
		#ifdef ENABLE_EXTERNAL_RAM
			SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionId);
			while(it.hasNext()) {
				currentItem = (responseListener_t*) it.next();
		#else
			for(uint8_t i = 0; i < LISTENER_NUM; i++) {
				currentItem = &listeners[i];
		#endif
				if(currentItem->timestamp > 0 && currentItem->timestamp < millis()) {
					#ifdef DEBUG_HANDLER_RESPONSE_ENABLE
						Serial.print(millis());
						Serial.print(F(": remove due to timeout: seq="));
						Serial.print(currentItem->seqNumber);
						Serial.print(F(" remote="));
						Serial.println(currentItem->remote);
					#endif
					currentItem->callbackObj->fail(currentItem->seqNumber, currentItem->remote);

					#ifdef ENABLE_EXTERNAL_RAM
						this->removeListener(it.getIteratorIndex()-1);
					#else
						this->removeListener(i);
					#endif
				}
			}
	}
}

uint8_t ResponseHandler::getListenerCallbacks(EventCallbackInterface** listenersList, const packet_type_application_t type, const seq_t seq, const l3_address_t remote) {
	#ifdef DEBUG_HANDLER_RESPONSE_ENABLE
		Serial.print(millis());
		Serial.print(F(": ResponseHandler::getListener() seq="));
		Serial.print(seq);
		Serial.print(F(" type="));
		Serial.print(type);
		Serial.print(F(" remote="));
		Serial.println(remote);
	#endif
	uint8_t found = 0;

	responseListener_t* currentItem = NULL;
	#ifdef ENABLE_EXTERNAL_RAM
		SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionId);
		while(it.hasNext()) {
			currentItem = (responseListener_t*) it.next();
	#else
		for(uint8_t i = 0; i < LISTENER_NUM; i++) {
			currentItem = &listeners[i];
	#endif
			if((currentItem->packetType == type || currentItem->seqNumber == seq) && (currentItem->remote == remote || currentItem->remote == 0)) {
				listenersList[found] = currentItem->callbackObj;
				found++;
			}
		}

	#ifdef DEBUG_HANDLER_RESPONSE_ENABLE
		Serial.print(F("\tfoundNum="));
		Serial.println(found);
	#endif
	return found;
}

void ResponseHandler::loop() {
	maintainListeners();
}

boolean ResponseHandler::removeListener(uint8_t i) {
	if(i >= LISTENER_NUM)
		return false;

	#ifdef ENABLE_EXTERNAL_RAM
		ram.memsetElement(memRegionId, i, 0);
	#else
		memset(&listeners[i], 0, sizeof(responseListener_t));
	#endif

	return true;
}

boolean ResponseHandler::unregisterListener(EventCallbackInterface* callbackObject) {
	boolean result = false;

	responseListener_t* currentItem = NULL;
	#ifdef ENABLE_EXTERNAL_RAM
		SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionId);
		while(it.hasNext()) {
			currentItem = (responseListener_t*) it.next();
			if(currentItem->callbackObj == callbackObject) {
				it.remove();
			}
		}
	#else
		for(uint8_t i = 0; i < LISTENER_NUM; i++) {
			currentItem = &listeners[i];
			if(currentItem->callbackObj == callbackObject) {
				result |= removeListener(i);
			}
		}
	#endif
	return result;
}
