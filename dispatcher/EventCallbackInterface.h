/*
 * EventCallbackInterface.h
 *
 * Created: 06.04.2015 18:56:48
 *  Author: helge
 */


#ifndef EVENTCALLBACKINTERFACE_H_
#define EVENTCALLBACKINTERFACE_H_

#include <dispatcher/Commands.h>
#include <networking/Packets.h>

class EventCallbackInterface {
		public:
			EventCallbackInterface() {
			}

			virtual ~EventCallbackInterface() {
			}

			virtual void doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq) {

			}

			virtual void fail(seq_t seq, l3_address_t remote) {

			}
};


#endif /* EVENTCALLBACKINTERFACE_H_ */