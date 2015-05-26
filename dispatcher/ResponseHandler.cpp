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
	responseListener_t* listener = getListener(seq, remote);

	//search for type listener if none found
	if(listener == NULL) {
		listener = getListener(type, remote);
	}

	if(listener == NULL)
		return false;

	if(type == NACK) {
		listener->callbackObj->fail(seq, remote);
		return true;
	}

	listener->callbackObj->doCallback(appPacket, remote, seq);

	return true;
}

boolean ResponseHandler::registerListener(const seq_t seqNumber, const l3_address_t remoteAddress, EventCallbackInterface* callbackObject) {
	return registerListener(seqNumber, (packet_type_application_t) 0xff, remoteAddress, callbackObject);
}
boolean ResponseHandler::registerListener(packet_type_application_t type, const l3_address_t remoteAddress, EventCallbackInterface* callbackObject) {
	return registerListener(0, type, remoteAddress, callbackObject);
}

boolean ResponseHandler::registerListener(const seq_t seqNumber, const packet_type_application_t type, const l3_address_t remoteAddress, EventCallbackInterface* callbackObject) {
	#ifdef DEBUG_HANDLER_ENABLE
		Serial.print(millis());
		Serial.print(F(": ResponseHandler::registerListener() remote="));
		Serial.print(remoteAddress);
		Serial.print(F(" seq="));
		Serial.println(seqNumber);
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

	listeners[index].timestamp = millis();
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
		if(listeners[i].timestamp == 0) {
			freeIndex= i;
			break;
		}
	}

	return freeIndex;
}

void ResponseHandler::maintainListeners() {
	if(activeListenersNum == 0)
		return;

	if(millis() - lastCheckedTimestampMillis > MAINTENANCE_PERIOD_MILLIS) {
		lastCheckedTimestampMillis = millis();


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

ResponseHandler::responseListener_t* ResponseHandler::getListener(const seq_t seq, const l3_address_t remote) {
	#ifdef DEBUG_HANDLER_ENABLE
		Serial.print(millis());
		Serial.print(F(": ResponseHandler::getListener() seq="));
		Serial.print(seq);
		Serial.print(F(" remote="));
		Serial.println(remote);
		Serial.flush();
		#endif
		for(uint8_t i = 0; i < LISTENER_NUM; i++) {
			if(listeners[i].timestamp > 0 && listeners[i].seqNumber == seq && listeners[i].remote == remote) {
				#ifdef DEBUG_HANDLER_ENABLE
				Serial.print(millis());
				Serial.println(F(":\tfound"));
				Serial.flush();
				#endif
				return &listeners[i];
			}
		}

		#ifdef DEBUG_HANDLER_ENABLE
		Serial.print(millis());
		Serial.println(F(": \tnone found."));
		Serial.flush();
		#endif
		return NULL;
}

ResponseHandler::responseListener_t* ResponseHandler::getListener(const packet_type_application_t type, const l3_address_t remote) {
	#ifdef DEBUG_HANDLER_ENABLE
	Serial.print(millis());
	Serial.print(F(": ResponseHandler::getListener() type="));
	Serial.print(type);
	Serial.print(F(" remote="));
	Serial.println(remote);
	Serial.flush();
	#endif
	for(uint8_t i = 0; i < LISTENER_NUM; i++) {
		if(listeners[i].timestamp > 0 && listeners[i].packetType == type && listeners[i].remote == remote) {
			#ifdef DEBUG_HANDLER_ENABLE
			Serial.print(millis());
			Serial.println(F(":\tfound"));
			Serial.flush();
			#endif
			return &listeners[i];
		}
	}

	#ifdef DEBUG_HANDLER_ENABLE
	Serial.print(millis());
	Serial.println(F(": \tnone found."));
	Serial.flush();
	#endif
	return NULL;
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
