/*
 * SpiRAM.h - Library for driving a 23k256 SPI attached SRAM chip
 *
 * Phil Stewart, 18/10/2009
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef SpiRAM_h
#define SpiRAM_h

#define SPIRAM_LIB_VERSION  "0.3.00"

#include <Arduino.h>
#include <SPI.h>


// SRAM opcodes
// #define WREN  6
// #define WRDI  4
#define RDSR  5
#define WRSR  1
#define READ  3
#define WRITE 2

// SRAM Hold line override
#define HOLD 1

// SRAM modes
#define BYTE_MODE (0x00 | HOLD)
#define PAGE_MODE (0x80 | HOLD)
#define STREAM_MODE (0x40 | HOLD)

// Clock speeds
#define RAMCLK4M   0
#define RAMCLK1M   1
#define RAMCLK250K 2
#define RAMCLK125K 3

class SpiRAM
{
  public:
	//define whether to shift in 3 or 2 bytes for an address
	enum addressLengthEnum{
		l16bit,
		l24bit
	};

    SpiRAM() {};

    SpiRAM(byte clock, byte ssPin, addressLengthEnum len);
    void enable();
    void disable();
    char read_byte(uint32_t address);
	/**
	 * from 23A1024/23LC1024 satasheet
	 * "Once the device is selected, the Write command can
	 * be started by issuing a WRITE instruction, followed by
	 * the 24-bit address, with the first seven MSB's of the
	 * address being a "don't care" bit, and then the data to be
	 * written."
	 *
	 * this means we are rewriting in a cyclic manner in case
	 * of too large addresses. This lib however does not ensure
	 * correct usage by now.
	 */
	void setAddress(uint32_t address);
    char write_byte(uint32_t address, char data_byte);
    void read_page(uint32_t address, char *buffer);
    void write_page(uint32_t address, char *buffer);
    void read_stream(uint32_t address, char *buffer, uint32_t length);
    void write_stream(uint32_t address, char *buffer, uint32_t length);
  private:
    char _current_mode;
    byte _ssPin;
    void _set_mode(char mode);

	addressLengthEnum addressLength;

};

extern SpiRAM SpiRam;

#endif
