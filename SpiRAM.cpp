/*
 * SpiRAM.cpp - Library for driving a 23k256 SPI attached SRAM chip
 *
 * Phil Stewart, 18/10/2009
 *
 * Copyright (c) 2009, Phil Stewart
 * All rights reserved.
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
/*
 * Updated to Arduino 1.0.5 and newer version of SPI
 * "2010 by Cristian Maglie"
 * by Fred Jan Kraan, fjkraan@xs4all.nl, 2014-02-28
 */

#include <SPI.h>
#include <SpiRAM.h>

// Constructor

SpiRAM::SpiRAM(byte clockDiv, byte ssPin, addressLengthEnum len)
{
  switch(len) {
	  case l24bit:
		this->addressLength = l24bit;
		break;
	  case l16bit:
	  default:
		this->addressLength = l16bit;
		break;
  }

  SPI.begin();
  // Ensure the RAM chip is disabled in the first instance
  disable();

  // Set the spi mode using the requested clock speed
//  SPI.mode(clock);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  _ssPin = ssPin;
  pinMode(_ssPin, OUTPUT);

  // Set the RAM operarion mode flag according to the chip default
  _current_mode = RAM_BYTE_MODE;
}

// Enable and disable helper functions

void SpiRAM::enable()
{
  digitalWrite(_ssPin, LOW);
}

void SpiRAM::disable()
{
  digitalWrite(_ssPin, HIGH);
}

// Byte transfer functions

char SpiRAM::read_byte(uint32_t address)
{
  char read_byte;

  // Set byte mode
  _set_mode(RAM_BYTE_MODE);

  // Write address, read data
  enable();
  SPI.transfer(RAM_READ);

  setAddress(address);

  read_byte = SPI.transfer(0xFF);
  disable();

  return read_byte;
}

char SpiRAM::write_byte(uint32_t address, char data_byte)
{
  // Set byte mode
  _set_mode(RAM_BYTE_MODE);

  // Write address, read data
  enable();
  SPI.transfer(RAM_WRITE);
  setAddress(address);
  SPI.transfer(data_byte);
  disable();

  return data_byte;
}

// Page transfer functions. Bound to current page. Passing the boundary
//  will wrap to the beginning
void SpiRAM::read_page(uint32_t address, char *buffer)
{
  int i;

  // Set byte mode
  _set_mode(RAM_PAGE_MODE);

  // Write address, read data
  enable();
  SPI.transfer(RAM_READ);
  setAddress(address);
  for (i = 0; i < 32; i++) {
    buffer[i] = SPI.transfer(0xFF);
  }
  disable();
}

void SpiRAM::write_page(uint32_t address, char *buffer)
{
  int i;

  // Set byte mode
  _set_mode(RAM_PAGE_MODE);

  // Write address, read data
  enable();
  SPI.transfer(RAM_WRITE);
  setAddress(address);
  for (i = 0; i < 32; i++) {
    SPI.transfer(buffer[i]);
  }
  disable();
}

// Stream transfer functions. Ignores page boundaries.
void SpiRAM::read_stream(uint32_t address, char *buffer, uint32_t length)
{
  uint32_t i;

  // Set byte mode
  _set_mode(RAM_STREAM_MODE);

  // Write address, read data
  enable();
  SPI.transfer(RAM_READ);
  setAddress(address);
  for (i = 0; i < length; i++) {
    buffer[i] = SPI.transfer(0xFF);
  }
  disable();
}

void SpiRAM::write_stream(uint32_t address, char *buffer, uint32_t length)
{
  uint32_t i;

  // Set byte mode
  _set_mode(RAM_STREAM_MODE);

  // Write address, read data
  enable();
  SPI.transfer(RAM_WRITE);
  setAddress(address);
  for (i = 0; i < length; i++) {
    SPI.transfer(buffer[i]);
  }
  disable();
}


// Mode handling
void SpiRAM::_set_mode(char mode)
{
  if (mode != _current_mode)
  {
    enable();
    SPI.transfer(RAM_WRSR);
    SPI.transfer(mode);
    disable();
    _current_mode = mode;
  }
}

void SpiRAM::setAddress(uint32_t address) {
	switch(addressLength) {
		case l24bit:
		  SPI.transfer((char)(address >> 16));
		case l16bit:
		default:
		  SPI.transfer((char)(address >> 8));
		  SPI.transfer((char)address);
	  }
}


// Preinstantiate SpiRAM object;
//SpiRAM SpiRam = SpiRAM(RAMCLK1M, 10);
