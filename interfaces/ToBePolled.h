/*
 * ToBePolled.h
 *
 * Created: 01.03.2015 20:51:38
 *  Author: helge
 */ 


#ifndef TOBEPOLLED_H_
#define TOBEPOLLED_H_

#include <dispatcher/Commands.h>

class ToBePolled {
	protected:
		uint32_t last_poll_time;

		virtual void updatePollingTime() {
			last_poll_time = millis();
		}

	public:
		virtual boolean hasPollResult() = 0;
		virtual boolean getPollResult(HardwareCommandResult* result) = 0;
	
};

#endif /* TOBEPOLLED_H_ */