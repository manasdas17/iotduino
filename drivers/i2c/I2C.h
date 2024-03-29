//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : I2C.h
//  @ Date : 20.10.2014
//  @ Author :
//
//


#if !defined(_I2C_H)
#define _I2C_H

#include <Arduino.h>
#include <drivers/HardwareDriver.h>
#include <Wire.h>

class I2C : public HardwareDriver {
	protected:
	uint8_t i2cAddress;

		virtual void init(uint8_t i2cAddress, uint8_t hwaddress) {
			this->i2cAddress = i2cAddress;
			HardwareDriver::init(hwaddress);

			Wire.begin();
		}

	public:
		virtual void init(uint8_t hwaddress) = 0;

		virtual boolean implementsInterface( HardwareTypeIdentifier type ) = 0;

		virtual boolean readVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) = 0;
		virtual boolean writeVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) = 0;

		inline virtual HardwareTypeIdentifier* getImplementedInterfaces(HardwareTypeIdentifier* arr, uint8_t maxLen) {
			return arr;
		}

		inline virtual uint8_t getI2CAddress() {
			return i2cAddress;
		}
};

#endif  //_I2C_H
