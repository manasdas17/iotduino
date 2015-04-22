/*
 * Interrupt.h
 *
 *  Created on: 29.10.2014
 *      Author: helge
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include <Arduino.h>

class Interrupt {
		static Interrupt* owner[5]; //mega has 5 external interrupts.

		virtual void serviceRoutine() = 0;

		static void handler1() __asm__("__vector_1") __attribute__((__signal__, __used__, __externally_visible__));
		static void handler2() __asm__("__vector_2") __attribute__((__signal__, __used__, __externally_visible__));
		static void handler3() __asm__("__vector_3") __attribute__((__signal__, __used__, __externally_visible__));
		static void handler4() __asm__("__vector_4") __attribute__((__signal__, __used__, __externally_visible__));
		static void handler5() __asm__("__vector_5") __attribute__((__signal__, __used__, __externally_visible__));

	public:
		static void record(uint8_t interruptNumber, Interrupt* i);
};

#endif /* INTERRUPT_H_ */
