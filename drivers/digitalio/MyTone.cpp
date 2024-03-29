/*
 * Tone.c
 *
 * Created: 17.05.2015 08:59:39
 *  Author: helge
 */

#include "../../interfaces/output/MyTone.h"

#ifdef TIMER_ENABLE

extern SimpleTimer timer;

boolean MyTone::implementsInterface( HardwareTypeIdentifier type ) {
	if(AnalogIOGeneric::implementsInterface(type) || type == HWType_tone)
		return true;
	return false;
}

const void MyTone::stopOutputHook(int timerNum) {
	MyTone* tmp = (MyTone*) timer.getCallbackContext(timerNum);
	if(tmp != NULL)
	tmp->write(0);
}

boolean MyTone::writeVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
	if(type == HWType_tone && result != NULL && result->getUint16ListNum() > 0) {
		uint16_t length = result->getUint16List()[0];

		uint8_t tone = this->myTone;
		if(result->getUint16ListNum() > 1)
		tone = result->getUint16List()[1];

		write(tone);
		int num = timer.setTimeout(length, (timer_callback) &MyTone::stopOutputHook);
		timer.setCallbackContext(num, (void*) this);

		return true;
	}

	return false;
}

#endif