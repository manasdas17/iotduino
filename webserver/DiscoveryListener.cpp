/*
 * DiscoveryListener.cpp
 *
 * Created: 14.06.2015 23:30:12
 *  Author: helge
 */
#include <webserver/DiscoveryListener.h>
#include <dispatcher/DiscoveryService.h>

void discoveryListener::init(l3_address_t remote, webserverListener::STATE state) {
	this->state = state;
	this->totalInfos = -1;
	this->gottenInfos = 0;
	this->remote = remote;
	memset(this->sensorInfos, 0, sizeof(this->sensorInfos));
}

void discoveryListener::doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq) {
	if(address != remote || appLayerPacket == NULL || appLayerPacket->packetType != HARDWARE_DISCOVERY_RES)
		return;

	packet_application_numbered_discovery_info_t* info = (packet_application_numbered_discovery_info_t*) appLayerPacket->payload;
	//how many infos do we expect?
	if(totalInfos == -1) {
		if(info->numTotalSensors > INTERFACES_BUF_SIZE) {
			state = FAILED;
			return;
		}

		state = AWAITING_ANSWER;
		totalInfos = info->numTotalSensors;
	}

	//copy
	memcpy(&sensorInfos[gottenInfos], info->infos, info->numSensors * sizeof(packet_application_numbered_discovery_info_helper_t));
	gottenInfos += info->numSensors;

	if(gottenInfos == totalInfos) {
		state = FINISHED;
	}
}

void discoveryListener::fail(seq_t seq, l3_address_t remote) {
	state = FAILED;
}
