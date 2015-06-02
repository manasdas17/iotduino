/*
 * PacketFactoryInterface.h
 *
 * Created: 14.04.2015 20:39:58
 *  Author: helge
 */


#ifndef PACKETFACTORYINTERFACE_H_
#define PACKETFACTORYINTERFACE_H_

#include <dispatcher/Commands.h>
#include <networking/Packets.h>
#include <drivers/HardwareID.h>
#include <networking/Layer3.h>

extern Layer3 l3;
/**
 * singleton. class for creating application packets.
 * @author Helge
 */
class PacketFactory {

	public:
		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @return success
		 */
		seq_t generateDiscoveryInfoRequest(Layer3::packet_t* p, l3_address_t destination);

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @return success
		 */
		seq_t generateSubscriptionInfoRquest(Layer3::packet_t* p, l3_address_t destination);

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param hwCmd
		 * @return success
		 */
		seq_t generateHardwareCommandWrite(Layer3::packet_t* p, l3_address_t destination, HardwareCommandResult* hwCmd);

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param info from discovery service
		 * @return success
		 */
		seq_t generateHardwareCommandReadForDiscoveryInfo(Layer3::packet_t* p, l3_address_t destination, packet_application_numbered_discovery_info_helper_t* info);

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param hwAddress
		 * @param hwType
		 * @return success
		 */
		seq_t generateHardwareCommandRead(Layer3::packet_t* p, l3_address_t destination, uint8_t hwAddress, HardwareTypeIdentifier hwType);

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param info from discovery serice
		 * @param delay in milliseconds
		 * @param callbackSeq for subscription executions
		 * @return success
		 */
		seq_t generateSubscriptionSetPeriodicForDiscoveryInfo(Layer3::packet_t* p, l3_address_t destination, packet_application_numbered_discovery_info_helper_t* info, uint16_t delay, seq_t callbackSeq);

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param info from discovery serice
		 * @param delay in milliseconds
		 * @param eventType to be registered
		 * @return success
		 * @see subscription_event_type_t
		 */
		seq_t generateSubscriptionSetEventForDiscoveryInfo(Layer3::packet_t* p, l3_address_t destination, packet_application_numbered_discovery_info_helper_t* info, subscription_event_type_t eventType, seq_t callbackSeq);

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param subscription info to delete
		 * @return success
		 */
		seq_t generateSubscriptionDeletion(Layer3::packet_t* p, l3_address_t destination, subscription_info_t* subscription);

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param localAddress is where the subscription infos are sent to, usually our own.
		 * @param hwAddress
		 * @param hwType
		 * @param eventType
		 * @param callbackSeq
		 * @return success
		 */
		seq_t generateSubscriptionSetEvent(Layer3::packet_t* p, l3_address_t destination, l3_address_t localAddress, uint8_t hwAddress, HardwareTypeIdentifier hwType, subscription_event_type_t eventType, seq_t callbackSequence);

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param localAddress is where the subscription infos are sent to, usually our own.
		 * @param hwAddress
		 * @param hwType
		 * @param period delay in milliseconds
		 * @param callbackSeq
		 * @return success
		 */
		seq_t generateSubscriptionSetPeriodic(Layer3::packet_t* p, l3_address_t destination, l3_address_t localAddress, uint8_t hwAddress, HardwareTypeIdentifier hwType, uint16_t period, seq_t callbackSequence);

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param localAddress
		 * @param hwAddress
		 * @param hwType
		 * @param period
		 * @param eventType
		 * @param callbackSeq
		 * @return success
		 */
		seq_t generateSubscriptionSetGeneric(Layer3::packet_t* p, l3_address_t destination, l3_address_t localAddress, uint8_t hwAddress, HardwareTypeIdentifier hwType, uint16_t period, subscription_event_type_t eventType, seq_t callbackSequence);

		/**
		 * generate subscription requerst based on subscription_helper_t object
		 * @param packet to write in
		 * @param destination
		 * @param subscription
		 */
		seq_t generateSubscriptionSetGeneric(Layer3::packet_t* p, l3_address_t destination, subscription_helper_t* subscription);

	protected:
		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param command
		 * @return success
		 */
		seq_t generateHardwareCommand(Layer3::packet_t* p, l3_address_t destination, HardwareCommandResult* command);

		/** prepape packet */
		void preparePacketNumbered(packet_numbered_t* numbered);
};

#endif /* PACKETFACTORYINTERFACE_H_ */