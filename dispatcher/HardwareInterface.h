/* 
* HardwareInterface.h
*
* Created: 13.02.2015 09:01:57
* Author: helge
*/


#ifndef __HARDWAREINTERFACE_H__
#define __HARDWAREINTERFACE_H__

#include <Arduino.h>
#include <HardwareDriver.h>
#include <HardwareID.h>
#include <dispatcher/Commands.h>
#include "Commands.h"

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
		 *
		 */
		uint8_t getFreeResultIndex();

		/**
		 * check for a certain hardware driver
		 * @param type
		 */
		boolean hasHardwareDriver(HardwareTypeIdentifier type);

		/**
		 * check for a certain hardware driver
		 * @param type
		 * @param address
		 */
		boolean hasHardwareDriver(HardwareTypeIdentifier type, uint8_t address);

		/**
		 * get a driver according to filter settings
		 * @param type
		 */
		HardwareDriver* getHardwareDriver(HardwareTypeIdentifier type);

		/**
		 * get a driver according to filter settings
		 * @param type
		 * @param address
		 */
		HardwareDriver* getHardwareDriver(HardwareTypeIdentifier type, uint8_t address);

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
		 * constructor
		 */
		HardwareInterface() {
			memset(resultSetInUse, 0, sizeof(resultSetInUse));
			memset(driver, 0, sizeof(driver));
		}
		
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
};

#endif //__HARDWAREINTERFACE_H__
