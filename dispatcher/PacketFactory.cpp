#include <dispatcher/PacketFactory.h>

seq_t PacketFactory::generateHardwareCommand(Layer3::packet_t* p, l3_address_t destination, HardwareCommandResult* command) {
	//numbered
	packet_numbered_t numbered;
	preparePacketNumbered(&numbered);


	packet_application_numbered_cmd_t* appCmd = (packet_application_numbered_cmd_t*) numbered.payload;
	//memset(&appCmd, 0, sizeof(appCmd));

	//write data into app packet.
	command->serialize((command_t*) appCmd->payload);

	appCmd->packetType = (!command->isReadRequest()) ? HARDWARE_COMMAND_WRITE : HARDWARE_COMMAND_READ;

	l3.createPacketGeneric(p, destination, PACKET_NUMBERED, (void*) &numbered, sizeof(packet_numbered_t));
	return numbered.seqNumber;
}

seq_t PacketFactory::generateSubscriptionSetGeneric(Layer3::packet_t* p, l3_address_t destination, subscription_helper_t* subscription) {
	//numbered
	packet_numbered_t numbered;
	preparePacketNumbered(&numbered);


	//app layer
	packet_application_numbered_cmd_t* appCmd = (packet_application_numbered_cmd_t*) numbered.payload;
	appCmd->packetType = HARDWARE_SUBSCRIPTION_SET;

	//subscription
	subscription_set_t* subscriptionSet = (subscription_set_t*) appCmd->payload;
	void* tmp = &subscriptionSet->info;
	memcpy(tmp, subscription, sizeof(subscription_helper_t));

	l3.createPacketGeneric(p, destination, PACKET_NUMBERED, &numbered, sizeof(packet_numbered_t));
	return numbered.seqNumber;
}

seq_t PacketFactory::generateSubscriptionSetGeneric(Layer3::packet_t* p, l3_address_t destination, l3_address_t localAddress, uint8_t hwAddress, HardwareTypeIdentifier hwType, uint16_t period, subscription_event_type_t eventType, seq_t callbackSequence) {
	subscription_helper_t subscriptionSet;
	memset(&subscriptionSet, 0, sizeof(subscriptionSet));

	subscriptionSet.address = l3.localAddress;
	subscriptionSet.hardwareAddress = hwAddress;
	subscriptionSet.hardwareType = hwType;
	subscriptionSet.millisecondsDelay = period;
	subscriptionSet.onEventType = eventType;
	subscriptionSet.sequence = callbackSequence;

	return generateSubscriptionSetGeneric(p, destination, &subscriptionSet);
}

seq_t PacketFactory::generateSubscriptionSetPeriodic(Layer3::packet_t* p, l3_address_t destination, l3_address_t localAddress, uint8_t hwAddress, HardwareTypeIdentifier hwType, uint16_t period, seq_t callbackSequence) {
	return generateSubscriptionSetGeneric(p, destination, localAddress, hwAddress, hwType, period, EVENT_TYPE_DISABLED, callbackSequence);
}

seq_t PacketFactory::generateSubscriptionSetEvent(Layer3::packet_t* p, l3_address_t destination, l3_address_t localAddress, uint8_t hwAddress, HardwareTypeIdentifier hwType, subscription_event_type_t eventType, seq_t callbackSequence) {
	return generateSubscriptionSetGeneric(p, destination, localAddress, hwAddress, hwType, 0, eventType, callbackSequence);
}

seq_t PacketFactory::generateSubscriptionDeletion(Layer3::packet_t* p, l3_address_t destination, subscription_info_t* subscription) {
	return generateSubscriptionSetGeneric(p, destination, subscription->forAddress, subscription->info.hardwareAddress, (HardwareTypeIdentifier) subscription->info.hardwareType, 0, EVENT_TYPE_DISABLED, 0);
}

seq_t PacketFactory::generateSubscriptionSetEventForDiscoveryInfo(Layer3::packet_t* p, l3_address_t destination, packet_application_numbered_discovery_info_helper_t* info, subscription_event_type_t eventType, seq_t callbackSeq) {
	if(info->canDetectEvents)
		return generateSubscriptionSetEvent(p, destination, l3.localAddress, info->hardwareAddress, (HardwareTypeIdentifier) info->hardwareType, eventType, callbackSeq);
	return false;
}

seq_t PacketFactory::generateSubscriptionSetPeriodicForDiscoveryInfo(Layer3::packet_t* p, l3_address_t destination, packet_application_numbered_discovery_info_helper_t* info, uint16_t delay, seq_t callbackSeq) {
	return generateSubscriptionSetPeriodic(p, destination, l3.localAddress, info->hardwareAddress, (HardwareTypeIdentifier) info->hardwareType, delay, callbackSeq);
}

seq_t PacketFactory::generateHardwareCommandRead(Layer3::packet_t* p, l3_address_t destination, uint8_t hwAddress, HardwareTypeIdentifier hwType) {
	HardwareCommandResult hwCmd = HardwareCommandResult();
	hwCmd.setAddress(hwAddress);
	hwCmd.setHardwareType(hwType);
	hwCmd.setReadRequest(1);

	return generateHardwareCommand(p, destination, &hwCmd);
}

seq_t PacketFactory::generateHardwareCommandReadForDiscoveryInfo(Layer3::packet_t* p, l3_address_t destination, packet_application_numbered_discovery_info_helper_t* info) {
	return generateHardwareCommandRead(p, destination, info->hardwareAddress, (HardwareTypeIdentifier) info->hardwareType);
}

seq_t PacketFactory::generateHardwareCommandWrite(Layer3::packet_t* p, l3_address_t destination, HardwareCommandResult* hwCmd) {
	hwCmd->setReadRequest(0);
	return generateHardwareCommand(p, destination, hwCmd);
}

seq_t PacketFactory::generateSubscriptionInfoRquest(Layer3::packet_t* p, l3_address_t destination) {
	//numbered
	packet_numbered_t numbered;
	preparePacketNumbered(&numbered);

	//app layer
	packet_application_numbered_cmd_t* appCmd = (packet_application_numbered_cmd_t*) numbered.payload;

	appCmd->packetType = HARDWARE_SUBSCRIPTION_INFO;
	subscription_info_t* subscriptionInfo = (subscription_info_t*) appCmd->payload;
	subscriptionInfo->forAddress = l3.localAddress;

	l3.createPacketGeneric(p, destination, PACKET_NUMBERED, &numbered, sizeof(packet_numbered_t));
	return numbered.seqNumber;
}

seq_t PacketFactory::generateDiscoveryInfoRequest(Layer3::packet_t* p, l3_address_t destination) {
	//numbered
	packet_numbered_t numbered;
	preparePacketNumbered(&numbered);

	//applayer
	packet_application_numbered_cmd_t* appCmd = (packet_application_numbered_cmd_t*) numbered.payload;
	appCmd->packetType = HARDWARE_DISCOVERY_REQ;

	packet_application_numbered_discovery_info_t* infoReq = (packet_application_numbered_discovery_info_t*) appCmd->payload;
	infoReq->numSensors = 0;

	l3.createPacketGeneric(p, destination, PACKET_NUMBERED, &numbered, sizeof(packet_numbered_t));
	return numbered.seqNumber;
}

void PacketFactory::preparePacketNumbered(packet_numbered_t* numbered) {
	memset(numbered, 0, sizeof(packet_numbered_t));
	numbered->seqNumber = random(1, 0xffff);
	numbered->payloadLen = sizeof(packet_application_numbered_cmd_t);
}

