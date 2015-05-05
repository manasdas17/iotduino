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
		virtual uint32_t checkForEvent(subscription_event_type_t type) {
			return 0;
		}

		/**
		 * check for an event of a uint8
		 * the logic should be quite similar for all hardware, thus it is put here.
		 * @param val_old
		 * @param val_new
		 * @param type
		 * @return eventDetected
		 */
		template<typename T>
		const static subscription_event_type_t checkForEvent(T val_old, T val_new) {
			if(val_old > val_new) {
				return EVENT_TYPE_EDGE_FALLING;
			}

			if(val_old < val_new) {
				return EVENT_TYPE_EDGE_RISING;
			}

			return EVENT_TYPE_DISABLED;
		}

		/**
		 * check is an event has a certain type
		 * this includes type CHANGE which is are more generic typ of EDGE_RISING and EDGE_FALLING
		 * @param type detected event
		 * @param reference event for comparison
		 * @return
		 */
		const static boolean isEvent(subscription_event_type_t type, subscription_event_type_t reference) {
			if(reference == type) {
				return true;
			}

			if(reference == EVENT_TYPE_CHANGE && type == EVENT_TYPE_EDGE_RISING) {
				return true;
			}

			if(reference == EVENT_TYPE_CHANGE && type == EVENT_TYPE_EDGE_FALLING) {
				return true;
			}

			return false;
		}
};


#endif /* EVENTDETECTOR_H_ */