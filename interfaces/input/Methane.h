//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : Methane.h
//  @ Date : 20.10.2014
//  @ Author :
//
//


#if !defined(_METHANE_H)
#define _METHANE_H

#include <Arduino.h>
#include <drivers/AnalogIOGeneric.h>

class Methane : public AnalogIOGeneric {
	public:
		void init(Multiplexible* pin, uint8_t hwaddress) {
			AnalogIOGeneric::init(pin, hwaddress);
		}
		void init(uint8_t pin, uint8_t hwaddress) {
			AnalogIOGeneric::init(pin, hwaddress);
		}

		virtual boolean implementsInterface( HardwareTypeIdentifier type );

		virtual boolean readVal( HardwareTypeIdentifier type, HardwareCommandResult* result );

		virtual HardwareTypeIdentifier* getImplementedInterfaces(HardwareTypeIdentifier* arr, uint8_t maxLen) {
			AnalogIOGeneric::getImplementedInterfaces(arr, maxLen);
			return this->addImplementedInterface(arr, maxLen, HWType_methane);
		}

	private:
		virtual boolean writeVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
			return false;
		}

};

#endif  //_METHANE_H
