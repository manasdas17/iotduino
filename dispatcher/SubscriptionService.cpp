
#include "SubscriptionService.h"

#ifdef ENABLE_SUBSCRIPTION_SERVICE

boolean SubscriptionService::executeSubscription(const subscription_helper_t* subscription) {
	if(networking == NULL || subscription == NULL || commandHandler == NULL)
		return false;

	//setup application packet data
	packet_application_numbered_cmd_t pan;
	memset(&pan, 0, sizeof(pan));
	//this is a hardware read
	pan.packetType = HARDWARE_COMMAND_READ;

	//the actual command gets the hardware type and address
	command_t* cmd = (command_t*) pan.payload;
	cmd->address = subscription->hardwareAddress;
	cmd->type = (HardwareTypeIdentifier) subscription->hardwareType;
	cmd->isRead = 1;

	//fire into the system - with networking callback.
	return commandHandler->handleHardwareCommand(&pan, networking->getCallbackInterface(), subscription->address, subscription->sequence);
}

boolean SubscriptionService::handleRequest(EventCallbackInterface* callback, const seq_t seq, const packet_type_application_t type, const l3_address_t remote, packet_application_numbered_cmd_t* appPacket) {
	if(callback == NULL || appPacket == NULL)
		return false;

	switch(type) {
		case HARDWARE_SUBSCRIPTION_SET:
			return handleSubscriptionRequest(callback, seq, type, remote, appPacket);
		case HARDWARE_SUBSCRIPTION_INFO:
			return handleSubscriptionInfoRequest(callback, seq, type, remote, appPacket);
		default:
			return false;
	}
}

SubscriptionService::SubscriptionService() {
}

void SubscriptionService::executeSubscriptions() {
	uint32_t now = millis();
	if(now - SUBSCRIPTION_CHECK_PERIOD_MILLIS > lastSubscriptionCheckTimestamp && now > SUBSCRIPTION_CHECK_PERIOD_MILLIS) {
		lastSubscriptionCheckTimestamp = millis();

		//#ifdef DEBUG_HANDLER_SUBSCRIPTION_ENABLE
		//Serial.print(millis());
		//Serial.println(F(": SubscriptionService::executeSubscriptions()"));
		//Serial.flush();
		//#endif

		uint32_t now = millis();

		subscription_helper_t* currentItem;
		uint32_t lastExecution;
		#ifdef ENABLE_EXTERNAL_RAM
			SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionSubscriptions);
			while(it.hasNext()) {
				currentItem = (subscription_helper_t*) it.next();
				ram.readElementIntoVar(memRegionSubscriptionLastExecutions, it.getIteratorIndex()-1, &lastExecution);
		#else
			for(uint8_t i = 0; i < numSubscriptionList; i++) {
				currentItem = subscriptions[i];
				lastExecution = subscriptionsLastExecution[i];
		#endif
				if(currentItem->address != 0 && now > currentItem->millisecondsDelay && now - lastExecution > currentItem->millisecondsDelay) {
					//if(subscriptions[i].address != 0 && subscriptions[i].onEvent == 0 && now - subscriptionsLastExecution[i] > subscriptions[i].millisecondsDelay) {

					#ifdef ENABLE_EXTERNAL_RAM
						ram.writeElementToRam(memRegionSubscriptionLastExecutions, it.getIteratorIndex()-1, &now);
					#else
						subscriptionsLastExecution[i] = now;
					#endif

					executeSubscription(currentItem);
				}
			}
	}
}

uint8_t SubscriptionService::getSubscriptionInfos(l3_address_t forAddress, subscription_helper_struct* buffer, uint8_t buffer_len) const {
	//sanity check.
	if(buffer == NULL || buffer_len < sizeof(subscription_helper_t) * numSubscriptionList) {
		return 0;
	}

	//iterate
	uint8_t numFound = 0;

	#ifdef ENABLE_EXTERNAL_RAM
		subscription_helper_t tmp;
		subscription_helper_t* currentItem = NULL;
		SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionSubscriptions);
		while(it.hasNext()) {
			currentItem = (subscription_helper_t*) it.next();
	#else
		for(uint8_t i = 0; i < numSubscriptionList; i++) {
			currentItem = subscriptions[i];
	#endif
			if(currentItem->address != 0 && (forAddress == 0 || currentItem->address == forAddress)) {
				#ifdef ENABLE_EXTERNAL_RAM
					ram.readElementIntoVar(memRegionSubscriptions, it.getIteratorIndex()-1, &tmp);
					ram.writeElementToRam(memRegionSubscriptionsTmpBuffer, it.getIteratorIndex()-1, &tmp);
				#else
					memcpy(&buffer[numFound], currentItem, sizeof(subscription_helper_t));
				#endif

				numFound++;
			}
		}

	return numFound;
}

boolean SubscriptionService::setSubscription(subscription_helper_t* s) {
	if(s == NULL)
		return false;

	//do we know this hardware?
	if(!hwinterface->hasHardwareDriver((HardwareTypeIdentifier) s->hardwareType, s->hardwareAddress))
		return false;

	//we do not want any update and this subscription has no event trigger
	if(s->millisecondsDelay == 0 && s->onEventType == EVENT_TYPE_DISABLED) {
		return deleteSubscription(s);
	}

	//get current subscription index in case it exists
	uint8_t index = getSubscriptionIndex(s);

	//do we already have a subscription?
	if(index == 0xff) {
		//find free slot
		index = getFreeSubscriptionIndex();
	}

	//update
	if(index != 0xff) {
		#ifdef ENABLE_EXTERNAL_RAM
			ram.writeElementToRam(memRegionSubscriptions, index, s);
			uint32_t tmp = 0;
			ram.writeElementToRam(memRegionSubscriptionLastExecutions, index, &tmp);
		#else
			memcpy(&subscriptions[index], s, sizeof(subscription_helper_t));
			subscriptionsLastExecution[index] = 0;
		#endif
		return true;
	}

	//something went wrong or list is full.
	return false;
}

boolean SubscriptionService::deleteSubscription(subscription_helper_t* s) {
	if(s == NULL)
		return true; //not present, it is "deleted"

	uint8_t index = getSubscriptionIndex(s);

	if(index == 0xff)
		return false;

	//delete
	#ifdef ENABLE_EXTERNAL_RAM
		ram.memsetElement(memRegionSubscriptions, index, 0);

		uint32_t tmp = 0;
		ram.writeElementToRam(memRegionSubscriptionLastExecutions, index, &tmp);
	#else
		memset(&subscriptions[index], 0, sizeof(subscription_helper_t));
		subscriptionsLastExecution[index] = 0;
	#endif
	return true;
}

uint8_t SubscriptionService::getSubscriptionIndex(subscription_helper_t* s) {
	if(s != NULL) {

		#ifdef ENABLE_EXTERNAL_RAM
			subscription_helper_t* currentItem = NULL;
			SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionSubscriptions);
			while(it.hasNext()) {
				currentItem = (subscription_helper_t*) it.next();
		#else
			for(uint8_t i = 0; i < numSubscriptionList; i++) {
				currentItem = subscriptions[i];
		#endif
				//do we have this subscription?
				if(currentItem->address == s->address && currentItem->hardwareAddress == s->hardwareAddress && currentItem->hardwareType == s->hardwareType) {
					#ifdef ENABLE_EXTERNAL_RAM
						return it.getIteratorIndex()-1;
					#else
						return i;
					#endif
				}
			}
	}
	return 0xff;
}

uint8_t SubscriptionService::getFreeSubscriptionIndex() {
	#ifdef ENABLE_EXTERNAL_RAM
		subscription_helper_t* currentItem = NULL;
		SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionSubscriptions);
		while(it.hasNext()) {
			currentItem = (subscription_helper_t*) it.next();
	#else
		for(uint8_t i = 0; i < numSubscriptionList; i++) {
			currentItem = subscriptions[i];
	#endif
			//do we have this subscription?
			if(currentItem->address == 0) {
				#ifdef ENABLE_EXTERNAL_RAM
					return it.getIteratorIndex()-1;
				#else
					return i;
				#endif
			}
		}
	return 0xff;
}

boolean SubscriptionService::handleSubscriptionInfoRequest(EventCallbackInterface* callback, seq_t seq, packet_type_application_t type, l3_address_t remote, packet_application_numbered_cmd_t* appPacket) const {
	if(appPacket == NULL || callback == NULL)
		return false;

	subscription_info_t* subscriptionInfo = (subscription_info_t*) appPacket->payload;

	//create buffer for info
	#ifdef ENABLE_EXTERNAL_RAM
		uint8_t num = getSubscriptionInfos(subscriptionInfo->forAddress, NULL, 0);
	#else
		subscription_helper_t buffer[getSubscriptionListSize()];
		memset(buffer, 0, sizeof(buffer));

		uint8_t num = getSubscriptionInfos(subscriptionInfo->forAddress, buffer, sizeof(buffer));
	#endif

	//we have no subscriptions
	if(num == 0) {
		callback->fail(seq, remote);
		return true;
	}

	//return answer
	packet_application_numbered_cmd_t newPkt;
	memset(&newPkt, 0, sizeof(newPkt));
	newPkt.packetType = HARDWARE_SUBSCRIPTION_INFO_RES;
	subscription_info_t* info = (subscription_info_t*) newPkt.payload;
	info->forAddress = subscriptionInfo->forAddress; //not yet changed, everything is fine.

	//send one packet for each entry - currently on same sequence, should be changed. TODO!
	for(; num > 0; num--) {
		info->numInfosFollowing = num - 1;

		#ifdef ENABLE_EXTERNAL_RAM
			SPIRamManager::memRegion_t region;
			ram.getRegionInfo(&region, memRegionSubscriptionsTmpBuffer);
			ram.memcpy_R(&info->info, region.ramStartAddress + num * region.elementSize, region.elementSize);
		#else
			memcpy(&info->info, &buffer[num - 1], sizeof(subscription_helper_t));
		#endif

		callback->doCallback(&newPkt, remote, seq);
		//increase sequence
		seq++;
	}
	return true;
}

boolean SubscriptionService::handleSubscriptionRequest(EventCallbackInterface* callback, const seq_t seq, const packet_type_application_t type, const l3_address_t remote, packet_application_numbered_cmd_t* appPacket) {
	if(appPacket == NULL || callback == NULL)
		return false;

	subscription_set_struct* subscriptionSet = (subscription_set_struct*) appPacket->payload;

	//set.
	boolean result = setSubscription(&subscriptionSet->info);

	//no failed
	if(!result) {
		callback->fail(seq, remote);
		return false;
	}

	//success
	appPacket->packetType = HARDWARE_SUBSCRIPTION_SET_RES;
	callback->doCallback(appPacket, remote, seq);
	return true;
}


#ifdef ENABLE_EVENTS
void SubscriptionService::doPollingForSubscriptions() {
	uint32_t now = millis();
	if(now - SUBSCRIPTION_POLLING_CHECK_PERIOD_MILLIS > lastSubscriptionPollingCheckTimestamp && now > SUBSCRIPTION_POLLING_CHECK_PERIOD_MILLIS) {
		lastSubscriptionPollingCheckTimestamp = now;

		//#ifdef DEBUG_HANDLER_SUBSCRIPTION_ENABLE
		//Serial.print(millis());
		//Serial.println(F(": SubscriptionService::doPollingForSubscriptions()"));
		//Serial.flush();
		//#endif


		#ifdef ENABLE_EXTERNAL_RAM
			subscription_helper_t* currentItem = NULL;
			SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionSubscriptions);
			while(it.hasNext()) {
				currentItem = (subscription_helper_t*) it.next();
		#else
			for(uint8_t i = 0; i < numSubscriptionList; i++) {
				currentItem = subscriptions[i];
		#endif
				//is this an event subscription?
				if(currentItem->onEventType != EVENT_TYPE_DISABLED) {
					//get driver
					HardwareDriver* drv = hwinterface->getHardwareDriver((HardwareTypeIdentifier) currentItem->hardwareType, currentItem->hardwareAddress);

					//does it support events?
					if(drv == NULL || !drv->canDetectEvents())
					continue;

					//maintain event data in driver
					drv->eventLoop();

					//did we detect an event in last check period and does the event match? - execute subscription
					if(drv->getLastEventTimestamp() > 0
						&& millis() - drv->getLastEventTimestamp() < SUBSCRIPTION_POLLING_CHECK_PERIOD_MILLIS
						&& drv->lastEventMatchesEventType(currentItem->onEventType))
					{
						#ifdef DEBUG_HANDLER_SUBSCRIPTION_ENABLE
						Serial.print(F("\tevent found, trigger subscription execution, matchingType="));
						Serial.println(currentItem->onEventType);
						Serial.flush();
						#endif
						executeSubscription(currentItem);
					}
				}
			}
	}
}
#endif

#endif