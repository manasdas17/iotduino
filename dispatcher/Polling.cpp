/* 
* Polling.cpp
*
* Created: 01.03.2015 20:50:17
* Author: helge
*/


#include "Polling.h"

// default constructor
Polling::Polling()
{
} //Polling

// default destructor
Polling::~Polling()
{
} //~Polling

boolean Polling::registerDriver( ToBePolled* drv ) {
	if(drv == NULL) {
		return false;
	}

	uint8_t freeIndex = 0xff;
	for(uint8_t i = 0; i < pollingPointerListSize; i++) {
		if(this->driver[i] == drv) {
			return false;
		} else if(driver[i] == NULL && freeIndex == 0xff){
			//found first free index
			freeIndex = i;
		}
	}
	
	if(freeIndex != 0xff) {
		driver[freeIndex] = drv;
		return true;
	}
	
	return false;
}

boolean Polling::unRegisterDriver( ToBePolled* drv ) {
	if(drv == NULL) {
		return false;
	}

	for(uint8_t i = 0; i < pollingPointerListSize; i++) {
		if(this->driver[i] == drv) {
			this->driver[i] = NULL;
			return true;
		}
	}
	return false;
}

void Polling::loop() {
	for(uint8_t i = 0; i < pollingPointerListSize; i++) {
		if(driver[i] != NULL) {
			if(driver[i]->hasPollResult()) {
				HardwareCommandResult hwres = HardwareCommandResult();
				driver[i]->getPollResult(&hwres);
			}
		}
	}
}
