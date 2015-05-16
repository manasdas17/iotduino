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
		boolean handleReponseNumbered(const seq_t seq, const packet_type_application_t type, const l3_address_t remote, packet_application_numbered_cmd_t* appPacket);

		/**
		 * register a new listener for a specific remote address and sequence number with callback
		 * @param sequence
		 * @param l3 remote address
		 * @param callback object
		 * @return true on success, false otherwise
		 */
		boolean registerListener(const seq_t seqNumber, const l3_address_t remoteAddress, EventCallbackInterface* callbackObject);

		/**
		 * maintenance.
		 */
		void loop();

		ResponseHandler() {}

		~ResponseHandler() {}

	protected:
		/**
		 * return a listener if available
		 * @param desired sequence number
		 * @param desired l3 remote address
		 */
		 responseListener_t* getListener(const seq_t seq, const l3_address_t remote);

		/**
		 * check callbacks for timeouts; in case of timeout, triggers FAIL() on callback
		 */
		void maintainListeners();

		/**
		 * @return free slot index, 255 otherwise
		 */
		uint8_t getListenerSlot() const;

	private:
}; //ResponseHandler

#endif //__RESPONSEHANDLER_H__
