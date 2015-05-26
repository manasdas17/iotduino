#include "CommandHandler.h"

void CommandHandler::setHardwareInterface(HardwareInterface* hwinterface) {
	this->hardwareInterface = hwinterface;
}

void CommandHandler::handleUnnumbered(EventCallbackInterface* callback, Layer3::packet_t packet) {

}

boolean CommandHandler::handleHardwareCommand(packet_application_numbered_cmd_t* appPacket, EventCallbackInterface* callback, const l3_address_t remote, const seq_t seq) {
	if(appPacket == NULL)
		return false;

	HardwareCommandResult cmd = HardwareCommandResult(); //todo, we use such kind of object for returning results anyway, however this takes place in the hwinterface
	cmd.deSerialize((command_t*) appPacket->payload);

	#ifdef DEBUG_HANDLER_ENABLE
		Serial.print(F("\tcmd=["));
		Serial.print(F("hwAdrress="));
		Serial.print(cmd.getAddress());
		Serial.print(F(" hwType="));
		Serial.print(cmd.getHardwareType());
		Serial.print(F(" isRead="));
		Serial.print(cmd.isReadRequest());
		Serial.println(F("]"));
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

	packet_application_numbered_cmd_t appLayerPacket;
	memset(&appLayerPacket, 0, sizeof(appLayerPacket));

	if(result == NULL) {
		appLayerPacket.packetType = NACK;

		if(callback != NULL)
			callback->doCallback(&appLayerPacket, remote, seq);
		return false;
	}

	//create response
	appLayerPacket.packetType = ACK;
	result->serialize((command_t*) appLayerPacket.payload);

	if(callback == NULL)
		return false;

	callback->doCallback(&appLayerPacket, remote, seq);

	return true;
}

boolean CommandHandler::handleNumbered(EventCallbackInterface* callback, const seq_t seq, const packet_type_application_t type, const l3_address_t remote, packet_application_numbered_cmd_t* appPacket) {
	#ifdef DEBUG_HANDLER_ENABLE
		Serial.print(millis());
		Serial.print(F(": CommandHandler::handleNumbered() from="));
		Serial.print(remote);
		Serial.print(F(" type="));
		Serial.print(type);
		Serial.print(F(" seq="));
		Serial.println(seq);
	#endif

	switch(type) {
		case HARDWARE_COMMAND_READ:
		case HARDWARE_COMMAND_WRITE:
			return handleHardwareCommand(appPacket, callback, remote, seq);
			break;

		default:
			return false;
	}
}

