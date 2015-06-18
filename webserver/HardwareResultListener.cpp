/*
 * HardwareResultListener.cpp
 *
 * Created: 14.06.2015 23:30:45
 *  Author: helge
 */
#include <webserver/HardwareResultListener.h>

void hardwareRequestListener::init(l3_address_t remote, HardwareTypeIdentifier hwtype, uint8_t hwaddress, webserverListener::STATE state) {
	this->remote = remote;
	this->state = state;
	this->hwtype = hwtype;
	this->hwaddress = hwaddress;

	memset(&cmd, 0, sizeof(cmd));
}

void hardwareRequestListener::doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq) {
	if(address != remote || appLayerPacket == NULL || appLayerPacket->packetType != HARDWARE_COMMAND_RES)
		return;

		command_t* info = (command_t*) appLayerPacket->payload;

		this->cmd = *info;

		state = FINISHED;
}

void hardwareRequestListener::fail(seq_t seq, l3_address_t remote) {
	state = FAILED;
}
