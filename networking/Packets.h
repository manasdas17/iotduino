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
*
* Hardware Result:
*       +--------------------------------------------------------------------------------------------+
*       | packet_numbered_t                                                                          |
*       +-----+-----+--------------------------------------------------------------------------------+
*       |     |     | packet_application_t                                                           |
*       |     |     +-----------------+--------------------------------------------------------------+
*       |     |     |                 | hwresult_t                                                   |
*       |     |     |                 +------+-----------+---------+----------+----------+-----------+
*       | seq | len | type in { ACK } | hwId | hwAddress | uintNum | floatNum | uintList | floatList |
*       +-----+-----+-----------------+------+-----------+---------+----------+----------+-----------+
* bytes: 2     1     1                 1       1          1         1          0..4       0..1
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

	HARDWARE_DISCOVERY_REQ,
	HARDWARE_DISCOVERY_RES,

	HARDWARE_SUBSCRIPTION_SET,
	HARDWARE_SUBSCRIPTION_INFO,

	ACK,
	NACK
} packet_type_application_t;

#define CONFIG_APP_LAYER_PAYLOAD_SIZE (CONFIG_L3_PACKET_NUMBERED_MAX_LEN - 1)

typedef struct packet_application_numbered_cmd_struct {
	int8_t packetType; //1b
	uint8_t payload[CONFIG_L3_PACKET_NUMBERED_MAX_LEN - 1];
} packet_application_numbered_cmd_t;

typedef struct packet_application_unnumbered_cmd_struct {
	int8_t packetType; //1b
	uint8_t payload[CONFIG_L3_PACKET_UNNUMBERED_MAX_LEN - 1];
} packet_application_unnumbered_cmd_t;

//discovery
typedef struct discoveryInfo_helper_struct {
	uint8_t hardwareAddress;
	int8_t hardwareType;
} packet_application_numbered_discovery_info_helper_t;

typedef struct discoveryInfo_struct {
	uint8_t numSensors;
	packet_application_numbered_discovery_info_helper_t infos[(CONFIG_L3_PACKET_UNNUMBERED_MAX_LEN - 1) / sizeof(packet_application_numbered_discovery_info_helper_t)];
} packet_application_numbered_discovery_info_t;

//subscriptions
typedef struct subscription_helper_struct {
	l3_address_t address;
	uint8_t hardwareAddress;
	uint8_t hardwareType;
	uint32_t millisecondsDelay;
	uint8_t onEvent;
	uint16_t onEventBlackout;
	seq_t sequence;
} subscription_helper_t;

typedef struct substcripton_info_struct {
	l3_address_t forAddress;
	uint8_t numInfosFollowing;
	subscription_helper_t info;
} subscription_info_t;

typedef struct subscription_set_struct {
	subscription_helper_t info;
} subscription_set_t;

#endif //__PACKETS_H__
