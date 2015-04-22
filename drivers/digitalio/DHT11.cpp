/* 
* DHT11.cpp
*
* Created: 11.11.2014 20:17:06
* Author: helge
*/


#include "DHT11.h"

#define BITS_IN(object) (8 * sizeof((object)))

// Various named constants.
enum {
    /*
     * Time required to signal the DHT11 to switch from low power mode to
     * running mode.  18 ms is the minimal, add a few extra ms to be safe.
     */
    START_SIGNAL_WAIT = 20,

    /*
     * Once the start signal has been sent, we wait for a response.  The doc
     * says this should take 20-40 us, we wait 5 ms to be safe.
     */
    RESPONSE_WAIT =  5,

    /*
     * The time threshold between a 0 bit and a 1 bit in the response.  Times
     * greater than this (in ms) will be considered a 1; otherwise they'll be
     * considered a 0.
     */
    ONE_THRESHOLD = 40,

    /*
     * The number of bytes we expect from the sensor.  This consists of one
     * byte for the integral part of the humidity, one byte for the fractional
     * part of the humidity, one byte for the integral part of the temperature,
     * one byte for the fractional part of the temperature, and one byte for
     * a checksum.  The DHT11 doesn't capture the fractional parts of the
     * temperature and humidity, but it doesn't transmit data during those
     * times.
     */
    RESPONSE_SIZE =  5,

    /*
     * The number of bits in a bytes.
     */
    BITS_PER_BYTE =  8,

    /*
     * The 0-base most significant bit in a byte.
     */
    BYTE_MS_BIT =  7,

    /*
     * The index in the response where the humidity reading is stored.
     */
    HUMIDITY_INDEX =  0,

    /*
     * The index in the response where the temperature is stored.
     */
    TEMPERATURE_INDEX =  2,

    /*
     * The index in the response where the checksum is stored.
     */
    CHECKSUM_INDEX =  4,
};

DHT11::ReadStatus DHT11::read() {
	//if(lastRead > 0 && millis() - lastRead < minDelayMs) {
		//return ERROR_DELAY;
	//}
	lastRead = millis();
	
	
    uint8_t    buffer[RESPONSE_SIZE] = { 0 };
    uint8_t    bitIndex              = BYTE_MS_BIT;
    ReadStatus status                = OK;

    // Request sample
    pinMode(this->getPIN(), OUTPUT);
    digitalWrite(this->getPIN(), LOW);
    delay(START_SIGNAL_WAIT);

    // Wait for response
    digitalWrite(this->getPIN(), HIGH);
    delayMicroseconds(RESPONSE_WAIT);
    pinMode(this->getPIN(), INPUT);

    // Acknowledge or timeout
    // Response signal should first be low for 80us...
    if ((status = this->waitForPinChange(LOW)) != OK) {
        goto done;
    }

    // ... then be high for 80us ...
    if ((status = this->waitForPinChange(HIGH)) != OK) {
        goto done;
    }

    /*
     * ... then provide 5 bytes of data that include the integral part of the
     * humidity, the fractional part of the humidity, the integral part of the
     * temperature, the fractional part of the temperature, and a checksum
     * that is the sum of the integral parts of humidity and temperature.
     */
    for (size_t i = 0; i < BITS_IN(buffer); i++) {
        if ((status = this->waitForPinChange(LOW)) != OK) {
            goto done;
        }

        unsigned long highStart = micros();

        if ((status = this->waitForPinChange(HIGH)) != OK) {
            goto done;
        }

        // 26-28 us = 0, 50 us = 1.  40 us is a good threshold between 0 and 1
        if ((micros() - highStart) > ONE_THRESHOLD) {
            buffer[i / BITS_PER_BYTE] |= (1 << bitIndex);
        }

        // Decrement or reset bitIndex
        bitIndex = (bitIndex > 0) ? bitIndex - 1 : BYTE_MS_BIT;
    }

    // Check the checksum.  Only if it's good, record the new values.
    if (buffer[CHECKSUM_INDEX] == (  buffer[HUMIDITY_INDEX]
                                   + buffer[TEMPERATURE_INDEX])) {
        // buffer[1] and buffer[3] should be the fractional parts of the
        // humidity and temperature, respectively, but the DHT11 doesn't
        // provide those values, so we omit them here.
        this->humidity    = buffer[HUMIDITY_INDEX];
        this->temperature = buffer[TEMPERATURE_INDEX];
    } else {
        status = ERROR_CHECKSUM;
    }

done:
    return status;
}

boolean DHT11::implementsInterface(HardwareTypeIdentifier type) {
	if(type == HWType_humidity || type == HWType_temprature)
		return true;
	return false;
}

boolean DHT11::readVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
	if(result == NULL)
		return false;
	
	if(type == HWType_temprature) {
		readTemperature(result);
		return true;
	} else if(type == HWType_humidity) {
		readHumidity(result);
		return true;
	}
	
	return false;
}
