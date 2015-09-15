/*
 * Globals.h
 *
 * Created: 26.05.2015 15:03:41
 *  Author: helge
 */


#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <Configuration.h>
#include <avr/wdt.h>

#ifdef ENABLE_EXTERNAL_RAM
	#include <ramManager.h>
	SPIRamManager ram = SPIRamManager(RAM_MGR_SS_PIN, RAM_MGR_ADDRESS_WIDTH, RAM_MGR_RAM_LEN);
#endif

#include <networking/Layer3.h>
#include <drivers/HardwareDriver.h>
#include <drivers/HardwareID.h>
#include <dispatcher/HardwareInterface.h>
#include <dispatcher/PacketDispatcher.h>
#include <dispatcher/PacketFactory.h>

uint16_t address_local;
#define PIN_CE A0
#define PIN_CSN SS
Layer2rf24 l2;
Layer3 l3;
PacketDispatcher dispatcher;
HardwareInterface hwInterface;
PacketFactory pf;

#ifdef TIMER_ENABLE
	#include <utils/SimpleTimer.h>
	SimpleTimer timer;
#endif

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

	#ifdef SDCARD_LOGGER_ENABLE
		#include <sdcard/SDHardwareResponseListener.h>
		SDHardwareRequestListener sdlistener;
	#endif

	#include <sdcard/DiscoveryManager.h>
	DiscoveryManager discoveryManager;

	#include <utils/NodeInfo.h>
	NodeInfo nodeInfo;
#endif

#ifdef WEBSERVER_ENABLE
	#include <webserver/WebServerWrapper.h>
	WebServerWrapper webServerWrapper;

	#include <webserver/StringConstants.h>

	#include <Ethernet/Ethernet.h>
	//ethernet start on defailt port 80
	EthernetServer server(80);

	byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
	#ifndef USE_DHCP_FOR_IP_ADDRESS
		//ip
		byte ip[] = { 192, 168, 0, 177 };
	#endif
#endif


DHT11 dht11;
RCSwitchTevionFSI07 rcsw;
MotionDetector motion;
Light light;

#endif /* GLOBALS_H_ */