/*
 * PacketFactoryInterface.h
 *
 * Created: 14.04.2015 20:39:58
 *  Author: helge
 */ 


#ifndef PACKETFACTORYINTERFACE_H_
#define PACKETFACTORYINTERFACE_H_

#include "Commands.h"
#include "../networking/Packets.h"
#include <HardwareID.h>

class PacketFactory {
	boolean generatePacket(Layer3::packet_t* packet, HardwareCommand* cmd)
};

#endif /* PACKETFACTORYINTERFACE_H_ */