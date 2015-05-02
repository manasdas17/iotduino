//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : MotionDetector.h
//  @ Date : 20.10.2014
//  @ Author :
//
//


#if !defined(_MOTIONDETECTOR_H)
#define _MOTIONDETECTOR_H

#include <Arduino.h>
#include "../../drivers/digitalio/DigitalIO.h"
#include "../../HardwareID.h"
#include <dispatcher/Commands.h>
#include "../ToBePolled.h"


class MotionDetector : public DigitalIO, public ToBePolled {
	public:
		volatile uint32_t timestampLastLow;
		volatile uint32_t timestampLastHigh;
		volatile uint32_t timestampLastRead;

		virtual boolean read() = 0;
		virtual void read(HardwareCommandResult* hwresult)=0;
		virtual void updateTimes(boolean val);

		virtual boolean implementsInterface( HardwareTypeIdentifier type );

		virtual boolean readVal( HardwareTypeIdentifier type, HardwareCommandResult* result );

		virtual boolean hasPollResult();
		virtual boolean getPollResult( HardwareCommandResult* result );

		virtual HardwareTypeIdentifier* getImplementedInterfaces(HardwareTypeIdentifier* arr, uint8_t maxLen) {
			DigitalIO::getImplementedInterfaces(arr, maxLen);
			return this->addImplementedInterface(arr, maxLen, HWType_motion);
		}

};

#endif  //_MOTIONDETECTOR_H
