/*
 * IRreceive.h
 *
 * Created: 10.11.2014 20:35:03
 *  Author: helge
 */ 


#ifndef IRRECEIVE_H_
#define IRRECEIVE_H_

#include <Arduino.h>
#include "../../HardwareID.h"

class IRreceive {
	public:
		IRreceive() {}
			
		virtual uint8_t availabe()=0;
		virtual uint32_t peek()=0;
		virtual uint32_t pop()=0;
		
		virtual void pop(HardwareCommandResult* hwresult)=0;
};



#endif /* IRRECEIVE_H_ */