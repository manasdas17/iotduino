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

	/** keys */
	String keys[RequestContentNumKeys];
	/** vals */
	String values[RequestContentNumKeys];
	/** num used */
	uint8_t num;

	/** initialise object */
	RequestContent() {
		num = 0;
		memset(keys, 0, sizeof(keys));
		memset(values, 0, sizeof(values));
	}

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
	int8_t putKey(String key, String value) {
		int8_t index = hasKey(key);

		if(index == -1) {
			index = num;

			if(num >= RequestContentNumKeys)
				return -1;
			keys[index] = key;
			num++;
		}

		values[index] = value;
		return index;
	}

	int8_t hasKey(PGM_P key) {
		char buf[32];
		strcpy_P(buf, key);

		return hasKey(String(buf));
	}

	int8_t hasKey(String key) {
		key.toLowerCase();
		for(uint8_t i = 0; i < num; i++) {
			if(keys[i].compareTo(key) == 0)
				return i;
		}

		return -1;
	}

	String* getValue(PGM_P key) {
		int8_t index = hasKey(key);
		if(index == -1)
			return NULL;

		return &values[index];

	}

	String* getValue(String key) {
		int8_t index = hasKey(key);

		if(index == -1)
			return NULL;

		return &values[index];
	}

	/**
	 * parse a x-www-form-urlencoded string
	 * @param requestContent
	 */
	void parse(String requestContent) {
		num = 0;

		char buf[STRING_BUFFER_SIZE];
		requestContent.toCharArray(buf, STRING_BUFFER_SIZE);
		char* tok = strtok(buf, "&");
		while(tok != NULL) {
			String kvPair = String(tok);

			String v = "";
			String k = "";
			if(kvPair.indexOf("=") != -1) {
				k = kvPair.substring(0, kvPair.indexOf("="));
				v = kvPair.substring(kvPair.indexOf("=")+1);
			} else {
				k = kvPair;
			}

			putKey(k, v);

			tok = strtok(NULL, "&");
		}
	}

};

#endif /* REQUESTCONTENT_H_ */