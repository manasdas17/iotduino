/*
 * DebugConfig.h
 *
 * Created: 08.04.2015 20:46:51
 *  Author: helge
 */


#ifndef DEBUGCONFIG_H_
#define DEBUGCONFIG_H_

//#define PRODUCTIVE_MEGA328P

#ifndef PRODUCTIVE_MEGA328P
	#define RTC_ENABLE
	#define SDCARD_ENABLE
	#define WEBSERVER_ENABLE
	#ifdef SDCARD_ENABLE
		#define SDCARD_LOGGER_ENABLE		//logging for sensor data.
	#endif
#endif

#define ENABLE_EVENTS				//program memory heavily depends on used drivers
#define ENABLE_SUBSCRIPTION_SERVICE //~1100 bytes progam memory
#define ENABLE_DISCOVERY_SERVICE	//~500 bytes program memory

//#define TIMER_ENABLE

#ifdef DEBUG
	#define DEBUG_NETWORK_ENABLE
	#define DEBUG_HARDWARE_ENABLE
	#define DEBUG_SD_ENABLE
	#define DEBUG_WEBSERVER_ENABLE
	#define DEBUG_SUBSCRIPTION_MGR_ENABLE
	#define DEBUG_HANDLER_DISPATCHER_ENABLE
	#define DEBUG_HANDLER_COMMAND_ENABLE
	#define DEBUG_HANDLER_RESPONSE_ENABLE
	#define DEBUG_HANDLER_DISCOVERY_ENABLE
	#define DEBUG_HANDLER_SUBSCRIPTION_ENABLE
	#define DEBUG_RAM_ENABLE
#endif

#endif /* DEBUGCONFIG_H_ */