/* 
* CommandHandler.h
*
* Created: 04.03.2015 22:32:52
* Author: helge
*/


#ifndef __COMMANDHANDLER_H__
#define __COMMANDHANDLER_H__

#include "HardwareInterface.h"
#include "../networking/Layer3.h"
#include "../networking/LayerConfig.h"
#include "../networking/Packets.h"
#include "EventCallbackInterface.h"

class CommandHandler {
	private:
		HardwareInterface* hardwareInterface;
	
	protected:
		
	public:
		/**
		 * handle numbered packet
		 * @param packet
		 */
		boolean handleNumbered( EventCallbackInterface* callback, seq_t seq, packet_type_application type, l3_address_t remote, packet_application_numbered_cmd_t* appPacket ) {
			
			if(type != HARDWARE_COMMAND_READ && type != HARDWARE_COMMAND_WRITE)
				return false;

			//this must be a hwcommand			
			HardwareCommandResult cmd = HardwareCommandResult();
			cmd.deSerialize((command_t*) appPacket->payload);

			#ifdef DEBUG_HANDLER_ENABLE
				Serial.print(millis());
				Serial.print(F(": CommandHandler::handleNumbered() from="));
				Serial.print(remote);
				Serial.print(F(" type="));
				Serial.println(type);
				Serial.print(F(" seq="));
				Serial.println(seq);
				Serial.print(F("\tcmd=["));
				Serial.print(F("hwAdrress="));
				Serial.print(cmd.getAddress());
				Serial.print(F(" hwType="));
				Serial.print(cmd.getHardwareType());
				Serial.print(F("isRead="));
				Serial.print(cmd.isReadRequest());
				Serial.println(F("\t]"));
				Serial.flush();
			#endif

			HardwareCommandResult* result = NULL;
			result = hardwareInterface->executeCommand(&cmd);
			#ifdef DEBUG_HANDLER_ENABLE
				Serial.print(millis());
				Serial.print(F(": &result="));
				Serial.print((uint16_t) result, HEX);
				Serial.println();
				Serial.flush();
			#endif

			//create response
			packet_application_numbered_cmd_t appLayerPacket;
			appLayerPacket.packetType = ACK;
			result->serialize((command_t*) appLayerPacket.payload);

			callback->doCallback(&appLayerPacket, remote, seq);
			hardwareInterface->releaseHardwareCommandResultEntry(result);
			
			return true;
		}

		/**
		 * handle UNnumbered packet
		 * @param packet
		 */
		void handleUnnumbered( EventCallbackInterface* callback, Layer3::packet_t packet ) {
			
		}

		void setHardwareInterface(HardwareInterface* hwinterface) {
			this->hardwareInterface = hwinterface;
		}
		/**
		 * constructor
		 */
		CommandHandler() {
			
		}
		
		/**
		 * generic destructor
		 */
		~CommandHandler() {
			
		}

}; //CommandHandler

#endif //__COMMANDHANDLER_H__
