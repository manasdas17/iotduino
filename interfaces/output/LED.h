//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : LED.h
//  @ Date : 20.10.2014
//  @ Author :
//
//


#if !defined(_LED_H)
#define _LED_H

#include <Arduino.h>
#include <drivers/HardwareID.h>
#include <drivers/DigitalIOGeneric.h>

class LED : public DigitalIOGeneric {
	public:
		/**
		 * @param pin
		 * @param address
		 */
		LED(uint8_t pin, uint8_t hwaddress) : DigitalIOGeneric(pin, hwaddress) {
		}
		void turnOn();
		void turnOff();
		void toggle();

		virtual boolean implementsInterface( HardwareTypeIdentifier type );

		virtual HardwareTypeIdentifier* getImplementedInterfaces(HardwareTypeIdentifier* arr, uint8_t maxLen) {
			arr = DigitalIOGeneric::getImplementedInterfaces(arr, maxLen);
			return this->addImplementedInterface(arr, maxLen, HWTYPE_led);
		}

		/**
		 * @param type
		 * @param result:
		 *        - uintliust[0]: 0=off, 1=on
		 */
		virtual boolean readVal( HardwareTypeIdentifier type, HardwareCommandResult* result );

		/**
		 * @param type
		 * @result result
		 *         - uintlist[0]: 0=off, 1=on, 2=toggle
		 */
		virtual boolean writeVal( HardwareTypeIdentifier type, HardwareCommandResult* result );

		/**
		 * @return false
		 */
		virtual boolean read();

		virtual HardwareTypeIdentifier* addImplementedInterface(HardwareTypeIdentifier* arr, uint8_t maxLen, HardwareTypeIdentifier type) {
			this->getImplementedInterfaces(arr, maxLen);
			return addImplementedInterface(arr, maxLen, HWTYPE_led);
		}
};

#endif  //_LED_H
