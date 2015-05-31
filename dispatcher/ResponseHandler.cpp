#include "ResponseHandler.h"

boolean ResponseHandler::handleReponseNumbered(const seq_t seq, const packet_type_application_t type, const l3_address_t remote, packet_application_numbered_cmd_t* appPacket) {
	#ifdef DEBUG_HANDLER_ENABLE
		Serial.print(millis());
		Serial.println(F(": ResponseHandler::handleResponseNumbered()"));
		Serial.print(F("\tseq="));
		Serial.print(seq);
		Serial.print(F(" type="));
		Serial.print(type);
		Serial.print(F(" remote="));
		Serial.println(remote);
		Serial.flush();
	#endif

	//search for seq listener
	responseListener_t* listenersList[LISTENER_NUM];
	uint8_t listenersFound = getListener(listenersList, type, seq, remote);

	if(listenersFound == 0)
		return false;

	//iterate
	for(uint8_t i = 0; i < listenersFound; i++) {
		if(type == NACK) {
			listenersList[i]->callbackObj->fail(seq, remote);
		} else {
			listenersList[i]->callbackObj->doCallback(appPacket, remote, seq);
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
	#ifdef DEBUG_HANDLER_ENABLE
		Serial.print(millis());
		Serial.print(F(": ResponseHandler::registerListener() remote="));
		Serial.print(remoteAddress);
		Serial.print(F(" seq="));
		Serial.print(seqNumber);
		Serial.print(F(" type="));
		Serial.println(type);
		Serial.flush();
	#endif

	if(callbackObject == NULL)
		return false;

	uint8_t index = getListenerSlot();

	if(index == 0xff) {
		return false;
	}

	listeners[index].timestamp = timeout;
	listeners[index].callbackObj = callbackObject;
	listeners[index].remote = remoteAddress;
	listeners[index].seqNumber = seqNumber;
	listeners[index].packetType = type;

	activeListenersNum++;

	return true;
}

uint8_t ResponseHandler::getListenerSlot() const {
	uint8_t freeIndex = 0xff;
	for(uint8_t i = 0; i < LISTENER_NUM; i++) {
		if(listeners[i].callbackObj == NULL) {
			freeIndex= i;
			break;
		}
	}

	return freeIndex;
}

void ResponseHandler::maintainListeners() {
	if(activeListenersNum == 0)
		return;

	uint32_t now = millis();
	if(now > MAINTENANCE_PERIOD_MILLIS && now - lastCheckedTimestampMillis > MAINTENANCE_PERIOD_MILLIS) {
		lastCheckedTimestampMillis = now;


		#ifdef DEBUG_HANDLER_ENABLE
		Serial.print(millis());
		Serial.println(F(": ResponseHandler::maintainListeners()"));
		Serial.flush();
		#endif

		for(uint8_t i = 0; i < LISTENER_NUM; i++) {
			if(listeners[i].timestamp > 0 && listeners[i].timestamp < millis()) {
				#ifdef DEBUG_HANDLER_ENABLE
					Serial.print(millis());
					Serial.print(F(": remove due to timeout: seq="));
					Serial.print(listeners[i].seqNumber);
					Serial.print(F(" remote="));
					Serial.println(listeners[i].remote);
					Serial.flush();
				#endif
				listeners[i].callbackObj->fail(listeners[i].seqNumber, listeners[i].remote);
				this->removeListener(i);
			}
		}
	}
}

uint8_t ResponseHandler::getListener(responseListener_t** listenersList, const packet_type_application_t type, const seq_t seq, const l3_address_t remote) {
	#ifdef DEBUG_HANDLER_ENABLE
		Serial.print(millis());
		Serial.print(F(": ResponseHandler::getListener() seq="));
		Serial.print(seq);
		Serial.print(F(" type="));
		Serial.println(type);
		Serial.print(F(" remote="));
		Serial.println(remote);
		Serial.flush();
	#endif
	uint8_t found = 0;
	for(uint8_t i = 0; i < LISTENER_NUM; i++) {
		if((listeners[i].packetType == type || listeners[i].seqNumber == seq) && (listeners[i].remote == remote || listeners[i].remote == 0)) {
			listenersList[found] = &listeners[i];
			found++;
		}
	}

	#ifdef DEBUG_HANDLER_ENABLE
		Serial.print(millis());
		Serial.println(F(": \tfoundNum="));
		Serial.print(found);
		Serial.flush();
	#endif
	return found;
}

void ResponseHandler::loop() {
	maintainListeners();
}

boolean ResponseHandler::removeListener(uint8_t i) {
	if(i >= LISTENER_NUM)
		return false;

	memset(&listeners[i], 0, sizeof(responseListener_t));
	activeListenersNum--;
	return true;
}

boolean ResponseHandler::unregisterListener(EventCallbackInterface* callbackObject) {
	boolean result = false;
	for(uint8_t i = 0; i < LISTENER_NUM; i++) {
		if(listeners->callbackObj == callbackObject) {
			result |= removeListener(i);
		}
	}
	return result;
}
