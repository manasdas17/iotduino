/*
* ResponseHandler.h
*
* Created: 14.04.2015 22:53:26
* Author: helge
*/


#ifndef __RESPONSEHANDLER_H__
#define __RESPONSEHANDLER_H__


#include <networking/Packets.h>
#include <dispatcher/EventCallbackInterface.h>
#include <ramManager.h>

extern SPIRamManager ram;

class ResponseHandler {
	//variables
	public:
	protected:
	private:
		typedef struct responseListener_struct {
			uint16_t timestamp;
			packet_type_application_t packetType;
			seq_t seqNumber;
			l3_address_t remote;
			EventCallbackInterface* callbackObj;
		} responseListener_t;

		/** listener storage */
		#ifdef ENABLE_EXTERNAL_RAM
			uint8_t memRegionId;
		#else
			responseListener_t listeners[LISTENER_NUM];
		#endif

		uint8_t activeListenersNum;

		/** timestamp for maintenance loop */
		uint32_t lastCheckedTimestampMillis;
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
		boolean handleReponseNumbered(const seq_t seq, const packet_type_application_t type, const l3_address_t remote, packet_application_numbered_cmd_t* appPacket);

		/**
		 * register a new listener for a specific remote address and sequence number with callback
		 * @param timeout 0=no timeout; absolute(!) internal clock timestamp for timeout
		 * @param sequence
		 * @param l3 remote address
		 * @param callback object
		 * @return true on success, false otherwise
		 */
		boolean registerListenerBySeq(uint32_t timeout, const seq_t seqNumber, const l3_address_t remoteAddress, EventCallbackInterface* callbackObject);

		/**
		 * register a new listener for a specific remote address and sequence number with callback
		 * @param timeout 0=no timeout; absolute(!) internal clock timestamp for timeout
		 * @param sequence
		 * @param type of packet
		 * @param callback object
		 * @return true on success, false otherwise
		 */
		boolean registerListenerByPacketType(uint32_t timeout, packet_type_application_t type, const l3_address_t remoteAddress, EventCallbackInterface* callbackObject);

		/**
		 * register a new listener for a specific remote address and sequence number with callback
		 * @param callback object
		 * @return true on success, false otherwise
		 */
		boolean unregisterListener(EventCallbackInterface* callbackObject);

		/**
		 * maintenance.
		 */
		void loop();

		void init() {
			activeListenersNum = 0;

			#ifdef ENABLE_EXTERNAL_RAM
			#ifdef DEBUG_HANDLER_RESPONSE_ENABLE
			Serial.print(millis());
			Serial.println(F(": creating region for responsehandler"));
			#endif
			memRegionId = ram.createRegion(sizeof(responseListener_t), LISTENER_NUM);
			#endif
		}


	protected:
		/**
		 * register a new listener for a specific remote address and sequence number with callback
		 * @param timeout absolute(!) internal clock timestamp for timeout
		 * @param sequence
		 * @param type
		 * @param l3 remote address
		 * @param callback object
		 * @return true on success, false otherwise
		 */
		boolean registerListener(uint32_t timeout, const seq_t seqNumber, const packet_type_application_t type, const l3_address_t remoteAddress, EventCallbackInterface* callbackObject);

		/**
		 * return a listener if available
		 * @param listeners array for listener pointers
		 * @param type packet type
		 * @param seq number
		 * @param remote address
		 */
		uint8_t getListenerCallbacks(EventCallbackInterface** listeners, const packet_type_application_t type, const seq_t seq, const l3_address_t remote);

		/**
		 * check callbacks for timeouts; in case of timeout, triggers FAIL() on callback
		 */
		void maintainListeners();

		/**
		 * remove a listener
		 * @param index
		 * @param success
		 */
		boolean removeListener(uint8_t index);

	private:
}; //ResponseHandler

#endif //__RESPONSEHANDLER_H__
