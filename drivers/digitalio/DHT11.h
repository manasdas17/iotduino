/*
* DHT11.h
*
* Created: 11.11.2014 20:17:06
* Author: helge
*/


#ifndef __DHT11_H__
#define __DHT11_H__

#include "DigitalIO.h"
#include "../../interfaces/input/Temperature.h"
#include "../../interfaces/input/Humidity.h"


class DHT11 : public DigitalIO, public Temperature, public Humidity {
	// The last read humidity value
	int humidity;

	// The last read temperature value
	int temperature;

	private:
		virtual boolean writeVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
			return false;
		}

	public:

		int readHumidity() {
			read();
			return this->getHumidity();
		}

		void readHumidity( HardwareCommandResult* hwresult ) {
			hwresult->setUintListNum(1);
			hwresult->getUintList()[0] = readHumidity();
		}


		int readTemperature() {
			read();
			return this->getTemperature();
		}

		void readTemperature( HardwareCommandResult* hwresult ) {
			hwresult->setUintListNum(1);
			hwresult->getUintList()[0] = readTemperature();
		}

		/*
		 * Dht11
		 *
		 * Constructs a new Dht11 object that communicates with a DHT11 sensor
		 * over the given pin.
		 * @param pin
		 * @param address
		 */
		DHT11(uint8_t newPin, uint8_t hwaddress) : DigitalIO(newPin, false, hwaddress) {
			humidity = -127;
			temperature = -127;

			lastRead = 0;
			minDelayMs = 200;
		}


//	protected:
		// An enumeration modeling the read status of the sensor.
		enum ReadStatus {
			OK,
			ERROR_CHECKSUM,
			ERROR_TIMEOUT,
			ERROR_DELAY,
		};

		/*
		 * read
		 *
		 * Update the humidity and temperature of this object from the sensor.
		 * Returns OK if the update was successful, ERROR_TIMEOUT if it times out
		 * waiting for a response from the sensor, or ERROR_CHECKSUM if the
		 * calculated checksum doesn't match the checksum provided by the sensor.
		 */
		ReadStatus read();


	protected:
		uint16_t minDelayMs;
		uint16_t lastRead;

		/*
		 * getHumidity
		 *
		 * Gets the last read relative humidity percentage.
		 */
		inline int getHumidity() const {
			return this->humidity;
		}

		/*
		 * getTemperature
		 *
		 * Gets the last read temperature value in degrees Celsius.
		 */
		inline int getTemperature() const {
			return this->temperature;
		}

//	private:
		enum {
			/*
			 * Default value for the maximum number of iterations performed by
			 * the waitForPinChange function.
			 */
			MAX_PIN_CHANGE_ITERATIONS = 10000,
		};


		/*
		 * waitForPinChange
		 *
		 * Wait for the the data pin on the sensor to change from the given oldValue
		 * to the opposite value (i.e., from HIGH -> LOW or from LOW -> HIGH).  The
		 * function performs a tight loop decrementing the given maxIterations.  If
		 * the state of the pin changes before maxIterations reaches 0, the function
		 * returns OK; otherwise it returns ERROR_TIMEOUT.
		 *
		 * This is a private method used only by the Dht11 class.
		 */
		inline ReadStatus waitForPinChange(const int oldValue,
										   unsigned  maxIterations =
												  MAX_PIN_CHANGE_ITERATIONS) {
			while ((--maxIterations > 0) && (digitalRead(this->getPIN()) == oldValue)) {
				// Just keep looping...
			}

			return (maxIterations > 0) ? OK : ERROR_TIMEOUT;
		}


	public:
		virtual boolean implementsInterface( HardwareTypeIdentifier type );

		virtual HardwareTypeIdentifier* getImplementedInterfaces(HardwareTypeIdentifier* arr, uint8_t maxLen) {
			arr = DigitalIO::getImplementedInterfaces(arr, maxLen);
			arr = this->addImplementedInterface(arr, maxLen, HWType_humidity);
			return this->addImplementedInterface(arr, maxLen, HWType_temprature);
		}


		/**
		 * @param type to read
		 * @param result object to fill data with
		 *        - uintlist[0] contains the value
		 * @result success
		 */
		virtual boolean readVal( HardwareTypeIdentifier type, HardwareCommandResult* result );
}; //DHT11

#endif //__DHT11_H__
