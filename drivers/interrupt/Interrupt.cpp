/*
 * Interrupt.cpp
 *
 *  Created on: 29.10.2014
 *      Author: helge
 */

#include "Interrupt.h"

Interrupt* Interrupt::owner[] = {0};

void Interrupt::handler1() {
	if(owner[0])
		owner[0]->serviceRoutine();
}

void Interrupt::handler2() {
	if(owner[1])
		owner[1]->serviceRoutine();
}

void Interrupt::handler3() {
	if(owner[2])
		owner[2]->serviceRoutine();
}

void Interrupt::handler4() {
	if(owner[3])
		owner[3]->serviceRoutine();
}

void Interrupt::handler5() {
	if(owner[4])
		owner[4]->serviceRoutine();
}

void Interrupt::record(uint8_t interruptNumber, Interrupt* i) {
	owner[interruptNumber - 1] = i;
}

//extern "C" void __cxa_pure_virtual() {
	//for(;;);
//}
