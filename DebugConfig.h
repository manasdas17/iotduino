/*
 * DebugConfig.h
 *
 * Created: 08.04.2015 20:46:51
 *  Author: helge
 */


#ifndef DEBUGCONFIG_H_
#define DEBUGCONFIG_H_

//#define PRODUCTIVE_MEGA328P

#define RTC_ENABLE
#define SDCARD_ENABLE
#define WEBSERVER_ENABLE

#define ENABLE_EVENTS				//program memory heavily depends on used drivers
#define ENABLE_SUBSCRIPTION_SERVICE //~1100 bytes progam memory
#define ENABLE_DISCOVERY_SERVICE	//~500 bytes program memory

//#define DEBUG_NETWORK_ENABLE
//#define DEBUG_HANDLER_ENABLE
//#define DEBUG_HARDWARE_ENABLE
//#define DEBUG_SD_ENABLE

#endif /* DEBUGCONFIG_H_ */