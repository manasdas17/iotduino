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

#define resultSetSize 10
#define driverPointerListSize 5
class HardwareInterface {
	private:
		//const static resultSetSize = 10;
		//const static driverPointerListSize = 5;

		HardwareCommandResult resultset[resultSetSize];
		boolean resultSetInUse[resultSetSize];

		HardwareDriver* driver[driverPointerListSize];

	protected:
		/**
		 * return the index of a free result set
		 * @return index, 0xff if there is none.
		 */
		uint8_t getFreeResultIndex() const;

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
		HardwareCommandResult* readHardware( HardwareDriver* driver, HardwareCommandResult* cmd );

		/**
		 * write hardware
		 * @param driver
		 * @param command
		 */
		HardwareCommandResult* writeHardware( HardwareDriver* driver, HardwareCommandResult* cmd );

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
		HardwareCommandResult* executeCommand(HardwareCommandResult* cmd);

		/**
		 * free the hardware result set
		 * @param set
		 */
		boolean releaseHardwareCommandResultEntry(HardwareCommandResult* ptr);

		/**
		 * get an unused resultset
		 * @return set
		 */
		HardwareCommandResult* getFreeHardwareCommandResultEntry();

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
