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
/** delay for subscription events */
#define SUBSCRIPTION_DEFAULT_PERIOD_MILLIS (60*1000UL)

/** delay for refreshing subscriptions */
#define SUBSCRIPTION_REFRESH_MILLIS (1*40*1000UL) //every 5 minutes
/** delay for sending subscription requests */
#define SUBSCRIPTION_REQUEST_DELAY_MILLIS (100)
/** delay after a subscription has timed out */
#define SUBSCRIPTION_TIMEOUT_MILLIS (1*60*1000UL) //timeout after 15 minutes

/** delay between discovery requests */
#define DISCOVERY_REQUEST_DELAY_MILLIS (1000)
/** delay between scanning neighbourtable and trigger discovery */
#define DISCOVERY_REQUEST_PERIOD_MILLIS (1*20*1000UL) //every 15 minutes

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
					parent->createSubscription(address, discoveryInfo->infos[i].hardwareAddress, (HardwareTypeIdentifier) discoveryInfo->infos[i].hardwareType);
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

			subscription_set_t* tmp = (subscription_set_t*) appLayerPacket->payload;
			subscription_helper_t* subscriptionInfo = (subscription_helper_t*) &tmp->info;
			int8_t index = parent->getSubscriptionSlot(address, subscriptionInfo->hardwareAddress, (HardwareTypeIdentifier) subscriptionInfo->hardwareType);

			if(index != -1) {
				parent->active[index] = true;
				#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
					Serial.print(millis());
					Serial.println(F(": SubscriptionManager::SubscriptionManagerSubscriptionCallback::doCallback() HARDWARE_SUBSCRIPTION_SET_RES"));
					Serial.print(F("\tremote="));
					Serial.print(address);
					Serial.print(F(" hwtype="));
					Serial.print(subscriptionInfo->hardwareType);
					Serial.print(F(" hwaddress="));
					Serial.print(subscriptionInfo->hardwareAddress);
					Serial.println(F(" refreshed."));
					Serial.flush();
				#endif
			}

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
	/** last full iteration for discovery */
	uint32_t lastDiscoveryRequestFullRun;

	/** next neighbourtable index to discover */
	uint8_t nextDiscoveryRequestNeighbourIndex;

	public:
		/** active subscriptions */
		subscription_helper_t activeSubscriptions[MAX_ACTIVE_SUBSCRIPTIONS]; //each 11b -> 550b
		l3_address_t activeSubscriptionNodeIDs[MAX_ACTIVE_SUBSCRIPTIONS]; //each 2b
		boolean active[MAX_ACTIVE_SUBSCRIPTIONS];
		/** last refresh of this subscription */
		uint32_t lastRefresh[MAX_ACTIVE_SUBSCRIPTIONS]; //each 4b -> 200b

		/**
		 * send a subscription request
		 * @param i
		 * @return success
		 */
		boolean sendSubscriptionRequest(uint8_t i) {
			#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				Serial.print(millis());
				Serial.print(F(": SubscriptionManager::sendSubscriptionRequest() destination="));
				Serial.print(activeSubscriptionNodeIDs[i]);
				Serial.print(F(" hwType="));
				Serial.print(activeSubscriptions[i].hardwareType);
				Serial.print(F(" hwAddress="));
				Serial.println(activeSubscriptions[i].hardwareAddress);
				Serial.flush();
			#endif

			//packet
			Layer3::packet_t p;
			pf.generateSubscriptionSetGeneric(&p, activeSubscriptionNodeIDs[i], &activeSubscriptions[i]);

			lastRefresh[i] = millis();
			active[i] = 0;
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
			memset(&activeSubscriptionNodeIDs, 0, sizeof(activeSubscriptionNodeIDs));
			memset(&lastRefresh, 0, sizeof(lastRefresh));
			memset(&active, 0, sizeof(active));

			subscriptionCallback.init(this);
			discoveryCallback.init(this);

			//register listeners by type.
			dispatcher.getResponseHandler()->registerListenerByPacketType(0, HARDWARE_SUBSCRIPTION_SET_RES, 0, &subscriptionCallback);
			dispatcher.getResponseHandler()->registerListenerByPacketType(0, HARDWARE_DISCOVERY_RES, 0, &discoveryCallback);

			lastSubscriptionRequest = 0;

			lastDiscoveryRequest = 0;
			lastDiscoveryRequestFullRun = 0;
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
						if(now - lastRefresh[i] > SUBSCRIPTION_TIMEOUT_MILLIS && active[i] == false) {
							deleteSubscription(i);
							continue;
						}

						//need to refresh?
						if(now - lastRefresh[i] > SUBSCRIPTION_REFRESH_MILLIS) {
							//refresh.
							sendSubscriptionRequest(i);

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
			neighbourData* neighbours = l3.getNeighbours();
			if(neighbours == NULL)
				return;
			uint32_t now = millis();

			//delay the sending.
			if(	now > DISCOVERY_REQUEST_DELAY_MILLIS && now - DISCOVERY_REQUEST_DELAY_MILLIS > lastDiscoveryRequest
				&& now > DISCOVERY_REQUEST_PERIOD_MILLIS && now - DISCOVERY_REQUEST_PERIOD_MILLIS > lastDiscoveryRequestFullRun) {

				//we have a node, send request.
				if(neighbours[nextDiscoveryRequestNeighbourIndex].nodeId > 0) {
					#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
						Serial.print(millis());
						Serial.println(F(": SubscriptionManager::maintainDiscoveries()"));

						Serial.print(F("\tnow="));
						Serial.print(now);
						Serial.print(F(" lastDicoveryRequest="));
						Serial.print(lastDiscoveryRequest);
						Serial.print(F(" lastDiscoveryRequestFullRun="));
						Serial.println(lastDiscoveryRequestFullRun);

						Serial.print(F("\tsending discovery request to neighbourIndex="));
						Serial.print(nextDiscoveryRequestNeighbourIndex);
						Serial.print(F(" nodeId="));
						Serial.println(neighbours[nextDiscoveryRequestNeighbourIndex].nodeId);
						Serial.flush();
					#endif

					sendDiscoveryRequest(neighbours[nextDiscoveryRequestNeighbourIndex].nodeId);
					lastDiscoveryRequest = now;
				}

				//iterate
				nextDiscoveryRequestNeighbourIndex++;
				if(nextDiscoveryRequestNeighbourIndex == CONFIG_L3_NUM_NEIGHBOURS - 1) {
					nextDiscoveryRequestNeighbourIndex = 0;
					lastDiscoveryRequestFullRun = now;
				}
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
				activeSubscriptionNodeIDs[index] = 0;
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
		 * @param destination
		 * @param hardwareAddres
		 * @param hardwareType
		 * @return index
		 */
		int8_t getSubscriptionSlot(l3_address_t address, uint8_t hardwareAddress, HardwareTypeIdentifier hardwareType) {
			for(uint8_t i = 0; i < MAX_ACTIVE_SUBSCRIPTIONS; i++) {
				//check ident subscription via nodeAddress, hwAddress and hwType
				if(activeSubscriptionNodeIDs[i] == address && activeSubscriptions[i].hardwareAddress == hardwareAddress && activeSubscriptions[i].hardwareType == hardwareType) {
					return i;
				}
			}

			return -1;
		}

		/**
		 * register active subscription and update timestamp
		 * @param destination
		 * @param hwAddress
		 * @param hwType
		 * @return success
		 */
		boolean createSubscription(l3_address_t address, uint8_t hardwareAddress, HardwareTypeIdentifier hardwareType) {
			#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				Serial.print(millis());
				Serial.print(F(": SubscriptionManager::createSubscription()"));
				Serial.flush();
			#endif

			//return in case we already have a subscription
			int8_t index = getSubscriptionSlot(address, hardwareAddress, hardwareType);
			if(index != -1) {
				#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
					Serial.print(F(" already exists@index="));
					Serial.println(index);
				#endif
				return false;
			}

			//get new slot
			index = getSubscriptionSlot();

			//failed?
			if(index == -1) {
				#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
					Serial.println(F(" list full."));
				#endif
				return false;
			}

			//save subscription
			seq_t seq = random(1, 0xffff);
			lastRefresh[index] = 0;
			activeSubscriptionNodeIDs[index] = address;

			activeSubscriptions[index].address = l3.localAddress;
			activeSubscriptions[index].hardwareAddress = hardwareAddress;
			activeSubscriptions[index].hardwareType = hardwareType;
			activeSubscriptions[index].millisecondsDelay = SUBSCRIPTION_DEFAULT_PERIOD_MILLIS;
			activeSubscriptions[index].onEventType = EVENT_TYPE_CHANGE;
			activeSubscriptions[index].sequence = seq;

			#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				Serial.print(F(" saved in slot="));
				Serial.println(index);
				Serial.print(F("\tforAddress="));
				Serial.print(l3.localAddress);
				Serial.print(F(" remote="));
				Serial.print(address);
				Serial.print(F(" hwAddress="));
				Serial.print(hardwareAddress);
				Serial.print(F(" hwType="));
				Serial.print(hardwareType);
				Serial.print(F(" delay="));
				Serial.print(SUBSCRIPTION_DEFAULT_PERIOD_MILLIS);
				Serial.print(F(" event="));
				Serial.print(EVENT_TYPE_CHANGE);
				Serial.print(F(" seq="));
				Serial.println(seq);
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
			}

			#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				Serial.println(F(" nope."));
				Serial.flush();
			#endif

			return false;
		}
}; //SubscriptionManager

#endif //__SUBSCRIPTIONMANAGER_H__
