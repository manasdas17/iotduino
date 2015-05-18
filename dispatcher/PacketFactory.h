/*
 * PacketFactoryInterface.h
 *
 * Created: 14.04.2015 20:39:58
 *  Author: helge
 */


#ifndef PACKETFACTORYINTERFACE_H_
#define PACKETFACTORYINTERFACE_H_

#include <utils/singleton.h>
#include <dispatcher/Commands.h>
#include <networking/Packets.h>
#include <drivers/HardwareID.h>

/**
 * singleton. class for creating application packets.
 * @author Helge
 */
class PacketFactory : public Singleton<PacketFactory> {
	private:
		Layer3* l3;

	public:
		/** uses the generic packet creation method from l3, mandatory.
		 * @param l3
		 */
		void setLayer3(Layer3* l3) {
			this->l3 = l3;
		}

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @return success
		 */
		boolean generateDiscoveryInfoRequest(Layer3::packet_t* p, l3_address_t destination) {
			if(l3 == NULL)
				return false;

			packet_application_numbered_cmd_t appCmd;
			memset(&appCmd, 0, sizeof(appCmd));

			appCmd.packetType = HARDWARE_DISCOVERY_REQ;

			return l3->createPacketGeneric(p, destination, PACKET_NUMBERED, (uint8_t*) &appCmd, sizeof(appCmd));
		}

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @return success
		 */
		boolean generateSubscriptionInfoRquest(Layer3::packet_t* p, l3_address_t destination) {
			if(l3 == NULL)
				return false;

			packet_application_numbered_cmd_t appCmd;
			memset(&appCmd, 0, sizeof(appCmd));

			appCmd.packetType = HARDWARE_SUBSCRIPTION_INFO;

			subscription_info_t* subscriptionInfo = (subscription_info_t*) appCmd.payload;
			subscriptionInfo->forAddress = l3->localAddress;

			return l3->createPacketGeneric(p, destination, PACKET_NUMBERED, (uint8_t*) &appCmd, sizeof(appCmd));
		}

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param hwCmd
		 * @return success
		 */
		boolean generateHardwareCommandWrite(Layer3::packet_t* p, l3_address_t destination, HardwareCommandResult* hwCmd) {
			hwCmd->setReadRequest(0);
			return generateHardwareCommand(p, destination, hwCmd);
		}

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param info from discovery service
		 * @return success
		 */
		boolean generateHardwareCommandReadForDiscoveryInfo(Layer3::packet_t* p, l3_address_t destination, packet_application_numbered_discovery_info_helper_t* info) {
			return generateHardwareCommandRead(p, destination, info->hardwareAddress, (HardwareTypeIdentifier) info->hardwareType);
		}

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param hwAddress
		 * @param hwType
		 * @return success
		 */
		boolean generateHardwareCommandRead(Layer3::packet_t* p, l3_address_t destination, uint8_t hwAddress, HardwareTypeIdentifier hwType) {
			HardwareCommandResult hwCmd = HardwareCommandResult();
			hwCmd.setAddress(hwAddress);
			hwCmd.setHardwareType(hwType);
			hwCmd.setReadRequest(1);

			return generateHardwareCommand(p, destination, &hwCmd);
		}

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param info from discovery serice
		 * @param delay in milliseconds
		 * @param callbackSeq for subscription executions
		 * @return success
		 */
		boolean generateSubscriptionSetPeriodicForDiscoveryInfo(Layer3::packet_t* p, l3_address_t destination, packet_application_numbered_discovery_info_helper_t* info, uint16_t delay, seq_t callbackSeq) {
			return generateSubscriptionSetPeriodic(p, destination, l3->localAddress, info->hardwareAddress, (HardwareTypeIdentifier) info->hardwareType, delay, callbackSeq);
		}

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param info from discovery serice
		 * @param delay in milliseconds
		 * @param eventType to be registered
		 * @return success
		 * @see subscription_event_type_t
		 */
		boolean generateSubscriptionSetEventForDiscoveryInfo(Layer3::packet_t* p, l3_address_t destination, packet_application_numbered_discovery_info_helper_t* info, subscription_event_type_t eventType, seq_t callbackSeq) {
			if(info->canDetectEvents)
				return generateSubscriptionSetEvent(p, destination, l3->localAddress, info->hardwareAddress, (HardwareTypeIdentifier) info->hardwareType, eventType, callbackSeq);
			return false;
		}

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param subscription info to delete
		 * @return success
		 */
		boolean generateSubscriptionDeletion(Layer3::packet_t* p, l3_address_t destination, subscription_info_t* subscription) {
			return generateSubscriptionSetGeneric(p, destination, subscription->forAddress, subscription->info.hardwareAddress, (HardwareTypeIdentifier) subscription->info.hardwareType, 0, EVENT_TYPE_DISABLED, 0);
		}

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
		boolean generateSubscriptionSetEvent(Layer3::packet_t* p, l3_address_t destination, l3_address_t localAddress, uint8_t hwAddress, HardwareTypeIdentifier hwType, subscription_event_type_t eventType, seq_t callbackSequence) {
			return generateSubscriptionSetGeneric(p, destination, localAddress, hwAddress, hwType, 0, eventType, callbackSequence);
		}

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
		boolean generateSubscriptionSetPeriodic(Layer3::packet_t* p, l3_address_t destination, l3_address_t localAddress, uint8_t hwAddress, HardwareTypeIdentifier hwType, uint16_t period, seq_t callbackSequence) {
			return generateSubscriptionSetGeneric(p, destination, localAddress, hwAddress, hwType, period, EVENT_TYPE_DISABLED, callbackSequence);
		}

	protected:
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
		boolean generateSubscriptionSetGeneric(Layer3::packet_t* p, l3_address_t destination, l3_address_t localAddress, uint8_t hwAddress, HardwareTypeIdentifier hwType, uint16_t period, subscription_event_type_t eventType, seq_t callbackSequence) {
			if(l3 == NULL)
				return false;

			packet_application_numbered_cmd_t appCmd;
			memset(&appCmd, 0, sizeof(appCmd));

			appCmd.packetType = HARDWARE_SUBSCRIPTION_SET;

			subscription_set_t* subscriptionSet = (subscription_set_t*) appCmd.payload;
			subscriptionSet->info.address = l3->localAddress;
			subscriptionSet->info.hardwareAddress = hwAddress;
			subscriptionSet->info.hardwareType = hwType;
			subscriptionSet->info.millisecondsDelay = period;
			subscriptionSet->info.onEventType = eventType;
			subscriptionSet->info.sequence = callbackSequence;

			return l3->createPacketGeneric(p, destination, PACKET_NUMBERED, (uint8_t*) &appCmd, sizeof(appCmd));
		}

		/**
		 * @param p l3 packet where data is returned
		 * @param destination l3 address
		 * @param command
		 * @return success
		 */
		boolean generateHardwareCommand(Layer3::packet_t* p, l3_address_t destination, HardwareCommandResult* command) {
			if(l3 == NULL)
			return false;

			packet_application_numbered_cmd_t appCmd;
			memset(&appCmd, 0, sizeof(appCmd));

			//write data into app packet.
			command->serialize((command_t*) appCmd.payload);

			appCmd.packetType = (!command->isReadRequest()) ? HARDWARE_COMMAND_WRITE : HARDWARE_COMMAND_READ;

			l3->createPacketGeneric(p, destination, PACKET_NUMBERED, (uint8_t*) &appCmd, sizeof(appCmd));
		}
};

#endif /* PACKETFACTORYINTERFACE_H_ */