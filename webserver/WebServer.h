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
#include <webserver/StringConstantsExtern.h>

#include <ramManager.h>
#include <utils/NodeInfo.h>

extern Layer3 l3;
extern PacketDispatcher dispatcher;
extern PacketFactory pf;
extern SPIRamManager ram;
extern DiscoveryManager discoveryManager;
extern NodeInfo nodeinfo;


extern EthernetServer server;
extern byte mac[];
extern byte ip[];

//#define USE_DHCP_FOR_IP_ADDRESS

/** webserver */
class WebServer {
	public:
	enum MethodType {
		MethodUnknown,
		MethodGet,
		MethodPost,
		MethodHead
	};


	#ifndef STRING_BUFFER_SIZE
		#define STRING_BUFFER_SIZE 128
	#endif
	typedef char BUFFER[STRING_BUFFER_SIZE];

	/** keep track of client request */
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

	/**  initialise ethernet server and variables */
	void init();

	// Http header token delimiters
	static const char *pSpDelimiters;
	//const char *pStxDelimiter = "\002";    // STX - ASCII start of text character

	/** print flash string to serial */
	void serialPrintP(const char* str);

	/**
	 * Read HTTP request, setting Uri Index, the requestContent and returning the method type.
	 * @param client
	 * @param uri
	 * @param req here the GET dats is stored
	 */
	MethodType readHttpRequest(EthernetClient* client, String* uri, RequestContent* req);

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
	MethodType readRequestLine(EthernetClient* client, BUFFER & readBuffer, String* uri, BUFFER & requestContent, RequestContent* req);

	/**
	 * Read each header of the request till we get the terminating CRLF
	 * @param client
	 * @param readBuffer
	 * @param nContentLenth
	 * @parambIsUrlEncoded
	 */
	void readRequestHeaders(EthernetClient* client, BUFFER & readBuffer, int & nContentLength, bool & bIsUrlEncoded);

	/**
	 * Read the entity body of given length (after all the headers) into the buffer.
	 * @param  client
	 * @param nContentLength
	 * @param content buffer
	 */
	void readEntityBody(EthernetClient* client, int nContentLength, BUFFER & content);

	/**
	 * eat headers.
	 * @param client
	 * @param readBuffer
	 */
	void getNextHttpLine(EthernetClient* client, BUFFER & readBuffer);


	/**
	 * close a client connection and delete callback
	 * @param socketNumber
	 * @return success
	 */
	boolean closeClient(EthernetClient* client);

	/**
	 * main loop.
	 */
	void loop();

	/**
	 * callback has finished, now execute this (usually actual page creation)
	 * @param clintId
	 */
	void handleFinishedCallback(EthernetClient* client);

	/**
	 * first method when client connects - read http request
	 * @param client
	 */
	void doClientHandling(EthernetClient* client);

};

#endif /* WEBSERVER_H_ */
