/*
 * EventDetector.h
 *
 * Created: 03.05.2015 22:16:51
 *  Author: helge
 */


#ifndef EVENTDETECTOR_H_
#define EVENTDETECTOR_H_

#include <Arduino.h>
#include <dispatcher/Commands.h>

class EventDetector {
	/** data of last event */
	HardwareCommandResult lastResult;

	/** timestamp of last event */
	uint32_t lastResultTimestamp;

	protected:
		/**
		 * @param result
		 */
		virtual void updateResult(const HardwareCommandResult* result) {
			updateTimestamp();
			memcpy(&lastResult, result, sizeof(HardwareCommandResult));
		}

		virtual void updateTimestamp() {
			this->lastResultTimestamp = millis();
		}

		/**
		 * @return result
		 */
		virtual HardwareCommandResult* getLastResult() {
			return &lastResult;
		}

		/**
		 * @return t
		 */
		virtual const uint32_t getLastResultTimestamp() const {
			return lastResultTimestamp;
		}

	public:
		EventDetector() {
			memset(&lastResult, 0, sizeof(lastResult));
			lastResultTimestamp = 0;
		}

		/**
		 * indicator for the capability of detecting events
		 * @return capability
		 */
		virtual boolean canDetectEvents() {
			return false;
		}

		/**
		 * @return true in case of event, false otherwise
		 */
		virtual uint32_t checkForEvent() {
			return 0;
		}
};


#endif /* EVENTDETECTOR_H_ */