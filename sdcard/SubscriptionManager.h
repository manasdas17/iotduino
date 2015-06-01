/*
* SubscriptionManager.h
*
* Created: 31.05.2015 23:24:59
* Author: helge
*/


#ifndef __SUBSCRIPTIONMANAGER_H__
#define __SUBSCRIPTIONMANAGER_H__

#include <Arduino.h>
#include <dispatcher/EventCallbackInterface.h>
#include <dispatcher/PacketDispatcher.h>
#include <dispatcher/PacketFactory.h>
#include <networking/Layer3.h>

extern PacketDispatcher dispatcher;
extern PacketFactory pf;
extern Layer3 l3;


#define MAX_ACTIVE_SUBSCRIPTIONS 50
#define SUBSCRIPTION_REFRESH_MILLIS (5*60*1000UL) //every 5 minutes
#define SUBSCRIPTION_DEFAULT_PERIOD_MILLIS (60*1000UL)
#define SUBSCRIPTION_REQUEST_DELAY_MILLIS (100)
#define SUBSCRIPTION_TIMEOUT_MILLIS (15*60*1000UL) //timeout after 15 minutes

#define DISCOVERY_REQUEST_DELAY_MILLIS (1000)
#define DISCOVERY_REQUEST_PERIOD_MILLIS (15*60*1000UL) //every 15 minutes

#define REGISTER_FOR_HWTYPES_NUM 17
const HardwareTypeIdentifier REGISTER_FOR_HWTYPES[] = {
	HWType_accelerometer,
	HWType_dcf77,
	HWType_gyroscope,
	HWType_humidity,
	HWType_hwSwitch,
	HWType_ir,
	HWType_keypad,
	HWTYPE_led,
	HWType_light,
	HWType_magneticField,
	HWType_methane,
	HWType_motion,
	HWType_pressure,
	HWType_relay,
	HWType_sonar,
	HWType_temprature,
	HWType_touchpad};

class SubscriptionManager {
	class SubscriptionManagerDiscoveryCallback : public EventCallbackInterface {
		/** parent */
		SubscriptionManager* parent;

		public:
		/**
		 * init
		 * @param parent
		 */
		void init (SubscriptionManager* parent) {
			this->parent = parent;
		};

		/**
		 * callback for HARDWARE_DISCOVERY_RES
		 * @param appLayerPacket
		 * @param address
		 * @param seq
		 */
		void doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq) {
			if(appLayerPacket == NULL || appLayerPacket->packetType != HARDWARE_DISCOVERY_RES)
				return;
			packet_application_numbered_discovery_info_t* discoveryInfo = (packet_application_numbered_discovery_info_t*) appLayerPacket->payload;

			for(uint8_t i = 0; i < discoveryInfo->numSensors; i++) {
				if(parent->registerForType((HardwareTypeIdentifier) discoveryInfo->infos[i].hardwareType)) {
					//did we already register?
					uint8_t index = parent->getSubscriptionSlot(address, discoveryInfo->infos[i].hardwareAddress, (HardwareTypeIdentifier) discoveryInfo->infos[i].hardwareType);
					if(index == -1) {
						//nope, do it with next refresh run.
						memcpy(&parent->activeSubscriptions[index], &discoveryInfo->infos[i], sizeof(packet_application_numbered_discovery_info_helper_t));
						parent->lastRefresh[index] = 0;
					}
				}
			}
		}
	};

	class SubscriptionManagerSubscriptionCallback : public EventCallbackInterface {
		/** parent */
		SubscriptionManager* parent;

		public:
		/**
		 * init
		 * @param parent
		 */
		void init(SubscriptionManager* parent) {
			this->parent = parent;
		};

		/**
		 * callback for HARDWARE_SUBSCRIPTION_SET_RES
		 * @param appLayerPacket
		 * @param address
		 * @param seq
		 */
		void doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq) {
			if(appLayerPacket->packetType != HARDWARE_SUBSCRIPTION_SET_RES)
				return;

			//may be unsafe, we only have info about ACK
			subscription_set_t* tmp = (subscription_set_t*) appLayerPacket->payload;
			subscription_helper_t* subscriptionInfo = (subscription_helper_t*) &tmp->info;

			#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				Serial.print(millis());
				Serial.println(F(": SubscriptionManager::SubscriptionManagerSubscriptionCallback::doCallback() HARDWARE_SUBSCRIPTION_SET_RES"));
				Serial.print(F("\tremote="));
				Serial.print(address);
				Serial.print(F(" hwtype="));
				Serial.print(subscriptionInfo->hardwareType);
				Serial.print(F(" hwaddress="));
				Serial.println(subscriptionInfo->hardwareAddress);
				Serial.flush();
			#endif

			parent->saveSubscription(subscriptionInfo);
		}
	};

	/** callback handler for subscriptions */
	SubscriptionManagerSubscriptionCallback subscriptionCallback;
	/** callback handler for discovery */
	SubscriptionManagerDiscoveryCallback discoveryCallback;


	/** last subscription request that has been sent */
	uint32_t lastSubscriptionRequest;
	/** last discovery request that has been sent */
	uint32_t lastDiscoveryRequest;
	/** next neighbourtable index to discover */
	uint8_t nextDiscoveryRequestNeighbourIndex;

	public:
		/** active subscriptions */
		subscription_helper_t activeSubscriptions[MAX_ACTIVE_SUBSCRIPTIONS]; //each 11b -> 550b
		/** last refresh of this subscription */
		uint32_t lastRefresh[MAX_ACTIVE_SUBSCRIPTIONS]; //each 4b -> 200b

		/**
		 * send a subscription request
		 * <code>period=SUBSCRIPTION_DEFAULT_PERIOD_MILLIS</code>
		 * <code>event=EVENT_TYPE_CHANGE</code>
		 * @param destination
		 * @param hwAddress
		 * @param hwType
		 */
		boolean sendSubscriptionRequest(l3_address_t destination, uint8_t hwAddress, HardwareTypeIdentifier hwType) {
			#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				Serial.print(millis());
				Serial.print(F(": SubscriptionManager::sendSubscriptionRequest() destination="));
				Serial.print(destination);
				Serial.print(F(" hwType="));
				Serial.print(hwType);
				Serial.print(F(" hwAddress="));
				Serial.println(hwAddress);
				Serial.flush();
			#endif

			Layer3::packet_t p;
			memset(&p, 0, sizeof(p));
			seq_t seq = random(1, 0xffff);
			pf.generateSubscriptionSetGeneric(&p, destination, l3.localAddress, hwAddress, hwType, SUBSCRIPTION_DEFAULT_PERIOD_MILLIS, EVENT_TYPE_CHANGE, seq);
			return l3.sendPacket(p);
		}

		/**
		 * send a discovery request to a node
		 * @param node
		 * @return success
		 */
		boolean sendDiscoveryRequest(l3_address_t node) {
			#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				Serial.print(millis());
				Serial.print(F(": SubscriptionManager::sendDiscoveryRequest() node="));
				Serial.println(node);
				Serial.flush();
			#endif

			Layer3::packet_t p;
			memset(&p, 0, sizeof(p));
			//seq_t seq = pf.generateDiscoveryInfoRequest(&p, node);
			pf.generateDiscoveryInfoRequest(&p, node);
			return l3.sendPacket(p);
		}

		/**
		 * initialise memory.
		 */
		void init() {
			memset(&activeSubscriptions, 0, sizeof(activeSubscriptions));
			memset(&lastRefresh, 0, sizeof(lastRefresh));

			subscriptionCallback.init(this);
			discoveryCallback.init(this);

			//register listeners by type.
			dispatcher.getResponseHandler()->registerListenerByPacketType(0, HARDWARE_SUBSCRIPTION_SET_RES, 0, &subscriptionCallback);
			dispatcher.getResponseHandler()->registerListenerByPacketType(0, HARDWARE_DISCOVERY_RES, 0, &discoveryCallback);

			lastSubscriptionRequest = 0;
			lastDiscoveryRequest = 0;
			nextDiscoveryRequestNeighbourIndex = 0;
		}

		/**
		 * maintenance loop.
		 */
		void loop() {
			maintainSubscriptions();
			maintainDiscoveries();
		}

		/**
		 * send a subscription request at max one per function call and after the delay SUBSCRIPTION_REQUEST_DELAY_MILLIS
		 * furthermore, subscriptions are updated after the last refresh is REFRESH_MILLIS ago.
		 */
		void maintainSubscriptions() {
			uint32_t now = millis();
			//delay the sending.
			if(now > SUBSCRIPTION_REQUEST_DELAY_MILLIS && now - SUBSCRIPTION_REQUEST_DELAY_MILLIS > lastSubscriptionRequest) {
				//non blocking, we only send one request per loop call
				for(uint8_t i = 0; i < MAX_ACTIVE_SUBSCRIPTIONS; i++) {
					if(activeSubscriptions[i].address > 0) {
						//timed out?
						if(now - lastRefresh[i] > SUBSCRIPTION_TIMEOUT_MILLIS) {
							deleteSubscription(i);
							continue;
						}

						//need to refresh?
						if(now - lastRefresh[i] > SUBSCRIPTION_REFRESH_MILLIS) {
							//refresh.
							sendSubscriptionRequest(
								activeSubscriptions[i].address,
								activeSubscriptions[i].hardwareAddress,
								(HardwareTypeIdentifier) activeSubscriptions[i].hardwareType);

							//set timestamp
							lastSubscriptionRequest = now;
							//break loop as we have sent a request
							break;
						}
					}
				}
			}
		}

		/**
		 * trigger discovery.
		 */
		void maintainDiscoveries() {
			uint32_t now = millis();
			//delay the sending.
			if(now > DISCOVERY_REQUEST_DELAY_MILLIS && now - DISCOVERY_REQUEST_DELAY_MILLIS > lastDiscoveryRequest) {
				neighbourData* neighbours = l3.getNeighbours();
				if(neighbours == NULL)
					return;

				//we have a node, send request.
				if(neighbours[nextDiscoveryRequestNeighbourIndex].nodeId > 0) {
					sendDiscoveryRequest(neighbours[nextDiscoveryRequestNeighbourIndex].nodeId);
				}

				//iterate
				nextDiscoveryRequestNeighbourIndex++;
				nextDiscoveryRequestNeighbourIndex %= CONFIG_L3_NUM_NEIGHBOURS;
			}
		}

		/**
		 * clean subscription
		 * @param index
		 */
		void deleteSubscription(uint8_t index) {
			if(index > 0 && index < MAX_ACTIVE_SUBSCRIPTIONS) {
				#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
					Serial.print(millis());
					Serial.print(F(": SubscriptionManager::deleteSubscription() index="));
					Serial.println(index);
					Serial.flush();
				#endif

				memset(&activeSubscriptions[index], 0, sizeof(subscription_helper_t));
				lastRefresh[index] = 0;
			}
		}

		/**
		 * get free subscription slot
		 * @return index
		 */
		int8_t getSubscriptionSlot() {
			for(uint8_t i = 0; i < MAX_ACTIVE_SUBSCRIPTIONS; i++) {
				if(activeSubscriptions[i].address == 0)
				return i;
			}

			return -1;
		}

		/**
		 * get existing subscription index
		 * @param subscription
		 * @return index
		 */
		int8_t getSubscriptionSlot(l3_address_t address, uint8_t hardwareAddress, HardwareTypeIdentifier hardwareType) {
			for(uint8_t i = 0; i < MAX_ACTIVE_SUBSCRIPTIONS; i++) {
				//check ident subscription via nodeAddress, hwAddress and hwType
				if(activeSubscriptions[i].address == address && activeSubscriptions[i].hardwareAddress == hardwareAddress && activeSubscriptions[i].hardwareType == hardwareType) {
					return i;
				}
			}

			return -1;
		}

		/**
		 * register active subscription and update timestamp
		 * @param subscriptionInfo
		 * @return success
		 */
		boolean saveSubscription(subscription_helper_t* subscriptionInfo) {
			//sanity check
			if(subscriptionInfo == NULL)
				return false;
			//valid subscription?
			else if(subscriptionInfo->millisecondsDelay > 0 && subscriptionInfo->onEventType != EVENT_TYPE_DISABLED)
				return false;

			#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				Serial.print(millis());
				Serial.print(F(": SubscriptionManager::saveSubscription() info=[forNode="));
				Serial.print(subscriptionInfo->address);
				Serial.print(F(" hwAddress="));
				Serial.print(subscriptionInfo->hardwareAddress);
				Serial.print(F(" hwType="));
				Serial.print(subscriptionInfo->hardwareType);
				Serial.print(F(" delay="));
				Serial.print(subscriptionInfo->millisecondsDelay);
				Serial.print(F(" event="));
				Serial.print(subscriptionInfo->onEventType);
				Serial.print(F(" seq="));
				Serial.print(subscriptionInfo->sequence);
				Serial.println(F("]"));
				Serial.flush();
			#endif

			//get existing slot
			int8_t index = getSubscriptionSlot(subscriptionInfo->address, subscriptionInfo->hardwareAddress, (HardwareTypeIdentifier) subscriptionInfo->hardwareType);

			//get new slot
			if(index == -1 )
				index = getSubscriptionSlot();

			//failed.
			if(index == -1)
				return false;

			//save
			lastRefresh[index] = millis();
			memcpy(&activeSubscriptions[index], subscriptionInfo, sizeof(subscription_helper_t));

			#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				Serial.print(F("\tsaved in slot="));
				Serial.print(index);
				Serial.print(F(" timestamp="));
				Serial.println(lastRefresh[index]);
				Serial.flush();
			#endif

			return true;
		}

		/**
		 * determine whether we want to subscribe to this hardware type
		 * @param type
		 * @return register
		 */
		boolean registerForType(HardwareTypeIdentifier type) {
			#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				Serial.print(millis());
				Serial.print(F(": SubscriptionManager::registerForType() type="));
				Serial.print(type);
				Serial.flush();
			#endif

			for(uint8_t i = 0; i < REGISTER_FOR_HWTYPES_NUM; i++) {
				if(REGISTER_FOR_HWTYPES[i] == type) {
					#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
						Serial.println(F(" yes."));
					#endif
					return true;
				}
				#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				else
					Serial.println(F(" nope."));
				#endif
			}

			return false;
		}
}; //SubscriptionManager

#endif //__SUBSCRIPTIONMANAGER_H__
