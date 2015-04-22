/* 
* Polling.h
*
* Created: 01.03.2015 20:50:17
* Author: helge
*/


#ifndef __POLLING_H__
#define __POLLING_H__

#include <interfaces/ToBePolled.h>

#define pollingPointerListSize 5
#define POLLING_DELAY_MS 500

class Polling {
	protected:
		ToBePolled* driver[pollingPointerListSize];
	public:
		Polling();
		~Polling();
		
		virtual boolean registerDriver(ToBePolled* drv) = 0;
		virtual boolean unRegisterDriver(ToBePolled* drv) = 0;

		virtual void loop() = 0;

}; //Polling

#endif //__POLLING_H__
