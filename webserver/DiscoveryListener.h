/*
 * DiscoveryListener.h
 *
 * Created: 11.06.2015 21:24:18
 *  Author: helge
 */


#ifndef DISCOVERYLISTENER_H_
#define DISCOVERYLISTENER_H_

#include <webserver/WebserverListener.h>

/**
 * listener class for discovery results
 */
class discoveryListener : public webserverListener {
	public:

	/** # infos */
	int8_t totalInfos;
	/** # infos we have received yet */
	uint8_t gottenInfos;
	/** remote */
	l3_address_t remote;

	/** actual infos */
	packet_application_numbered_discovery_info_helper_t sensorInfos[INTERFACES_BUF_SIZE];

	/**
	 * object init
	 * @param remote
	 */
	void init(l3_address_t remote, webserverListener::STATE state) {
		this->state = state;
		this->totalInfos = -1;
		this->gottenInfos = 0;
		this->remote = remote;
		memset(sensorInfos, 0, sizeof(sensorInfos));
	}

	/**
	 * callback on response - store info and change state to finished
	 * @param appLayerPacket
	 * @param address remote
	 * @param seq
	 */
	virtual void doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq) {
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

	/**
	 * update state
	 * @param seq
	 * @param remote
	 */
	virtual void fail(seq_t seq, l3_address_t remote) {
		state = FAILED;
	}
};


#endif /* DISCOVERYLISTENER_H_ */