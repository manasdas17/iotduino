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
#include <avr/pgmspace.h>
#include <dispatcher/EventCallbackInterface.h>
#include <avr/wdt.h>

#include <networking/Layer3.h>
#include <dispatcher/PacketDispatcher.h>
#include <drivers/HardwareID.h>

#include <sdcard/SDcard.h>

#include <webserver/DiscoveryListener.h>
#include <webserver/HardwareResultListener.h>
#include <webserver/RequestContent.h>

extern Layer3 l3;
extern PacketDispatcher dispatcher;
extern PacketFactory pf;
extern SDcard sdcard;

//#define USE_DHCP_FOR_IP_ADDRESS


//ethernet start on defailt port 80
EthernetServer server(80);
//max
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
#ifndef USE_DHCP_FOR_IP_ADDRESS
	//ip
	byte ip[] = { 192, 168, 0, 177 };
#endif

const char pageAddressMain[] PROGMEM = {"/"};
const char pageAddressGetSensorInfo[] PROGMEM = {"/getSensorInfo"};
const char pageNodes[] PROGMEM = {"/nodes"};
const char pageCss[] PROGMEM = {"/css"};
const char pageRequestSensor[] PROGMEM = {"/sensor"};
const char pageWriteSensor[] PROGMEM = {"/sensorWrite"};
PGM_P pageAddresses[] = {NULL, pageAddressMain, pageAddressGetSensorInfo, pageNodes, pageCss, pageRequestSensor, pageWriteSensor};

const char pageTitleMain[] PROGMEM = {"Start"};
const char pageTitleGetSensorInfo[] PROGMEM = {"Sensor Info"};
const char pageTitleNodes[] PROGMEM = {"Nodes"};
const char pageTitleRequestSensor[] PROGMEM = {"Rquested Sensor Information"};
const char pageTitleWriteSensor[] PROGMEM = {"Write to Sensor"};

PGM_P pageTitles[] = {NULL, pageTitleMain, pageTitleGetSensorInfo, pageTitleNodes, NULL, pageTitleRequestSensor, pageTitleWriteSensor};
enum PAGES {PAGE_NONE, PAGE_MAIN, PAGE_GETSENSORINFO, PAGE_NODES, PAGE_CSS, PAGE_REQUEST_SENSOR, PAGE_WRITE_SENSOR};

const char variableRemote[] PROGMEM = {"remote"};
const char variableHwAddress[] PROGMEM = {"hwaddress"};
const char variableHwType[] PROGMEM = {"hwtype"};
const char variableVal[] PROGMEM = {"val"};
const char variableListType[] PROGMEM = {"listtype"};
const char variableListNum[] PROGMEM = {"listnum"};

const char linkNameX[] PROGMEM = {"x"};
const char linkNameOn[] PROGMEM = {"on"};
const char linkNameOff[] PROGMEM = {"off"};
const char linkNameToggle[] PROGMEM = {"toggle"};

const char linkCmdValDefault[] PROGMEM = {"0x00"};
const char linkCmdListChecked[] PROGMEM = {" checked='checked'"};
const char linkCmdListTypeUint8[] PROGMEM = {"u8"};
const char linkCmdListTypeUint16[] PROGMEM = {"u16"};
const char linkCmdListTypeInt8[] PROGMEM = {"i8"};
const char linkCmdListTypeInt16[] PROGMEM = {"i16"};


/** hw type strs */
const char strHWType_UNKNOWN[] PROGMEM = {"UNKNOWN"};
const char strHWType_ANALOG[] PROGMEM = {"ANALOG"};
const char strHWType_DIGITAL[] PROGMEM = {"DIGITAL"};
const char strHWType_accelerometer[] PROGMEM = {"Accelerometer"};
const char strHWType_button[] PROGMEM = {"Button"};
const char strHWType_gyroscope[] PROGMEM = {"Gyroscope"};
const char strHWType_humidity[] PROGMEM = {"Humidity"};
const char strHWType_ir[] PROGMEM = {"IR"};
const char strHWType_keypad[] PROGMEM = {"Keypad"};
const char strHWType_magneticField[] PROGMEM = {"Magnetic Field"};
const char strHWType_methane[] PROGMEM = {"Methane"};
const char strHWType_motion[] PROGMEM = {"Motion"};
const char strHWType_pressure[] PROGMEM = {"Pressure"};
const char strHWType_rtc[] PROGMEM = {"RTC"};
const char strHWType_dcf77[] PROGMEM = {"DCF77"};
const char strHWType_sonar[] PROGMEM = {"Sonar"};
const char strHWType_hwSwitch[] PROGMEM = {"Switch"};
const char strHWType_temprature[] PROGMEM = {"Temperature"};
const char strHWType_touchpad[] PROGMEM = {"Touchpad"};
const char strHWType_led[] PROGMEM = {"LED"};
const char strHWType_rcswitch[] PROGMEM = {"RC Switch"};
const char strHWType_relay[] PROGMEM = {"Relay"};
const char strHWType_light[] PROGMEM = {"Light"};
const char strHWType_tone[] PROGMEM = {"Tone"};

/** string representations of hwtypes */
PGM_P hardwareTypeStrings[] = {	strHWType_UNKNOWN,
	strHWType_ANALOG,
	strHWType_DIGITAL,
	strHWType_accelerometer,
	strHWType_button,
	strHWType_gyroscope,
	strHWType_humidity,
	strHWType_ir,
	strHWType_keypad,
	strHWType_magneticField,
	strHWType_methane,
	strHWType_motion,
	strHWType_pressure,
	strHWType_rtc,
	strHWType_dcf77,
	strHWType_sonar,
	strHWType_hwSwitch,
	strHWType_temprature,
	strHWType_touchpad,
	strHWType_led,
	strHWType_rcswitch,
	strHWType_relay,
	strHWType_light,
	strHWType_tone};

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
	 * print to ethernet client from PGM space
	 * @param clientId
	 * @param str
	 */
	void printP(uint8_t clientId, const char * str) {
		if(str == NULL)
			return;

		EthernetClient client = EthernetClient(clientId);
		// copy data out of program memory into local storage, write out in
		// chunks of 32 bytes to avoid extra short TCP/IP packets
		uint8_t buffer[32];
		size_t bufferEnd = 0;

		while (buffer[bufferEnd++] = pgm_read_byte(str++)){
			if (bufferEnd == 32) {
				client.write(buffer, 32);
				bufferEnd = 0;
			}
		}

		// write out everything left but trailing NUL
		if (bufferEnd > 1) {
			client.write(buffer, bufferEnd - 1);
		}
	}

	/**
	 * 404 page
	 * @param clientId
	 */
	void sendHttp404WithBody(uint8_t clientId) {
		EthernetClient client = EthernetClient(clientId);
		client.println(F("HTTP/1.1 404 Not Found"));
		client.println(F("Content-Type: text/html"));
		//client.println(F("Content-Length: 16"));
		client.println(F("Connection: close"));  // the connection will be closed after completion of the response
		client.println();
		client.println(F("404 - Not found."));

		closeClient(clientId);
	}

	/**
	 * 500 page
	 * @param clientId
	 */
	void sendHttp500WithBody(uint8_t clientId) {
		EthernetClient client = EthernetClient(clientId);
		client.println(F("HTTP/1.1 500 Internal error"));
		client.println(F("Content-Type: text/html"));
		//client.println(F("Content-Length: 19"));
		client.println(F("Connection: close"));  // the connection will be closed after completion of the response
		client.println();
		client.println(F("500 Internal error. <a href='/'>return to main.</a>"));

		closeClient(clientId);
	}

	/**
	 * 200 ok head
	 * @param clientId
	 */
	void sendHttpOk(uint8_t clientId) {
		sendHttpOk(clientId, 0);
	}

	void sendHttpOk(uint8_t clientId, uint32_t cacheTime) {
		EthernetClient client = EthernetClient(clientId);
		client.println(F("HTTP/1.1 200 OK"));
		client.println(F("Content-Type: text/html"));
		client.println(F("Connection: close"));  // the connection will be closed after completion of the response
		if(cacheTime > 0) {
			client.print(F("Cache-Control: no-transform,public,max-age="));
			client.println(cacheTime);
		}
		client.println();
		client.flush();
	}

	/**
	 * html head
	 * @param clientId
	 */
	void sendHtmlHeader(uint8_t clientId, PGM_P title, boolean refresh = true) {
		EthernetClient client = EthernetClient(clientId);
		client.print(F("<html><header><link rel='stylesheet' href='css' type='text/css'><title>"));
		printP(clientId, title);
		client.print(F("</title>"));

		if(refresh)
			client.print(F("<meta http-equiv='refresh' content='300'>"));

		client.println(F("</header><body>"));
		client.flush();
	}

	/**
	 * html menu
	 * @param clientId
	 */
	void sendHtmlMenu(uint8_t clientId) {
		EthernetClient client = EthernetClient(clientId);
		uint8_t menuPages[] = {PAGE_MAIN, PAGE_NODES};
		uint8_t menuPagesNum = 2;
		client.print(F("<h1>Menu</h1><ul>"));

		for(uint8_t i = 0; i < menuPagesNum; i++) {
			client.print(F("<li><a href='"));
			this->printP(clientId, pageAddresses[menuPages[i]]);
			client.print("'>");
			this->printP(clientId, pageTitles[menuPages[i]]);
			client.print(F("</a></li>"));
		}

		client.print(F("</ul><br/><hr/><br/><a href='javascript:window.history.back();'>&laquo; back</a> &middot; <a href='javascript:location.reload();'>reload</a> &middot; <a href='javascript:window.history.forward();'>forward &raquo;</a><br/>"));
		client.flush();
	}

	/**
	 * html footer
	 * @param clientId
	 */
	void sendHtmlFooter(uint8_t clientId) {
		EthernetClient client = EthernetClient(clientId);
		client.print(F("<br/><br/><hr/><br/><span style='text-align: right; font-style: italic;'><a href='http://iotduino.de'>iotduino</a> webserver. "));
		printDate(clientId, now());
		client.println(F("</span></body></html>"));
		client.flush();
	}


	/**
	 * Read HTTP request, setting Uri Index, the requestContent and returning the method type.
	 * @param clientId
	 * @param uri
	 * @param req here the GET dats is stored
	 */
	MethodType readHttpRequest(uint8_t clientId, String* uri, RequestContent* req)
	{
		BUFFER requestContent;
		BUFFER readBuffer;    // Just a work buffer into which we can read records
		int nContentLength = 0;
		bool bIsUrlEncoded;

		requestContent[0] = 0;    // Initialize as an empty string
		// Read the first line: Request-Line setting Uri Index and returning the method type.
		MethodType eMethod = readRequestLine(clientId, readBuffer, uri, requestContent, req);
		// Read any following, non-empty headers setting content length.
		readRequestHeaders(clientId, readBuffer, nContentLength, bIsUrlEncoded);

		if (nContentLength > 0)
		{
			// If there is some content then read it and do an elementary decode.
			readEntityBody(clientId, nContentLength, requestContent);
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
	 * @param clientId
	 * @param readBuffer
	 * @param uri
	 * @param requestContent buffer
	 * @param req request content map object
	 */
	MethodType readRequestLine(uint8_t clientId, BUFFER & readBuffer, String* uri, BUFFER & requestContent, RequestContent* req)
	{
		MethodType eMethod;
		// Get first line of request:
		// Request-Line = Method SP Request-URI SP HTTP-Version CRLF
		getNextHttpLine(clientId, readBuffer);
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
	 * @param clientID
	 * @param readBuffer
	 * @param nContentLenth
	 * @parambIsUrlEncoded
	 */
	void readRequestHeaders(uint8_t clientId, BUFFER & readBuffer, int & nContentLength, bool & bIsUrlEncoded)
	{
		nContentLength = 0;      // Default is zero in cate there is no content length.
		bIsUrlEncoded  = true;   // Default encoding
		// Read various headers, each terminated by CRLF.
		// The CRLF gets removed and the buffer holds each header as a string.
		// An empty header of zero length terminates the list.
		do
		{
			getNextHttpLine(clientId, readBuffer);
			//    Serial.println(readBuffer); // DEBUG

			char * pFieldName  = strtok(readBuffer, pSpDelimiters);
			char * pFieldValue = strtok(NULL, pSpDelimiters);

			if (strcmp(pFieldName, "Content-Length:") == 0)
			{
				nContentLength = atoi(pFieldValue);
			}
			else if (strcmp(pFieldName, "Content-Type:") == 0)
			{
				if (strcmp(pFieldValue, "application/x-www-form-urlencoded") != 0)
				bIsUrlEncoded = false;
			}
		} while (strlen(readBuffer) > 0);    // empty string terminates
	}

	/**
	 * Read the entity body of given length (after all the headers) into the buffer.
	 * @param  clientId
	 * @param nContentLength
	 * @param content buffer
	 */
	void readEntityBody(uint8_t clientId, int nContentLength, BUFFER & content)
	{
		int i;
		char c;

		if (nContentLength >= (int) sizeof(BUFFER))
			nContentLength = sizeof(BUFFER) - 1;  // Should never happen!

		for (i = 0; i < nContentLength; ++i)
		{
			c = EthernetClient(clientId).read();
			//    Serial.print(c); // DEBUG
			content[i] = c;
		}

		content[nContentLength] = 0;  // Null string terminator

		//  Serial.print("Content: ");
		//  Serial.println(content);
	}

	/**
	 * eat headers.
	 * @param clientId
	 * @param readBuffer
	 */
	void getNextHttpLine(uint8_t clientId, BUFFER & readBuffer)
	{
		char c;
		uint8_t bufindex = 0; // reset buffer

		EthernetClient client = EthernetClient(clientId);

		// reading next header of HTTP request
		if (client.connected() && client.available())
		{
			// read a line terminated by CRLF
			readBuffer[0] = client.read();
			readBuffer[1] = client.read();
			bufindex = 2;
			for (int i = 2; readBuffer[i - 2] != '\r' && readBuffer[i - 1] != '\n'; ++i)
			{
				// read full line and save it in buffer, up to the buffer size
				c = client.read();
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
	boolean closeClient(uint8_t clientId) {
		#ifdef DEBUG_WEBSERVER_ENABLE
		Serial.print(millis());
		Serial.print(F(": WebServer::closeClient() clientSlot="));
		Serial.println(clientId);
		#endif

		/*
		//is there a pending request?
		if(EthernetClient(i).available()) {
			#ifdef DEBUG_WEBSERVER
			Serial.print(F("\tnot closing"));
			#endif

			clientStatus[i].waiting = false;

			return false;
		}*/

		delay(1);
		EthernetClient(clientId).stop();
		clientStatus[clientId].inUse = false;
		clientStatus[clientId].requestType = PAGE_NONE;
		clientStatus[clientId].waiting = false;

		if(clientStatus[clientId].callback != NULL && dispatcher.getResponseHandler() != NULL) {
			dispatcher.getResponseHandler()->unregisterListener(clientStatus[clientId].callback);
		}

		return true;
	}

	/**
	 * main loop.
	 */
	void loop() {
		server.available();

		////iterate clients
		for(uint8_t i = 0; i < CLIENT_INSTANCES_NUM; i++) {
			//new connections
			if(clientStatus[i].inUse == false && EthernetClient(i).available()) {
				clientStatus[i].inUse = true;
			}

			//check for answers from listeners
			if(clientStatus[i].waiting == true) {
				if(clientStatus[i].callback != NULL && clientStatus[i].callback->state == webserverListener::FINISHED) {
					handleFinishedCallback(i);
				} else if(clientStatus[i].callback != NULL && clientStatus[i].callback != NULL && (clientStatus[i].callback->state == webserverListener::FAILED || millis() - clientStatus[i].timestamp > TIMEOUT_MILLIS)) {
					//no answer.
					sendHttp500WithBody(i);
				}
			}

			//handle conenctions
			if(clientStatus[i].inUse == true && clientStatus[i].waiting == false) {
				#ifdef DEBUG_WEBSERVER_ENABLE
				Serial.print(millis());
				Serial.print(F(": WebServer::loop() handle clientSlot="));
				Serial.println(i);
				#endif
				doClientHandling(i);
			}
	}
}

	/**
	 * callback has finished, now execute this (usually actual page creation)
	 * @param clintId
	 */
	void handleFinishedCallback(uint8_t clientId) {
		if(clientStatus[clientId].requestType == PAGE_REQUEST_SENSOR) {
			doPageRequestSensor2(clientId);
		//} else if(clientStatus[clientId].requestType == PAGE_GETSENSORINFO) {
			//doPageSensorInfo2(clientId);
		} else if (clientStatus[clientId].requestType == PAGE_WRITE_SENSOR) {
			doPageWriteSensor2(clientId);
		} else {
			//unknown request
			sendHttp500WithBody(clientId);
			closeClient(clientId);
		}
	}

	/**
	 * welcome page
	 * @param clientId
	 */
	void doPageStart(uint8_t clientId) {
		#ifdef DEBUG
		Serial.print(millis());
		Serial.print(F(": doPageStart() on clientId="));
		Serial.println(clientId);
		#endif

		EthernetClient client = EthernetClient(clientId);

		#ifdef DEBUG
		Serial.println(F("\tdoHttpHeaderOK"));
		#endif
		sendHttpOk(clientId);

		#ifdef DEBUG
		Serial.println(F("\tdoHtmlHeader"));
		#endif
		sendHtmlHeader(clientId, pageTitles[PAGE_MAIN]);

		#ifdef DEBUG
		Serial.println(F("\tdoHtmlMenu"));
		#endif
		sendHtmlMenu(clientId);
		client.println(F("<h1>Oh Hai!</h1>"));
		client.println(F("Welcome to the <a href='http://iotduino.de'>iotduino</a> gateway system."));

		#ifdef DEBUG
		Serial.println(F("\tdoHtmlFooter"));
		#endif
		sendHtmlFooter(clientId);
	}

	/**
	 * stylesheet
	 * @param clientId
	 */
	void doPageCss(uint8_t clientId) {
		#ifdef DEBUG
		Serial.print(millis());
		Serial.print(F(": doPageCss() on clientId="));
		Serial.println(clientId);
		#endif
				sendHttpOk(clientId, 300);
		EthernetClient client = EthernetClient(clientId);
		client.println(F("a, a:link, a:visited { color: #5F5F5F; text-decoration: underline; font-weight: normal; }"));
		client.println(F("a:active { font-weight: bold; }"));
		client.println(F("a:hover { text-decoration: none; background-color: #FFD8D8; }"));
		client.println(F("table, th, td, body { font-family: Arial; font-size: 9pt; }"));
		client.println(F("table { border: 1px lightgray dashed; vertical-align: top; padding: 4px; background-position: center; width: 800px; border-spacing: 2px; -webkit-border-horizontal-spacing: 5px; -webkit-border-vertical-spacing: 2px;}"));
		client.println(F("td { font-size: 9pt; border-bottom: 1px dotted; }"));
		client.println(F("th { font-size: 10pt; font-weight: bold; border-bottom: 2px solid }"));
		client.println(F(".bg1 { background-color: #fafafa; }"));
		client.println(F(".bg2 { background-color: #efefef; }"));
		client.println(F(".bg1:hover { background-color: #FFD8D8; }"));
		client.println(F(".bg2:hover { background-color: #FFD8D8; }"));
		client.println(F(".warning { color: red; }"));
		client.println(F(".ok { color: green; }"));
		client.println(F("hr { border:none; height: 1px; background-color: black; }"));
		client.println(F("input { font-size: 9pt; height: 20px; margin: 0; padding: 0px; background-color: white; border: 1px solid darkgray; }"));
		client.println(F("input[type='submit'] { width: 50px; }"));
		client.println(F("input[type='text'] { width: 120px }"));
		client.println(F(".centered { text-align: center; }"));
		client.println(F(".righted { text-align: right; }"));
		client.println(F(".inline { border: none; }"));
		client.flush();
	}

	/**
	 * node overview (from routing table)
	 * @param clientId
	 */
	void doPageNodes(uint8_t clientId) {
		#ifdef DEBUG
		Serial.print(millis());
		Serial.print(F(": doPageNodes() on clientId="));
		Serial.println(clientId);
		#endif

		sendHttpOk(clientId);
		sendHtmlHeader(clientId, pageTitles[PAGE_NODES]);
		sendHtmlMenu(clientId);

		EthernetClient client = EthernetClient(clientId);

		//page title
		client.println(F("<h1>Nodes</h1>"));

		//table
		client.println(F("<table><tr><th>ID</th><th>NodeInfo</th><th>lastDiscovery</th><th>active</th><th>nextHop</th><th>#hops</th><th>routeAge</th><th>info</th></tr>"));
		uint8_t numNodes = 0;
		uint32_t nowSystem = millis();
		//uint32_t rtcTime = now();

		//data to display per node
		char nodeInfoString[NODE_INFO_SIZE];
		boolean neighbourActive;
		uint32_t neighbourLastKeepAlive;
		uint8_t neighbourHops;
		uint8_t neighbourNextHop;

		SDcard::SD_nodeInfoTableEntry_t infoTable;
		////SDcard::SD_nodeDiscoveryInfoTableEntry_t discoveryInfo[SD_DISCOVERY_NUM_INFOS_PER_NODE];
		//ietrate possible nodes
		for(uint8_t i = 1; i < SD_DISCOVERY_NUM_NODES; i++) {
			wdt_reset();

			//get string info
			sdcard.getNodeInfoString(i, (uint8_t*) nodeInfoString, NODE_INFO_SIZE);

			//reset data
			neighbourActive = 0;
			neighbourLastKeepAlive = 0;
			neighbourHops = -1;
			neighbourNextHop = 0;
			//get route info
			getRouteInfoForNode(i, neighbourActive, neighbourLastKeepAlive, neighbourHops, neighbourNextHop);

			//node info
			sdcard.getDiscoveryNodeInfo(i, &infoTable);

			//////discovery info
			////if(infoTable.nodeId != 0) {
				////sdcard.getDiscoveryInfosForNode(i, discoveryInfo, SD_DISCOVERY_NUM_INFOS_PER_NODE);
			////} else {
				////memset(discoveryInfo, 0, sizeof(discoveryInfo));
			////}

			//print node info?
			if(neighbourActive == 1 || infoTable.nodeId != 0) {
				numNodes++;

				//node info
				client.print(F("<tr"));
				sendHtmlBgColorAlternate(clientId, numNodes);
				client.print(F("><td class='righted'>"));
				client.print(i);
				client.print(F("</td><td>"));
				client.print(nodeInfoString);
				client.print(F("</td><td class='righted'>"));

				uint32_t t = infoTable.lastDiscoveryRequest;
				printDate(clientId, t);


				client.print(F("</td><td class='righted'>"));
				client.print(neighbourActive);
				client.print(F("</td><td class='righted'>"));
				client.print(neighbourNextHop);
				client.print(F("</td><td class='righted'>"));
				client.print(neighbourHops);
				client.print(F("</td><td class='righted'>"));
				if(i != l3.localAddress) {
					if(neighbourLastKeepAlive > 0) {
						client.print((nowSystem - neighbourLastKeepAlive) / 1000);
						client.print(F("s"));
					} else {
						client.print(F("<i>no route to host</i>"));
					}
				} else {
					client.print(F(" <i>loopback</i>"));
				}
				//discover
				client.print(F("</td><td class='centered'><a href='"));
				printP(clientId, pageAddresses[PAGE_GETSENSORINFO]);
				client.print(F("?"));
				printP(clientId, variableRemote);
				client.print(F("="));
				client.print(i);
				client.println(F("'>x</a>"));
				client.println(F("</td>"));
				//tr end
				client.println(F("</tr>"));
			}
		}

		//num entries
		client.print(F("<tr><th colspan='8'>"));
		client.print(numNodes);
		client.println(F(" entries</th></tr></table>"));

		//footer
		sendHtmlFooter(clientId);
	}


	void trailing0(uint8_t clientId, uint8_t a) {
		if(a < 10) {
			EthernetClient client = EthernetClient(clientId);
			client.print(F("0"));
		}
	}

	void printDate(uint8_t clientId, uint32_t t) {
		EthernetClient client = EthernetClient(clientId);
		client.print(year(t));
		client.print(F("-"));
		trailing0(clientId, month(t));
		client.print(month(t));
		client.print(F("-"));
		trailing0(clientId, day(t));
		client.print(day(t));
		client.print(F(" "));
		trailing0(clientId, hour(t));
		client.print(hour(t));
		client.print(F(":"));
		trailing0(clientId, minute(t));
		client.print(minute(t));
		client.print(F(":"));
		trailing0(clientId, second(t));
		client.print(second(t));
	}

/**
 * get route infos for a node if available
 * @param nodeId
 * @param neighbourActive
 * @param neighbourLastKeepAlive
 * @param neighbourHops
 * @param neighbourNextHop
 * @return success
 */
boolean getRouteInfoForNode(uint8_t nodeId, boolean &neighbourActive, uint32_t &neighbourLastKeepAlive, uint8_t &neighbourHops, uint8_t &neighbourNextHop) {
	neighbourData* neighbours = l3.getNeighbours();
	for(uint8_t j = 0; j < CONFIG_L3_NUM_NEIGHBOURS; j++) {
		if(neighbours[j].nodeId == nodeId) {
			neighbourActive = 1;
			neighbourLastKeepAlive = neighbours[j].timestamp;
			neighbourHops = neighbours[j].hopCount;
			neighbourNextHop = neighbours[j].hopNextNodeId;
			return true;
		}
	}

	if(nodeId == l3.localAddress) {
		//this is us!
		neighbourActive = 1;
		neighbourLastKeepAlive = millis();
		neighbourHops = 0;
		neighbourNextHop = l3.localAddress;
	}
	return false;
}

	/**
	 * helper for alternating background color str
	 * @param clientId
	 * @param i
	 */
	void sendHtmlBgColorAlternate(uint8_t clientId, uint8_t i) {
		EthernetClient client = EthernetClient(clientId);
		if(i % 2 == 0) {
			client.print(F(" class='bg1'"));
			return;
		}
		client.print(F(" class='bg2'"));
	}

	static boolean hwIsReadable(HardwareTypeIdentifier type) {
		switch(type) {
			case HWType_rcswitch:
			case HWType_tone:
				return false;
			default:
				return true;
		}
	}

	void printExecutableLinks(uint8_t clientId, l3_address_t remote, HardwareTypeIdentifier type, uint8_t address) {
		EthernetClient client = EthernetClient(clientId);

		//conversion buffers
		char buf1[3];
		itoa(remote, buf1, 10);
		char buf2[3];
		itoa(address, buf2, 10);
		char buf3[3];
		itoa(type, buf3, 10);

		//buf for rc switch values.
		//                 012345
		char bufVal[] = { "0x0000" };
		char bufListType[10] = { "" };

		//						0				1					2					3					4
		//						remote id		hw address			hw type				var to write		listtype
		const uint8_t numKV = 5;
		const char* keys[numKV] = {variableRemote,	variableHwAddress,	variableHwType,		variableVal,		variableListType};
		const char* vals[numKV] = {buf1,			buf2,				buf3,				bufVal,				bufListType};


		switch(type) {
			case HWType_relay:
			case HWType_DIGITAL:
				//off
				bufVal[4] = '\0';
				strcpy_P(bufListType, linkCmdListTypeUint8);

				printLink(clientId, pageAddresses[PAGE_WRITE_SENSOR], keys, vals, linkNameOff, numKV);
				client.print(F(" &middot; "));
				//on
				bufVal[3] = '1';
				strcpy_P(bufListType, linkCmdListTypeUint8);
				printLink(clientId, pageAddresses[PAGE_WRITE_SENSOR], keys, vals, linkNameOn, numKV);
				return;
			case HWType_rcswitch:
				client.print(F("<table class='inline' style='width: 99%'"));
				client.print(F("<tr><td class='centered'>1</td><td class='centered'>2</td><td class='centered'>3</td><td class='centered'>4</td><td class='centered'>all</td></tr>"));
				client.print(F("<tr>"));
				strcpy_P(bufListType, linkCmdListTypeUint8);
				for(uint8_t i = 0; i < 5; i++) {
					client.print(F("<td class='inline centered'>"));
					bufVal[5] = '0'+i+1; //device
					bufVal[3] = '0';//val lower
					printLink(clientId, pageAddresses[PAGE_WRITE_SENSOR], keys, vals, linkNameOff, numKV);

					client.print(F(" &middot; "));

					bufVal[3] = '1';//val lower
					printLink(clientId, pageAddresses[PAGE_WRITE_SENSOR], keys, vals, linkNameOn, numKV);
					client.print(F("</td>"));
				}
				client.print(F("</tr>"));
				client.print(F("</table>"));
				return;
			case HWType_ANALOG:
			case HWType_rtc:
			case HWType_tone:
				client.print(F("<form action='"));
				printP(clientId, pageAddresses[PAGE_WRITE_SENSOR]);
				client.print(F("' method='get'>"));

				client.print(F("<input type='hidden' name='"));
				printP(clientId, variableHwAddress);
				client.print(F("' value='"));
				client.print(address);
				client.print(F("'>"));

				client.print(F("<input type='hidden' name='"));
				printP(clientId, variableHwType);
				client.print(F("' value='"));
				client.print(type);
				client.print(F("' style='width: 200px'>"));

				client.print(F("<input type='hidden' name='"));
				printP(clientId, variableRemote);
				client.print(F("' value='"));
				client.print(remote);
				client.print(F("'>"));

				client.print(F("HexInput (msbf): <input type='text' name='"));
				printP(clientId, variableVal);
				client.print(F("' value='"));
				printP(clientId, linkCmdValDefault);
				client.print(F("'/>"));
				client.print(F(" <input type='submit'><br/>"));


				client.print(F(" u8:<input type='radio' name='"));
				printP(clientId, variableListType);
				client.print(F("' value='"));
				printP(clientId, linkCmdListTypeUint8);
				client.print(F("'"));
				printP(clientId, linkCmdListChecked);
				client.print(F("/>"));

				client.print(F(" u16:<input type='radio' name='"));
				printP(clientId, variableListType);
				client.print(F("' value='"));
				printP(clientId, linkCmdListTypeUint16);
				client.print(F("'/>"));

				client.print(F(" i8:<input type='radio' name='"));
				printP(clientId, variableListType);
				client.print(F("' value='"));
				printP(clientId, linkCmdListTypeInt8);
				client.print(F("'/>"));

				client.print(F(" i16:<input type='radio' name='"));
				printP(clientId, variableListType);
				client.print(F("' value='"));
				printP(clientId, linkCmdListTypeInt16);
				client.print(F("'/>"));

				client.print(F("</form>"));
				return;
			default:
				client.print('-');
		}
	}

	void printLink(uint8_t clientId, const char* baseUrl, const char** keys, const char** vals, const char* name, uint8_t num) {
		EthernetClient client = EthernetClient(clientId);

		#ifdef DEBUG_WEBSERVER_ENABLE
		Serial.print(millis());
		Serial.print(F(": printLink() baseUrl="));
		serialPrintP(baseUrl);
		Serial.println();
		#endif

		client.print(F("<a href='"));
		printP(clientId, baseUrl);
		client.print('?');

		for(uint8_t i = 0; i < num; i++) {
			#ifdef DEBUG_WEBSERVER_ENABLE
			Serial.print(F("\t"));
			serialPrintP(keys[i]);
			Serial.print('=');
			Serial.println(vals[i]);
			Serial.flush();
			#endif

			if(i > 0)
				client.print('&');
			printP(clientId, keys[i]);
			client.print('=');
			client.print(vals[i]);
		}

		client.print(F("'>"));
		printP(clientId, name);
		client.print(F("</a>"));
	}

	/**
	 * discovery has finished - print result
	 * @param clientId
	 */
	void doPageSensorInfo2(uint8_t clientId, RequestContent* req) {
		#ifdef DEBUG
		Serial.print(millis());
		Serial.print(F(": doPageSensorInfo2() on clientId="));
		Serial.println(clientId);
		#endif

		//yay!
		EthernetClient client = EthernetClient(clientId);

		if(req == NULL) {
			sendHttp500WithBody(clientId);
			return;
		}

		String* id = req->getValue(variableRemote);
		uint8_t idInt = id->toInt();

		if(id == NULL) {
			sendHttp500WithBody(clientId);
			return;
		}

		sendHttpOk(clientId);
		sendHtmlHeader(clientId, pageTitles[PAGE_GETSENSORINFO]);
		sendHtmlMenu(clientId);

		//general info
		char nodeInfoString[NODE_INFO_SIZE];
		sdcard.getNodeInfoString(idInt, (uint8_t*) nodeInfoString, NODE_INFO_SIZE);

		SDcard::SD_nodeInfoTableEntry_t infoTable;
		sdcard.getDiscoveryNodeInfo(idInt, &infoTable);

		boolean neighbourActive = 0;
		uint32_t neighbourLastKeepAlive = 0;
		uint8_t neighbourHops = -1;
		uint8_t neighbourNextHop = 0;
		getRouteInfoForNode(idInt, neighbourActive, neighbourLastKeepAlive, neighbourHops, neighbourNextHop);


		client.print(F("<h1>Sensor Info id="));
		client.print(idInt);
		client.print(F(" ("));
		if(strlen(nodeInfoString) > 0) {
			client.print(nodeInfoString);
		} else {
			client.print(F("<i>NA</i>"));
		}
		client.println(F(")</h1>"));

		client.print(F("<p>"));

		if(neighbourActive == 0) {
			client.print(F("<span class='warning'>"));
		} else {
			client.print(F("<span class='ok'>"));
		}
		client.print(F("active="));
		client.print(neighbourActive);
		client.print(F("</span>"));

		client.print(F("<br/>nextHop="));
		client.print(neighbourNextHop);
		client.print(F("<br/>hops="));
		client.print(neighbourHops);
		client.print(F("<br/>lastKeepAliveAge="));
		client.print((millis() - neighbourLastKeepAlive) / 1000);
		client.print(F("s</p>"));

		SDcard::SD_nodeDiscoveryInfoTableEntry_t discoveryInfo[SD_DISCOVERY_NUM_INFOS_PER_NODE];
		sdcard.getDiscoveryInfosForNode(idInt, discoveryInfo, SD_DISCOVERY_NUM_INFOS_PER_NODE);
		client.println(F("<table><tr><th>HardwareAddress</th><th>HardwareType</th><th>LastUpdated</th><th>requestSensor</th><th>writeSensor</th></tr>"));
		uint8_t numInfos = 0;
		//conversion buffers
		char buf1[3];
		char buf2[3];
		char buf3[3];
		uint8_t num = 0;
		for(uint8_t i = 0; i < SD_DISCOVERY_NUM_INFOS_PER_NODE; i++) {
			if(discoveryInfo[i].hardwareAddress > 0 && discoveryInfo[i].hardwareType > 0) {
				//table
				client.print(F("<tr "));
				sendHtmlBgColorAlternate(clientId, num);
				client.print(F(">"));
				num++;

				client.print(F("<td class='righted'>"));
				client.print(discoveryInfo[i].hardwareAddress);
				client.print(F("</td>"));

				client.print(F("<td>"));
				printP(clientId, hardwareTypeStrings[discoveryInfo[i].hardwareType]);
				client.print(F("</td>"));

				client.print(F("<td class='righted'>"));
				printDate(clientId, discoveryInfo[i].rtcTimestamp);
				client.print(F("</td>"));

				client.print(F("<td class='centered'>"));
				if(hwIsReadable((HardwareTypeIdentifier) discoveryInfo[i].hardwareType)) {
					itoa(idInt, buf1, 10);
					itoa(discoveryInfo[i].hardwareAddress, buf2, 10);
					itoa(discoveryInfo[i].hardwareType, buf3, 10);
					const char* keys[3] = {variableRemote, variableHwAddress, variableHwType};
					const char* vals[3] = {buf1, buf2, buf3};
					printLink(clientId, pageAddresses[PAGE_REQUEST_SENSOR], keys, vals, linkNameX, 3);
				} else {
					client.print(F("-"));
				}
				client.print(F("</td>"));

				client.print(F("<td class='centered'>"));
				printExecutableLinks(clientId, idInt, (HardwareTypeIdentifier) discoveryInfo[i].hardwareType, discoveryInfo[i].hardwareAddress);
				client.print(F("</td>"));

				client.print(F("</tr>"));
				client.print(F("</form>"));

				numInfos++;
			}
		}
		client.print(F("<tr><th colspan='5'>"));
		client.print(numInfos);
		client.println(F(" entries</th></tr></table>"));

		sendHtmlFooter(clientId);

		//listener->init(0, webserverListener::START);
		closeClient(clientId);
	}

	/**
	 * first method when client connects - read http request
	 * @param clientId
	 */
	void doClientHandling(uint8_t clientId) {
		#ifdef DEBUG
		Serial.print(millis());
		Serial.print(F(": doClientHandling() on clientId="));
		Serial.println(clientId);
		#endif

		// now client is connected to arduino we need to extract the
		// following fields from the HTTP request.
		String    uri;  // Gives the index into table of recognized URIs or -1 for not found.

		RequestContent req;
		MethodType eMethod = readHttpRequest(clientId, &uri, &req);

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
			doPageStart(clientId);
		} else if(strcmp_P(uriChars, pageAddresses[PAGE_NODES]) == 0) {
			doPageNodes(clientId);
		} else if(strcmp_P(uriChars, pageAddresses[PAGE_CSS]) == 0) {
			doPageCss(clientId);
		} else if(strcmp_P(uriChars, pageAddresses[PAGE_GETSENSORINFO]) == 0) {
			//doPageSensorInfo(clientId, &req);
			//clientStatus[clientId].requestType = PAGE_GETSENSORINFO;
			//clientStatus[clientId].waiting = true;
			//clientStatus[clientId].timestamp = millis();
			doPageSensorInfo2(clientId, &req);
		} else if(strcmp_P(uriChars, pageAddresses[PAGE_REQUEST_SENSOR]) == 0) {
			doPageRequestSensor(clientId, &req);
			clientStatus[clientId].requestType = PAGE_REQUEST_SENSOR;
			clientStatus[clientId].waiting = true;
			clientStatus[clientId].timestamp = millis();
		} else if(strcmp_P(uriChars, pageAddresses[PAGE_WRITE_SENSOR]) == 0) {
			doPageWriteSensor(clientId, &req);
			clientStatus[clientId].requestType = PAGE_WRITE_SENSOR;
			clientStatus[clientId].waiting = true;
			clientStatus[clientId].timestamp = millis();
		} else {
			sendHttp404WithBody(clientId);
		}

		#ifdef DEBUG
		Serial.println(F("\tpage sent out. checking for connection close."));
		#endif

		if(!clientStatus[clientId].waiting) {
			#ifdef DEBUG
			Serial.println(F("\tnot waiting, do close."));
			#endif
			closeClient(clientId);
		}
	}

	/**
	 * initiate sensor write
	 * @param clientId
	 * @param req
	 * TODO! ****************************************************************************************************************************************************************
	 */
	void doPageWriteSensor(uint8_t clientId, RequestContent* req) {
		#ifdef DEBUG
		Serial.print(millis());
		Serial.print(F(": doPageWriteSensor() on clientId="));
		Serial.println(clientId);
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
			sendHttp500WithBody(clientId);
			return;
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
			sendHttp500WithBody(clientId);
			closeClient(clientId);
			return;
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
			sendHttp500WithBody(clientId);
			closeClient(clientId);
			return;
		}

		Layer3::packet_t p;
		cmd.setAddress(hwaddress);
		cmd.setHardwareType((HardwareTypeIdentifier) hwtype);
		seq_t sequence = pf.generateHardwareCommandWrite(&p, idInt, &cmd);
		listenerHardwareRequest.init(idInt, (HardwareTypeIdentifier) hwtype, hwaddress, webserverListener::AWAITING_ANSWER);

		clientStatus[clientId].callback = &listenerHardwareRequest;

		boolean success = dispatcher.getResponseHandler()->registerListenerBySeq(millis()+TIMEOUT_MILLIS, sequence, idInt, &listenerHardwareRequest);

		success &= l3.sendPacket(p);

		if(!success) {
			sendHttp500WithBody(clientId);
			closeClient(clientId);
			dispatcher.getResponseHandler()->unregisterListener(&listenerHardwareRequest);
			return;
		}
	}

	void doPageWriteSensor2(uint8_t clientId) {
		#ifdef DEBUG
		Serial.print(millis());
		Serial.print(F(": doPageWriteSensor2() on clientId="));
		Serial.println(clientId);
		#endif

		EthernetClient client = EthernetClient(clientId);
		hardwareRequestListener* listener = (hardwareRequestListener*) clientStatus[clientId].callback;

		sendHttpOk(clientId);
		sendHtmlHeader(clientId, pageTitles[PAGE_WRITE_SENSOR], false);
		sendHtmlMenu(clientId);
		client.print(F("<h1>Sensor Write id="));
		client.print(listener->remote);
		client.println(F("</h1>"));
		client.print(F("<h2>hwaddress="));
		client.print(listener->hwaddress);
		client.print(F(" hwtype="));
		printP(clientId, hardwareTypeStrings[listener->hwtype]);
		if(listener->cmd.isRead) {
			client.print(F(" read"));
			} else {
			client.print(F(" write"));
		}
		client.println(F("</h2>"));

		sendHtmlFooter(clientId);
		listener->init(0, HWType_UNKNOWN, 0, webserverListener::START);

		closeClient(clientId);
	}


	/**
	 * initiate sensor reading
	 * @param clientId
	 * @param req
	 */
	void doPageRequestSensor(uint8_t clientId, RequestContent* req) {
		#ifdef DEBUG
		Serial.print(millis());
		Serial.print(F(": doPageRequestSensor() on clientId="));
		Serial.println(clientId);
		#endif

		String* id = req->getValue(variableRemote);
		String* hwAddressStr = req->getValue(variableHwAddress);
		String* hwtypeStr = req->getValue(variableHwType);
		if(id == NULL || hwAddressStr == NULL || hwtypeStr == NULL) {
			sendHttp500WithBody(clientId);
			return;
		}

		int8_t idInt = id->toInt();
		int8_t hwaddress = hwAddressStr->toInt();
		int8_t hwtype = hwtypeStr->toInt();

		if(idInt == -1 || hwaddress == -1 || hwtype == -1) {
			sendHttp500WithBody(clientId);
			closeClient(clientId);
			return;
		}

		Layer3::packet_t p;
		seq_t sequence = pf.generateHardwareCommandRead(&p, idInt, hwaddress, (HardwareTypeIdentifier) hwtype);
		listenerHardwareRequest.init(idInt, (HardwareTypeIdentifier) hwtype, hwaddress, webserverListener::AWAITING_ANSWER);

		clientStatus[clientId].callback = &listenerHardwareRequest;

		boolean success = dispatcher.getResponseHandler()->registerListenerBySeq(millis()+TIMEOUT_MILLIS, sequence, idInt, &listenerHardwareRequest);

		success &= l3.sendPacket(p);

		if(!success) {
			sendHttp500WithBody(clientId);
			closeClient(clientId);
			dispatcher.getResponseHandler()->unregisterListener(&listenerHardwareRequest);
			return;
		}
	}

	/**
	 * sensor reading has finished - print
	 * @param clientId
	 */
	void doPageRequestSensor2(uint8_t clientId) {
		#ifdef DEBUG
		Serial.print(millis());
		Serial.print(F(": doPageRequestSensor2() on clientId="));
		Serial.println(clientId);
		#endif

		EthernetClient client = EthernetClient(clientId);
		hardwareRequestListener* listener = (hardwareRequestListener*) clientStatus[clientId].callback;

		sendHttpOk(clientId);
		sendHtmlHeader(clientId, pageTitles[PAGE_REQUEST_SENSOR], false);
		sendHtmlMenu(clientId);
		client.print(F("<h1>Sensor Info id="));
		client.print(listener->remote);
		client.println(F("</h1>"));
		client.print(F("<h2>hwaddress="));
		client.print(listener->hwaddress);
		client.print(F(" hwtype="));
		printP(clientId, hardwareTypeStrings[listener->hwtype]);
		if(listener->cmd.isRead) {
			client.print(F(" read"));
		} else {
			client.print(F(" write"));
		}
		client.println(F("</h2>"));

		client.println(F("<code><pre>"));
		if(listener->cmd.numUint8 > 0) {
			client.print(F("Uint8:\t"));
			for(uint8_t i = 0; i < listener->cmd.numUint8; i++)	 {
				client.print(listener->cmd.uint8list[i]);
				client.print(F("\t"));
			}
			client.println(F("<br/>"));
		}
		if(listener->cmd.numUint16 > 0) {
			client.print(F("Uint16:\t"));
			for(uint8_t i = 0; i < listener->cmd.numUint16; i++) {
				client.print(listener->cmd.uint16list[i]);
				client.print(F("\t"));
			}
			client.println(F("<br/>"));
		}
		if(listener->cmd.numInt8 > 0) {
			client.print(F("Int8:\t"));
			for(uint8_t i = 0; i < listener->cmd.numInt8; i++) {
				client.print(listener->cmd.int8list[i]);
				client.print(F("\t"));
			}
			client.println(F("<br/>"));
		}
		if(listener->cmd.numInt16 > 0) {
			client.print(F("Int16:\t"));
			for(uint8_t i = 0; i < listener->cmd.numInt16; i++) {
				client.print(listener->cmd.int16list[i]);
				client.print(F("\t"));
			}
			client.println(F("<br/>"));
		}
		client.println(F("</pre></code>"));

		sendHtmlFooter(clientId);
		listener->init(0, HWType_UNKNOWN, 0, webserverListener::START);

		closeClient(clientId);
	}


};

#endif /* WEBSERVER_H_ */
