/*
* nrf24Mesh.h
*
* Created: 04.01.2015 01:44:26
* Author: helge
*/


#ifndef __NRF24MESH_H__
#define __NRF24MESH_H__

#include <Arduino.h>
#include <SPI.h>
#include <networking/LayerConfig.h>
#include <networking/rf24/nRF24L01.h>
#include <networking/rf24/RF24.h>

class Layer3;

/**
 * basic layer 2 implementation - blocking writes, single receive queue
 */
class Layer2rf24 {
	//variables
	public:
		const static uint8_t l2PayloadMaxLen = CONFIG_L3_PAYLOAD_SIZE;

		typedef uint16_t address_t;

		typedef union{
			struct {
				address_t destination;	//2b
				address_t source;		//2b
				uint8_t payloadLen;		//1b
				byte payload[l2PayloadMaxLen];
			} data;
			uint8_t bytes[CONFIG_L2_PAYLOAD_SIZE];
		} frame_t;

	private:
		uint8_t pin_ce;
		uint8_t pin_csn;
		RF24 radio;
		uint64_t deviceAddress;

		frame_t receiveQueue[CONFIG_L2_RECEIVE_BUFFER_LEN];
		uint8_t receiveQueueFirst;
		uint8_t receiveQueueNum;

		Layer3* l3;

	//functions
	public:
		void setLayer3(Layer3* l3) {
			this->l3 = l3;
		}
		/**
		 * create a new frame
		 * @param frame
		 * @param destination adddress
		 * @param payload lendgth in bytes
		 * @param actual payload
		 * @return success
		 */
		boolean createFrame(frame_t* f, address_t destination, uint8_t payloadLen, uint8_t* payload );

		/**
		 * get receive queue size
		 * @return size
		 */
		uint8_t receiveQueueSize();

		/**
		 * push item to receive queu
		 * @param frame
		 * @return success
		 */
		boolean receiveQueuePush(frame_t* f);

		/**
		 * pop an item from the receive queue
		 * @param frame to fill
		 */
		boolean receiveQueuePop(frame_t* f);


		Layer2rf24();

		/**
		 * init.
		 * @param ce pin
		 * @param csn pin
		 * @param local device address
		 */
		void init(Layer3* l3, uint8_t pin_ce, uint8_t pin_csn, uint16_t deviceAddress);

		/**
		 * desctructor.
		 */
		~Layer2rf24() {
		}

		/**
		 * receive all available data and push it to the queue.
		 * @return num frames received (max will typically be 3 due to buffer size.)
		 */
		uint8_t receive();

		/**
		 * blocking write call
		 * automatically distinguishes between broadcast (non-acked, multicast) and device addresses (auto acked)
		 * @return success
		 */
		boolean sendFrame(frame_t* frame);

	private:
		/**
		 * init the radio with all settings
		 */
		void setupRadio();

}; //nrf24Mesh

#endif //__NRF24MESH_H__
