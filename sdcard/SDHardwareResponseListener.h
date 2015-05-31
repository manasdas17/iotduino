/*
* SDHardwareResponseListener.h
*
* Created: 31.05.2015 21:25:20
* Author: helge
*/


#ifndef __SDHARDWARERESPONSELISTENER_H__
#define __SDHARDWARERESPONSELISTENER_H__


#include <Arduino.h>
#include <sdcard/SDcard.h>
#include <interfaces/input/RTC.h>

extern RTC rtc;
extern SDcard sdcard;

/**
 * handler for hardware request responses - writes everything to sdcard.
 */
class SDHardwareRequestListener : public EventCallbackInterface {
	public:
	/**
	 * @param appLayerPacket information
	 * @param address remote
	 * @param seq
	 */
	void doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq);

	/** unused. */
	void fail(seq_t seq, l3_address_t remote) {
	}
};

#endif //__SDHARDWARERESPONSELISTENER_H__
