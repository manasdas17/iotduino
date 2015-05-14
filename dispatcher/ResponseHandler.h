/*
* ResponseHandler.h
*
* Created: 14.04.2015 22:53:26
* Author: helge
*/


#ifndef __RESPONSEHANDLER_H__
#define __RESPONSEHANDLER_H__


#include "../networking/Packets.h"
#include "EventCallbackInterface.h"

#define responseTimeoutMillis (1000*10) //10s
#define LISTENER_NUM 10
#define MAINTENANCE_PERIOD_MILLIS (1000*1) //1s

class ResponseHandler {
	//variables
	public:
	protected:
	private:
		typedef struct responseListener_struct {
			uint16_t timestamp;
			seq_t seqNumber;
			l3_address_t remote;
			EventCallbackInterface* callbackObj;
		} responseListener_t;

		/** listener storage */
		responseListener_t listeners[LISTENER_NUM];

		/** timestamp for maintenance loop */
		uint16_t lastCheckedTimestampMillis;
	//functions
	public:
		/**
		 * handles a numbered response packet
		 * @param sequence
		 * @param application type (redundant for now - maybe I will move this field into actual numberead packet.)
		 * @param l3 remote address
		 * @param applayer packet
		 * @return success if listener is found.
		 */
		boolean handleReponseNumbered(const seq_t seq, const packet_type_application_t type, const l3_address_t remote, packet_application_numbered_cmd_t* appPacket) {
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

			responseListener_t* listener = getListener(seq, remote);

			if(listener == NULL)
				return false;

			if(type == NACK) {
				listener->callbackObj->fail(seq, remote);
				return true;
			}

			listener->callbackObj->doCallback(appPacket, remote, seq);

			return true;
		}

		/**
		 * register a new listener for a specific remote address and sequence number with callback
		 * @param sequence
		 * @param l3 remote address
		 * @param callback object
		 * @return true on success, false otherwise
		 */
		boolean registerListener(const seq_t seqNumber, const l3_address_t remoteAddress, EventCallbackInterface* callbackObject) {
			#ifdef DEBUG_HANDLER_ENABLE
				Serial.print(millis());
				Serial.print(F(": ResponseHandler::registerListener() remote="));
				Serial.print(remoteAddress);
				Serial.print(F(" seq="));
				Serial.println(seqNumber);
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

			return true;
		}

		/**
		 * maintenance.
		 */
		void loop() {
			maintainListeners();
		}

		ResponseHandler() {

		}

		~ResponseHandler() {

		}

	protected:
		/**
		 * return a listener if available
		 * @param desired sequence number
		 * @param desired l3 remote address
		 */
		 responseListener_t* getListener(const seq_t seq, const l3_address_t remote) {
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

		/**
		 * check callbacks for timeouts; in case of timeout, triggers FAIL() on callback
		 */
		void maintainListeners() {
			#ifdef DEBUG_HANDLER_ENABLE
				Serial.print(millis());
				Serial.println(F("ResponseHandler::maintainListeners()"));
				Serial.flush();
			#endif
			if(millis() - lastCheckedTimestampMillis > MAINTENANCE_PERIOD_MILLIS) {
				lastCheckedTimestampMillis = millis();

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
						memset(&listeners[i], 0, sizeof(responseListener_t));
					}
				}
			}
		}

		/**
		 * @return free slot index, 255 otherwise
		 */
		uint8_t getListenerSlot() const {
			uint8_t freeIndex = 0xff;
			for(uint8_t i = 0; i < LISTENER_NUM; i++) {
				if(listeners[i].timestamp == 0) {
					freeIndex= i;
					break;
				}
			}

			return freeIndex;
		}

	private:
}; //ResponseHandler

#endif //__RESPONSEHANDLER_H__
