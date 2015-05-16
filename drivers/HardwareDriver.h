//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : HardwareDriver.h
//  @ Date : 20.10.2014
//  @ Author :
//
//


#if !defined(_HARDWAREDRIVER_H)
#define _HARDWAREDRIVER_H

#include <Arduino.h>
#include "HardwareID.h"
#include <dispatcher/Commands.h>
#include <dispatcher/EventDetector.h>

class HardwareDriver
#ifdef ENABLE_EVENTS
: public EventDetector
#endif
{
	public:
		uint8_t hardwareAddress;

		HardwareDriver() {
			this->hardwareAddress = random();
		}

		HardwareDriver(uint8_t hardwareAddress) {
			this->hardwareAddress = hardwareAddress;
		}

		virtual uint8_t getAddress() {
			return hardwareAddress;
		}

		virtual ~HardwareDriver() {

		}

		virtual boolean readVal(HardwareTypeIdentifier type, HardwareCommandResult* result)=0;
		virtual boolean writeVal(HardwareTypeIdentifier type, HardwareCommandResult* result)=0;

		virtual boolean implementsInterface(HardwareTypeIdentifier type) = 0;

		virtual HardwareTypeIdentifier* getImplementedInterfaces(HardwareTypeIdentifier* arr, uint8_t maxLen)=0;

		virtual HardwareTypeIdentifier* addImplementedInterface(HardwareTypeIdentifier* arr, uint8_t maxLen, HardwareTypeIdentifier type) {
			if(arr == NULL)
				return NULL;

			for(uint8_t i = 0; i < maxLen; i++) {
				if(arr[i] == 0) {
					arr[i] = type;
					return arr;
				}
			}
			return NULL;
		}
};

#endif  //_HARDWAREDRIVER_H
