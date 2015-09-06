/*
 * DS1307RTC.h - library for DS1307 RTC

  Copyright (c) Michael Margolis 2009
  This library is intended to be uses with Arduino Time.h library functions

  The library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  30 Dec 2009 - Initial release
  5 Sep 2011 updated for Arduino 1.0
 */

#include <Wire.h>
#include <interfaces/input/RTC.h>

#define DS1307_CTRL_ID 0x68

// PUBLIC FUNCTIONS
time_t RTC::get() {
  tmElements_t tm;
  if (read(tm) == false) return 0;
	return(makeTime(tm));
}

bool RTC::set(time_t t) {
  setTime(t);
  tmElements_t tm;
  breakTime(t, tm);
  tm.Second |= 0x80;  // stop the clock
  write(tm);
  tm.Second &= 0x7f;  // start the clock
  return write(tm);
}

// Aquire data from the RTC chip in BCD format
bool RTC::read(tmElements_t &tm) {
	uint8_t sec;
	Wire.beginTransmission(i2cAddress);
	#if ARDUINO >= 100
		Wire.write((uint8_t)0x00);
	#else
		Wire.send(0x00);
	#endif
	if (Wire.endTransmission() != 0) {
		exists = false;
		return false;
	}
	exists = true;

	// request the 7 data fields   (secs, min, hr, dow, date, mth, yr)
	Wire.requestFrom(i2cAddress, tmNbrFields);
	if (Wire.available() < tmNbrFields)
		return false;
	#if ARDUINO >= 100
		sec = Wire.read();
		tm.Second = bcd2dec(sec & 0x7f);
		tm.Minute = bcd2dec(Wire.read() );
		tm.Hour =   bcd2dec(Wire.read() & 0x3f);  // mask assumes 24hr clock
		tm.Wday = bcd2dec(Wire.read() );
		tm.Day = bcd2dec(Wire.read() );
		tm.Month = bcd2dec(Wire.read() );
		tm.Year = y2kYearToTm((bcd2dec(Wire.read())));
	#else
		sec = Wire.receive();
		tm.Second = bcd2dec(sec & 0x7f);
		tm.Minute = bcd2dec(Wire.receive() );
		tm.Hour =   bcd2dec(Wire.receive() & 0x3f);  // mask assumes 24hr clock
		tm.Wday = bcd2dec(Wire.receive() );
		tm.Day = bcd2dec(Wire.receive() );
		tm.Month = bcd2dec(Wire.receive() );
		tm.Year = y2kYearToTm((bcd2dec(Wire.receive())));
	#endif

	setTime(makeTime(tm));

	if (sec & 0x80)
		return false; // clock is halted
	return true;
}

bool RTC::write(tmElements_t &tm) {
	Wire.beginTransmission(i2cAddress);
	#if ARDUINO >= 100
		Wire.write((uint8_t)0x00); // reset register pointer
		Wire.write(dec2bcd(tm.Second)) ;
		Wire.write(dec2bcd(tm.Minute));
		Wire.write(dec2bcd(tm.Hour));      // sets 24 hour format
		Wire.write(dec2bcd(tm.Wday));
		Wire.write(dec2bcd(tm.Day));
		Wire.write(dec2bcd(tm.Month));
		Wire.write(dec2bcd(tmYearToY2k(tm.Year)));
	#else
		Wire.send(0x00); // reset register pointer
		Wire.send(dec2bcd(tm.Second)) ;
		Wire.send(dec2bcd(tm.Minute));
		Wire.send(dec2bcd(tm.Hour));      // sets 24 hour format
		Wire.send(dec2bcd(tm.Wday));
		Wire.send(dec2bcd(tm.Day));
		Wire.send(dec2bcd(tm.Month));
		Wire.send(dec2bcd(tmYearToY2k(tm.Year)));
	#endif
	if (Wire.endTransmission() != 0) {
		exists = false;
		return false;
	}
	exists = true;
	return true;
}

// PRIVATE FUNCTIONS

// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t RTC::dec2bcd(uint8_t num)
{
	return ((num/10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t RTC::bcd2dec(uint8_t num)
{
	return ((num/16 * 10) + (num % 16));
}

bool RTC::exists = false;



bool RTC::getTime(const char *str, tmElements_t* tm)
{
	int Hour, Min, Sec;

	if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
	tm->Hour = Hour;
	tm->Minute = Min;
	tm->Second = Sec;
	return true;
}

bool RTC::getDate(const char *str, tmElements_t* tm)
{
	const char *monthName[12] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	char Month[12];
	int Day, Year;
	uint8_t monthIndex;

	if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
	for (monthIndex = 0; monthIndex < 12; monthIndex++) {
		if (strcmp(Month, monthName[monthIndex]) == 0) break;
	}
	if (monthIndex >= 12) return false;
	tm->Day = Day;
	tm->Month = monthIndex + 1;
	tm->Year = CalendarYrToTm(Year);
	return true;
}

void RTC::init(uint8_t hwaddress) {
	I2C::init(DS1307_CTRL_ID, hwaddress);

	//try to read - this sets existance flag.
	read();

	if(!chipPresent()) {
		tmElements_t tm;
		if (getDate(__DATE__, &tm) && getTime(__TIME__, &tm)) {
			write(tm);
		}
	}
}

HardwareTypeIdentifier* RTC::getImplementedInterfaces(HardwareTypeIdentifier* arr, uint8_t maxLen) {
	arr = I2C::getImplementedInterfaces(arr, maxLen);
	return addImplementedInterface(arr, maxLen, HWType_rtc);
}

boolean RTC::writeVal(HardwareTypeIdentifier type, HardwareCommandResult* result) {
	result->setReadRequest(0);
	if(result->getUint8ListNum() < 4)
		return false;
	uint32_t time = 0;
	//MSB first in data
	time |= ((uint32_t) result->getUint8List()[3]) << 24;
	time |= ((uint32_t) result->getUint8List()[2]) << 16;
	time |= ((uint32_t) result->getUint8List()[1]) << 8;
	time |= ((uint32_t) result->getUint8List()[0]);

	RTC::set(time);

	return true;
}

boolean RTC::readVal(HardwareTypeIdentifier type, HardwareCommandResult* result) {
	read();
	uint32_t timeNow = now();

	result->setReadRequest(1);
	result->setUint8ListNum(4);

	//MSB first
	result->getUint8List()[0] = (timeNow >> 24) & 0xff;
	result->getUint8List()[1] |= (timeNow >> 16) & 0xff;
	result->getUint8List()[2] |= (timeNow >> 8) & 0xff;
	result->getUint8List()[3] |= (timeNow) & 0xff;

	return true;
}

boolean RTC::implementsInterface(HardwareTypeIdentifier type) {
	if(type == HWType_rtc)
		return true;
	return false;
}

uint32_t RTC::read() {
	return get();
}