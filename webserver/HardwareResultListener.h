/*
 * HardwareResultListener.h
 *
 * Created: 11.06.2015 21:24:43
 *  Author: helge
 */


#ifndef HARDWARERESULTLISTENER_H_
#define HARDWARERESULTLISTENER_H_

#include <webserver/WebserverListener.h>
#include <drivers/HardwareID.h>
/**
 * listener class for hardware results
 */
class hardwareRequestListener : public webserverListener {
	public:
	/** storage for cmd */
	command_t cmd;

	/** remote address */
	l3_address_t remote;
	/** hw type */
	HardwareTypeIdentifier hwtype;
	/** hw address */
	uint8_t hwaddress;

	/**
	 * initialise object
	 * @param remote
	 * @param hwtype
	 * @param hwaddress
	 */
	void init (l3_address_t remote, HardwareTypeIdentifier hwtype, uint8_t hwaddress, webserverListener::STATE state) {
		this->remote = remote;
		this->state = state;
		this->hwtype = hwtype;
		this->hwaddress = hwaddress;

		memset(&cmd, 0, sizeof(cmd));
	}

	/**
	 * callback on response - store info and change state to finished
	 * @param appLayerPacket
	 * @param address remote
	 * @param seq
	 */
	virtual void doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq) {
		if(address != remote || appLayerPacket == NULL || appLayerPacket->packetType != HARDWARE_COMMAND_RES)
			return;

		command_t* info = (command_t*) appLayerPacket->payload;

		this->cmd = *info;

		state = FINISHED;
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

#endif /* HARDWARERESULTLISTENER_H_ */