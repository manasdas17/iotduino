//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : RTC.h
//  @ Date : 20.10.2014
//  @ Author :
//
//


#if !defined(_RTC_H)
#define _RTC_H

#include <Arduino.h>
#include <Time.h>
#include "../../drivers/i2c/I2C.h"



class RTC : I2C {

	protected:
		time_t get();
		bool set(uint32_t t);
		bool read(tmElements_t &tm);
		bool write(tmElements_t &tm);
		bool chipPresent() { return exists; }

	private:
		static bool exists;
		static uint8_t dec2bcd(uint8_t num);
		static uint8_t bcd2dec(uint8_t num);
		bool getDate(const char *str, tmElements_t* tm);
		bool getTime(const char *str, tmElements_t* tm);

	public:
		RTC() {};

		void init(uint8_t hwaddress);

		uint32_t read();

		boolean implementsInterface(HardwareTypeIdentifier type);

		boolean readVal(HardwareTypeIdentifier type, HardwareCommandResult* result);

		boolean writeVal(HardwareTypeIdentifier type, HardwareCommandResult* result);

		HardwareTypeIdentifier* getImplementedInterfaces(HardwareTypeIdentifier* arr, uint8_t maxLen);
};

#endif  //_RTC_H
