/*
* Routing.h
*
* Created: 04.01.2015 02:37:58
* Author: helge
*/


#ifndef __LAYER3_H__
#define __LAYER3_H__

#include <Arduino.h>
#include <dispatcher/EventCallbackInterface.h>
#include <dispatcher/Commands.h>
#include "LayerConfig.h"
#include "neighbourData.h"
#include "Layer2rf24.h"
#include "Packets.h"

/** 3-Way Handshake. TBA!
*          DEVICE A              DEVICEB2
*          ========              ========
*           CLOSED                CLOSED
*              |-----SYN(seqA)----->|
*              V                    |
*             SYN                   |
*              |<-ACK(seqA+1,seqB)--|
*              |                    V
*              |                   ACK
*              |--SYNACK(seqB+1)--->|
*              V                    V
*           ACTIVE               ACTIVE
*/
class Layer3 {

	class callbackClass : public EventCallbackInterface {
		Layer3* parent;

		public:
			callbackClass(Layer3* l3) {
				parent = l3;
			}

			void doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq);
			virtual void fail(seq_t seq, l3_address_t remote);
	};

	//variables
	public:
		typedef union packet_t_union {
			struct {
				uint8_t type; //1b
				l3_address_t destination; //2b
				l3_address_t source; //2b
				uint8_t hopcount; //1b
				uint8_t payloadLen; //1b
				byte payload[CONFIG_APP_PAYLOAD_SIZE];
			} data;
			uint8_t bytes[CONFIG_L3_PAYLOAD_SIZE];
		} packet_t;

		typedef struct packet_sending_queue_item_struct {
			packet_t packet;
			uint8_t retransmissions;
			uint32_t lasttimestamp;
		} packet_sending_queue_item_t;

		l3_address_t localAddress;

		Layer2rf24* l2;

		callbackClass* eventCallbackClass;

		neighbourData neighbours[CONFIG_L3_NUM_NEIGHBOURS];
	protected:
	private:

		packet_t receiveQueue[CONFIG_L3_RECEIVE_BUFFER_LEN];
		uint8_t receiveQueueFirst;
		uint8_t receiveQueueNum;

		packet_sending_queue_item_t sendingNumberedBuffer[CONFIG_L3_SEND_BUFFER_LEN];

		uint32_t beaconLastTimestamp;

	//functions
	public:
		EventCallbackInterface* getCallbackInterface();

		void Loop();

		~Layer3() {
			delete eventCallbackClass;
		}

		Layer3();

		/**
		 * init this class
		 * @param localAddress
		 */
		void init(l3_address_t localAddress);

		void setLayer2(Layer2rf24* l2) {
			this->l2 = l2;
			l2->setLayer3(this);
		}

		/**
		* receive a new packet.
		* @param payload from l2
		* @return success
		*/
		boolean receive(void* payload);

		/**
		* queue size.
		*/
		uint8_t receiveQueueSize();

		/**
		* get next element from queue without deleting it
		* @return packet
		*/
		packet_t* receiveQueuePeek();

		/**
		* @param packet
		* @return success
		*/
		boolean receiveQueuePop( packet_t* f );

		/**
		* remove next item.
		* @return success
		*/
		boolean receiveQueuePop();

		/**
		* get the number of neighbours
		* @return num of neighbours
		*/
		uint8_t neighboursSize();

		/**
		* get the complete neighbourtable
		* @return neighbourData
		*/
		neighbourData* getNeighbours();

		/**
		* send numbered packet
		* @param destination
		* @param seq
		* @param payload
		* @param payload length
		* @return sequence number
		*/
		seq_t sendNumbered(l3_address_t destination, seq_t seq, void* payload, uint8_t payloadLen);

		/**
		* send numbered packet (random seq)
		* @param destination
		* @param payload
		* @param payload length
		* @return sequence number
		*/
		seq_t sendNumbered(l3_address_t destination, void* payload, uint8_t payloadLen);

		/**
		* send unnumbered packet
		* @param destination
		* @param payload
		* @param payload length
		* @return success
		*/
		boolean sendUnnumbered(l3_address_t destination, void* payload, uint8_t payloadLen);

		/**
		* send unnumbered packet
		* @param destination
		* @param payload
		* @param payload length
		* @return succes
		*/
		boolean sendBroadcast(void* payload, uint8_t payloadLen);

		/**
		* send a packet to destination
		* @param packet
		*/
		boolean sendPacket(packet_t &packet);

		/**
		* @param packet
		* @param l3 destination
		* @param l3 frame type
		* @param payload
		* @param payload length
		* @return success
		*/
		boolean createPacketGeneric(packet_t* packet, l3_address_t destination, l3_packetType type, void* payload, uint8_t payloadLen);


		/**
		* cleans the neighbourlist and sends beacon
		*/
		boolean sendBeacon();

		protected:
		private:

		/**
		* print debug information.
		* @param packet
		*/
		void printPacketInformation(packet_t* packet);

		/**
		* add numbered packet to sending queue - retries on layer3
		*/
		boolean addToSendingQueue(packet_t* packet);

		/**
		* check sending queue and resend
		*/
		void updateSendingBuffer();

		/**
		* handle an acked packet
		*/
		boolean handleAck(packet_t* packet);

		/**
		* send ACK for this packet.
		*/
		boolean sendAck(packet_t* packet);

		/**
		* get neighbour row.
		* @param neighbour
		*/
		neighbourData* getNeighbour(l3_address_t detination);

		/**
		* push into receive queue
		* @param packet
		* @return success
		*/
		boolean receiveQueuePush( packet_t* f );

		/**
		* route packet in case we have a suitable neighbour-entry.
		* @param packet
		* @return success
		*/
		boolean routePacket( packet_t* packet );

		/**
		* handle beacon packet.
		* @param packet
		* @return success
		*/
		boolean handleBeacon(packet_t* packet);

		/**
		* update/add new neighbour
		* @param packet
		* @return success
		*/
		boolean updateNeighbour(packet_t* packet);

		/**
		* @param neighbour addrtess
		* @param next hop
		* @param hops
		* @return success
		*/
		boolean updateNeighbour( l3_address_t destination, l3_address_t nextHop, uint8_t hopCount);

		/**
		* update neighbours
		* @param beacon
		* @return success
		*/
		boolean updateNeighbours(packet_beacon_t* beacon);

		/**
		* neighbourtable maintenance, to be called periodically
		*/
		void cleanNeighbours();


}; //Routing

#endif

