/*
 * RequestContent.cpp
 *
 * Created: 14.06.2015 23:31:06
 *  Author: helge
 */
#include <webserver/RequestContent.h>

 RequestContent::RequestContent() {
	num = 0;
	memset(keys, 0, sizeof(keys));
	memset(values, 0, sizeof(values));
}

int8_t RequestContent::putKey(String key, String value) {
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

int8_t RequestContent::hasKey(String key) {
	key.toLowerCase();
	for(uint8_t i = 0; i < num; i++) {
		if(keys[i].compareTo(key) == 0)
			return i;
	}

	return -1;
}

int8_t RequestContent::hasKey(PGM_P key) {
	char buf[32];
	strcpy_P(buf, key);

	return hasKey(String(buf));
}

String* RequestContent::getValue(PGM_P key) {
	int8_t index = hasKey(key);
	if(index == -1)
		return NULL;

	return &values[index];
}

String* RequestContent::getValue(String key) {
	int8_t index = hasKey(key);

	if(index == -1)
		return NULL;

	return &values[index];
}

void RequestContent::parse(String requestContent) {
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
