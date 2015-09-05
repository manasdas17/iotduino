/*
 * Web Server - multi-page.  5 Feb 2011.
 * Need an Ethernet Shield over Arduino.
 *
 * by Martyn Woerner extending the good work of
 * Alessandro Calzavara, alessandro(dot)calzavara(at)gmail(dot)com
 * and Alberto Capponi, bebbo(at)fast-labs net
 * for Arduino community! :-)
 *
 * Pro:
 * - HTTP Requests GET & POST
 * - Switch page selection.
 * - HTML pages in flash memory.
 * - Button to turn LED on/off
 * - Favicon & png images
 *
 */
/*
 * WebServer.h
 *
 * Created: 28.05.2015 09:37:52
 *  Author: helge
 */


#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet/Ethernet.h>
#include <dispatcher/EventCallbackInterface.h>
#include <avr/wdt.h>

#include <networking/Layer3.h>
#include <dispatcher/PacketDispatcher.h>
#include <drivers/HardwareID.h>

#include <sdcard/DiscoveryManager.h>

#include <webserver/DiscoveryListener.h>
#include <webserver/HardwareResultListener.h>
#include <webserver/RequestContent.h>
#include <webserver/PageMaker.h>

#include <ramManager.h>
#include <utils/NodeInfo.h>

extern Layer3 l3;
extern PacketDispatcher dispatcher;
extern PacketFactory pf;
extern SPIRamManager ram;
extern DiscoveryManager discoveryManager;
extern NodeInfo nodeinfo;

//#define USE_DHCP_FOR_IP_ADDRESS


//ethernet start on defailt port 80
EthernetServer server(80);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
#ifndef USE_DHCP_FOR_IP_ADDRESS
	//ip
	byte ip[] = { 192, 168, 0, 177 };
#endif


/** webserver */
class WebServer {
	public:
	enum MethodType {
		MethodUnknown,
		MethodGet,
		MethodPost,
		MethodHead
	};

	#define TIMEOUT_MILLIS (2*1000)

	#ifndef STRING_BUFFER_SIZE
		#define STRING_BUFFER_SIZE 128
	#endif
	typedef char BUFFER[STRING_BUFFER_SIZE];

	typedef struct clientInstances_struct {
		PAGES requestType;
		boolean waiting;
		uint32_t timestamp;
		webserverListener* callback;
		boolean inUse;
	} clientInstances_t;

	#define CLIENT_INSTANCES_NUM 4
	clientInstances_t clientStatus[CLIENT_INSTANCES_NUM];

	/** static dicoverylistener object */
	discoveryListener listenerDiscovery;
	/** static hardwarerequestlistener object */
	hardwareRequestListener listenerHardwareRequest;

	WebServer() {
		for(uint8_t i = 0; i < CLIENT_INSTANCES_NUM; i++) {
			clientStatus[i].inUse = false;
			clientStatus[i].callback = NULL;
			clientStatus[i].requestType = PAGE_NONE;
			clientStatus[i].waiting = false;
		}
	}

	void init() {
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

	// Http header token delimiters
	const char *pSpDelimiters = " \r\n";
	const char *pStxDelimiter = "\002";    // STX - ASCII start of text character

	#ifdef DEBUG_WEBSERVER_ENABLE
	void serialPrintP(const char* str) {
		if(str == NULL)
			return;

		char buf[120];
		strcpy_P(buf, str);
		Serial.print(buf);
	}
	#endif


	/**
	 * Read HTTP request, setting Uri Index, the requestContent and returning the method type.
	 * @param client
	 * @param uri
	 * @param req here the GET dats is stored
	 */
	MethodType readHttpRequest(EthernetClient* client, String* uri, RequestContent* req)
	{
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

	/**
	 * Read the first line of the HTTP request, setting Uri Index and returning the method type.
	 * If it is a GET method then we set the requestContent to whatever follows the '?'. For a other
	 * methods there is no content except it may get set later, after the headers for a POST method.
	 * @param client
	 * @param readBuffer
	 * @param uri
	 * @param requestContent buffer
	 * @param req request content map object
	 */
	MethodType readRequestLine(EthernetClient* client, BUFFER & readBuffer, String* uri, BUFFER & requestContent, RequestContent* req)
	{
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

	/**
	 * Read each header of the request till we get the terminating CRLF
	 * @param client
	 * @param readBuffer
	 * @param nContentLenth
	 * @parambIsUrlEncoded
	 */
	void readRequestHeaders(EthernetClient* client, BUFFER & readBuffer, int & nContentLength, bool & bIsUrlEncoded)
	{
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

	/**
	 * Read the entity body of given length (after all the headers) into the buffer.
	 * @param  client
	 * @param nContentLength
	 * @param content buffer
	 */
	void readEntityBody(EthernetClient* client, int nContentLength, BUFFER & content)
	{
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

	/**
	 * eat headers.
	 * @param client
	 * @param readBuffer
	 */
	void getNextHttpLine(EthernetClient* client, BUFFER & readBuffer)
	{
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


	/**
	 * close a client connection and delete callback
	 * @param socketNumber
	 * @return success
	 */
	boolean closeClient(EthernetClient* client) {
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

	/**
	 * main loop.
	 */
	void loop() {
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
				} else if(clientStatus[i].callback != NULL && clientStatus[i].callback != NULL && (clientStatus[i].callback->state == webserverListener::FAILED || millis() - clientStatus[i].timestamp > TIMEOUT_MILLIS)) {
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

	/**
	 * callback has finished, now execute this (usually actual page creation)
	 * @param clintId
	 */
	void handleFinishedCallback(EthernetClient* client) {
		if(clientStatus[client->_sock].requestType == PAGE_REQUEST_SENSOR) {
			doPageRequestSensor2(client, (hardwareRequestListener*) clientStatus[client->_sock].callback);
		//} else if(clientStatus[client->_sock].requestType == PAGE_GETSENSORINFO) {
			//doPageSensorInfo2(client);
		} else if (clientStatus[client->_sock].requestType == PAGE_WRITE_SENSOR) {
			doPageWriteSensor2(client, (hardwareRequestListener*) clientStatus[client->_sock].callback);
		} else {
			//unknown request
			PageMaker::sendHttp500WithBody(client);
		}

		closeClient(client);
	}



	/**
	 * first method when client connects - read http request
	 * @param client
	 */
	void doClientHandling(EthernetClient* client) {
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
			PageMaker::doPageNodes(client);
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
			if(doPageRequestSensor(client, &req, &listenerHardwareRequest)) {
				clientStatus[client->_sock].callback = &listenerHardwareRequest;
				clientStatus[client->_sock].requestType = PAGE_REQUEST_SENSOR;
				clientStatus[client->_sock].waiting = true;
				clientStatus[client->_sock].timestamp = millis();
			}
		} else if(strcmp_P(uriChars, pageAddresses[PAGE_WRITE_SENSOR]) == 0) {
			if(doPageWriteSensor(client, &req, &listenerHardwareRequest)) {
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

	/**
	 * initiate sensor write
	 * @param client
	 * @param req
	 */
	static boolean doPageWriteSensor(EthernetClient* client, RequestContent* req, hardwareRequestListener* listenerHardwareRequest) {
		#ifdef DEBUG
		Serial.print(millis());
		Serial.print(F(": doPageWriteSensor() on clientId="));
		Serial.println(client->_sock);
		#endif

		//get request strings
		String* id = req->getValue(variableRemote);
		String* hwAddressStr = req->getValue(variableHwAddress);
		String* hwtypeStr = req->getValue(variableHwType);
		String* listTypeStr = req->getValue(variableListType);
		String* val = req->getValue(variableVal);

		HardwareCommandResult cmd;

		//check if available
		if(id == NULL || hwAddressStr == NULL || hwtypeStr == NULL || hwtypeStr == NULL || listTypeStr == NULL || val == NULL || val->length() % 2 == 1 || (val->length()-2)/2 > sizeUInt8List) {
			PageMaker::sendHttp500WithBody(client);
			return false;
		}

		//convert addresses
		int8_t idInt = id->toInt();
		int8_t hwaddress = hwAddressStr->toInt();
		int8_t hwtype = hwtypeStr->toInt();

		//get list numbers
		uint8_t uint8 = 0;
		uint8_t uint16 = 0;
		uint8_t int8 = 0;
		uint8_t int16 = 0;


		int8_t numBytes = (val->length()-2) / 2;
		if(strcmp_P((const char*) listTypeStr, linkCmdListTypeUint16) == 0) {
			uint16 = numBytes;
			cmd.setUint16ListNum(numBytes);
		} else if(strcmp_P((const char*) listTypeStr, linkCmdListTypeInt8) == 0) {
			int8 = numBytes;
			cmd.setInt8ListNum(numBytes);
		} else if(strcmp_P((const char*) listTypeStr, linkCmdListTypeInt16) == 0) {
			int16 = numBytes;
			cmd.setInt16ListNum(numBytes);
		} else { //uint8
			uint8 = numBytes;
			cmd.setUint8ListNum(numBytes);
		}

		if(numBytes < 0 || 2*uint16 > sizeUInt8List || 2*int16 > sizeUInt8List || uint8 > sizeUInt8List || int8 > sizeUInt8List) {
			PageMaker::sendHttp500WithBody(client);
			return false;
		}


		//get value
		val->toLowerCase();
		char valBuf[sizeUInt8List*2+2];
		val->toCharArray(valBuf, sizeof(valBuf));
		for(uint8_t i = 2; i < val->length(); i += 2) {
			sscanf((char*) &valBuf[i], "%2hhx", (char*) &cmd.getUint8List()[numBytes-1]);
			numBytes--;
		}

		//check if all is ok.
		if(idInt == -1 || hwaddress == -1 || hwtype == -1) {
			PageMaker::sendHttp500WithBody(client);
			return false;
		}

		Layer3::packet_t p;
		cmd.setAddress(hwaddress);
		cmd.setHardwareType((HardwareTypeIdentifier) hwtype);
		seq_t sequence = pf.generateHardwareCommandWrite(&p, idInt, &cmd);
		listenerHardwareRequest->init(idInt, (HardwareTypeIdentifier) hwtype, hwaddress, webserverListener::AWAITING_ANSWER);

		boolean success = dispatcher.getResponseHandler()->registerListenerBySeq(millis()+TIMEOUT_MILLIS, sequence, idInt, listenerHardwareRequest);

		success &= l3.sendPacket(p);

		if(!success) {
			PageMaker::sendHttp500WithBody(client);
			dispatcher.getResponseHandler()->unregisterListener(listenerHardwareRequest);
			return false;
		}

		return true;
	}

	static void doPageWriteSensor2(EthernetClient* client, hardwareRequestListener* listener) {
		#ifdef DEBUG
		Serial.print(millis());
		Serial.print(F(": doPageWriteSensor2() on clientId="));
		Serial.println(client->_sock);
		#endif

		PageMaker::sendHttpOk(client);
		PageMaker::sendHtmlHeader(client, PAGE_WRITE_SENSOR, false, false);

		client->print(F("<h1>Sensor Write id="));
		client->print(listener->remote);
		client->println(F("</h1>"));
		client->print(F("<h2>hwaddress="));
		client->print(listener->hwaddress);
		client->print(F(" hwtype="));
		PageMaker::printP(client, hardwareTypeStrings[listener->hwtype]);
		if(listener->cmd.isRead) {
			client->print(F(" read"));
			} else {
			client->print(F(" write"));
		}
		client->println(F("</h2>"));

		PageMaker::sendHtmlFooter(client);
		listener->init(0, HWType_UNKNOWN, 0, webserverListener::START);
	}


	/**
	 * initiate sensor reading
	 * @param client
	 * @param req
	 */
	static boolean doPageRequestSensor(EthernetClient* client, RequestContent* req, hardwareRequestListener* hwListener) {
		#ifdef DEBUG
		Serial.print(millis());
		Serial.print(F(": doPageRequestSensor() on client="));
		Serial.println(client->_sock);
		#endif

		String* id = req->getValue(variableRemote);
		String* hwAddressStr = req->getValue(variableHwAddress);
		String* hwtypeStr = req->getValue(variableHwType);
		if(id == NULL || hwAddressStr == NULL || hwtypeStr == NULL) {
			PageMaker::sendHttp500WithBody(client);
			return false;
		}

		int8_t idInt = id->toInt();
		int8_t hwaddress = hwAddressStr->toInt();
		int8_t hwtype = hwtypeStr->toInt();

		if(idInt == -1 || hwaddress == -1 || hwtype == -1) {
			PageMaker::sendHttp500WithBody(client);
			return false;
		}

		Layer3::packet_t p;
		seq_t sequence = pf.generateHardwareCommandRead(&p, idInt, hwaddress, (HardwareTypeIdentifier) hwtype);
		hwListener->init(idInt, (HardwareTypeIdentifier) hwtype, hwaddress, webserverListener::AWAITING_ANSWER);

		boolean success = dispatcher.getResponseHandler()->registerListenerBySeq(millis()+TIMEOUT_MILLIS, sequence, idInt, hwListener);

		success &= l3.sendPacket(p);

		if(!success) {
			PageMaker::sendHttp500WithBody(client);
			dispatcher.getResponseHandler()->unregisterListener(hwListener);
			return false;
		}

		return true;
	}

	/**
	 * sensor reading has finished - print
	 * @param client
	 */
	static void doPageRequestSensor2(EthernetClient* client, hardwareRequestListener* listener) {
		#ifdef DEBUG
		Serial.print(millis());
		Serial.print(F(": doPageRequestSensor2() on clientId="));
		Serial.println(client->_sock);
		#endif

		PageMaker::sendHttpOk(client);
		PageMaker::sendHtmlHeader(client, PAGE_REQUEST_SENSOR, false);

		client->print(F("<h1>Sensor Info id="));
		client->print(listener->remote);
		client->println(F("</h1>"));
		client->print(F("<h2>hwaddress="));
		client->print(listener->hwaddress);
		client->print(F(" hwtype="));
		PageMaker::printP(client, hardwareTypeStrings[listener->hwtype]);
		if(listener->cmd.isRead) {
			client->print(F(" read"));
		} else {
			client->print(F(" write"));
		}
		client->println(F("</h2>"));

		client->println(F("<code><pre>"));
		if(listener->cmd.numUint8 > 0) {
			client->print(F("Uint8:\t"));
			for(uint8_t i = 0; i < listener->cmd.numUint8; i++)	 {
				client->print(listener->cmd.uint8list[i]);
				client->print(F("\t"));
			}
			client->println(F("<br/>"));
		}
		if(listener->cmd.numUint16 > 0) {
			client->print(F("Uint16:\t"));
			for(uint8_t i = 0; i < listener->cmd.numUint16; i++) {
				client->print(listener->cmd.uint16list[i]);
				client->print(F("\t"));
			}
			client->println(F("<br/>"));
		}
		if(listener->cmd.numInt8 > 0) {
			client->print(F("Int8:\t"));
			for(uint8_t i = 0; i < listener->cmd.numInt8; i++) {
				client->print(listener->cmd.int8list[i]);
				client->print(F("\t"));
			}
			client->println(F("<br/>"));
		}
		if(listener->cmd.numInt16 > 0) {
			client->print(F("Int16:\t"));
			for(uint8_t i = 0; i < listener->cmd.numInt16; i++) {
				client->print(listener->cmd.int16list[i]);
				client->print(F("\t"));
			}
			client->println(F("<br/>"));
		}
		client->println(F("</pre></code>"));

		PageMaker::sendHtmlFooter(client);
		listener->init(0, HWType_UNKNOWN, 0, webserverListener::START);
	}
};

#endif /* WEBSERVER_H_ */
