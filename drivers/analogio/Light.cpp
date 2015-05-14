/*
 * Light.cpp
 *
 * Created: 14.05.2015 15:18:44
 *  Author: helge
 */

#include "../../interfaces/input/Light.h"

boolean Light::implementsInterface( HardwareTypeIdentifier type ) {
	if(type == HWType_light)
		return true;
	return false;
}

boolean Light::readVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
	return AnalogIOGeneric::readVal(HWType_ANALOG, result);
}