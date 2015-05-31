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

#include <networking/Layer3.h>
#include <dispatcher/PacketDispatcher.h>
#include <drivers/HardwareID.h>

extern Layer3 l3;
extern PacketDispatcher dispatcher;
extern PacketFactory pf;

#define DEBUG_WEBSERVER

#define STRING_BUFFER_SIZE 128
//#define USE_DHCP_FOR_IP_ADDRESS


EthernetServer server(80);
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
#ifndef USE_DHCP_FOR_IP_ADDRESS
	byte ip[] = { 192, 168, 0, 177 };
#endif


class webserverListener : public EventCallbackInterface {
	public:
	enum STATE {START, AWAITING_ANSWER, FINISHED, FAILED};
	STATE state;
};

class discoveryListener : public webserverListener {
	public:

	int8_t totalInfos;
	uint8_t gottenInfos;
	l3_address_t remote;

	packet_application_numbered_discovery_info_helper_t sensorInfos[INTERFACES_BUF_SIZE];

	void init(l3_address_t remote) {
		this->state = START;
		this->totalInfos = -1;
		this->gottenInfos = 0;
		this->remote = remote;
		memset(sensorInfos, 0, sizeof(sensorInfos));
	}

	virtual void doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq) {
		if(address != remote || appLayerPacket == NULL || appLayerPacket->packetType != HARDWARE_DISCOVERY_RES)
			return;

		packet_application_numbered_discovery_info_t* info = (packet_application_numbered_discovery_info_t*) appLayerPacket->payload;
		//how many infos do we expect?
		if(totalInfos == -1) {
			if(info->numTotalSensors > INTERFACES_BUF_SIZE) {
				state = FAILED;
				return;
			}

			state = AWAITING_ANSWER;
			totalInfos = info->numTotalSensors;
		}

		//copy
		memcpy(&sensorInfos[gottenInfos], info->infos, info->numSensors * sizeof(packet_application_numbered_discovery_info_helper_t));
		gottenInfos += info->numSensors;

		if(gottenInfos == totalInfos) {
			state = FINISHED;
		}
	}

	virtual void fail(seq_t seq, l3_address_t remote) {
		state = FAILED;
	}
};


class hardwareRequestListener : public webserverListener {
	public:
	l3_address_t remote;

	command_t cmd;
	HardwareTypeIdentifier hwtype;
	uint8_t hwaddress;

	void init (l3_address_t remote, HardwareTypeIdentifier hwtype, uint8_t hwaddress) {
		this->remote = remote;
		this->state = AWAITING_ANSWER;
		this->hwtype = hwtype;
		this->hwaddress = hwaddress;

		memset(&cmd, 0, sizeof(cmd));
	}

	virtual void doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq) {
		if(address != remote || appLayerPacket == NULL || appLayerPacket->packetType != HARDWARE_COMMAND_RES)
			return;

		command_t* info = (command_t*) appLayerPacket->payload;

		this->cmd = *info;

		state = FINISHED;
	}

	virtual void fail(seq_t seq, l3_address_t remote) {
		state = FAILED;
	}
};

/**
 * tokenises string <code>key1=val1&key2=val2</code>
 */
class RequestContent {
	#define RequestContentNumKeys 10

	public:

	String keys[RequestContentNumKeys];
	String values[RequestContentNumKeys];
	uint8_t num;

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

	int8_t putKey(String key, String value) {
		int8_t index = hasKey(key);

		if(index == -1) {
			index = num;
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

const char pageAddressMain[] PROGMEM = {"/"};
const char pageAddressGetSensorInfo[] PROGMEM = {"/getSensorInfo"};
const char pageNodes[] PROGMEM = {"/nodes"};
const char pageCss[] PROGMEM = {"/css"};
const char pageRequestSensor[] PROGMEM = {"/sensor"};
PGM_P pageAddresses[] = {NULL, pageAddressMain, pageAddressGetSensorInfo, pageNodes, pageCss, pageRequestSensor};

const char pageTitleMain[] PROGMEM = {"Start"};
const char pageTitleGetSensorInfo[] PROGMEM = {"Sensor Info"};
const char pageTitleNodes[] PROGMEM = {"Nodes"};
const char pageTitleRequestSensor[] PROGMEM = {"Rquested Sensor Information"};

PGM_P pageTitles[] = {NULL, pageTitleMain, pageTitleGetSensorInfo, pageTitleNodes, NULL, pageTitleRequestSensor};
enum PAGES {PAGE_NONE, PAGE_MAIN, PAGE_GETSENSORINFO, PAGE_NODES, PAGE_CSS, PAGE_REQUEST_SENSOR};

const char variableRemote[] PROGMEM = {"remote"};
const char variableHwAddress[] PROGMEM = {"hwaddress"};
const char variableHwType[] PROGMEM = {"hwtype"};

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

class WebServer {

	public:
	enum MethodType {
		MethodUnknown,
		MethodGet,
		MethodPost,
		MethodHead
	};

	#define TIMEOUT_MILLIS (2*1000)

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

	discoveryListener listenerDiscovery;
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
	}

	// Http header token delimiters
	const char *pSpDelimiters = " \r\n";
	const char *pStxDelimiter = "\002";    // STX - ASCII start of text character


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

	/** HTML */
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

	void sendHttp500WithBody(uint8_t clientId) {
		EthernetClient client = EthernetClient(clientId);
		client.println(F("HTTP/1.1 500 Internal error"));
		client.println(F("Content-Type: text/html"));
		//client.println(F("Content-Length: 19"));
		client.println(F("Connection: close"));  // the connection will be closed after completion of the response
		client.println();
		client.println(F("500 Internal error."));

		closeClient(clientId);
	}

	void sendHttpOk(uint8_t clientId) {
		EthernetClient client = EthernetClient(clientId);
		client.println(F("HTTP/1.1 200 OK"));
		client.println(F("Content-Type: text/html"));
		client.println(F("Connection: close"));  // the connection will be closed after completion of the response
		client.println();
	}

	void sendHtmlHeader(uint8_t clientId, PGM_P title) {
		EthernetClient client = EthernetClient(clientId);
		client.print(F("<html><header><link rel='stylesheet' href='css' type='text/css'><title>"));
		printP(clientId, title);
		client.println(F("</title></header><body>"));
	}

	void sendHtmlMenu(uint8_t clientId) {
		EthernetClient client = EthernetClient(clientId);
		uint8_t menuPages[] = {PAGE_MAIN, PAGE_NODES};
		client.print("<h1>Menu</h1><ul>");

		for(uint8_t i = 0; i < sizeof(menuPages); i++) {
			client.print("<li><a href='");
			this->printP(clientId, pageAddresses[menuPages[i]]);
			client.print("'>");
			this->printP(clientId, pageTitles[menuPages[i]]);
			client.print("</a></li>");
		}

		client.print("</ul><br/><hr/><br/>");
	}

	void sendHtmlFooter(uint8_t clientId) {
		EthernetClient client = EthernetClient(clientId);
		client.println(F("<br/><hr/><br/><span style='text-align: right; font-style: italic;'><a href='http://iotduino.de'>iotduino</a> webserver.</span></body></html>"));
	}


	// Read HTTP request, setting Uri Index, the requestContent and returning the method type.
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

	// Read the first line of the HTTP request, setting Uri Index and returning the method type.
	// If it is a GET method then we set the requestContent to whatever follows the '?'. For a other
	// methods there is no content except it may get set later, after the headers for a POST method.
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

	// Read each header of the request till we get the terminating CRLF
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

	// Read the entity body of given length (after all the headers) into the buffer.
	void readEntityBody(uint8_t clientId, int nContentLength, BUFFER & content)
	{
		int i;
		char c;

		if (nContentLength >= (int) sizeof(content))
		nContentLength = sizeof(content) - 1;  // Should never happen!

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
				if (bufindex < sizeof(readBuffer))
				readBuffer[bufindex++] = c;
			}
			readBuffer[bufindex - 2] = 0;  // Null string terminator overwrites '\r'
		}
	}


	boolean closeClient(uint8_t i) {
		#ifdef DEBUG_WEBSERVER
		Serial.print(millis());
		Serial.print(F(": WebServer::closeClient() clientSlot="));
		Serial.println(i);
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
		EthernetClient(i).stop();
		clientStatus[i].inUse = false;
		clientStatus[i].requestType = PAGE_NONE;
		clientStatus[i].waiting = false;

		if(clientStatus[i].callback != NULL) {
			dispatcher.getResponseHandler()->unregisterListener(clientStatus[i].callback);
		}

		return true;
	}

	void loop() {
		server.available();

		//iterate clients
		for(uint8_t i = 0; i < CLIENT_INSTANCES_NUM; i++) {
			//new connections
			if(clientStatus[i].inUse == false && EthernetClient(i).available()) {
				clientStatus[i].inUse = true;
			}

			//check for answers from listeners
			if(clientStatus[i].waiting == true) {
				if(clientStatus[i].callback != NULL && clientStatus[i].callback->state == webserverListener::FINISHED) {
					handleFinishedCallback(i);
				} else if(clientStatus[i].callback != NULL && (clientStatus[i].callback->state == webserverListener::FAILED || millis() - clientStatus[i].timestamp > TIMEOUT_MILLIS)) {
					//no answer.
					sendHttp500WithBody(i);
				}
			}

			//handle conenctions
			if(clientStatus[i].inUse == true && clientStatus[i].waiting == false) {
				#ifdef DEBUG_WEBSERVER
				Serial.print(millis());
				Serial.print(F(": WebServer::loop() handle clientSlot="));
				Serial.println(i);
				#endif
				doClientHandling(i);
			}
		}
	}

	void handleFinishedCallback(uint8_t clientId) {
		if(clientStatus[clientId].requestType == PAGE_REQUEST_SENSOR) {
			doPageRequestSensor2(clientId);
		} else if(clientStatus[clientId].requestType == PAGE_GETSENSORINFO) {
			doPageSensorInfo2(clientId);
		} else {
			//unknown request
			sendHttp500WithBody(clientId);
			closeClient(clientId);
		}
	}

	void doPageStart(uint8_t clientId) {
		EthernetClient client = EthernetClient(clientId);
		sendHttpOk(clientId);
		sendHtmlHeader(clientId, pageTitles[PAGE_MAIN]);
		sendHtmlMenu(clientId);
		client.println(F("<h1>Oh Hai!</h1>"));
		client.println(F("Welcome to the <a href='http://iotduino.de'>iotduino</a> gateway system."));
		sendHtmlFooter(clientId);
	}

	void doPageCss(uint8_t clientId) {
		sendHttpOk(clientId);
		EthernetClient client = EthernetClient(clientId);
		client.println(F("a, a:link, a:visited { color: #5F5F5F; text-decoration: underline; font-weight: normal; }"));
		client.println(F("a:active { font-weight: bold; }"));
		client.println(F("a:hover { text-decoration: none; background-color: #FFD8D8; }"));
		client.println(F("table, th, td, body { font-family: Arial; font-size: 9pt; }"));
		client.println(F("table { border: none; vertical-align: top; padding: 1px; background-position: center; width: 800px}"));
		client.println(F("td { padding: 0px; font-size: 9pt; border-bottom: 1px dotted; }"));
		client.println(F("th { font-size: 10pt; font-weight: bold; border-bottom: 2px solid; padding: 0px; }"));
		client.println(F(".bg1 { background-color: #fafafa; }"));
		client.println(F(".bg2 { background-color: #efefef; }"));
		client.println(F(".bg1:hover { background-color: #FFD8D8; }"));
		client.println(F(".bg2:hover { background-color: #FFD8D8; }"));
		client.println(F("hr { border:none; height: 1px; background-color: black; }"));
	}

	void doPageNodes(uint8_t clientId) {
		neighbourData* neighbours = l3.getNeighbours();

		if(neighbours == NULL) {
			sendHttp500WithBody(clientId);
			return;
		}

		sendHttpOk(clientId);
		sendHtmlHeader(clientId, pageTitles[PAGE_MAIN]);
		sendHtmlMenu(clientId);

		EthernetClient client = EthernetClient(clientId);

		//page title
		client.println(F("<h1>Nodes</h1>"));

		//table
		client.println(F("<table><tr><th>ID</th><th>nextHop</th><th>#hops</th><th>age</th><th>discover</th></tr>"));
		uint8_t numNeighbours = 0;
		uint32_t now = millis();
		for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
			if(neighbours[i].nodeId != 0) {
				numNeighbours++;

				//node info
				client.print(F("<tr"));
				sendHtmlBgColorAlternate(client, i);
				client.print(F("><td>"));
				client.print(neighbours[i].nodeId);
				client.print(F("</td><td>"));
				client.print(neighbours[i].hopNextNodeId);
				client.print(F("</td><td>"));
				client.print(neighbours[i].hopCount);
				client.print(F("</td><td>"));
				client.print((now - neighbours[i].timestamp) / 1000);
				client.print(F("s</td>"));
				//discover
				client.print(F("<td><a href='"));
				printP(clientId, pageAddresses[PAGE_GETSENSORINFO]);
				client.print(F("?"));
				printP(clientId, variableRemote);
				client.print(F("="));
				client.print(neighbours[i].nodeId);
				client.println(F("'>x</a>"));
				client.println(F("</td>"));
				//tr end
				client.println(F("</tr>"));
			}
		}

		//num entries
		client.print(F("<tr><th colspan='5'>"));
		client.print(numNeighbours);
		client.println(F(" entries</th></tr></table>"));

		//footer
		sendHtmlFooter(client);
	}

	void sendHtmlBgColorAlternate(uint8_t clientId, uint8_t i) {
		EthernetClient client = EthernetClient(clientId);
		if(i % 2 == 0) {
			client.print(F(" class='bg1'"));
			return;
		}
		client.print(F(" class='bg2'"));
	}

	void doPageSensorInfo(uint8_t clientId, RequestContent* req) {
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

		Layer3::packet_t p;
		pf.generateDiscoveryInfoRequest(&p, idInt);
		listenerDiscovery.init(idInt);

		clientStatus[clientId].callback = &listenerDiscovery;

		boolean success = dispatcher.getResponseHandler()->registerListenerByPacketType(millis()+TIMEOUT_MILLIS, HARDWARE_DISCOVERY_RES, idInt, &listenerDiscovery);


		if(success) {
			success = l3.sendPacket(p);
		}
		if(!success) {
			sendHttp500WithBody(clientId);
			dispatcher.getResponseHandler()->unregisterListener(&listenerDiscovery);
			return;
		}
	}

	void doPageSensorInfo2(uint8_t clientId) {
		//yay!
		EthernetClient client = EthernetClient(clientId);
		discoveryListener* listener = (discoveryListener*) clientStatus[clientId].callback;

		if(listener == NULL) {
			sendHttp500WithBody(clientId);
			return;
		}

		sendHttpOk(clientId);
		sendHtmlHeader(clientId, pageTitles[PAGE_GETSENSORINFO]);
		sendHtmlMenu(clientId);
		client.print(F("<h1>Sensor Info id="));
		client.print(listener->remote);
		client.println(F("</h1>"));

		client.println(F("<table><tr><th>HardwareAddress</th><th>HardwareType</th><th>HasEvents</th><th>RequestInfo</th></tr>"));
		for(uint8_t i = 0; i < listener->gottenInfos; i++) {
			client.print(F("<tr"));
			sendHtmlBgColorAlternate(client, i);
			client.print(F("><td>"));
			client.print(listener->sensorInfos[i].hardwareAddress);
			client.print(F("</td><td>"));

			uint8_t tmp = listener->sensorInfos[i].hardwareType;
			if(tmp > sizeof(hardwareTypeStrings)) tmp = 0;
			printP(clientId, hardwareTypeStrings[tmp]);

			client.print(F("</td><td>"));
			client.print(listener->sensorInfos[i].canDetectEvents);
			client.print(F("<td><a href='"));
			printP(clientId, pageAddresses[PAGE_REQUEST_SENSOR]);
			client.print(F("?"));
			printP(clientId, variableRemote);
			client.print(F("="));
			client.print(listener->remote);
			client.print(F("&"));
			printP(clientId, variableHwAddress);
			client.print(F("="));
			client.print(listener->sensorInfos[i].hardwareAddress);
			client.print(F("&"));
			printP(clientId, variableHwType);
			client.print(F("="));
			client.print(listener->sensorInfos[i].hardwareType);
			client.print(F("'>x</a>"));
			client.print(F("</td>"));
			client.println(F("</tr>"));
		}
		client.print(F("<tr><th colspan='4'>"));
		client.print(listener->gottenInfos);
		client.println(F(" entries</th></tr></table>"));

		sendHtmlFooter(clientId);

		closeClient(clientId);
	}

	void doClientHandling(uint8_t clientId) {

		// now client is connected to arduino we need to extract the
		// following fields from the HTTP request.
		String    uri;  // Gives the index into table of recognized URIs or -1 for not found.

		RequestContent req;
		MethodType eMethod = readHttpRequest(clientId, &uri, &req);

		#ifdef DEBUG
		Serial.print("Read Request type: ");
		Serial.print(eMethod);
		Serial.print(" Uri: ");
		Serial.print(uri);
		Serial.println(" content: ");
		for(uint8_t i = 0; i < req.getNum(); i++) {
			Serial.print(F("\t"));
			Serial.print(req.getKeys()[i]);
			Serial.print(F("="));
			Serial.println(req.getValues()[i]);
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
			doPageSensorInfo(clientId, &req);
			clientStatus[clientId].requestType = PAGE_GETSENSORINFO;
			clientStatus[clientId].waiting = true;
			clientStatus[clientId].timestamp = millis();
		} else if(strcmp_P(uriChars, pageAddresses[PAGE_REQUEST_SENSOR]) == 0) {
			doPageRequestSensor(clientId, &req);
			clientStatus[clientId].requestType = PAGE_REQUEST_SENSOR;
			clientStatus[clientId].waiting = true;
			clientStatus[clientId].timestamp = millis();
		} else {
			sendHttp404WithBody(clientId);
		}

		if(!clientStatus[clientId].waiting) {
			closeClient(clientId);
		}
	}

	void doPageRequestSensor(uint8_t clientId, RequestContent* req) {
		String* id = req->getValue(variableRemote);
		if(id == NULL) {
			sendHttp500WithBody(clientId);
			return;
		}
		int8_t idInt = id->toInt();

		String* hwAddressStr = req->getValue(variableHwAddress);
		if(hwAddressStr == NULL) {
			sendHttp500WithBody(clientId);
			return;
		}
		int8_t hwaddress = hwAddressStr->toInt();

		String* hwtypeStr = req->getValue(variableHwType);
		if(hwtypeStr == NULL) {
			sendHttp500WithBody(clientId);
			return;
		}
		int8_t hwtype = hwtypeStr->toInt();

		if(idInt == -1 || hwaddress == -1 || hwtype == -1) {
			sendHttp500WithBody(clientId);
			closeClient(clientId);
			return;
		}

		Layer3::packet_t p;
		seq_t sequence = pf.generateHardwareCommandRead(&p, idInt, hwaddress, (HardwareTypeIdentifier) hwtype);
		listenerHardwareRequest.init(idInt, (HardwareTypeIdentifier) hwtype, hwaddress);

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

	void doPageRequestSensor2(uint8_t clientId) {
		EthernetClient client = EthernetClient(clientId);
		hardwareRequestListener* listener = (hardwareRequestListener*) clientStatus[clientId].callback;

		sendHttpOk(clientId);
		sendHtmlHeader(clientId, pageTitles[PAGE_REQUEST_SENSOR]);
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
		closeClient(clientId);
	}


};


#endif /* WEBSERVER_H_ */