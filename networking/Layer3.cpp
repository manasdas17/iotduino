#include "Layer3.h"

Layer3::Layer3() {
}

void Layer3::init(l3_address_t localAddress) {
	randomSeed(analogRead(0));

	this->eventCallbackClass = new callbackClass(this);

	memset(sendingNumberedBuffer, 0, sizeof(sendingNumberedBuffer));
	memset(neighbours, 0, sizeof(neighbours));
	//set hopcount to max!
	for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
		neighbours[i].hopCount = 0xff;
	}

	this->localAddress = localAddress;
}



neighbourData* Layer3::getNeighbour( l3_address_t destination )
{
	for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
		if(neighbours[i].nodeId == destination)
			return &neighbours[i];
	}

	return NULL;
}

boolean Layer3::sendPacket( packet_t &packet )
{
	#ifdef DEBUG_NETWORK_ENABLE
		Serial.print(millis());
		Serial.println(F(": Layer3::sendPacket()"));
		printPacketInformation(&packet);
		Serial.flush();
	#endif

	Layer2rf24::address_t l2DestinationAddress = CONFIG_L2_ADDR_BROADCAST;

	//broascast? - does not need any available destination.
	if(packet.data.destination != CONFIG_L3_ADDRESS_BROADCAST) {
		//get neighbour
		neighbourData* neighbour = getNeighbour(packet.data.destination);

		//unknown.
		if(neighbour == NULL) {
			#ifdef DEBUG_NETWORK_ENABLE
				Serial.println(F("\tno neighbour found - no route to host."));
			#endif

			return false;
		}

		l2DestinationAddress = static_cast<Layer2rf24::address_t>(neighbour->hopNextNodeId);
	}


	boolean result = true;

	//numbered? - handle via queue ONLY IF the packet is not routed.
	if(packet.data.type == PACKET_NUMBERED && packet.data.source == localAddress) {
		#ifdef DEBUG_NETWORK_ENABLE
			Serial.print(F("\tnumbered: if new - enqueue: remote="));
			Serial.print(packet.data.destination);
			Serial.print(F(" seq="));
			packet_numbered_t* tmp = (packet_numbered_t*) packet.data.payload;
			Serial.println(tmp->seqNumber);
		#endif
		result &= addToSendingQueue(&packet);
	}

	//one shot: send directly. give it to layer2
	Layer2rf24::frame_t f;
	result &= l2->createFrame(&f, l2DestinationAddress, sizeof(packet_t), packet.bytes);

	if(result)
		result = l2->sendFrame(&f);

	return result;
}

void Layer3::printPacketInformation(packet_t* packet) {
	#ifdef DEBUG_NETWORK_ENABLE
		Serial.print(F("\ttype="));
		switch(packet->data.type) {
			case PACKET_ACK:
				Serial.println(F("ACK"));
				break;
			case PACKET_BEACON:
				Serial.print(F("BEACON"));
				break;
			case PACKET_NUMBERED: {
				Serial.print(F("NUMBERED"));
				packet_numbered_t* tmpNumbered = (packet_numbered_t*) packet->data.payload;
				Serial.print(F(" seq="));
				Serial.print(tmpNumbered->seqNumber);
				packet_application_numbered_cmd_t* tmpApp = (packet_application_numbered_cmd_t*) tmpNumbered->payload;
				Serial.print(F(" appType="));
				Serial.println(tmpApp->packetType);
				break;
			}
			case PACKET_UNNUMBERED:
				Serial.println(F("UNNUMBERED"));
				break;
			default:
				Serial.println(F("unknown"));
				break;
		}
		Serial.print(F("\tsource="));
		Serial.print(packet->data.source);
		Serial.print(F(" dest="));
		Serial.print(packet->data.destination);
		Serial.print(F(" hopcount="));
		Serial.print(packet->data.hopcount);
		Serial.print(F(" payloadLen="));
		Serial.println(packet->data.payloadLen);
		Serial.flush();
	#endif
}
/**
 * add received packet to receive queue
 */
boolean Layer3::receive( void* payload )
{
	packet_t* packet = (packet_t*) payload;

	#ifdef DEBUG_NETWORK_ENABLE
		Serial.print(millis());
		Serial.println(F(": Layer3::receive()"));
		printPacketInformation(packet);
		Serial.flush();
	#endif

	//is this a beacon?
	if(packet->data.type == PACKET_BEACON) {
		return handleBeacon(packet);
	}

	//do we have to route the packet? - no broadcast routing.
	if(packet->data.destination != localAddress && packet->data.destination != CONFIG_L3_ADDRESS_BROADCAST) {
		return routePacket(packet);
	}

	//is this an ack? (it is for us!)
	if(packet->data.destination == localAddress && packet->data.type == PACKET_ACK) {
		return handleAck(packet);
	}

	//nope, store it here.
	boolean success = receiveQueuePush(packet);

	//is this a numbered packet? - send ACK if required.
	if(packet->data.type == PACKET_NUMBERED) {
		success &= sendAck(packet);
	}

	if(!success) {
		return false;
	}

	return true;
}


boolean Layer3::sendAck(packet_t* packet) {
	if(packet->data.type != PACKET_NUMBERED)
		return false;

	//get seq number to ack
	packet_numbered_t* numbered = (packet_numbered_t*) &packet->data.payload;
	seq_t seq = numbered->seqNumber;

	//create ack payload
	packet_ack_t ack;
	ack.ack = seq;

	//create l3 packet
	packet_t answer;
	if(!createPacketGeneric(&answer, packet->data.source, PACKET_ACK, (uint8_t*) &ack, sizeof(packet_ack_t)))
		return false;

	//send.
	return sendPacket(answer);
}

boolean Layer3::createPacketGeneric( packet_t* packet, l3_address_t destination, l3_packetType type, void* payload, uint8_t payloadLen )
{
	if(packet == NULL || payloadLen > CONFIG_APP_PAYLOAD_SIZE) {
		return false;
	}

	//create packet.
	memset(packet, 0, sizeof(packet_t));
	packet->data.type = type;
	packet->data.source = localAddress;
	packet->data.destination = destination;
	packet->data.payloadLen = payloadLen;
	memcpy(packet->data.payload, payload, payloadLen);

	return true;
}

boolean Layer3::receiveQueuePush( packet_t* f )
{
	if(receiveQueueNum >= CONFIG_L3_RECEIVE_BUFFER_LEN)
		return false;

	//copy into queue
	uint8_t index = (receiveQueueFirst + receiveQueueNum) % CONFIG_L3_RECEIVE_BUFFER_LEN;
	memcpy(&receiveQueue[index], f, sizeof(packet_t));
	receiveQueueNum++;

	return true;
}

uint8_t Layer3::receiveQueueSize()
{
	return receiveQueueNum;
}

Layer3::packet_t* Layer3::receiveQueuePeek() {
	if(receiveQueueNum <= 0)
		return NULL;

	return &receiveQueue[receiveQueueFirst];
}

boolean Layer3::receiveQueuePop( packet_t* f )
{
	if(receiveQueueNum <= 0)
	return false;

	//copy data
	memcpy(f, &receiveQueue[receiveQueueFirst], sizeof(packet_t));

	//and adjust queue parameters
	return receiveQueuePop();
}

boolean Layer3::receiveQueuePop() {
	if(receiveQueueNum <= 0)
		return false;

	//discard next item
	receiveQueueFirst = (receiveQueueFirst + 1) % CONFIG_L3_RECEIVE_BUFFER_LEN;
	receiveQueueNum--;
	return true;

}

boolean Layer3::routePacket( packet_t* packet )
{
	#ifdef DEBUG_NETWORK_ENABLE
		Serial.print(millis());
		Serial.println(F(" Layer3::routePacket()"));
		Serial.flush();
	#endif

	//hopcount exceeded?
	if(packet->data.hopcount >= CONFIG_L3_MAX_HOPCOUNT) {
		#ifdef DEBUG_NETWORK_ENABLE
			Serial.println(F("\thopcount exceeded, discarding."));
			Serial.flush();
		#endif
		return false;
	}

	//no.
	packet->data.hopcount++;

	//actual route handling will be done by sendPacket; i.e. l2 addressing.
	return sendPacket(*packet);
}

boolean Layer3::handleBeacon( packet_t* packet ) {
	#ifdef DEBUG_NETWORK_ENABLE
		Serial.print(millis());
		Serial.println(F(": Layer3::handleBeacon()"));
		Serial.flush();
	#endif

	boolean result = true;
	result |= updateNeighbour(packet);
	result |= updateNeighbours((packet_beacon_t*) packet->data.payload);

	return result;
}

boolean Layer3::updateNeighbour( packet_t* packet ) {
	return updateNeighbour(packet->data.source, packet->data.source, 0);
}

boolean Layer3::updateNeighbour( l3_address_t destination, l3_address_t nextHop, uint8_t hopCount) {
	#ifdef DEBUG_NETWORK_ENABLE
		Serial.print(millis());
		Serial.print(F(": updateNeighbour()"));
		Serial.print(F(" destination="));
		Serial.print(destination);
		Serial.print(F(" nextHop="));
		Serial.print(nextHop);
		Serial.print(F(" hopCount="));
		Serial.println(hopCount);
		Serial.flush();
	#endif

	if(destination == localAddress) {
		Serial.println(F("\tthis is us, no routing update!"));
		return true;
	}
	if(hopCount > CONFIG_L3_MAX_HOPCOUNT) {
		Serial.println(F("\ttoo many hops, discard."));
		return true;
	}

	neighbourData* n = getNeighbour(destination);

	//we do not have it yet - create.
	if(n == NULL) {
		//search free index
		uint8_t freeIndex = 0xff;
		for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
			if(neighbours[i].nodeId == 0) {
				freeIndex = i;
				break;
			}
		}

		//do we have space?
		if(freeIndex == 0xff) {
			return false;
		#ifdef DEBUG_NETWORK_ENABLE
			Serial.println(F("\ttable full."));
			Serial.flush();
		#endif
		}

		n = &neighbours[freeIndex];

		#ifdef DEBUG_NETWORK_ENABLE
			Serial.print(F("\tnew entry@index="));
			Serial.print(freeIndex);
			Serial.print(F(": "));
			Serial.flush();
		#endif
	}
	#ifdef DEBUG_NETWORK_ENABLE
		else {
			Serial.print(F("\texisting: "));
			Serial.flush();
		}
	#endif

	//update data - if better.
	#ifdef DEBUG_NETWORK_ENABLE
		Serial.print(F("currentHopCount="));
		Serial.print(n->hopCount);
		Serial.print(F(" newHopCount="));
		Serial.print(hopCount);
		Serial.flush();
	#endif

	if(hopCount < n->hopCount)  {
		n->nodeId = destination;
		n->timestamp = millis();
		n->hopNextNodeId = nextHop;
		n->hopCount = hopCount;

		#ifdef DEBUG_NETWORK_ENABLE
			Serial.println(F(" - updated."));
			Serial.flush();
		#endif
	} else if(hopCount == n->hopCount && nextHop == n->hopNextNodeId) {
		n->timestamp = millis();
		#ifdef DEBUG_NETWORK_ENABLE
			Serial.println(F(" - same information, updated timestamp."));
			Serial.flush();
		#endif
	}
	#ifdef DEBUG_NETWORK_ENABLE
		else
			Serial.println(F(" - not updated."));
			Serial.flush();
	#endif

	//handled, return true.
	return true;
}

boolean Layer3::updateNeighbours(packet_beacon_t* beacon) {
	uint8_t num = beacon->numNeighbourInfo;

	boolean result = true;
	for(uint8_t i = 0; i < num; i++) {
		result |= updateNeighbour(beacon->neighbours[i].nodeId, beacon->nodeId, beacon->neighbours[i].hopcount);
	}

	return result;
}

void Layer3::cleanNeighbours() {
	#ifdef DEBUG_NETWORK_ENABLE
		Serial.print(millis());
		Serial.println(F(": Layer3::cleanNeighbours()"));
		Serial.flush();
	#endif

	for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
		//do we have a valid neighbour?
		if(neighbours[i].nodeId > 0) {
			#ifdef DEBUG_NETWORK_ENABLE
				Serial.print(F("\tindex="));
				Serial.print(i);
				Serial.print(F(" nodeId="));
				Serial.print(neighbours[i].nodeId);
				Serial.print(F(" timestamp="));
				Serial.print(neighbours[i].timestamp);
				Serial.print(F(" hops="));
				Serial.print(neighbours[i].hopCount);
				Serial.print(F(" nextHop="));
				Serial.print(neighbours[i].hopNextNodeId);
			#endif

			if(millis() - neighbours[i].timestamp > CONFIG_L3_NEIGHBOUR_MAX_AGE_MS) {
				//remove
				memset(&neighbours[i], 0, sizeof(neighbourData));
				neighbours[i].hopCount = 0xff;

				#ifdef DEBUG_NETWORK_ENABLE
					Serial.println(F(" cleared!"));
					Serial.flush();
				#endif
			}
			#ifdef DEBUG_NETWORK_ENABLE
				else {
					Serial.println();
					Serial.flush();
				}
			#endif
		}
	}
}

uint8_t Layer3::neighboursSize() {
	uint8_t num = 0;
	for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
		if(neighbours[i].nodeId != 0)
			num++;
	}

	return num;
}

boolean Layer3::sendBeacon() {
	cleanNeighbours();

	#ifdef DEBUG_NETWORK_ENABLE
		Serial.print(millis());
		Serial.println(F(": Layer3::sendBeacon()"));
		Serial.flush();
	#endif

	//debug information
	//neighbours[0].nodeId = 100;
	//neighbours[0].hopCount = 1;
	//neighbours[1].nodeId = 101;
	//neighbours[1].hopCount = 2;
	//neighbours[2].nodeId = 102;
	//neighbours[2].hopCount = 3;
	//neighbours[3].nodeId = 103;
	//neighbours[3].hopCount = 4;
	//neighbours[4].nodeId = 104;
	//neighbours[4].hopCount = 5;
	//neighbours[5].nodeId = 105;
	//neighbours[5].hopCount = 6;
	//neighbours[6].nodeId = 106;
	//neighbours[6].hopCount = 7;
	//neighbours[7].nodeId = 107;
	//neighbours[7].hopCount = 8;
	//neighbours[8].nodeId = 108;
	//neighbours[8].hopCount = 9;

	//number of neighbours in table
	uint8_t numNeighbours = neighboursSize();
	//number of beacons to send
	uint8_t numBeacons = ceil(1.0 * numNeighbours / CONFIG_L3_BEACON_NUM_INFOS);

	boolean result = true;

	if(numBeacons == 0) {
		//send empty beacon info
		//beacon datagram containing neighbourinfo
		packet_beacon_t beacon;
		memset(&beacon, 0, sizeof(beacon));
		beacon.nodeId = localAddress;

		//actual l3 packet
		packet_t packet;
		uint8_t payloadLen = sizeof(beacon);
		l3_address_t addr = CONFIG_L3_ADDRESS_BROADCAST;
		createPacketGeneric(&packet, addr, PACKET_BEACON, (uint8_t*) &beacon, payloadLen);

		//sending
		return sendPacket(packet);
	}

	//iterate trough beacons
	for(uint8_t i = 0; i < numBeacons; i++) {
		//number of neighbours in this beacon
		uint8_t neighboursInfoNum = ((i+1) * CONFIG_L3_BEACON_NUM_INFOS > numNeighbours) ? numNeighbours % CONFIG_L3_BEACON_NUM_INFOS : CONFIG_L3_BEACON_NUM_INFOS;

		//routeinfo
		routeInfo_t info[neighboursInfoNum];
		//and its data
		for(uint8_t j = 0; j < neighboursInfoNum; j++) {
			uint8_t currentNeighbourIndex = i * CONFIG_L3_BEACON_NUM_INFOS + j;

			info[j].hopcount = neighbours[currentNeighbourIndex].hopCount + 1;
			info[j].nodeId = neighbours[currentNeighbourIndex].nodeId;
		}

		//beacon datagram containing neighbourinfo
		packet_beacon_t beacon;
		memset(&beacon, 0, sizeof(beacon));
		beacon.nodeId = localAddress;
		beacon.numNeighbourInfo = neighboursInfoNum;
		memcpy(&beacon.neighbours, &info, sizeof(info));

		//actual l3 packet
		packet_t packet;
		uint8_t payloadLen = sizeof(beacon);
		l3_address_t addr = CONFIG_L3_ADDRESS_BROADCAST;
		createPacketGeneric(&packet, addr, PACKET_BEACON, (uint8_t*) &beacon, payloadLen);

		//sending
		result &= sendPacket(packet);
	}

	return result;
}

boolean Layer3::addToSendingQueue( packet_t* packet ) {
	//do we have a packet to be acked?
	if(packet->data.type != PACKET_NUMBERED) {
		return false;
	}

	//do we already have this packet?

	packet_numbered_t* numberedPacket = (packet_numbered_t*) &packet->data.payload;
	seq_t seqNumber = numberedPacket->seqNumber;

	for(uint8_t i = 0; i < CONFIG_L3_SEND_BUFFER_LEN; i++) {
		if(sendingNumberedBuffer[i].packet.data.destination == 0) {
			//no content here.
			continue;
		}

		packet_numbered_t* numberedQueuePacket = (packet_numbered_t*) &sendingNumberedBuffer[i].packet.data.payload;
		seq_t numberedQueuePacketSeq = numberedQueuePacket->seqNumber;

		if(sendingNumberedBuffer[i].packet.data.destination == packet->data.destination //same destination
			&& seqNumber == numberedQueuePacketSeq //same seq
		) {

			#ifdef DEBUG_NETWORK_ENABLE
				Serial.print(millis());
				Serial.println(F(": Layer3::addToSendingQueue() - duplicate, discarding."));
				Serial.flush();
			#endif

			//break, this is a duplicate.
			return false;
		}
	}

	//search free buffer entry
	uint8_t freeIndex = 0xff;
	for(uint8_t i = 0; i < CONFIG_L3_SEND_BUFFER_LEN; i++) {
		if(sendingNumberedBuffer[i].packet.data.destination == 0) {
			freeIndex = i;
			break;
		}
	}

	//did we find one?
	if(freeIndex == 0xff) {
		return false;
	}

	//create copy.
	sendingNumberedBuffer[freeIndex].lasttimestamp = 0; //millis();
	sendingNumberedBuffer[freeIndex].retransmissions = 0;
	memcpy(&sendingNumberedBuffer[freeIndex].packet, packet, sizeof(packet_t));

	return true;
}

void Layer3::updateSendingBuffer() {
	////#ifdef DEBUG_NETWORK_ENABLE
		////Serial.print(millis());
		////Serial.println(F(": Layer3::updateSendingBuffer()"));
		////Serial.flush();
	////#endif

	uint32_t now = millis();
	for(uint8_t i = 0; i < CONFIG_L3_SEND_BUFFER_LEN; i++) {
		//is this a packet to be retransmitted?
		if(sendingNumberedBuffer[i].packet.data.destination != 0) {
			//do we exceed max retransmissions?
			if(sendingNumberedBuffer[i].retransmissions > CONFIG_L3_NUMBERED_RETRANSMISSIONS) {
				#ifdef DEBUG_NETWORK_ENABLE
					Serial.print(millis());
					Serial.print(F(": resend index="));
					Serial.print(i);
					Serial.println(F(" retransmissions exceeded - discarding."));
					Serial.flush();
				#endif

				//clear.
				memset(&sendingNumberedBuffer[i], 0, sizeof(packet_sending_queue_item_t));

				//next packet.
				continue;
			}

			//has it timed out? - or it may also be new (timestamp 0)
			if(sendingNumberedBuffer[i].lasttimestamp == 0
				|| (now > sendingNumberedBuffer[i].lasttimestamp
					&& now - sendingNumberedBuffer[i].lasttimestamp > CONFIG_L3_NUMBERED_TIMEOUT_MS
					&& now > CONFIG_L3_NUMBERED_TIMEOUT_MS))
			{
				#ifdef DEBUG_NETWORK_ENABLE
					Serial.print(millis());
					Serial.print(F(": resend index="));
					Serial.print(i);
					Serial.print(F(": (re)sending #"));
					Serial.print(sendingNumberedBuffer[i].retransmissions);
					Serial.print(F(" lastTimestamp="));
					Serial.println(sendingNumberedBuffer[i].lasttimestamp);
					Serial.flush();
				#endif

				//linear backoff.
				sendingNumberedBuffer[i].lasttimestamp = now + sendingNumberedBuffer[i].retransmissions * 20;
				sendingNumberedBuffer[i].retransmissions++;

				//yep, resend it.
				//boolean result =
				sendPacket(sendingNumberedBuffer[i].packet);

				//optionally delay.
				//delay(50);
			}
			//#ifdef DEBUG_NETWORK_ENABLE
				//else
					//Serial.println(F("not timed out yet"));
			//#endif

		}
	}
}

boolean Layer3::handleAck( packet_t* packet ) {
	//do we have an ack?
	if(packet->data.type != PACKET_ACK)
		return false;

	//get seq number
	packet_ack_t* ptr = (packet_ack_t*) packet->data.payload;
	seq_t seq = ptr->ack;

	#ifdef DEBUG_NETWORK_ENABLE
		Serial.print(millis());
		Serial.println(F(": Layer3::handleAck()"));
		Serial.print(F("\tseqNum="));
		Serial.print(seq);
		Serial.flush();
	#endif


	//search index
	for(uint8_t i = 0; i < CONFIG_L3_SEND_BUFFER_LEN; i++) {
		packet_numbered_t* ptr = (packet_numbered_t*) sendingNumberedBuffer[i].packet.data.payload;
		if(ptr->seqNumber == seq) {
			#ifdef DEBUG_NETWORK_ENABLE
				uint16_t rtt = millis() - sendingNumberedBuffer[i].lasttimestamp;
				Serial.print(F(" probable RTT="));
				Serial.print(rtt);
				Serial.println(F(" - found & cleared."));
				Serial.flush();
			#endif

			return true;
		}
	}

	#ifdef DEBUG_NETWORK_ENABLE
		Serial.println(F(" - unknown."));
		Serial.flush();
	#endif
	return false;
}

void Layer3::Loop() {
	//l2 check, receive
	l2->receive();
	while(l2->receiveQueueSize() > 0) {
		//get l2 data frame
		Layer2rf24::frame_t f;
		l2->receiveQueuePop(&f);

		//receive on l3
		receive((uint8_t*) &f.data.payload);
	}

	//send pending packet
	updateSendingBuffer();

	//beacon - neighbour announcement.
	if(millis() - beaconLastTimestamp > CONFIG_L3_BEACON_PERIOD_MS)	{
		beaconLastTimestamp = millis();
		sendBeacon();
	}
}

neighbourData* Layer3::getNeighbours() {
	return neighbours;
}

seq_t Layer3::sendNumbered( l3_address_t destination, seq_t seq, void* payload, uint8_t payloadLen ) {
	if(payloadLen > CONFIG_L3_PACKET_NUMBERED_MAX_LEN)
		return 0;

	//create random sequence
	if(seq == 0)
		seq = random(1, 0xffffffff);

	//create enclosing numbered payload
	packet_numbered_t pn;
	pn.seqNumber = seq;
	pn.payloadLen = payloadLen;
	memcpy(&pn.payload, payload, payloadLen);

	//create packet
	packet_t p;
	createPacketGeneric(&p, destination, PACKET_NUMBERED, (uint8_t*) &pn, sizeof(packet_numbered_t));

	//send.
	if(sendPacket(p)) {
		return seq;
	}

	return 0;
}

seq_t Layer3::sendNumbered(l3_address_t destination, void* payload, uint8_t payloadLen) {
	return sendNumbered(destination, 0, payload, payloadLen);
};

boolean Layer3::sendUnnumbered(l3_address_t destination, void* payload, uint8_t payloadLen) {
	if(payloadLen > CONFIG_L3_PACKET_UNNUMBERED_MAX_LEN)
		return 0;

	//create enclosing unnumbered payload
	packet_unnumbered_t pun;
	memcpy(&pun.payload, payload, payloadLen);

	//create packet
	packet_t p;
	createPacketGeneric(&p, destination, PACKET_UNNUMBERED, (uint8_t*) &pun, sizeof(packet_numbered_t));

	//send.
	return sendPacket(p);
}

boolean Layer3::sendBroadcast(void* payload, uint8_t payloadLen) {
	return sendUnnumbered(CONFIG_L3_ADDRESS_BROADCAST, payload, payloadLen);
}

EventCallbackInterface* Layer3::getCallbackInterface() {
	return (EventCallbackInterface*) eventCallbackClass;
}


/***********************************************************************************************************/

void Layer3::callbackClass::doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq) {
	#ifdef DEBUG_HANDLER_ENABLE
		Serial.print(millis());
		Serial.println(F(": Layer3::callbaclClass::doCallback()"));
	#endif

	this->parent->sendNumbered(address, seq, (byte*) appLayerPacket, CONFIG_L3_PACKET_NUMBERED_MAX_LEN);
}

void Layer3::callbackClass::fail(seq_t seq, l3_address_t remote) {
}
