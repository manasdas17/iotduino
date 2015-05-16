/*
* CommandHandler.h
*
* Created: 04.03.2015 22:32:52
* Author: helge
*/


#ifndef __COMMANDHANDLER_H__
#define __COMMANDHANDLER_H__

#include <dispatcher/HardwareInterface.h>
#include <networking/Layer3.h>

class CommandHandler {
	private:
		HardwareInterface* hardwareInterface;

	protected:

	public:
		/**
		 * handle numbered packet
		 * by now only for hardware requests
		 * @param packet
		 */
		boolean handleNumbered( EventCallbackInterface* callback, const seq_t seq, const packet_type_application_t type, const l3_address_t remote, packet_application_numbered_cmd_t* appPacket );

		/**
		 * handle hardware command payload.
		 */
		boolean handleHardwareCommand(packet_application_numbered_cmd_t* appPacket, EventCallbackInterface* callback, const l3_address_t remote, const seq_t seq);

		/**
		 * handle UNnumbered packet
		 * @param packet
		 */
		void handleUnnumbered( EventCallbackInterface* callback, Layer3::packet_t packet );

		void setHardwareInterface(HardwareInterface* hwinterface);
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
