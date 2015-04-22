/*
 * IRreceiveATtiny85.h
 *
 * Created: 10.11.2014 21:26:27
 *  Author: helge
 */ 


#ifndef IRRECEIVEATTINY85_H_
#define IRRECEIVEATTINY85_H_

#include <Arduino.h>
#include "../../interfaces/input/IRreceive.h"
#include "../i2c/I2C.h"
#include "../../utils/Queue.h"

#define CMD_RESET 0x10
#define CMD_PEEK 0x20
#define CMD_POP 0x30
#define CMD_POP_ALL 0x40
#define CMD_GET_NUM_READINGS 0x50
#define CMD_OK 0xf0
#define CMD_ERROR 0xf1
#define MAX_WAIT_FOR_ANSWER_MILLIS 200

class IRreceiverATtiny85 : public I2C, public IRreceive {
	private:
		Queue<uint32_t> queue;
		
		//blocking call.
		boolean waitUntilAvailable(uint8_t milliseconds) {
			uint16_t start = millis();
			
			while(millis() - start < milliseconds) {
				if(Wire.available())
					return true;
			}
			return false;
		}
	public:
		IRreceiverATtiny85() {
			this->address = 0x04;
			Wire.begin();
		}
		
		void pollSensor() {
			Wire.beginTransmission(this->address);
			Wire.write(CMD_POP_ALL);
			Wire.endTransmission();
			Wire.requestFrom(this->address, 2);
				
			//get answer
			if(!waitUntilAvailable(MAX_WAIT_FOR_ANSWER_MILLIS)) return;
			uint8_t tmp = Wire.read();
				
			//should be OK
			if(tmp == CMD_OK) {
				//get number of available bytes
				if(!waitUntilAvailable(MAX_WAIT_FOR_ANSWER_MILLIS)) return;
				tmp = Wire.read();
						
				//are there available ones? - we are looking for uint32...
				if(tmp > 0 && tmp % 4 == 0) {
					//request the number of bytes for reading.
					Wire.requestFrom(address, tmp);
						
					//construct the uint - lsb first.
					uint32_t reading = 0;
					for(uint8_t i = 0; i < tmp / 4; i++) {
						reading = Wire.read();
						reading |= ((uint16_t) Wire.read()) << 8;
						reading |= ((uint32_t) Wire.read()) << 16;
						reading |= ((uint32_t) Wire.read()) << 24;
						queue.push(reading);
					}
						
				}
			}
		}
		
		virtual uint8_t availabe() {
			pollSensor();
			return queue.size();
		}
	
		virtual uint32_t peek() {
			if(availabe())
				return queue.peek();
			return 0;
		}
	
		virtual uint32_t pop() {
			if(availabe())
				return queue.pop();
			return 0;
		}

		virtual void pop( HardwareResult* hwresult ) {
			if(!availabe()) return;
			
			hwresult->setUintListNum(4);
			uint8_t* list = hwresult->getUintList();
			
			uint32_t val = pop();
			list[0] = val >> 24;
			list[1] = val >> 16;
			list[2] = val >> 8;
			list[3] = val 0xff;
		}

		virtual boolean readVal( HardwareTypeIdentifier type, HardwareResult* result ) {
			if(implementsInterface(type))
				return pop(result);
			return false;
		}

		virtual boolean implementsInterface( HardwareTypeIdentifier type ) {
			if(type == HWType_ir)
				return true;
			return false;
		}
};


#endif /* IRRECEIVEATTINY85_H_ */