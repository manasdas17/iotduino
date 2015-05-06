//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : AnalogIOGeneric.h
//  @ Date : 20.10.2014
//  @ Author :
//
//


#if !defined(_ANALOGIOGENERIC_H)
#define _ANALOGIOGENERIC_H

#include <Arduino.h>
#include <drivers/HardwareID.h>
#include <drivers/analogio/AnalogIO.h>
#include <dispatcher/Commands.h>

#define CALIBRATION_MEASUREMENTS_NUM 5

class AnalogIOGeneric : public AnalogIO {
	protected:
		uint8_t zeroVal;

	public:
		AnalogIOGeneric(Multiplexible* pin) : AnalogIO(pin) {
		}

		AnalogIOGeneric(uint8_t pin) : AnalogIO(pin) {
		}

		virtual boolean calibrateZero();

		virtual uint8_t read();
		virtual void read(HardwareCommandResult* hwresult);
		virtual int16_t readCalibrated();
		virtual void readCalibrated(HardwareCommandResult* hwresult);
		virtual void write(uint8_t val);

		virtual boolean implementsInterface( HardwareTypeIdentifier type );;
		virtual boolean readVal( HardwareTypeIdentifier type, HardwareCommandResult* result );
		virtual boolean writeVal( HardwareTypeIdentifier type, HardwareCommandResult* result );

};

#endif  //_ANALOGIOGENERIC_H