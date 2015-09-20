/*
 * main.cpp
 *
 * Created: 08.01.2015 21:20:37
 *  Author: helge
 */
#include <Arduino.h>
#include <Globals.h>

/**
 * read pin 4..9 inverted
 */
void getAddress() {
	#ifdef ADDRESS_PIN0
	pinMode(ADDRESS_PIN0, INPUT_PULLUP);
	#endif
	#ifdef ADDRESS_PIN1
	pinMode(ADDRESS_PIN1, INPUT_PULLUP);
	#endif
	#ifdef ADDRESS_PIN2
	pinMode(ADDRESS_PIN2, INPUT_PULLUP);
	#endif
	#ifdef ADDRESS_PIN3
	pinMode(ADDRESS_PIN3, INPUT_PULLUP);
	#endif
	#ifdef ADDRESS_PIN4
	pinMode(ADDRESS_PIN4, INPUT_PULLUP);
	#endif
	#ifdef ADDRESS_PIN5
	pinMode(ADDRESS_PIN5, INPUT_PULLUP);
	#endif
	#ifdef ADDRESS_PIN6
	pinMode(ADDRESS_PIN6, INPUT_PULLUP);
	#endif
	#ifdef ADDRESS_PIN7
	pinMode(ADDRESS_PIN7, INPUT_PULLUP);
	#endif

	#define invert(x) ((x == 1) ? 0 : 1)
	address_local = 0;
	#ifdef ADDRESS_PIN7
	address_local |= invert(digitalRead(ADDRESS_PIN7)) << 7;
	#endif
	#ifdef ADDRESS_PIN6
	address_local |= invert(digitalRead(ADDRESS_PIN6)) << 6;
	#endif
	#ifdef ADDRESS_PIN5
	address_local |= invert(digitalRead(ADDRESS_PIN5)) << 5;
	#endif
	#ifdef ADDRESS_PIN4
	address_local |= invert(digitalRead(ADDRESS_PIN4)) << 4;
	#endif
	#ifdef ADDRESS_PIN3
	address_local |= invert(digitalRead(ADDRESS_PIN3)) << 3;
	#endif
	#ifdef ADDRESS_PIN2
	address_local |= invert(digitalRead(ADDRESS_PIN2)) << 2;
	#endif
	#ifdef ADDRESS_PIN1
	address_local |= invert(digitalRead(ADDRESS_PIN1)) << 1;
	#endif
	#ifdef ADDRESS_PIN0
	address_local |= invert(digitalRead(ADDRESS_PIN0)) << 0;
	#endif
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
	Serial.begin(SERIAL_SPEED);

	randomSeed(analogRead(1));

	#ifdef DEBUG
		Serial.println("start test...");
		Serial.print(F(" CE="));
		Serial.print(PIN_CE);
		Serial.print(F(" CSN="));
		Serial.print(PIN_CSN);
		Serial.print(F(" MISO="));
		Serial.print(MISO);
		Serial.print(F(" MOSI="));
		Serial.print(MOSI);
		Serial.print(F(" SCK="));
		Serial.println(SCK);
	#endif

	#ifdef RTC_ENABLE
		rtc.init(90);
		uint32_t tmp = rtc.read();
		setTime(tmp);
		Serial.print(millis());
		Serial.print(F(": time is: "));
		Serial.print(tmp);
		Serial.print(F(" = "));

		tmElements_t tmElems;
		breakTime(tmp, tmElems);
		Serial.print((uint16_t) 1970 + tmElems.Year);
		Serial.print(F("/"));
		Serial.print(tmElems.Month);
		Serial.print(F("/"));
		Serial.print(tmElems.Day);
		Serial.print(F(" "));
		Serial.print(tmElems.Hour);
		Serial.print(F(":"));
		Serial.print(tmElems.Minute);
		Serial.print(F(":"));
		Serial.println(tmElems.Second);
	#endif


	//enable watchdog
	wdt_enable(WDTO_8S);

	//get address config
	getAddress();

	#ifdef WEBSERVER_ENABLE
		webServerWrapper.init();
	#endif

	#ifdef SDCARD_ENABLE
		sdcard.initSD();
		discoveryManager.init();
		nodeInfo.init();
	#endif

	dispatcher.init();
	hwInterface.init();

	//init network
	l3.init(address_local);
	l2.init(&l3, PIN_CE, PIN_CSN, address_local);
	l3.setLayer2(&l2);

	dht11.init(A2, 20);

	#ifdef __AVR_ATmega2560__
	rcsw.init(14, 21);
	#else
	rcsw.init(A6, 21);
	#endif

	bmp180.init(80);
	motion.init(A3, 50);
	light.init(A7, 60);
	//mytone.init(A8, 70);

	#ifdef RTC_ENABLE
		hwInterface.registerDriver((HardwareDriver*) &rtc);
	#endif

	hwInterface.registerDriver((HardwareDriver*) &dht11);
	hwInterface.registerDriver((HardwareDriver*) &rcsw);
	hwInterface.registerDriver((HardwareDriver*) &motion);
	hwInterface.registerDriver((HardwareDriver*) &light);
	hwInterface.registerDriver((HardwareDriver*) &bmp180);
	//hwInterface.registerDriver((HardwareDriver*) &mytone);

	dispatcher.init(&l3, &hwInterface);

	#ifdef SDCARD_LOGGER_ENABLE
		dispatcher.getResponseHandler()->registerListenerByPacketType(0, HARDWARE_COMMAND_RES, 0, &sdlistener);
	#endif

	#ifdef TIMER_ENABLE
	timer.init();
	#endif

	/*/test rcsw
	//HardwareCommandResult cmd;
	//memset(&cmd, 0, sizeof(cmd));
	//cmd.setHardwareType(HWType_rcswitch);
	//cmd.setAddress(rcsw.getAddress());
	//cmd.setUint8ListNum(2);
	//cmd.getUint8List()[0] = 3;
	//cmd.getUint8List()[1] = 0;
	//hwInterface.executeCommand(&cmd);
	///*/


	/*
	SDcard::SD_nodeDiscoveryInfoTableEntry_t infos[2];
	infos[0].hardwareAddress = 0xAA;
	infos[0].hardwareType = 0xBB;
	infos[0].rtcTimestamp = 0xCC;
	infos[1].hardwareAddress = 0xDD;
	infos[1].hardwareType = 0xEE;
	infos[1].rtcTimestamp = 0xFF;
	sdcard.saveDiscoveryInfos(1, infos, 2);

	SDcard::SD_nodeDiscoveryInfoTableEntry_t discovery[SD_DISCOVERY_NUM_INFOS_PER_NODE];
	sdcard.getDiscoveryInfosForNode(1, discovery, SD_DISCOVERY_NUM_INFOS_PER_NODE);
	Serial.print(discovery[0].hardwareAddress);
	Serial.print(F(" "));
	Serial.print(discovery[0].hardwareType);
	Serial.print(F(" "));
	Serial.println(discovery[0].rtcTimestamp);
	Serial.print(discovery[1].hardwareAddress);
	Serial.print(F(" "));
	Serial.print(discovery[1].hardwareType);
	Serial.print(F(" "));
	Serial.println(discovery[1].rtcTimestamp);
	Serial.flush();
	*/

/*
	#ifdef SDCARD_ENABLE
		char buf[17];
		memset(buf, 0, sizeof(buf));
		Serial.print(millis());
		Serial.print(F(": nodeinfo"));
		if(sdcard.getNodeInfo(l3.localAddress, (uint8_t*) buf, sizeof(buf))) {
			buf[16] = '\0';
			Serial.print(F("=\""));
			Serial.print(buf);
			Serial.println(F("\""));
			} else {
			Serial.println(F(" not available"));
		}

		dispatcher.getResponseHandler()->registerListenerByPacketType(0, HARDWARE_COMMAND_RES, 0, &sdlistener);

		packet_application_numbered_cmd_t appPacket;
		memset(&appPacket, 0, sizeof(appPacket));
		appPacket.packetType = HARDWARE_COMMAND_RES;
		command_t* hwcmd = (command_t*) appPacket.payload;
		hwcmd->address = 10;
		hwcmd->type = HWType_rtc;
		hwcmd->numUint8 = 4;
		uint32_t now = rtc.read();
		hwcmd->uint8list[0] = now >> 24;
		hwcmd->uint8list[1] = (now >> 16) & 0xff;
		hwcmd->uint8list[2] = (now >> 8) & 0xff;
		hwcmd->uint8list[3] = now & 0xff;
		dispatcher.handleNumbered(13, HARDWARE_COMMAND_RES, 123, &appPacket);
		//sdlistener.doCallback(&appPacket, 17, 1234);
	#endif
*/

/*
	//mySimpleTimer = SimpleTimer::instance();

	//Serial.println("### testHardwareCommand: Tone ###");
	//Serial.flush();
	//testHardwareCommandTone();
//
//
	//Serial.println("### testHardwareCommand: Light ###");
	//Serial.flush();
	//testHardwareCommand(0, HWType_light, true);
	//testHardwareCommand(0, HWType_light, true);
	//testHardwareCommand(0, HWType_light, true);
//
	//Serial.println("### testHardwareCommand: Temperature ###");
	//Serial.flush();
	//testHardwareCommand(0, HWType_temprature, true);
//
	//Serial.println("### testHardwareCommand: Motion ###");
	//Serial.flush();
	//testHardwareCommand(0, HWType_motion, true);
//
	//Serial.println("### testHardwareCommandRead ###");
	//Serial.flush();
	//testHardwareCommandRead();
//
	//Serial.println("### testDiscovery ###");
	//Serial.flush();
	//testDiscovery();
//
	//Serial.println("### testSubscriptionSet ###");
	//Serial.flush();
	//testSubscriptionSet();
//
	//Serial.println("### testSubscriptionInfo ###");
	//Serial.flush();
	//testSubscriptionInfo();
//
	//Serial.println("### testSubscriptionExecution ###");
	//Serial.flush();
	//testSubscriptionExecution();
//
	//Serial.println("### testSubscriptionPolling ###");
	//Serial.flush();
	//testSubscriptionPolling();
//
	//Serial.println("### testSubscriptionPolling Light ###");
	//Serial.flush();
	//testSubscriptionPollingLight();
//
	////turn off LED
	//pinMode(LED_BUILTIN, LOW);*/
}

void loop() {
	//do networking.
	l3.Loop();

	//do handling.
	dispatcher.loop();

	#ifdef WEBSERVER_ENABLE
		webServerWrapper.loop();
	#endif

	//watchdog reset.
	wdt_reset();

	#ifdef SDCARD_ENABLE
		discoveryManager.loop();
	#endif
}

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


/*
//#ifndef PRODUCTIVE_MEGA328P
//void testHardwareCommandRead() {
	//////create network packet
	////cmd
	//command_t cmdX;
	//memset(&cmdX, 0, sizeof(cmdX));
	//cmdX.address = dht11->getAddress();
	//cmdX.type = HWType_temprature;
	//cmdX.isRead = 1;
//
	////app layer
	//packet_application_numbered_cmd_t appCmd;
	//memset(&appCmd, 0, sizeof(appCmd));
	//appCmd.packetType = HARDWARE_COMMAND_READ;
	//memcpy(appCmd.payload, (byte*) &cmdX, sizeof(cmdX));
//
	////networking numbered
	//packet_numbered_t numbered;
	//memset(&numbered, 0, sizeof(numbered));
	//numbered.seqNumber = 37;
	//memcpy(numbered.payload, (byte*) &appCmd, sizeof(appCmd));
	//numbered.payloadLen = sizeof(appCmd);
//
	////network packet
	//Layer3::packet_t p;
	//memset(&p, 0, sizeof(p));
	//p.data.destination = address_local;
	//p.data.hopcount = 5;
	//p.data.source = 12;
	//p.data.type = PACKET_NUMBERED;
	//memcpy(p.data.payload, (byte*) &numbered, sizeof(numbered));
	//p.data.payloadLen = sizeof(numbered);
//
	////processing
	//dispatcher.handleNumberedFromNetwork(p);
//}
//
//void testHardwareCommand(uint8_t address, HardwareTypeIdentifier type, uint8_t isRead) {
	//command_t cmd_packet;
	//memset(&cmd_packet, 0, sizeof(cmd_packet));
	//cmd_packet.address = address;
	//cmd_packet.isRead = isRead;
	//cmd_packet.type = type;
	//HardwareCommandResult cmd = HardwareCommandResult();
	//cmd.deSerialize(&cmd_packet);
	//hwInterface.executeCommand(&cmd);
//}
//
//void testHardwareCommandTone() {
	//SimpleTimer::instance()->run();
//
	//command_t cmd_packet;
	//memset(&cmd_packet, 0, sizeof(cmd_packet));
	//cmd_packet.address = mytone.getAddress();
	//cmd_packet.isRead = false;
	//cmd_packet.type = HWType_tone;
	//cmd_packet.numUint16 = 1;
	//cmd_packet.uint16list[0] = 2000;
	//HardwareCommandResult cmd = HardwareCommandResult();
	//cmd.deSerialize(&cmd_packet);
	//hwInterface.executeCommand(&cmd);
//
	//SimpleTimer::instance()->run();
	//delay(2500);
	//SimpleTimer::instance()->run();
//}
//
//void testSubscriptionSet() {
		////numbered for subscription
	////app layer
	//subscription_set_t cmdSubscription;
	//memset(&cmdSubscription, 0, sizeof(cmdSubscription));
	//cmdSubscription.info.address = 2; //node address
	//cmdSubscription.info.hardwareAddress = dht11->getAddress();
	//cmdSubscription.info.hardwareType = HWType_temprature;
	//cmdSubscription.info.millisecondsDelay = 1347;
	//cmdSubscription.info.sequence = 1337;
	//cmdSubscription.info.onEventType = EVENT_TYPE_DISABLED;
//
	//packet_application_numbered_cmd_t appCmd3;
	//memset(&appCmd3, 0, sizeof(appCmd3));
	//appCmd3.packetType = HARDWARE_SUBSCRIPTION_SET;
	//memcpy(&appCmd3.payload, &cmdSubscription, sizeof(cmdSubscription));
//
	////networking numbered
	//packet_numbered_t numbered3;
	//memset(&numbered3, 0, sizeof(numbered3));
	//numbered3.seqNumber = 37;
	//memcpy(numbered3.payload, (byte*) &appCmd3, sizeof(appCmd3));
	//numbered3.payloadLen = sizeof(appCmd3);
//
	////network packet
	//Layer3::packet_t p3;
	//p3.data.destination = address_local;
	//p3.data.hopcount = 5;
	//p3.data.source = 12;
	//p3.data.type = PACKET_NUMBERED;
	//memcpy(p3.data.payload, (byte*) &numbered3, sizeof(numbered3));
	//p3.data.payloadLen = sizeof(numbered3);
//
	////processing
	//dispatcher.handleNumberedFromNetwork(p3);
//}
//
//void testDiscovery() {
		////numbered for discovery
	////app layer
	//packet_application_numbered_cmd_t appCmd2;
	//memset(&appCmd2, 0, sizeof(appCmd2));
	//appCmd2.packetType = HARDWARE_DISCOVERY_REQ;
//
	////networking numbered
	//packet_numbered_t numbered2;
	//memset(&numbered2, 0, sizeof(numbered2));
	//numbered2.seqNumber = 37;
	//memcpy(numbered2.payload, (byte*) &appCmd2, sizeof(appCmd2));
	//numbered2.payloadLen = sizeof(appCmd2);
//
	////network packet
	//Layer3::packet_t p2;
	//memset(&p2, 0, sizeof(p2));
	//p2.data.destination = address_local;
	//p2.data.hopcount = 5;
	//p2.data.source = 12;
	//p2.data.type = PACKET_NUMBERED;
	//memcpy(p2.data.payload, (byte*) &numbered2, sizeof(packet_numbered_t));
	//p2.data.payloadLen = sizeof(numbered2);
//
	////processing
	//dispatcher.handleNumberedFromNetwork(p2);
//}
//
//void testSubscriptionInfo() {
	//////subscription info
	////app layer
	//packet_application_numbered_cmd_t appCmd4;
	//memset(&appCmd4, 0, sizeof(appCmd4));
	//appCmd4.packetType = HARDWARE_SUBSCRIPTION_INFO;
	//memset(&appCmd4.payload, 0, sizeof(appCmd4.payload));
//
	////networking numbered
	//packet_numbered_t numbered4;
	//memset(&numbered4, 0, sizeof(numbered4));
	//numbered4.seqNumber = 37;
	//memcpy(numbered4.payload, (byte*) &appCmd4, sizeof(appCmd4));
	//numbered4.payloadLen = sizeof(appCmd4);
//
	////network packet
	//Layer3::packet_t p4;
	//p4.data.destination = address_local;
	//p4.data.hopcount = 5;
	//p4.data.source = 12;
	//p4.data.type = PACKET_NUMBERED;
	//memcpy(p4.data.payload, (byte*) &numbered4, sizeof(numbered4));
	//p4.data.payloadLen = sizeof(numbered4);
//
	////processing
	//dispatcher.handleNumberedFromNetwork(p4);
//}
//
//void testSubscriptionPolling() {
		////numbered for subscription
		////app layer
		//subscription_set_t cmdSubscription;
		//memset(&cmdSubscription, 0, sizeof(cmdSubscription));
		//cmdSubscription.info.address = 147; //node address
		//cmdSubscription.info.hardwareAddress = motion->getAddress();
		//cmdSubscription.info.hardwareType = HWType_motion;
		//cmdSubscription.info.millisecondsDelay = 0;
		//cmdSubscription.info.sequence = 1337;
		//cmdSubscription.info.onEventType = EVENT_TYPE_CHANGE;
//
		//packet_application_numbered_cmd_t appCmd3;
		//memset(&appCmd3, 0, sizeof(appCmd3));
		//appCmd3.packetType = HARDWARE_SUBSCRIPTION_SET;
		//memcpy(&appCmd3.payload, &cmdSubscription, sizeof(cmdSubscription));
//
		////networking numbered
		//packet_numbered_t numbered3;
		//memset(&numbered3, 0, sizeof(numbered3));
		//numbered3.seqNumber = 37;
		//memcpy(numbered3.payload, (byte*) &appCmd3, sizeof(appCmd3));
		//numbered3.payloadLen = sizeof(appCmd3);
//
		////network packet
		//Layer3::packet_t p3;
		//p3.data.destination = address_local;
		//p3.data.hopcount = 5;
		//p3.data.source = 12;
		//p3.data.type = PACKET_NUMBERED;
		//memcpy(p3.data.payload, (byte*) &numbered3, sizeof(numbered3));
		//p3.data.payloadLen = sizeof(numbered3);
//
		////processing
		//dispatcher.handleNumberedFromNetwork(p3);
//
		////wait
		//delay(1000);
		//dispatcher.loop();
//}
//
//void testSubscriptionPollingLight() {
	////numbered for subscription
	////app layer
	//subscription_set_t cmdSubscription;
	//memset(&cmdSubscription, 0, sizeof(cmdSubscription));
	//cmdSubscription.info.address = 147; //node address
	//cmdSubscription.info.hardwareAddress = light->getAddress();
	//cmdSubscription.info.hardwareType = HWType_light;
	//cmdSubscription.info.millisecondsDelay = 0;
	//cmdSubscription.info.sequence = 1337;
	//cmdSubscription.info.onEventType = EVENT_TYPE_CHANGE;
//
	//packet_application_numbered_cmd_t appCmd3;
	//memset(&appCmd3, 0, sizeof(appCmd3));
	//appCmd3.packetType = HARDWARE_SUBSCRIPTION_SET;
	//memcpy(&appCmd3.payload, &cmdSubscription, sizeof(cmdSubscription));
//
	////networking numbered
	//packet_numbered_t numbered3;
	//memset(&numbered3, 0, sizeof(numbered3));
	//numbered3.seqNumber = 37;
	//memcpy(numbered3.payload, (byte*) &appCmd3, sizeof(appCmd3));
	//numbered3.payloadLen = sizeof(appCmd3);
//
	////network packet
	//Layer3::packet_t p3;
	//p3.data.destination = address_local;
	//p3.data.hopcount = 5;
	//p3.data.source = 12;
	//p3.data.type = PACKET_NUMBERED;
	//memcpy(p3.data.payload, (byte*) &numbered3, sizeof(numbered3));
	//p3.data.payloadLen = sizeof(numbered3);
//
	////processing
	//dispatcher.handleNumberedFromNetwork(p3);
//
	////wait
	//delay(1000);
	//dispatcher->loop();
//}
//
//void testSubscriptionExecution() {
	//delay(2000); //enough for our test.
	//dispatcher.loop();
//}
//#endif*/
