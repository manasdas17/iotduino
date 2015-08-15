/*
* SubscriptionManager.h
*
* Created: 31.05.2015 23:24:59
* Author: helge
*/


#ifndef __SUBSCRIPTIONMANAGER_H__
#define __SUBSCRIPTIONMANAGER_H__

#include <Arduino.h>
#include <avr/wdt.h>
#include <dispatcher/EventCallbackInterface.h>
#include <dispatcher/PacketDispatcher.h>
#include <dispatcher/PacketFactory.h>
#include <networking/Layer3.h>
#include <sdcard/SDcard.h>
#include <webserver/DiscoveryListener.h>

extern PacketDispatcher dispatcher;
extern PacketFactory pf;
extern Layer3 l3;
extern SDcard sdcard;


#define DISCOVERY_NUM_NODES 30
/** delay for subscription events */
//#define SUBSCRIPTION_DEFAULT_PERIOD_MILLIS (60*1000UL)

/** delay for refreshing subscriptions */
//#define SUBSCRIPTION_REFRESH_MILLIS (1*40*1000UL) //every 5 minutes
/** delay for sending subscription requests */
//#define SUBSCRIPTION_REQUEST_DELAY_MILLIS (100)
/** delay after a subscription has timed out */
//#define SUBSCRIPTION_TIMEOUT_MILLIS (1*60*1000UL) //timeout after 15 minutes

/** delay between discovery requests */
#define DISCOVERY_REQUEST_DELAY_MILLIS (2000)
/** delay between scanning neighbourtable and trigger discovery */
#define DISCOVERY_REQUEST_PERIOD_MILLIS (1*20*1000UL) //every 15 minutes

#define DISCOVERY_TIMEOUT 2000


class SubscriptionManager {
	/** static dicoverylistener object */
	discoveryListener listenerDiscovery;

	/** saved node information */
	l3_address_t knownNodes[DISCOVERY_NUM_NODES];


	/** last discovery request that has been sent */
	uint32_t lastDiscoveryRequest;
	/** last full iteration for discovery */
	uint32_t lastDiscoveryRequestFullRun;
	/** next neighbour id to process */
	uint8_t nextDiscoveryRequestNeighbourIndex;

	public:
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
			listenerDiscovery.init(0, webserverListener::START);

			memset(knownNodes, 0, sizeof(knownNodes));

			readKnownNodesFromSD();

			lastDiscoveryRequest = 0;
			lastDiscoveryRequestFullRun = 0;
			nextDiscoveryRequestNeighbourIndex = 0;
		}

		/**
		 * maintenance loop.
		 */
		void loop() {
			//maintainSubscriptions();
			maintainDiscoveries();
			maintainListeners();
		}

		void maintainListeners() {
			if(listenerDiscovery.state == webserverListener::FINISHED) {
				//init buffer for sd storage
				SDcard::SD_nodeDiscoveryInfoTableEntry_t buf[SD_DISCOVERY_NUM_INFOS_PER_NODE];
				memset(buf, 0, sizeof(buf));

				//put data into buffer
				for(uint8_t i = 0; i < listenerDiscovery.gottenInfos; i++) {
					buf[i].hardwareAddress = listenerDiscovery.sensorInfos[i].hardwareAddress;
					buf[i].hardwareType = listenerDiscovery.sensorInfos[i].hardwareType;
					//buf[i].rtcTimestamp = now();
					buf[i].rtcTimestamp = 0;
				}

				//store node info
				SDcard::SD_nodeInfoTableEntry_t infoObj;
				memset(&infoObj, 0, sizeof(infoObj));
				if(!sdcard.getDiscoveryNodeInfo(listenerDiscovery.remote, &infoObj)) {
					#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
					Serial.println(F("\tgetInfo failed."));
					#endif
				}
				if(infoObj.nodeId != listenerDiscovery.remote) {
					#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
					Serial.println(F("\tunknown node, adding."));
					#endif
					infoObj.nodeId = listenerDiscovery.remote;
				}
				infoObj.lastDiscoveryRequest = now();

				//store info object
				if(!sdcard.saveNodeInfo(listenerDiscovery.remote, &infoObj)) {
					#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
					Serial.println(F("\tsaving failed."));
					#endif
				}

				//store discovery info in case of new data.
				#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				Serial.println(F("\tgetting old info"));
				#endif
				SDcard::SD_nodeDiscoveryInfoTableEntry_t oldInfo[SD_DISCOVERY_NUM_INFOS_PER_NODE];
				sdcard.getDiscoveryInfosForNode(listenerDiscovery.remote, oldInfo, SD_DISCOVERY_NUM_INFOS_PER_NODE);

				if(memcmp(oldInfo, buf, sizeof(buf)) != 0) {
					//not equal for the new info buffer data
					#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
					Serial.println(F("\tnew information, update."));
					#endif
					sdcard.saveDiscoveryInfos(listenerDiscovery.remote, buf, SD_DISCOVERY_NUM_INFOS_PER_NODE);
				} else {
					#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
					Serial.println(F("\tnothing new."));
					#endif
				}

				listenerDiscovery.init(0, webserverListener::START);
				dispatcher.getResponseHandler()->unregisterListener(&listenerDiscovery);
			} else if(listenerDiscovery.state == webserverListener::FAILED) {
					#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
					Serial.print(millis());
					Serial.print(F(": discovery failed for node="));
					Serial.println(listenerDiscovery.remote);
					#endif
				listenerDiscovery.init(0, webserverListener::START);
				dispatcher.getResponseHandler()->unregisterListener(&listenerDiscovery);
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
			if(	listenerDiscovery.state == webserverListener::START && now > DISCOVERY_REQUEST_DELAY_MILLIS && now - DISCOVERY_REQUEST_DELAY_MILLIS > lastDiscoveryRequest
				&& now > DISCOVERY_REQUEST_PERIOD_MILLIS && now - DISCOVERY_REQUEST_PERIOD_MILLIS > lastDiscoveryRequestFullRun) {

				l3_address_t remote = neighbours[nextDiscoveryRequestNeighbourIndex].nodeId;

				//loopback?
				if(nextDiscoveryRequestNeighbourIndex == CONFIG_L3_NUM_NEIGHBOURS) {
					remote = l3.localAddress;
				}

				//we have a node, send request.
				if(remote > 0) {
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
						Serial.println(remote);
						Serial.flush();
					#endif

					//register listener by type.
					listenerDiscovery.init(remote, webserverListener::AWAITING_ANSWER);
					dispatcher.getResponseHandler()->registerListenerByPacketType(millis() + DISCOVERY_TIMEOUT, HARDWARE_DISCOVERY_RES, remote, &listenerDiscovery);

					//send request
					sendDiscoveryRequest(remote);
					lastDiscoveryRequest = now;
				}

				//iterate
				nextDiscoveryRequestNeighbourIndex++;
				if(nextDiscoveryRequestNeighbourIndex > CONFIG_L3_NUM_NEIGHBOURS) {
					nextDiscoveryRequestNeighbourIndex = 0;
					lastDiscoveryRequestFullRun = now;
				}
			}
		}

		/**
		 * register active subscription and update timestamp
		 * @param destination
		 * @param hwAddress
		 * @param hwType
		 * @return success
		 */
		//boolean createSubscription(l3_address_t address, uint8_t hardwareAddress, HardwareTypeIdentifier hardwareType) {
			//#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				//Serial.print(millis());
				//Serial.print(F(": SubscriptionManager::createSubscription()"));
				//Serial.flush();
			//#endif
//
			////return in case we already have a subscription
			//int8_t index = getSubscriptionSlot(address, hardwareAddress, hardwareType);
			//if(index != -1) {
				//#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
					//Serial.print(F(" already exists@index="));
					//Serial.println(index);
				//#endif
				//return false;
			//}
//
			////get new slot
			//index = getSubscriptionSlot();
//
			////failed?
			//if(index == -1) {
				//#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
					//Serial.println(F(" list full."));
				//#endif
				//return false;
			//}
//
			////save subscription
			////seq_t seq = random(1, 0xffff);
			////lastRefresh[index] = 0;
			////activeSubscriptionNodeIDs[index] = address;
////
			////activeSubscriptions[index].address = l3.localAddress;
			////activeSubscriptions[index].hardwareAddress = hardwareAddress;
			////activeSubscriptions[index].hardwareType = hardwareType;
			////activeSubscriptions[index].millisecondsDelay = SUBSCRIPTION_DEFAULT_PERIOD_MILLIS;
			////activeSubscriptions[index].onEventType = EVENT_TYPE_CHANGE;
			////activeSubscriptions[index].sequence = seq;
//
			//#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				//Serial.print(F(" saved in slot="));
				//Serial.println(index);
				//Serial.print(F("\tforAddress="));
				//Serial.print(l3.localAddress);
				//Serial.print(F(" remote="));
				//Serial.print(address);
				//Serial.print(F(" hwAddress="));
				//Serial.print(hardwareAddress);
				//Serial.print(F(" hwType="));
				//Serial.print(hardwareType);
				//Serial.print(F(" delay="));
				//Serial.print(SUBSCRIPTION_DEFAULT_PERIOD_MILLIS);
				//Serial.print(F(" event="));
				//Serial.print(EVENT_TYPE_CHANGE);
				//Serial.print(F(" seq="));
				//Serial.println();
				//Serial.flush();
			//#endif
//
			//return true;
		//}

		/**
		 * read all node info, set active if actual information is present
		 */
		void readKnownNodesFromSD() {
			uint8_t index = 0;
			SDcard::SD_nodeInfoTableEntry_t info;

			#if DEBUG
			Serial.print(millis());
			Serial.print(F(": reading nodeInfo"));
			#endif
			for(uint8_t i = 0; i < SD_DISCOVERY_NUM_NODES; i++) {
				#if DEBUG
					if(i%10==0)
						Serial.print('.');
				#endif

				wdt_reset();

				info.nodeId = 0;
				sdcard.getDiscoveryNodeInfo(i, &info);

				if(info.nodeId != 0) {
					#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
						Serial.println();
						Serial.print(F("\tNodeId="));
						Serial.print(i);
						Serial.print(F(" lastDiscovery="));
						Serial.println(info.lastDiscoveryRequest);
					#endif
					knownNodes[index] = i;
					index++;
				}
			}
			#ifdef DEBUG_SUBSCRIPTION_MGR_ENABLE
				Serial.println();
				Serial.print(millis());
				Serial.print(F(": readKnownNodesFromSD() foundNum="));
				Serial.println(index);
			#endif
		}


}; //SubscriptionManager

#endif //__SUBSCRIPTIONMANAGER_H__
