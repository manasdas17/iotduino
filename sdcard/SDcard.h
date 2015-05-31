/*
 * SDcard.h
 *
 * Created: 31.05.2015 10:32:51
 *  Author: helge
 */


#ifndef SDCARD_H_
#define SDCARD_H_

#include <Arduino.h>
#include <DebugConfig.h>
#include <SD/SD.h>
#include <dispatcher/EventCallbackInterface.h>
#include <dispatcher/Commands.h>

#define PIN_SD_SS 4

class SDcard {
	private:
		/** working object */
		File myFile;

		/** modes */
		enum FileMode {READ = FILE_READ, WRITE = FILE_WRITE};

	public:
		/** empty constructor*/
		SDcard();

		/**
		 * initialiser must be called before using SDcard functions
		 * @return success
		 */
		boolean init();

		/**
		 * append data to file
		 * @param fileName
		 * @param buf
		 * @param len
		 * @return success
		 */
		boolean appendToFile(char* fileName, uint8_t* buf, uint8_t bufSize);

		//deinition - 16 bytes info for each node
		#define NODE_INFO_SIZE (0x10)
		/**
		 * get node info from file.
		 * structure:
		 * Node 0:   0x00000000..0x00000000f [unused]
		 * Node 1:   0x00000010..0x00000001f
		 *  ...
		 * Node 255: 0x00000ff0..0x000000fff[broadcast]
		 */
		uint8_t getNodeInfo(uint8_t nodeId, uint8_t* buf, uint8_t bufSize);

	protected:
		/**
		 * internal method for opening a file
		 * @param filName
		 * @param mode
		 * @return success
		 */
		boolean openFile(char* fileName, FileMode mode);
};
#endif /* SDCARD_H_ */