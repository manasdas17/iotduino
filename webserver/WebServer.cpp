/*
 * WebServer.cpp
 *
 * Created: 05.09.2015 14:23:16
 *  Author: helge
 */

#include <webserver/WebServer.h>

void WebServer::init() {
	for(uint8_t i = 0; i < CLIENT_INSTANCES_NUM; i++) {
		clientStatus[i].inUse = false;
		clientStatus[i].callback = NULL;
		clientStatus[i].requestType = PAGE_NONE;
		clientStatus[i].waiting = false;
	}

	#ifdef USE_DHCP_FOR_IP_ADDRESS
	Ethernet.begin(mac);  // Use DHCP to get an IP address
	#else
	// ip represents the fixed IP address to use if DHCP is disabled.
	Ethernet.begin(mac, ip);
	#endif
	server.begin();

	listenerHardwareRequest.init(0, HWType_UNKNOWN, 0, webserverListener::START);
	listenerDiscovery.init(0, webserverListener::START);
}

void WebServer::serialPrintP(const char* str) {
	if(str == NULL)
		return;

	char buf[120];
	strcpy_P(buf, str);
	Serial.print(buf);
}

WebServer::MethodType WebServer::readHttpRequest(EthernetClient* client, String* uri, RequestContent* req) {
	BUFFER requestContent;
	BUFFER readBuffer;    // Just a work buffer into which we can read records
	int nContentLength = 0;
	bool bIsUrlEncoded;

	requestContent[0] = 0;    // Initialize as an empty string
	// Read the first line: Request-Line setting Uri Index and returning the method type.
	MethodType eMethod = readRequestLine(client, readBuffer, uri, requestContent, req);
	// Read any following, non-empty headers setting content length.
	readRequestHeaders(client, readBuffer, nContentLength, bIsUrlEncoded);

	if (nContentLength > 0)
	{
		// If there is some content then read it and do an elementary decode.
		readEntityBody(client, nContentLength, requestContent);
		if (bIsUrlEncoded)
		{
			// The '+' encodes for a space, so decode it within the string
			for (char * pChar = requestContent; (pChar = strchr(pChar, '+')) != NULL; )
			*pChar = ' ';    // Found a '+' so replace with a space
		}
	}

	return eMethod;
}

WebServer::MethodType WebServer::readRequestLine(EthernetClient* client, BUFFER & readBuffer, String* uri, BUFFER & requestContent, RequestContent* req) {
	MethodType eMethod;
	// Get first line of request:
	// Request-Line = Method SP Request-URI SP HTTP-Version CRLF
	getNextHttpLine(client, readBuffer);
	// Split it into the 3 tokens
	char * pMethod  = strtok(readBuffer, pSpDelimiters);
	char * pUri     = strtok(NULL, pSpDelimiters);
	//char * pVersion = strtok(NULL, pSpDelimiters);
	strtok(NULL, pSpDelimiters);
	// URI may optionally comprise the URI of a queryable object a '?' and a query
	// see http://www.ietf.org/rfc/rfc1630.txt
	strtok(pUri, "?");
	char * pQuery   = strtok(NULL, "?");
	if (pQuery != NULL)
	{
		strcpy(requestContent, pQuery);
		// The '+' encodes for a space, so decode it within the string
		for (pQuery = requestContent; (pQuery = strchr(pQuery, '+')) != NULL; )
		*pQuery = ' ';    // Found a '+' so replace with a space

		//Serial.print("Get query string: ");
		//Serial.println(requestContent);

		req->parse(requestContent);
	}
	if (strcmp(pMethod, "GET") == 0)
		eMethod = MethodGet;
	else if (strcmp(pMethod, "POST") == 0)
		eMethod = MethodPost;
	else if (strcmp(pMethod, "HEAD") == 0)
		eMethod = MethodHead;
	else
		eMethod = MethodUnknown;

	// See if we recognize the URI and get its index
	uri->concat(pUri);

	return eMethod;
}

void WebServer::readRequestHeaders(EthernetClient* client, BUFFER & readBuffer, int & nContentLength, bool & bIsUrlEncoded) {
	nContentLength = 0;      // Default is zero in cate there is no content length.
	bIsUrlEncoded  = true;   // Default encoding
	// Read various headers, each terminated by CRLF.
	// The CRLF gets removed and the buffer holds each header as a string.
	// An empty header of zero length terminates the list.
	do
	{
		getNextHttpLine(client, readBuffer);
		//    Serial.println(readBuffer); // DEBUG

		char * pFieldName  = strtok(readBuffer, pSpDelimiters);
		char * pFieldValue = strtok(NULL, pSpDelimiters);

		if (strcmp(pFieldName, "Content-Length:") == 0) {
			nContentLength = atoi(pFieldValue);
		} else if (strcmp(pFieldName, "Content-Type:") == 0) {
			if (strcmp(pFieldValue, "application/x-www-form-urlencoded") != 0)
			bIsUrlEncoded = false;
		} else if (strcmp(pFieldName, "User-Agent:") == 0) {
			if(strstr("Mobile", readBuffer) != NULL) {
				//mobile!
			}
		}
	} while (strlen(readBuffer) > 0);    // empty string terminates
}

void WebServer::readEntityBody(EthernetClient* client, int nContentLength, BUFFER & content) {
	int i;
	char c;

	if (nContentLength >= (int) sizeof(BUFFER))
		nContentLength = sizeof(BUFFER) - 1;  // Should never happen!

	for (i = 0; i < nContentLength; ++i)
	{
		c = client->read();
		//    Serial.print(c); // DEBUG
		content[i] = c;
	}

	content[nContentLength] = 0;  // Null string terminator

	//  Serial.print("Content: ");
	//  Serial.println(content);
}

void WebServer::getNextHttpLine(EthernetClient* client, BUFFER & readBuffer) {
	char c;
	uint8_t bufindex = 0; // reset buffer

	// reading next header of HTTP request
	if (client->connected() && client->available())
	{
		// read a line terminated by CRLF
		readBuffer[0] = client->read();
		readBuffer[1] = client->read();
		bufindex = 2;
		for (int i = 2; readBuffer[i - 2] != '\r' && readBuffer[i - 1] != '\n'; ++i)
		{
			// read full line and save it in buffer, up to the buffer size
			c = client->read();
			if (bufindex < sizeof(BUFFER))
			readBuffer[bufindex++] = c;
		}
		readBuffer[bufindex - 2] = 0;  // Null string terminator overwrites '\r'
	}
}

boolean WebServer::closeClient(EthernetClient* client) {
	#ifdef DEBUG_WEBSERVER_ENABLE
	Serial.print(millis());
	Serial.print(F(": WebServer::closeClient() clientSlot="));
	Serial.println(client->_sock);
	#endif

	/*
	//is there a pending request?
	if(EthernetClient(i).available()) {
		#ifdef DEBUG_WEBSERVER_ENABLE
		Serial.print(F("\tnot closing"));
		#endif

		clientStatus[i].waiting = false;

		return false;
	}*/

	client->flush();

	delay(1);
	clientStatus[client->_sock].inUse = false;
	clientStatus[client->_sock].requestType = PAGE_NONE;
	clientStatus[client->_sock].waiting = false;

	if(clientStatus[client->_sock].callback != NULL && dispatcher.getResponseHandler() != NULL) {
		dispatcher.getResponseHandler()->unregisterListener(clientStatus[client->_sock].callback);
	}
	client->stop();

	return true;
}

void WebServer::loop() {
	server.available();

	////iterate clients
	for(uint8_t i = 0; i < CLIENT_INSTANCES_NUM; i++) {
		EthernetClient client = EthernetClient(i);
		//new connections
		if(clientStatus[i].inUse == false && client.available()) {
			clientStatus[i].inUse = true;
		}

		//check for answers from listeners
		if(clientStatus[i].waiting == true) {
			if(clientStatus[i].callback != NULL && clientStatus[i].callback->state == webserverListener::FINISHED) {
				handleFinishedCallback(&client);
			} else if(clientStatus[i].callback != NULL && clientStatus[i].callback != NULL && (clientStatus[i].callback->state == webserverListener::FAILED || millis() - clientStatus[i].timestamp > WEBSERVER_REQUEST_TIMEOUT_MILLIS)) {
				//no answer.
				PageMaker::sendHttp500WithBody(&client);
			}
		}

		//handle conenctions
		if(clientStatus[i].inUse == true && clientStatus[i].waiting == false) {
			#ifdef DEBUG_WEBSERVER_ENABLE
			Serial.print(millis());
			Serial.print(F(": WebServer::loop() handle clientSlot="));
			Serial.println(i);
			#endif
			doClientHandling(&client);
		}
	}
}

void WebServer::handleFinishedCallback(EthernetClient* client) {
	if(clientStatus[client->_sock].requestType == PAGE_REQUEST_SENSOR) {
		PageMaker::doPageRequestSensor2(client, (hardwareRequestListener*) clientStatus[client->_sock].callback);
	//} else if(clientStatus[client->_sock].requestType == PAGE_GETSENSORINFO) {
		//doPageSensorInfo2(client);
	} else if (clientStatus[client->_sock].requestType == PAGE_WRITE_SENSOR) {
		PageMaker::doPageWriteSensor2(client, (hardwareRequestListener*) clientStatus[client->_sock].callback);
	} else {
		//unknown request
		PageMaker::sendHttp500WithBody(client);
	}

	closeClient(client);
}

void WebServer::doClientHandling(EthernetClient* client) {
	#ifdef DEBUG
	Serial.print(millis());
	Serial.print(F(": doClientHandling() on clientId="));
	Serial.println(client->_sock);
	#endif

	// now client is connected to arduino we need to extract the
	// following fields from the HTTP request.
	String    uri;  // Gives the index into table of recognized URIs or -1 for not found.

	RequestContent req;
	MethodType eMethod = readHttpRequest(client, &uri, &req);

	#ifdef DEBUG
	Serial.print(F("\tRead Request type: "));
	Serial.print(eMethod);
	Serial.print(F(" Uri: "));
	Serial.print(uri);
	Serial.println(F(" content: "));
	for(uint8_t i = 0; i < req.getNum(); i++) {
		Serial.print(F("\t"));
		Serial.print(req.getKeys()[i]);
		Serial.print(F("="));
		Serial.println(req.getValues()[i]);
		Serial.flush();
	}
	#endif

	BUFFER uriChars;
	uri.toCharArray(uriChars, STRING_BUFFER_SIZE);
	//select page
	if(strcmp_P(uriChars, pageAddresses[PAGE_MAIN]) == 0) {
		PageMaker::doPageStart(client);
	} else if(strcmp_P(uriChars, pageAddresses[PAGE_NODES]) == 0) {
		PageMaker::doPageNodes(client, &req);
	} else if(strcmp_P(uriChars, pageAddresses[PAGE_CSS]) == 0) {
		PageMaker::doPageCss(client);
	} else if(strcmp_P(uriChars, pageAddresses[PAGE_LIST_FILES]) == 0) {
		PageMaker::doPageListFiles(client, &req);
	} else if(strcmp_P(uriChars, pageAddresses[PAGE_MAINTAIN_NODE_INFO]) == 0) {
		PageMaker::doPageMaintenanceNodeInfo(client, &req);
	} else if(strcmp_P(uriChars, pageAddresses[PAGE_GETSENSORINFO]) == 0) {
		//doPageSensorInfo(client, &req);
		//clientStatus[client->_sock].requestType = PAGE_GETSENSORINFO;
		//clientStatus[client->_sock].waiting = true;
		//clientStatus[client->_sock].timestamp = millis();
		PageMaker::doPageSensorInfo2(client, &req);
	} else if(strcmp_P(uriChars, pageAddresses[PAGE_REQUEST_SENSOR]) == 0) {
		if(PageMaker::doPageRequestSensor(client, &req, &listenerHardwareRequest)) {
			clientStatus[client->_sock].callback = &listenerHardwareRequest;
			clientStatus[client->_sock].requestType = PAGE_REQUEST_SENSOR;
			clientStatus[client->_sock].waiting = true;
			clientStatus[client->_sock].timestamp = millis();
		}
	} else if(strcmp_P(uriChars, pageAddresses[PAGE_WRITE_SENSOR]) == 0) {
		if(PageMaker::doPageWriteSensor(client, &req, &listenerHardwareRequest)) {
			clientStatus[client->_sock].callback = &listenerHardwareRequest;
			clientStatus[client->_sock].requestType = PAGE_WRITE_SENSOR;
			clientStatus[client->_sock].waiting = true;
			clientStatus[client->_sock].timestamp = millis();
		}
	} else {
		PageMaker::sendHttp404WithBody(client);
	}

	#ifdef DEBUG_WEBSERVER_ENABLE
	Serial.println(F("\tpage sent out. checking for connection close."));
	#endif

	if(!clientStatus[client->_sock].waiting) {
		#ifdef DEBUG_WEBSERVER_ENABLE
		Serial.println(F("\tnot waiting, do close."));
		#endif
		closeClient(client);
	}
}
