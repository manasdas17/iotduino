/*
 * RequestContent.h
 *
 * Created: 11.06.2015 21:26:21
 *  Author: helge
 */


#ifndef REQUESTCONTENT_H_
#define REQUESTCONTENT_H_

#include <Arduino.h>
#include <avr/pgmspace.h>

#define STRING_BUFFER_SIZE 128

/**
 * simple map.
 */
class RequestContent {
	/** max number of keys*/
	#define RequestContentNumKeys 10

	public:
	boolean mobileDevice = false;

	/** keys */
	String keys[RequestContentNumKeys];
	/** vals */
	String values[RequestContentNumKeys];
	/** num used */
	uint8_t num;

	/** initialise object */
	RequestContent();

	inline uint8_t getNum() {
		return num;
	}

	inline String* getKeys() {
		return keys;
	}

	inline String* getValues() {
		return values;
	}

	/**
	 * @pram key
	 * @pram value
	 * @return index -1 if full
	 */
	int8_t putKey(String key, String value);

	int8_t hasKey(PGM_P key);

	int8_t hasKey(String key);

	String* getValue(PGM_P key);

	String* getValue(String key);

	/**
	 * parse a x-www-form-urlencoded string
	 * @param requestContent
	 */
	void parse(String requestContent);

};

#endif /* REQUESTCONTENT_H_ */