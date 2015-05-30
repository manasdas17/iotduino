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



class discoveryListener : public EventCallbackInterface {
	enum STATE {START, AWAITING_ANSWER, FINISHED, FAILED};
	STATE state;
	int8_t totalInfos;
	uint8_t gottenInfos;
	l3_address_t remote;

	packet_application_numbered_discovery_info_helper_t sensorInfos[INTERFACES_BUF_SIZE];

	public:

	discoveryListener(l3_address_t remote) {
		this->state = START;
		this->totalInfos = -1;
		this->gottenInfos = 0;
		this->remote = remote;
		memset(sensorInfos, 0, sizeof(sensorInfos));
	}

	virtual void doCallback(packet_application_numbered_cmd_t* appLayerPacket, l3_address_t address, seq_t seq) {
		if(address != remote)
		return;
		if(appLayerPacket->packetType != HARDWARE_DISCOVERY_RES)
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
PGM_P pageAddresses[] = {NULL, pageAddressMain, pageAddressGetSensorInfo, pageNodes, pageCss};

const char pageTitleMain[] PROGMEM = {"Start"};
const char pageTitleGetSensorInfo[] PROGMEM = {"Sensor Info"};
const char pageTitleNodes[] PROGMEM = {"Nodes"};

PGM_P pageTitles[] = {NULL, pageTitleMain, pageTitleGetSensorInfo, pageTitleNodes, NULL};
enum PAGES {PAGE_NONE, PAGE_MAIN, PAGE_GETSENSORINFO, PAGE_NODES, PAGE_CSS};

const char variableRemote[] PROGMEM = {"remote"};
const char variableHwAddress[] PROGMEM = {"hwaddress"};
const char variableHwType[] PROGMEM = {"hwtype"};

class WebServer {

	public:
	enum MethodType {
		MethodUnknown,
		MethodGet,
		MethodPost,
		MethodHead
	};

	typedef char BUFFER[STRING_BUFFER_SIZE];

	typedef struct clientInstances_struct {
		PAGES requestType;
		boolean waiting;
		EventCallbackInterface* callback;
		boolean inUse;
	} clientInstances_t;

	#define CLIENT_INSTANCES_NUM 4
	clientInstances_t clientStatus[CLIENT_INSTANCES_NUM];

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
		if (bufferEnd > 1)
		client.write(buffer, bufferEnd - 1);
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
	}

	void sendHttp500WithBody(uint8_t clientId) {
		EthernetClient client = EthernetClient(clientId);
		client.println(F("HTTP/1.1 500 Internal error"));
		client.println(F("Content-Type: text/html"));
		//client.println(F("Content-Length: 19"));
		client.println(F("Connection: close"));  // the connection will be closed after completion of the response
		client.println();
		client.println(F("500 Internal error."));
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

	/**********************************************************************************************************************
	*                                              Method for read HTTP Header Request from web client
	*
	* The HTTP request format is defined at http://www.w3.org/Protocols/HTTP/1.0/spec.html#Message-Types
	* and shows the following structure:
	*  Full-Request and Full-Response use the generic message format of RFC 822 [7] for transferring entities. Both messages may include
	optional header fields
	*  (also known as "headers") and an entity body. The entity body is separated from the headers by a null line (i.e., a line with nothing
	preceding the CRLF).
	*      Full-Request   = Request-Line
	*                       *( General-Header
	*                        | Request-Header
	*                        | Entity-Header )
	*                       CRLF
	*                       [ Entity-Body ]
	*
	* The Request-Line begins with a method token, followed by the Request-URI and the protocol version, and ending with CRLF. The elements are
	separated by SP characters.
	* No CR or LF are allowed except in the final CRLF sequence.
	*      Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
	* HTTP header fields, which include General-Header, Request-Header, Response-Header, and Entity-Header fields, follow the same generic
	format.
	* Each header field consists of a name followed immediately by a colon (":"), a single space (SP) character, and the field value.
	* Field names are case-insensitive. Header fields can be extended over multiple lines by preceding each extra line with at least one SP or
	HT, though this is not recommended.
	*      HTTP-header    = field-name ":" [ field-value ] CRLF
	***********************************************************************************************************************/
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
			// Process a header. We only need to extract the (optionl) content
			// length for the binary content that follows all these headers.
			// General-Header = Date | Pragma
			// Request-Header = Authorization | From | If-Modified-Since | Referer | User-Agent
			// Entity-Header  = Allow | Content-Encoding | Content-Length | Content-Type
			//                | Expires | Last-Modified | extension-header
			// extension-header = HTTP-header
			//       HTTP-header    = field-name ":" [ field-value ] CRLF
			//       field-name     = token
			//       field-value    = *( field-content | LWS )
			//       field-content  = <the OCTETs making up the field-value
			//                        and consisting of either *TEXT or combinations
			//                        of token, tspecials, and quoted-string>
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

	/**********************************************************************************************************************
	* Read the next HTTP header record which is CRLF delimited.  We replace CRLF with string terminating null.
	***********************************************************************************************************************/
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


	int8_t getFreeClientSlot() {
		for(uint8_t i = 0; i < CLIENT_INSTANCES_NUM; i++) {
			if(clientStatus->inUse == false)
				return i;
		}

		return -1;
	}

	boolean closeClient(uint8_t i) {
		if(clientStatus[i].inUse == false)
			return false;

		#ifdef DEBUG_WEBSERVER
		Serial.print(millis());
		Serial.print(F(": WebServer::closeClient() clientSlot="));
		Serial.println(i);
		#endif

		//is there a pending request?
		if(EthernetClient(i).available()) {
			#ifdef DEBUG_WEBSERVER
			Serial.print(F("\tnot closing"));
			#endif

			clientStatus[i].waiting = false;

			return false;
		}

		delay(1);
		EthernetClient(i).stop();
		clientStatus[i].inUse = false;
		clientStatus[i].requestType = PAGE_NONE;
		clientStatus[i].waiting = false;

		if(clientStatus[i].callback != NULL) {
			dispatcher.getResponseHandler()->unregisterListener(clientStatus[i].callback);
			delete clientStatus[i].callback;
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
		sendHttpOk(clientId);
		sendHtmlHeader(clientId, pageTitles[PAGE_MAIN]);
		sendHtmlMenu(clientId);

		EthernetClient client = EthernetClient(clientId);

		//page title
		client.println(F("<h1>Nodes</h1>"));

		//table
		client.println(F("<table><tr><th>ID</th><th>nextHop</th><th>#hops</th><th>age</th><th>discover</th></tr>"));
		neighbourData* neighbours = l3.getNeighbours();
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
		EthernetClient client = EthernetClient(clientId);
		String* id = req->getValue(variableRemote);
		uint8_t idInt = id->toInt();

		if(id == NULL) {
			sendHttp500WithBody(clientId);
			return;
		}

		Layer3::packet_t p;
		pf.generateDiscoveryInfoRequest(&p, idInt);
		discoveryListener* tmp = new discoveryListener(idInt);
		boolean success = dispatcher.getResponseHandler()->registerListenerByPacketType(millis()+2000, HARDWARE_DISCOVERY_RES, idInt, tmp);

		sendHttpOk(clientId);
		sendHtmlHeader(clientId, pageTitles[PAGE_MAIN]);
		sendHtmlMenu(clientId);

		//page title
		client.print(F("<h1>Node Info (id="));
		client.print(idInt);
		client.println(F(")</h1>"));
		sendHtmlFooter(clientId);
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
			closeClient(clientId);
		} else if(strcmp_P(uriChars, pageAddresses[PAGE_NODES]) == 0) {
			doPageNodes(clientId);
			closeClient(clientId);
		} else if(strcmp_P(uriChars, pageAddresses[PAGE_CSS]) == 0) {
			doPageCss(clientId);
			closeClient(clientId);
		} else if(strcmp_P(uriChars, pageAddresses[PAGE_GETSENSORINFO]) == 0) {
			doPageSensorInfo(clientId, &req);
			clientStatus[clientId].waiting = true;
		} else {
			sendHttp404WithBody(clientId);
			closeClient(clientId);
		}

	}
};


#endif /* WEBSERVER_H_ */