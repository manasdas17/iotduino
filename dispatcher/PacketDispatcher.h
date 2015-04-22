/* 
* PacketDispatcher.h
*
* Created: 14.04.2015 23:24:41
* Author: helge
*/


#ifndef __PACKETDISPATCHER_H__
#define __PACKETDISPATCHER_H__

#include "../networking/Packets.h"
#include "ResponseHandler.h"
#include "CommandHandler.h"

class PacketDispatcher {
	//variables
	public:
	protected:
	private:
		CommandHandler commandHandler;
		ResponseHandler responseHandler;
		Layer3* networking;

	//functions
	public:
		PacketDispatcher(Layer3* networking, HardwareInterface* hwinterface) {
			this->networking = networking;
			this->commandHandler.setHardwareInterface(hwinterface);
		}
		
		~PacketDispatcher() {
			
		}
		
		void loop() {
			#ifdef DEBUG_HANDLER_ENABLE
				Serial.print(millis());
				Serial.println(F("PacketDispatcher::loop()"));
				Serial.flush();
			#endif
	
			while(l3->receiveQueueSize() > 0) {
				Layer3::packet_t packet;
				l3->receiveQueuePop(&packet);

				#ifdef DEBUG_HANDLER_ENABLE
					Serial.print(millis());
					Serial.print(F("\treceived packetType="));
					Serial.println(packet.data.type);
					Serial.flush();
				#endif
		
				switch(packet.data.type) {
					case PACKET_NUMBERED:
							handleNumberedFromNetwork(packet);
						break;
					case PACKET_UNNUMBERED:
						break;
					case PACKET_ACK:
					case PACKET_BEACON:
					default:
						continue;
				}
			}
			
		}

		boolean handleNumberedFromNetwork( Layer3::packet_t packet ) {
			packet_numbered_t* numbered = (packet_numbered_t*) packet.data.payload;
			packet_application_numbered_cmd_t* appPacket = (packet_application_numbered_cmd_t*) numbered->payload;
			
			packet_type_application type = (packet_type_application) appPacket->packetType;
			seq_t seq = numbered->seqNumber;
			l3_address_t remote = packet.data.source;
			
			return handleNumbered(seq, type, remote, appPacket);
		}

		boolean handleNumbered( seq_t seq, packet_type_application type, l3_address_t remote, packet_application_numbered_cmd_t* appPacket) {
			switch(type) {
				case HARDWARE_COMMAND_READ:
				case HARDWARE_COMMAND_WRITE:
					return commandHandler.handleNumbered(l3->getCallbackInterface(), seq, type, remote, appPacket);
				
				case ACK:
					return responseHandler.handleReponseNumbered(seq, type, remote, appPacket);
				
				case HARDWARE_DISCOVERY_REQ:
				case HARDWARE_DISCOVERY_RES:
				default:
					return false;
			}
		}


}; //PacketDispatcher

#endif //__PACKETDISPATCHER_H__
