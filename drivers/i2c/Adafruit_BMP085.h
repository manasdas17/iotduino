/***************************************************
  This is a library for the BMP085 Barometric Pressure & Temp Sensor

  Designed specifically to work with the Adafruit BMP085 Breakout
  ----> https://www.adafruit.com/products/391

  These displays use I2C to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Arduino.h>
#include <Wire.h>
#include "I2C.h"
#include "../../interfaces/input/Pressure.h"
#include "../../interfaces/input/Temperature.h"

#define BMP085_DEBUG 0

#define BMP085_I2CADDR 0x77

#define BMP085_ULTRALOWPOWER 0
#define BMP085_STANDARD      1
#define BMP085_HIGHRES       2
#define BMP085_ULTRAHIGHRES  3
#define BMP085_CAL_AC1           0xAA  // R   Calibration data (16 bits)
#define BMP085_CAL_AC2           0xAC  // R   Calibration data (16 bits)
#define BMP085_CAL_AC3           0xAE  // R   Calibration data (16 bits)
#define BMP085_CAL_AC4           0xB0  // R   Calibration data (16 bits)
#define BMP085_CAL_AC5           0xB2  // R   Calibration data (16 bits)
#define BMP085_CAL_AC6           0xB4  // R   Calibration data (16 bits)
#define BMP085_CAL_B1            0xB6  // R   Calibration data (16 bits)
#define BMP085_CAL_B2            0xB8  // R   Calibration data (16 bits)
#define BMP085_CAL_MB            0xBA  // R   Calibration data (16 bits)
#define BMP085_CAL_MC            0xBC  // R   Calibration data (16 bits)
#define BMP085_CAL_MD            0xBE  // R   Calibration data (16 bits)

#define BMP085_CONTROL           0xF4
#define BMP085_TEMPDATA          0xF6
#define BMP085_PRESSUREDATA      0xF6
#define BMP085_READTEMPCMD          0x2E
#define BMP085_READPRESSURECMD            0x34


class Adafruit_BMP085 : public I2C, public Pressure, public Temperature {
 public:
  Adafruit_BMP085() {};

  boolean begin(uint8_t mode = BMP085_ULTRAHIGHRES);  // by default go highres

void readTemperature(HardwareCommandResult* hwresult);


void readPressure(HardwareCommandResult* hwresult);


void init(uint8_t hwaddress);

boolean implementsInterface(HardwareTypeIdentifier type);

boolean readVal(HardwareTypeIdentifier type, HardwareCommandResult* result);


protected:
 void getAltitude(HardwareCommandResult* hwresult) {};
  uint16_t readPressure(void);
  uint16_t readRawTemperature(void);
  uint32_t readRawPressure(void);
  int8_t readTemperature(void);

 private:
  uint8_t read8(uint8_t addr);
  uint16_t read16(uint8_t addr);
  void write8(uint8_t addr, uint8_t data);

  uint8_t oversampling;

  int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
  uint16_t ac4, ac5, ac6;


	boolean writeVal(HardwareTypeIdentifier type, HardwareCommandResult* result) { return false; }
	uint16_t getAltitude() { return -1; }
  float readAltitude(float sealevelPressure = 101325); // std atmosphere

virtual HardwareTypeIdentifier* getImplementedInterfaces(HardwareTypeIdentifier* arr, uint8_t maxLen);

};

