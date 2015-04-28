/*
 * main.cpp
 *
 * Created: 08.01.2015 21:20:37
 *  Author: helge
 */
#include <Arduino.h>

#include "networking/Layer3.h"

#define address00 0x1000
#define address01 0x1001
#define address10 0x1002
#define address11 0x1003

uint16_t address_local;
uint16_t address_relay;
uint16_t address_remote;
boolean isServer;

#define PIN_CE A0
#define PIN_CSN 10
Layer2rf24* l2;
Layer3* l3;

#include <drivers/digitalio/DHT11.h>
#include <interfaces/output/RCSwitchTevionFSI07.h>
#include <interfaces/output/LED.h>

#include <HardwareDriver.h>
#include "dispatcher/HardwareInterface.h"
#include "dispatcher/PacketDispatcher.h"
#include <HardwareID.h>



/**
 * read pin 4..9 inverted
 */
void getAddress() {
	pinMode(4, INPUT_PULLUP);
	pinMode(5, INPUT_PULLUP);
	pinMode(6, INPUT_PULLUP);
	pinMode(7, INPUT_PULLUP);
	pinMode(8, INPUT_PULLUP);
	pinMode(9, INPUT_PULLUP);

	#define invert(x) ((x == 1) ? 0 : 1)
	address_local = 0;
	address_local |= invert(digitalRead(4)) << 5;
	address_local |= invert(digitalRead(5)) << 4;
	address_local |= invert(digitalRead(6)) << 3;
	address_local |= invert(digitalRead(7)) << 2;
	address_local |= invert(digitalRead(8)) << 1;
	address_local |= invert(digitalRead(9)) << 0;
	#undef invert

	#ifdef DEBUG
		Serial.print(millis());
		Serial.print(F(": address=0b"));
		Serial.print(address_local, BIN);
		Serial.print(F("="));
		Serial.println(address_local);
		Serial.flush();
	#endif
}



void setup() {
	Serial.begin(115200);
	Serial.println("start test...");
	Serial.flush();

	//get address config
	getAddress();

	//init network
	l3 = new Layer3(address_local);
	l2 = new Layer2rf24(l3, PIN_CE, PIN_CSN, address_local);
	l3->setLayer2(l2);

	////setupEventBus();
	DHT11 dht11 = DHT11(17, 20);
	RCSwitchTevionFSI07 rcsw = RCSwitchTevionFSI07(14, 21);
	LED led = LED(30, 22);

	led.turnOn();
	delay(500);
	led.turnOff();


	HardwareInterface hwInterface = HardwareInterface();
	hwInterface.registerDriver((HardwareDriver*) &dht11);
	hwInterface.registerDriver((HardwareDriver*) &rcsw);


	PacketDispatcher dispatcher = PacketDispatcher(l3, &hwInterface);

	command_t cmd_packet;
	cmd_packet.address = 0;
	cmd_packet.isRead = true;
	cmd_packet.type = HWType_temprature;
	HardwareCommandResult cmd = HardwareCommandResult();
	cmd.deSerialize(&cmd_packet);
	hwInterface.executeCommand(&cmd);


	////create network packet
	//cmd
	command_t cmdX;
	cmdX.address = dht11.getAddress();
	cmdX.type = HWType_temprature;
	cmdX.isRead = 1;

	//app layer
	packet_application_numbered_cmd_t appCmd;
	appCmd.packetType = HARDWARE_COMMAND_READ;
	memcpy(appCmd.payload, (byte*) &cmdX, sizeof(cmdX));

	//networking numbered
	packet_numbered_t numbered;
	numbered.seqNumber = 37;
	memcpy(numbered.payload, (byte*) &appCmd, sizeof(appCmd));
	numbered.payloadLen = sizeof(appCmd);

	//network packet
	Layer3::packet_t p;
	p.data.destination = address_local;
	p.data.hopcount = 5;
	p.data.source = 12;
	p.data.type = PACKET_NUMBERED;
	memcpy(p.data.payload, (byte*) &numbered, sizeof(numbered));
	p.data.payloadLen = sizeof(numbered);

	//processing
	dispatcher.handleNumberedFromNetwork(p);

	//numbered for discovery
	//app layer
	packet_application_numbered_cmd_t appCmd2;
	appCmd2.packetType = HARDWARE_DISCOVERY_REQ;

	//networking numbered
	packet_numbered_t numbered2;
	numbered2.seqNumber = 37;
	memcpy(numbered2.payload, (byte*) &appCmd2, sizeof(appCmd2));
	numbered2.payloadLen = sizeof(appCmd2);

	//network packet
	Layer3::packet_t p2;
	p2.data.destination = address_local;
	p2.data.hopcount = 5;
	p2.data.source = 12;
	p2.data.type = PACKET_NUMBERED;
	memcpy(p2.data.payload, (byte*) &numbered2, sizeof(numbered2));
	p2.data.payloadLen = sizeof(numbered2);

	//processing
	dispatcher.handleNumberedFromNetwork(p2);


	//turn off LED
	pinMode(LED_BUILTIN, LOW);
}

void loop() {
	//do networking.
	l3->Loop();
	//l2->radio->printDetails();

/***
	if(millis() - lastBeaconT > 60*1000UL) {
		lastBeaconT = millis();
		l3->sendBeacon();
	}
	l2->receive();
	while(l2->receiveQueueSize() > 0) {
		Layer2rf24::frame_t f;
		l2->receiveQueuePop(&f);
		l3->receive((uint8_t*) &f.data.payload);
	}
	l3->updateSendingBuffer();

	delay(5000);

	//did we find more than one neighbour?
	if(isServer && l3->neighboursSize() > 1) {
		Layer3::packet_t p;
		packet_numbered_t payload;
		payload.payloadLen = 0;
		payload.seqNumber = 1234L;
		l3->createPacketGeneric(&p, address_remote, PACKET_NUMBERED, (uint8_t*) &payload, sizeof(payload));

		//manipulate neighbourtable for routing!
		for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
			if(l3->neighbours[i].nodeId == address_remote) {
				l3->neighbours[i].hopCount = 1;
				l3->neighbours[i].hopNextNodeId = address_relay;
				l3->neighbours[i].timestamp = millis();

				break;
			}
		}

		l3->sendPacket(p);
****/
/***
	//create beacon
	packet_beacon_t beacon;
	memset(&beacon, 0, sizeof(packet_beacon_t));
	routeInfo_t routeInfo[3];
	routeInfo[0].nodeId = 123;
	routeInfo[0].hopcount = 3;
	routeInfo[1].nodeId = 465;
	routeInfo[1].hopcount = 4;
	routeInfo[2].nodeId = 465; //not an actual real test example, but should do for testing.
	routeInfo[2].hopcount = 2;

	beacon.nodeId = address_remote;
	memcpy(beacon.neighbours, routeInfo, sizeof(routeInfo));
	beacon.numNeighbourInfo = 3;

	Layer3::packet_t packet_beacon;
	memset(&packet_beacon, 0, sizeof(Layer3::packet_t));
	packet_beacon.data.destination = CONFIG_L3_ADDRESS_BROADCAST;
	packet_beacon.data.hopcount = 0;
	packet_beacon.data.payloadLen = sizeof(beacon);
	memcpy(packet_beacon.data.payload, &beacon, sizeof(beacon));
	packet_beacon.data.source = address_local;
	packet_beacon.data.type = PACKET_BEACON;

	//receive beacon frame
	Layer2rf24::frame_t f1;
	l2->createFrame(&f1, (Layer2rf24::address_t) CONFIG_L2_ADDR_BROADCAST, sizeof(packet_beacon), (uint8_t*) &packet_beacon );
	l2->receiveQueuePush(&f1);

	Layer2rf24::frame_t f2;
	l2->receiveQueuePop(&f2);
	l3->receive(f2.data.payload);

	//create packet for forwarding
	Layer3::packet_t packetForward;
	packetForward.data.destination = 465;
	packetForward.data.source = address_local;
	packetForward.data.payloadLen = 0;
	packetForward.data.hopcount = 0;
	packetForward.data.type = PACKET_UNNUMBERED;
	l3->receive((uint8_t*) &packetForward);

	Layer3::packet_t packetForward2;
	packetForward2.data.destination = 465;
	packetForward2.data.source = address_local;
	packetForward2.data.payloadLen = 0;
	packetForward2.data.hopcount = CONFIG_L3_MAX_HOPCOUNT;
	packetForward2.data.type = PACKET_UNNUMBERED;
	l3->receive((uint8_t*) &packetForward2);

	//create numbered packet.
	packet_numbered_t numbered;
	numbered.payloadLen = 0;
	numbered.seqNumber = 12346UL;

	Layer3::packet_t packetNumbered;
	l3->createPacketGeneric(&packetNumbered, 465, PACKET_NUMBERED, (uint8_t*) &numbered, sizeof(packet_numbered_t));

	l3->sendPacket(packetNumbered);
	l3->updateSendingBuffer();


	//ack packet
	packet_ack_t ack;
	ack.ack = 12346UL;

	Layer3::packet_t packetAck;
	l3->createPacketGeneric(&packetAck, address_local, PACKET_ACK, (uint8_t*) &ack, sizeof(packet_ack_t));
	l3->receive((uint8_t*) &packetAck);


	delay(5000);
***/
}