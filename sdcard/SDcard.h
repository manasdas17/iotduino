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

};

#endif /* SDCARD_H_ */