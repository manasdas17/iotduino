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

	uint32_t lastEventTimestmap;
	subscription_event_type_t lastEventType;

	protected:
		/**
		 * @param result
		 */
		virtual void updateResult(const HardwareCommandResult* result, subscription_event_type_t type) {
			memcpy(&lastResult, result, sizeof(HardwareCommandResult));
			lastEventType = type;
			lastEventTimestmap = millis();
		}

		/**
		 * @return result
		 */
		virtual HardwareCommandResult* getLastResult() {
			return &lastResult;
		}

		/**
		 * check for an event of a uint8
		 * the logic should be quite similar for all hardware, thus it is put here.
		 * @param newReading
		 * @param val_old
		 * @param val_new
		 * @return eventDetected
		 */
		template<typename T>
		subscription_event_type_t checkForEvent(HardwareCommandResult* newReading, T val_old, T val_new) {
			//check is we have a new event
			subscription_event_type_t type = EVENT_TYPE_DISABLED;

			if(val_old > val_new) {
				type = EVENT_TYPE_EDGE_FALLING;
			} else if(val_old < val_new) {
				type = EVENT_TYPE_EDGE_RISING;
			}

			//update
			if(type != EVENT_TYPE_DISABLED) {
				updateResult(newReading, type);
				#ifdef DEBUG_HARDWARE_ENABLE
					Serial.print(millis());
					Serial.print(F(":EventDetector::CheckForEvent() eventTypeDetected="));
					Serial.println(type);
					Serial.flush();
				#endif
			}

			return type;
		}

	public:
		EventDetector() {
			memset(&lastResult, 0, sizeof(lastResult));
			lastEventType = EVENT_TYPE_DISABLED;
			lastEventTimestmap = 0;
		}

		/**
		 * actual event detection loop
		 * @return detected event type
		 */
		subscription_event_type_t eventLoop() {
			return EVENT_TYPE_DISABLED;
		}


		/**
		 * indicator for the capability of detecting events
		 * @return capability
		 */
		virtual boolean canDetectEvents() {
			return false;
		}

		/**
		 * @return t
		 */
		virtual const uint32_t getLastEventTimestamp() const {
			return lastEventTimestmap;
		}

		/**
		 * @return type
		 */
		virtual const subscription_event_type_t getLastEventType() const {
			return lastEventType;
		}

		/**
		 * check if we have a "virtual" CHANGE event due to a RISING or FALLING edge event
		 * @return comparison success
		 */
		const boolean lastEventMatchesEventType(subscription_event_type_t type) {
			subscription_event_type_t lastType = getLastEventType();

			if(type == EVENT_TYPE_EDGE_RISING && lastType == EVENT_TYPE_EDGE_RISING)
				return true;
			if(type == EVENT_TYPE_EDGE_FALLING && lastType == EVENT_TYPE_EDGE_FALLING)
				return true;
			if(type == EVENT_TYPE_CHANGE && (lastType == EVENT_TYPE_EDGE_RISING || lastType == EVENT_TYPE_EDGE_FALLING))
				return true;
			//if(lastType == EVENT_TYPE_DISABLED)
				//return false;
			//if(type == EVENT_TYPE_DISABLED)
				//return false;
			return false;
		}
};


#endif /* EVENTDETECTOR_H_ */