//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : Methane.cpp
//  @ Date : 20.10.2014
//  @ Author :
//
//


#include "../../interfaces/input/Methane.h"

boolean Methane::implementsInterface( HardwareTypeIdentifier type ) {
	if(AnalogIOGeneric::implementsInterface(type) ||type == HWType_methane)
		return true;
	return false;
}

boolean Methane::readVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
	return AnalogIOGeneric::readVal(HWType_ANALOG, result);
}
