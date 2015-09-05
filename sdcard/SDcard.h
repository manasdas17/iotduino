/*
 * SDcard.h
 *
 * Created: 31.05.2015 10:32:51
 *  Author: helge
 */


#ifndef SDCARD_H_
#define SDCARD_H_

#include <Arduino.h>
#include <SD.h>

/** SS pin for SD reader */
#define PIN_SD_SS 4

class SDcard {
	//private:
	public:

		/**
		 * initialiser must be called before using SDcard functions
		 * @return success
		 */
		boolean initSD() {
			if (!SD.begin(PIN_SD_SS)) {
				#ifdef DEBUG_SD_ENABLE
				Serial.print(millis());
				Serial.println(F(": init failed"));
				#endif
				return false;
			}
			#ifdef DEBUG_SD_ENABLE
			Serial.print(millis());
			Serial.println(F(": sd init done"));
			#endif
		}

		static boolean fillFile(File* fd, uint8_t val, uint32_t destinationSize) {
			if(!fd)
				return false;

			uint8_t buflen = 64;
			uint8_t buf[buflen];
			memset(&buf, val, buflen);

			while(fd->size() < destinationSize) {
				if(destinationSize - fd->size() > buflen) {
					if(fd->write(buf, buflen) != buflen)
						return false;
				} else {
					if(!fd->write(val)) {
						return false;
					}
				}
			}

			fd->flush();
			return true;

		}

};

#endif /* SDCARD_H_ */