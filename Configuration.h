/*
 * Configuration.h
 *
 * Created: 07.09.2015 20:12:00
 *  Author: helge
 */


#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <DebugConfig.h>

//only for scrappy source parser - may be removed
//#define __AVR_ATmega328p__
//#undef __AVR_ATmega2560__
//*


/** modules */
#ifdef __AVR_ATmega2560__
	#define RTC_ENABLE
	#define WEBSERVER_ENABLE
	#define SDCARD_ENABLE
	#define SDCARD_LOGGER_ENABLE		//logging for sensor data.
#endif

#define ENABLE_EVENTS
#define ENABLE_SUBSCRIPTION_SERVICE
#define ENABLE_DISCOVERY_SERVICE
//#define TIMER_ENABLE				//by now only used by tone driver.


/*****************/
/* External RAM */
/*****************/
#define ENABLE_EXTERNAL_RAM

#define RAM_MGR_MAX_REGIONS 32
#define RAM_MGR_BUF_SIZE 128
#include <SpiRAM.h>
#ifdef __AVR_ATmega2560__
	#define RAM_MGR_SS_PIN 40
	#define RAM_MGR_ADDRESS_WIDTH (SpiRAM::l24bit)
	#define RAM_MGR_RAM_LEN 128000
#else
#define RAM_MGR_SS_PIN A1
#define RAM_MGR_ADDRESS_WIDTH (SpiRAM::l16bit)
#define RAM_MGR_RAM_LEN 32768
#endif
	//#define RAM_MGR_SS_PIN 40
	////#define RAM_MGR_ADDRESS_WIDTH (SpiRAM::l16bit)
	////#define RAM_MGR_RAM_LEN 32768

/************/
/* HARDWARE */
/************/
#define SERIAL_SPEED 115200
#define MAX_INTERFACES_PER_DRIVER 5 //used in discovery service

#define ADDRESS_PIN0 4
#define ADDRESS_PIN1 5
#define ADDRESS_PIN2 6
#define ADDRESS_PIN3 7
#define ADDRESS_PIN4 8
#define ADDRESS_PIN5 9
//#define ADDRESS_PIN6
//#define ADDRESS_PIN7

//nrf24
#define PIN_CE A0

#ifdef __AVR_ATmega2560__
	#define PIN_CSN 53
#else
	#define PIN_CSN 10
#endif

/************/
/* nodeinfo */
/************/
#define NODE_INFO_SIZE 32
#define NODE_INFO_MAX 255
#define NODEINFO_SD_FILENAME "NODEINFO.TXT"

/*********/
/* timer */
/*********/
#define TIMER_MAX_TIMERS 10

/******/
/* SD */
/******/
#define PIN_SD_SS 4


/*********************/
/* discovery manager */
/*********************/
#define DISCOVERY_TIMEOUT 2000
#define DISCOVERY_REQUEST_PERIOD_MILLIS (1*20*1000UL)
#define DISCOVERY_REQUEST_DELAY_MILLIS 2000
#define NUM_KNOWN_NODES 256
#define NUM_INFOS_PER_NODE 15

#define DISCOVERY_SD_WRITE_PERIOD_MILLIS (60*1000UL)

#define DISCOVERY_SD_FILENAME "DICOVERY.BIN"

/**************/
/* networking */
/**************/

/******/
/* L2 */
/******/
#include <networking/rf24/RF24.h>
#define CONFIG_RF_PA_LEVEL RF24_PA_LOW
#define CONFIG_RF_CHANNEL 70
#define CONFIG_CRC RF24_CRC_16 //RF24_CRC_DISABLED //RF24_CRC_8
#define CONFIG_RF_DATARATE RF24_250KBPS //RF24_1MBPS //RF24_250KBPS //RF24_2MBPS
#define CONFIG_L2_PAYLOAD_SIZE 32
#define CONFIG_L2_RETRY_MAX 15
#define CONFIG_L2_RETRY_DELAY_250US 15

#define CONFIG_L2_ADDR_BROADCAST	0x000000FFFFLL
#define CONFIG_RF_PIPE_BROADCAST	0 //pipe 0 and 1..5 must be used
#define CONFIG_RF_PIPE_DEVICE		1

/******/
/* L3 */
/******/
#define CONFIG_L3_ADDRESS_BROADCAST CONFIG_L2_ADDR_BROADCAST

#define CONFIG_L3_BEACON_PERIOD_MS (30*1000L)
#define CONFIG_L3_NEIGHBOUR_MAX_AGE_MS (300*1000L)

#define CONFIG_L3_MAX_HOPCOUNT 10

#ifdef ENABLE_EXTERNAL_RAM
	#define CONFIG_L2_RECEIVE_BUFFER_LEN 10
	#define CONFIG_L3_NUM_NEIGHBOURS 32
	#define CONFIG_L3_RECEIVE_BUFFER_LEN 10
	#define CONFIG_L3_SEND_BUFFER_LEN 10
#else
	#define CONFIG_L2_RECEIVE_BUFFER_LEN 3
	#define CONFIG_L3_NUM_NEIGHBOURS 15
	#define CONFIG_L3_RECEIVE_BUFFER_LEN 5
	#define CONFIG_L3_SEND_BUFFER_LEN 5
#endif

#define CONFIG_L3_NUMBERED_RETRANSMISSIONS 1
#define CONFIG_L3_NUMBERED_TIMEOUT_MS (1000)

typedef uint16_t l3_seq_t;
typedef uint32_t l3_timestamp;
typedef uint16_t l3_address_t;

/****************************************/
/* subscription service (detects events */
/****************************************/
#ifdef ENABLE_EXTERNAL_RAM
	#define numSubscriptionList 10
#else
	#define numSubscriptionList 5
#endif
/** period for subscription execution */
#define SUBSCRIPTION_CHECK_PERIOD_MILLIS (1*1000UL)
/** period for subscription polling check */
#define SUBSCRIPTION_POLLING_CHECK_PERIOD_MILLIS (200) //250

/********************/
/* Response handler */
/********************/
#define LISTENER_NUM 5
#define MAINTENANCE_PERIOD_MILLIS (15*1000)

/****************/
/* hw interface */
/****************/
#define driverPointerListSize 6

/*********************/
/* discovery service */
/*********************/
#define INTERFACES_BUF_SIZE 16

/************/
/* commands */
/************/
#define sizeUInt8List 8 //enough size for 4 16bit vars.


/*************/
/* webserver */
/*************/
#define WEBSERVER_PORT 80
#define WEBSERVER_IP0 192
#define WEBSERVER_IP1 168
#define WEBSERVER_IP2 0
#define WEBSERVER_IP3 177
//#define USE_DHCP_FOR_IP_ADDRESS

#define STRING_BUFFER_SIZE 128



/** do not edit below. */
//sanity check.
//#if (CONFIG_L3_PACKET_NUMBERED_MAX_LEN < (7 + sizeUInt8List*1))
	//#error maximum payload len exceeded.
//#endif

#endif /* CONFIGURATION_H_ */