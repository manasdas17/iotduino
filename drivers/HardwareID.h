//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : HardwareID.h
//  @ Date : 20.10.2014
//  @ Author :
//
//


#if !defined(_HARDWAREID_H)
#define _HARDWAREID_H

#include <Arduino.h>

//enum class HardwareTypeIdentifier : uint8_t {
enum HardwareTypeIdentifier {
	HWType_UNKNOWN = 0,
	HWType_ANALOG,
	HWType_DIGITAL,
	HWType_accelerometer,
	HWType_button,
	HWType_gyroscope,
	HWType_humidity,
	HWType_ir,
	HWType_keypad,
	HWType_magneticField,
	HWType_methane,
	HWType_motion,
	HWType_pressure,
	HWType_rtc,
	HWType_dcf77,
	HWType_sonar,
	HWType_hwSwitch,
	HWType_temprature,
	HWType_touchpad,
	HWTYPE_led,
	HWType_rcswitch,
	HWType_relay,
	HWType_light,
	HWType_tone
};

#endif  //_HARDWAREID_H
