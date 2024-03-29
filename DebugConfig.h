/*
 * DebugConfig.h
 *
 * Created: 08.04.2015 20:46:51
 *  Author: helge
 */


#ifndef DEBUGCONFIG_H_
#define DEBUGCONFIG_H_

#ifdef DEBUG
	#ifdef __AVR_ATmega2560__
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
		#define DEBUG_NEIGHBOUR_ENABLE
	#else
		#define DEBUG_NETWORK_ENABLE
		//#define DEBUG_HARDWARE_ENABLE
		//#define DEBUG_SD_ENABLE
		//#define DEBUG_WEBSERVER_ENABLE
		//#define DEBUG_SUBSCRIPTION_MGR_ENABLE
		//#define DEBUG_HANDLER_DISPATCHER_ENABLE
		//#define DEBUG_HANDLER_COMMAND_ENABLE
		//#define DEBUG_HANDLER_RESPONSE_ENABLE
		//#define DEBUG_HANDLER_DISCOVERY_ENABLE
		//#define DEBUG_HANDLER_SUBSCRIPTION_ENABLE
		//#define DEBUG_RAM_ENABLE
		//#define DEBUG_NEIGHBOUR_ENABLE
	#endif 
#endif


#endif /* DEBUGCONFIG_H_ */