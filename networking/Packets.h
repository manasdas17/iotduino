/*
* Packets.h
*
* Created: 07.01.2015 22:36:02
* Author: helge
*
* Hardware Command:
*       +-------------------------------------------------------------------------------+
*       | packet_numbered_t                                                             |
*       +-----+-----+-------------------------------------------------------------------+
*       |     |     | packet_application_t                                              |
*       |     |     +-----------------------------+-------------------------------------+
*       |     |     |                             | command_t                           |
*       |     |     |                             +--------+------+-----------+---------+
*       | seq | len | type in {HW_READ, HW_WRITE} | isRead | hwId | hwAddress | payload |
*       +-----+-----+-----------------------------+--------+------+-----------+---------+
* bytes: 2     1     1                             1        1      1           0
*/


#ifndef __PACKETS_H__
#define __PACKETS_H__

#include <Arduino.h>
#include "LayerConfig.h"

/** type for packet identifiers numbers*/
typedef uint16_t seq_t;

/** struct for ack */
typedef struct packet_ack_t_struct {
	seq_t ack;
} packet_ack_t;


/** struct for numbered packets */
#define CONFIG_L3_PACKET_NUMBERED_MAX_LEN (CONFIG_APP_PAYLOAD_SIZE - 3)
typedef struct packet_numbered_t_struct{
	seq_t seqNumber; //2b
	uint8_t payloadLen; //1b
	uint8_t payload[CONFIG_L3_PACKET_NUMBERED_MAX_LEN];
} packet_numbered_t;

/* struct for unnumbered packets */
#define CONFIG_L3_PACKET_UNNUMBERED_MAX_LEN (CONFIG_APP_PAYLOAD_SIZE)
typedef struct packet_unnumbered_t_struct{
	uint8_t payload[CONFIG_L3_PACKET_UNNUMBERED_MAX_LEN];
} packet_unnumbered_t;

/* struct for holding node and hopcount information */
typedef struct routeInfo_t_struct {
	l3_address_t nodeId; //2b
	uint8_t hopcount; //1b
} routeInfo_t;

/* struct for holding beacon information */
#define CONFIG_L3_BEACON_NUM_INFOS (CONFIG_APP_PAYLOAD_SIZE / sizeof(routeInfo_t) - 1)
typedef struct packet_beacon_t_struct{
	l3_address_t nodeId; //2b
	uint8_t numNeighbourInfo; //1b
	routeInfo_t neighbours[CONFIG_L3_BEACON_NUM_INFOS];
} packet_beacon_t;


/**
 * Application packet
 *
 */
typedef enum packet_type_application_enum {
	HARDWARE_COMMAND_WRITE,
	HARDWARE_COMMAND_READ,
	HARDWARE_COMMAND_RES,

	HARDWARE_DISCOVERY_REQ,
	HARDWARE_DISCOVERY_RES,

	HARDWARE_SUBSCRIPTION_SET,
	HARDWARE_SUBSCRIPTION_INFO,
	HARDWARE_SUBSCRIPTION_INFO_RES,

	ACK,
	NACK
} packet_type_application_t;

#define CONFIG_APP_LAYER_PAYLOAD_SIZE (CONFIG_L3_PACKET_NUMBERED_MAX_LEN - 1)

/** generic struct for numbered application packet */
typedef struct packet_application_numbered_cmd_struct {
	int8_t packetType; //1b
	uint8_t payload[CONFIG_L3_PACKET_NUMBERED_MAX_LEN - 1];
} packet_application_numbered_cmd_t;

/** generic struct for unnumbered application packet, unused by now. */
typedef struct packet_application_unnumbered_cmd_struct {
	int8_t packetType; //1b
	uint8_t payload[CONFIG_L3_PACKET_UNNUMBERED_MAX_LEN - 1];
} packet_application_unnumbered_cmd_t;

/** strcut for hardware discovery - keeps information of an interface */
typedef struct discoveryInfo_helper_struct {
	uint8_t hardwareAddress;
	int8_t hardwareType;
	#ifdef ENABLE_EVENTS
		uint8_t canDetectEvents;
	#endif
} packet_application_numbered_discovery_info_helper_t;

/** struct for hardware discovery information */
#define PACKET_APP_NUMBERED_DISCOVERY_DRIVERS_NUM (CONFIG_L3_PACKET_NUMBERED_MAX_LEN - 1) / sizeof(packet_application_numbered_discovery_info_helper_t)
typedef struct discoveryInfo_struct {
	uint8_t numTotalSensors;
	uint8_t numSensors;
	packet_application_numbered_discovery_info_helper_t infos[PACKET_APP_NUMBERED_DISCOVERY_DRIVERS_NUM];
} packet_application_numbered_discovery_info_t;

//subscriptions
enum subscription_event_type_t {EVENT_TYPE_DISABLED = 0, EVENT_TYPE_CHANGE, EVENT_TYPE_EDGE_RISING, EVENT_TYPE_EDGE_FALLING};

//max 19b
typedef struct subscription_helper_struct {
	l3_address_t address;					//2b
	uint8_t hardwareAddress;				//1b
	uint8_t hardwareType;					//1b
	uint32_t millisecondsDelay;				//4b
	subscription_event_type_t onEventType;	//1b
	seq_t sequence;							//2b
} subscription_helper_t;					//sum=13

/** subscription information */
typedef struct substcripton_info_struct {
	l3_address_t forAddress;
	uint8_t numInfosFollowing;
	subscription_helper_t info;
} subscription_info_t;

/** set subscription - just keeps subscription information */
typedef struct subscription_set_struct {
	subscription_helper_t info;
} subscription_set_t;

#endif //__PACKETS_H__
