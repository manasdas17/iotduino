#include "Layer2rf24.h"

boolean Layer2rf24::receiveQueuePush( frame_t* f )
{
	if(receiveQueueNum >= CONFIG_L2_RECEIVE_BUFFER_LEN)
		return false;

	uint8_t index = (receiveQueueFirst + receiveQueueNum) % CONFIG_L2_RECEIVE_BUFFER_LEN;

	#ifdef ENABLE_EXTERNAL_RAM
		if(!ram.writeElementToRam(memRegionId, index, f))
			return false;
	#else
		memcpy(&receiveQueue[index], f, sizeof(frame_t));
	#endif

	receiveQueueNum++;

	return true;
}

uint8_t Layer2rf24::receiveQueueSize()
{
	return receiveQueueNum;
}

boolean Layer2rf24::receiveQueuePop( frame_t* f )
{
	if(receiveQueueNum <= 0)
		return false;

	#ifdef ENABLE_EXTERNAL_RAM
		if(!ram.readElementIntoVar(memRegionId, receiveQueueFirst, f))
			return false;
	#else
		memcpy(f, &receiveQueue[receiveQueueFirst], sizeof(frame_t));
	#endif

	receiveQueueFirst = (receiveQueueFirst + 1) % CONFIG_L2_RECEIVE_BUFFER_LEN;
	receiveQueueNum--;

	return true;
}

Layer2rf24::Layer2rf24() {
}

void Layer2rf24::init(Layer3* l3, uint8_t pin_ce, uint8_t pin_csn, uint16_t deviceAddress) {
	this->l3 = l3;
	this->pin_ce = pin_ce;
	this->pin_csn = pin_csn;
	this->deviceAddress = deviceAddress;

	#ifdef ENABLE_EXTERNAL_RAM
		#ifdef DEBUG_RAM_ENABLE
			Serial.print(F("region for l2 receive queue: "));
		#endif
		memRegionId = ram.createRegion(sizeof(frame_t), CONFIG_L2_RECEIVE_BUFFER_LEN);
	#else
		memset(receiveQueue, 0, CONFIG_L2_RECEIVE_BUFFER_LEN * sizeof(frame_t));
	#endif

	receiveQueueFirst = 0;
	receiveQueueNum = 0;

	radio.init(this->pin_ce, this->pin_csn);
	setupRadio();
}

uint8_t Layer2rf24::receive()
{
	uint8_t num = 0;
	while(radio.available()) {
		frame_t frame;
		memset(&frame, 0, sizeof(frame));
		radio.read(frame.bytes, sizeof(frame_t));

		//sanity checks.
		if(frame.data.source == 0 || frame.data.payloadLen == 255) {
			#ifdef DEBUG_NETWORK_ENABLE
			Serial.print(millis());
			Serial.println(F(": L2.receive() discarded"));
			#endif
			continue; //discard.
		}

		#ifdef DEBUG_NETWORK_ENABLE
			Serial.print(millis());
			Serial.println(F(": L2.receive()"));
			Serial.print(F("\tto="));
			Serial.print(frame.data.destination);
			Serial.print(F(" from="));
			Serial.print(frame.data.source);
			Serial.print(F(" len="));
			Serial.println(frame.data.payloadLen);

			printBytes(&frame);
		#endif

		if(receiveQueuePush(&frame)) {
			num++;
		}
		#ifdef FAILURE_HANDLING
			else {
				Serial.print(millis()));
				Serial.println(F(": L2.receiveQueue full."));
			}
		#endif
	}

	return num;
}

void Layer2rf24::printBytes(frame_t* frame) {
		Serial.print(F("\t"));
		for(uint8_t i = 0; i < sizeof(frame->bytes); i++) {
			if(i % 2 == 0) {
				Serial.print(F(" "));
			}
			Serial.print(frame->bytes[i], HEX);
		}
		Serial.println();
}

boolean Layer2rf24::sendFrame( frame_t* frame )
{
	//stop listening and open writing pipe
	radio.stopListening();
	radio.openWritingPipe(frame->data.destination);

	#ifdef DEBUG_NETWORK_ENABLE
		Serial.print(millis());
		Serial.println(F(": L2.snd()"));
		Serial.print(F("\tsrc="));
		Serial.print(frame->data.source);
		Serial.print(F(" to="));
		Serial.print(frame->data.destination);
		Serial.print(F(" l="));
		Serial.println(frame->data.payloadLen);
		
		printBytes(frame);
	#endif


	boolean result = true;
	//if(frame->data.destination == CONFIG_L2_ADDR_BROADCAST) {
		////broadcast
		//radio.write(frame->bytes, sizeof(frame_t), false); //do not request aclk
	//} else {
		////others
		//result = radio.write(frame->bytes, sizeof(frame_t), true); //request ack
	//}
	radio.write(frame->bytes, sizeof(frame_t));
	radio.txStandBy();

	//reenable listening
	radio.startListening();
	return result;
}

void Layer2rf24::setupRadio()
{
	radio.begin();

	radio.setPALevel(CONFIG_RF_PA_LEVEL);
	radio.setChannel(CONFIG_RF_CHANNEL);
	radio.setCRCLength(CONFIG_CRC);
	radio.setDataRate(CONFIG_RF_DATARATE);
	radio.setPayloadSize(CONFIG_L2_PAYLOAD_SIZE);
	radio.setRetries(CONFIG_L2_RETRY_DELAY_250US, CONFIG_L2_RETRY_MAX);

	//maybe not to use...
	radio.enableAckPayload();
	radio.enableDynamicPayloads();
	radio.enableDynamicAck();

	//open reading pipe for this device.
	radio.openReadingPipe(CONFIG_RF_PIPE_DEVICE, deviceAddress);
	radio.openReadingPipe(CONFIG_RF_PIPE_BROADCAST, CONFIG_L2_ADDR_BROADCAST);
	radio.setAutoAck(CONFIG_RF_PIPE_DEVICE, true);
	radio.setAutoAck(CONFIG_RF_PIPE_BROADCAST, false);
	//radio.setAutoAck(true);

	//start listening
	radio.startListening();
}

boolean Layer2rf24::createFrame(frame_t* f, address_t destination, uint8_t payloadLen, uint8_t* payload )
{
	if(payloadLen > l2PayloadMaxLen) {
		return false;
	}

	memset(f, 0, sizeof(frame_t));
	f->data.destination = destination;
	f->data.source = this->deviceAddress;
	f->data.payloadLen = payloadLen;
	memcpy(f->data.payload, payload, payloadLen);

	return true;
}
