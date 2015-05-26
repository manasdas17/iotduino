/*
 * Light.h
 *
 * Created: 14.05.2015 15:17:40
 *  Author: helge
 */


#ifndef LIGHT_H_
#define LIGHT_H_


//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : Light.h
//  @ Date : 14.05.2015
//  @ Author : Helge Reelfs
//
//	Basic hardware layout as follows:
// <pre><code>
//        +----------+     +---------+
//	5V ---| e.g. 20k |--+--| R light |--- GND
//        +----------+  |  +---------+
//                      |
//                  analog in
// </pre></code>
// then hight voltage corresponds to bright light - low voltage corresponds to darker environments
// the resistance drops at a higher light level...; my resistors are at ~20k at usual in-house daylight

#include <Arduino.h>
#include <drivers/AnalogIOGeneric.h>

#define EVENT_MIN_DIFF 50

class Light : public AnalogIOGeneric {
	public:
		void init(Multiplexible* pin, uint8_t hwaddress) {
			AnalogIOGeneric::init(pin, hwaddress) ;
		}
		void init(uint8_t pin, uint8_t hwaddress) {
			AnalogIOGeneric::init(pin, hwaddress) ;
		}

		virtual boolean implementsInterface( HardwareTypeIdentifier type );

		virtual boolean readVal( HardwareTypeIdentifier type, HardwareCommandResult* result );


		virtual HardwareTypeIdentifier* getImplementedInterfaces(HardwareTypeIdentifier* arr, uint8_t maxLen) {
			AnalogIOGeneric::getImplementedInterfaces(arr, maxLen);
			return this->addImplementedInterface(arr, maxLen, HWType_light);
		}

		#ifdef ENABLE_EVENTS
		virtual boolean canDetectEvents();
		virtual subscription_event_type_t eventLoop();
		#endif
	private:
		virtual boolean writeVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
			return false;
		}

};

#endif /* LIGHT_H_ */