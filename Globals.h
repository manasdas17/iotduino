/*
 * Globals.h
 *
 * Created: 26.05.2015 15:03:41
 *  Author: helge
 */


#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <DebugConfig.h>

#include <networking/Layer3.h>
#include <drivers/HardwareDriver.h>
#include <drivers/HardwareID.h>
#include <dispatcher/HardwareInterface.h>
#include <dispatcher/PacketDispatcher.h>
#include <dispatcher/PacketFactory.h>
#include <utils/SimpleTimer.h>

uint16_t address_local;
#define PIN_CE A0
#define PIN_CSN SS
Layer2rf24 l2;
Layer3 l3;
PacketDispatcher dispatcher;
HardwareInterface hwInterface;
PacketFactory pf;
SimpleTimer timer;

#include <drivers/digitalio/DHT11.h>
#include <interfaces/output/RCSwitchTevionFSI07.h>
#include <interfaces/input/MotionDetector.h>
#include <interfaces/input/Light.h>

#ifdef RTC_ENABLE
	#include <interfaces/input/RTC.h>
	RTC rtc;
#endif

#ifdef SDCARD_ENABLE
	#include <sdcard/SDcard.h>
	SDcard sdcard;

	#include <sdcard/SDHardwareResponseListener.h>
	SDHardwareRequestListener sdlistener;

	#include <sdcard/SubscriptionManager.h>
	SubscriptionManager subscriptionManager;
#endif

#ifdef WEBSERVER_ENABLE
	#include <webserver/WebServer.h>
	WebServer webServer;
#endif


DHT11 dht11;
RCSwitchTevionFSI07 rcsw;
MotionDetector motion;
Light light;

#endif /* GLOBALS_H_ */