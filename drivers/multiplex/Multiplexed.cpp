//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : Multiplexed.cpp
//  @ Date : 20.10.2014
//  @ Author : 
//
//


#include "Multiplexed.h"

uint8_t Multiplexed::getPIN() {
	//prepare pin - set bits from used address to address lanes
	for(uint8_t i = 0; i < numAddressLanes; i++) {
		digitalWrite(AddressLanes[i], usedPIN >> i & 0x1);
	}

	return DataLanes[0];
}

