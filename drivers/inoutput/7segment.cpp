/*
 * _7segment.cpp
 *
 * Created: 02.01.2015 03:32:35
 *  Author: helge
 */
#include <Arduino.h>

#define SHIFT_RIGHT 1
#define NUM_ELEMENTS 4
#define INVERT 1
#define PIN_A
#define PIN_B
#define PIN_C
#define PIN_D
#define PIN_E
#define PIN_F
#define PIN_G

/*        +---u---+
 *   S_0 -|       |- V_CC
 * ~OE_1 -|   7   |- S_1
 * ~OE_2 -|   4   |- D_SL
 * I/O_6 -|   H   |- Q7
 * I/O_4 -|   C   |- I/O_7
 * I/O_2 -|   2   |- I/O_5
 * I/O_0 -|   9   |- I/O_3
 *   Q_0 -|   9   |- I/O_1
 *   ~MR -|       |- CP
 *   GND -|       |- D_SR
 *        +-------+
 */

class sevenSegment {
	public:
		/** shift direction */
		boolean shiftRight;
		/** number of attached elements */
		uint8_t numElements;
		/** invert the ouput */
		boolean invert;
		/** mapping digit part A, B, C, D, E, F, G, DP -> IO PIN shift register */
		int8_t pinConfig[8] = {0, 1, 2, 3, 4, 5, 6, 7};
		/** CLK PIN */
		uint8_t pinClk;
		/** DATA PIN */
		uint8_t pinData;

		/** default construtor, initiales defaults. */
		sevenSegment(uint8_t clk, uint8_t data, uint8_t numElements) {
			this->shiftRight = true;
			this->numElements = numElements;
			this->invert = true;
			this->pinClk = clk;
			this->pinData = data;

			pinMode(pinClk, OUTPUT);
			pinMode(pinData, OUTPUT);
		}

		/** clear display */
		void clear() {
			for(uint8_t i = 0; i < numElements; i++) {
				writeChar(0);
			}
		}

		boolean write(uint8_t num) {
			if(num < 17) {
				writeChar(num);
				return true;
			}
			return false;
		}

	private:
		/** write a char to the display */
		boolean writeChar(uint8_t id) {
			if(id >= 17)
				return false;

			//create val from pin config mapping
			uint8_t val = 0;
			uint8_t tmp = 0;
			for(uint8_t i = 0; i < 8; i++) {
				tmp = i;
				if(!shiftRight) {
					tmp = 7 - tmp;
				}
				val |= ((charConfig[id] & _BV(tmp)) >> tmp) << pinConfig[tmp];
			}

			if(invert) {
				val = ~val;
			}

			//shit out
			shiftOut(pinData, pinClk, LSBFIRST, val);
			return true;
		}

		/** output config for chars */
		byte charConfig[17] = {
			//xGFEDCBA
			0b00111111, //0
			0b00000110, //1
			0b01011011, //2
			0b01001111, //3
			0b01100110, //4
			0b01101101, //5
			0b01111101, //6
			0b00000111, //7
			0b01111111, //8
			0b01101111, //9
			0b01110111, //A
			0b01111100, //B
			0b00111001, //C
			0b01011110, //D
			0b01111001, //E
			0b01110001, //F
			0b00000000 //clear
			};
};