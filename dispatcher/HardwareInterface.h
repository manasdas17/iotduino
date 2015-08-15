/*
* HardwareInterface.h
*
* Created: 13.02.2015 09:01:57
* Author: helge
*/


#ifndef __HARDWAREINTERFACE_H__
#define __HARDWAREINTERFACE_H__

#include <Arduino.h>
#include <drivers/HardwareDriver.h>
#include <drivers/HardwareID.h>
#include <dispatcher/Commands.h>

#define driverPointerListSize 6
class HardwareInterface {
	private:
		//const static driverPointerListSize = 5;

		HardwareDriver* driver[driverPointerListSize];

	protected:
		/**
		 * check for a certain hardware driver
		 * @param type
		 */
		boolean hasHardwareDriver(HardwareTypeIdentifier type) const;

		/**
		 * get a driver according to filter settings
		 * @param type
		 */
		HardwareDriver* getHardwareDriver(HardwareTypeIdentifier type) const;

		/**
		 * read hardware
		 * @param driver
		 * @param command
		 */
		boolean readHardware( HardwareDriver* driver, HardwareCommandResult* cmd );

		/**
		 * write hardware
		 * @param driver
		 * @param command
		 */
		boolean writeHardware( HardwareDriver* driver, HardwareCommandResult* cmd );

	public:
		/**
		 * get a driver according to filter settings
		 * @param type
		 * @param address
		 */
		HardwareDriver* getHardwareDriver(const HardwareTypeIdentifier type, const uint8_t address) const;

		/**
		 * constructor
		 */
		HardwareInterface();

		/**
		 * destructor
		 */
		~HardwareInterface();

		/**
		 * register a new driver
		 * @param driver
		 */
		boolean registerDriver(HardwareDriver* driver);

		/**
		 * execute a command according to internal information
		 * @param command
		 */
		boolean executeCommand(HardwareCommandResult* cmd);

		/**
		 * @return list of pointers to driver instances
		 */
		inline HardwareDriver** getHardwareDrivers() {
			return driver;
		}

		/**
		 * get the maximum size of the driver pointer list size
		 */
		inline uint8_t getHardwareDriversListSize() const {
			return driverPointerListSize;
		}

		/**
		 * check for a certain hardware driver
		 * @param type
		 * @param address
		 */
		boolean hasHardwareDriver(const HardwareTypeIdentifier type, const uint8_t address) const;

};

#endif //__HARDWAREINTERFACE_H__
