/*
* PageMaker.h
*
* Created: 05.09.2015 13:58:50
* Author: helge
*/

#ifndef __PAGEMAKER_H__
#define __PAGEMAKER_H__

#include <Arduino.h>
#include <Configuration.h>
#include <SD.h>
#include <sdcard/DiscoveryManager.h>
#include <utils/NodeInfo.h>
#include <Ethernet/EthernetClient.h>
#include <webserver/RequestContent.h>
#include <webserver/HardwareResultListener.h>
#include <webserver/StringConstantsExtern.h>

extern SPIRamManager ram;
extern DiscoveryManager discoveryManager;
extern NodeInfo nodeinfo;

#define WEBSERVER_REQUEST_TIMEOUT_MILLIS (2*1000)

enum MIME_TYPES {
	HTML,
	CSS,
	BINARY,
	CSV
};
enum PAGES {
	PAGE_NONE,				//0
	PAGE_MAIN,				//1
	PAGE_GETSENSORINFO,		//2
	PAGE_NODES,				//3
	PAGE_CSS,				//4
	PAGE_REQUEST_SENSOR,	//5
	PAGE_WRITE_SENSOR,		//6
	PAGE_LIST_FILES,		//7
	PAGE_MAINTAIN_NODE_INFO	//8
};


enum errorCode_t {
	INTERNAL_ERROR = -10,
	INTERNAL_ERROR_RAM = -12,
	INTERNAL_ERROR_LIST_FULL = -15,

	INTERNAL_ERROR_SD = -20,
	INTERNAL_ERROR_SD_FILE_NOT_FOUND = -21,
	INTERNAL_ERROR_SD_FILE_OPEN_FAILED = -22,

	HARDWARE_ERROR = -50,
	HARDWARE_ERROR_REGISTER_FAILED = -51,
	HARDWARE_ERROR_DEVICE_NOT_FOUND = -60,
	HARDWARE_ERROR_METHOD_NOT_IMPLEMENTED = -61,
	HARDWARE_ERROR_NO_EVENTS = -62,

	NODEINFO_ERROR = -70,

	WEBSERVER_ERROR = -80,
	WEBSERVER_INVALID_PARAMS = -81,
	WEBSERVER_UNKNOWN_URI = -82,
	WEBSERVER_TIMED_OUT = -83,

	NETWORK_ERROR = -100,
	NETWORK_ERROR_L2 = -110,
	NETWORK_ERROR_L2_HW = -111,
	NETWORK_ERROR_L3 = -120,
	NETWORK_ERROR_L3_NO_ROUTE = -121,
	NETWORK_ERROR_TIMED_OUT = -122,

	ERROR_UNKNOWN = -127
};

class PageMaker
{
	private:

	public:

	/**
	 * print to ethernet client from PGM space
	 * @param client
	 * @param str
	 */
	static void printP(EthernetClient* client, const char * str);

	/**
	 * 404 page
	 * @param client
	 */
	static void sendHttp404WithBody(EthernetClient* client, int16_t ERRNO = ERROR_UNKNOWN);

	/**
	 * 500 page
	 * @param client
	 */
	static void sendHttp500WithBody(EthernetClient* client, int16_t ERRNO = ERROR_UNKNOWN);

	/**
	 * print error code
	 * @param client
	 * @param errno
	 */
	static void printErrnoStr(EthernetClient* client, int16_t ERRNO);

	/**
	 * 200 ok head
	 * @param client
	 */
	static void sendHttpOk(EthernetClient* client);

	/**
	 * 200 ok head
	 * @param client
	 * @param cacheTime in seconds
	 */
	static void sendHttpOk(EthernetClient* client, uint32_t cacheTime);

	/**
	 * send header - all options.
	 * @param client
	 * @param cacheTime in seconds
	 * @param mime MIME type
	 * @param filenameForDownload
	 * @param len content length
	 */
	static void sendHttpOk(EthernetClient* client, uint32_t cacheTime, MIME_TYPES mime, char* filenameForDownload, uint32_t len);


	/**
	 * html head
	 * @param client
	 * @param pageId
	 * @param refresh meta autorefresh
	 * @param printTitle
	 */
	static void sendHtmlHeader(EthernetClient* client, uint8_t pageId, boolean refresh = true, boolean printTitle = true);

	/**
	 * html menu
	 * @param client
	 * @param page
	 */
	static void sendHtmlMenu(EthernetClient* client, uint8_t page);

	/**
	 * html footer
	 * @param client
	 */
	static void sendHtmlFooter(EthernetClient* client);

	/**
	 * welcome page
	 * @param client
	 */
	static void doPageStart(EthernetClient* client);

	/**
	 * stylesheet
	 * @param client
	 */
	static void doPageCss(EthernetClient* client);

	/**
	 * add trailing 0 to numbers <10
	 * @param client
	 * @param a value
	 */
	static void trailing0(EthernetClient* client, uint8_t a) {
		if(a < 10) {
			client->print(F("0"));
		}
	}

	/** 
	 * pretty print unix timestamp "YYY-MM-DD HH:ii:ss"
	 * @param client
	 * @param t
	 */
	static void printDate(EthernetClient* client, uint32_t t);

	/**
	 * is this hardware type readable?
	 * @param type
	 */
	static boolean hwIsReadable(HardwareTypeIdentifier type);

	/**
	 * print out links for execuratble actions
	 * @param client
	 * @param remote
	 * @param type
	 * @param address
	 */
	static void printExecutableLinks(EthernetClient* client, l3_address_t remote, HardwareTypeIdentifier type, uint8_t address);

	/** 
	 * print link with arguments
	 * @param client
	 * @param baseUrl
	 * @param keys
	 * @param vals
	 * @param name
	 * @param num
	 */
	static void printLink(EthernetClient* client, const char* baseUrl, const char** keys, const char** vals, const char* name, uint8_t num);

	/**
	 * discovery has finished - print result
	 * @param client
	 * @param req
	 */
	static void doPageSensorInfo2(EthernetClient* client, RequestContent* req);

	/**
	 * print hex digit
	 * @param hex
	 */
	static uint8_t hexdigit( char hex );
	
	/**
	 * print hex byte
	 * @param hex
	 */
	static uint8_t hexbyte( char* hex );

	/**
	 * list files or download one, depending on given? filename
	 * @param client
	 * @param req
	 */
	static void doPageListFiles(EthernetClient* client, RequestContent* req);

	/**
	 * download/show
	 * @param client
	 * @param filename
	 * @param filetype
	 */
	static void doPageListFile(EthernetClient* client, const char* filename, const char* filetype);

	/**
	 * list files.
	 * @param client
	 */
	static void doPageListFilesStart(EthernetClient* client);

	/**
	 * print maintenance page
	 * @param client
	 * @param req
	 */
	static void doPageMaintenanceNodeInfo(EthernetClient* client, RequestContent* req);


	/**
	 * node overview (from routing table)
	 * @param client
	 * @param req
	 */
	 static void doPageNodes(EthernetClient* client, RequestContent* req);


	/**
	 * initiate sensor write
	 * @param client
	 * @param req
	 * @param listenerHardwareRequest
	 */
	static boolean doPageWriteSensor(EthernetClient* client, RequestContent* req, hardwareRequestListener* listenerHardwareRequest);

	/**
	 * print write sensor page 2
	 * @param client
	 * @param listener
	 */
	static void doPageWriteSensor2(EthernetClient* client, hardwareRequestListener* listener);


	/**
	 * initiate sensor reading
	 * @param client
	 * @param req
	 * @param hwListener
	 */
	static boolean doPageRequestSensor(EthernetClient* client, RequestContent* req, hardwareRequestListener* hwListener);

	/**
	 * sensor reading has finished - print
	 * @param client
	 * @param listener
	 */
	static void doPageRequestSensor2(EthernetClient* client, hardwareRequestListener* listener);

	/**
	 * get route infos for a node if available
	 * @param nodeId
	 * @param neighbourActive
	 * @param neighbourLastKeepAlive
	 * @param neighbourHops
	 * @param neighbourNextHop
	 * @return success
	 */
	static boolean getRouteInfoForNode(uint8_t nodeId, boolean &neighbourActive, uint32_t &neighbourLastKeepAlive, uint8_t &neighbourHops, uint8_t &neighbourNextHop);

	/**
	 * pretty print cmd
	 * @param client
	 * @param cmd
	 */
	static void prettyPrintCommandResultGeneric(EthernetClient* client, command_t* cmd);

	/**
	 * pretty print cmd
	 * @param client
	 * @param cmd
	 */
	static void prettyPrintCommandResult(EthernetClient* client, command_t* cmd);

	/**
	 * pretty print cmd
	 * @param client
	 * @param cmd
	 */
	static void prettyPrintLed(command_t* cmd, EthernetClient* client);

	/**
	 * pretty print cmd
	 * @param client
	 * @param cmd
	 */
	static void prettyPrintButton(command_t* cmd, EthernetClient* client);

	/**
	 * pretty print cmd
	 * @param client
	 * @param cmd
	 */
	static void prettyPrintMotion(command_t* cmd, EthernetClient* client);

	/**
	 * pretty print cmd
	 * @param client
	 * @param cmd
	 */
	static void prettyPrintHumidity(EthernetClient* client, command_t* cmd);

	/**
	 * pretty print cmd
	 * @param client
	 * @param cmd
	 */
	static void prettyPrintPressure(command_t* cmd, EthernetClient* client);

	/**
	 * pretty print cmd
	 * @param client
	 * @param cmd
	 */
	static void prettyPrintTemperature(EthernetClient* client, command_t* cmd);

	/**
	 * pretty print cmd
	 * @param client
	 * @param cmd
	 */
	static void prettyLinkRTC(command_t* cmd, EthernetClient* client);

	/**
	 * pretty print cmd
	 * @param client
	 * @param cmd
	 */
	static void prettyPrintLight(command_t* cmd, EthernetClient* client);

}; //PageMaker

#endif //__PAGEMAKER_H__
