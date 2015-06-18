/*
 * DiscoveryListener.h
 *
 * Created: 11.06.2015 21:24:18
 *  Author: helge
 */


#ifndef DISCOVERYLISTENER_H_
#define DISCOVERYLISTENER_H_

#include <webserver/WebserverListener.h>
#include <dispatcher/DiscoveryService.h>

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
	void init(l3_address_t remote, webserverListener::STATE state);

	/**
	 * callback on response - store info and change state to finished
	 * @param appLayerPacket
	 * @param address remote
	 * @param seq
	 */
	virtual void doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq);

	/**
	 * update state
	 * @param seq
	 * @param remote
	 */
	virtual void fail(seq_t seq, l3_address_t remote);
};


#endif /* DISCOVERYLISTENER_H_ */