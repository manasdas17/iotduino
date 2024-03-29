/*
* PageMaker.cpp
*
* Created: 05.09.2015 13:58:50
* Author: helge
*/


#include "PageMaker.h"


void PageMaker::printP(EthernetClient* client, const char * str)
{
	if(str == NULL)
		return;

	// copy data out of program memory into local storage, write out in
	// chunks of 32 bytes to avoid extra short TCP/IP packets
	uint8_t buffer[32];
	size_t bufferEnd = 0;

	while ((buffer[bufferEnd++] = pgm_read_byte(str++))){
		if (bufferEnd == 32) {
			client->write(buffer, 32);
			bufferEnd = 0;
		}
	}

	// write out everything left but trailing NUL
	if (bufferEnd > 1) {
		client->write(buffer, bufferEnd - 1);
	}
}

void PageMaker::sendHttp404WithBody(EthernetClient* client, int16_t ERRNO /*= ERROR_UNKNOWN*/)
{
	client->println(F("HTTP/1.1 404 Not Found"));
	client->println(F("Content-Type: text/html"));
	//client->println(F("Content-Length: 16"));
	client->println(F("Connection: close"));  // the connection will be closed after completion of the response
	client->println();
	sendHtmlHeader(client, 0, false, false);
	client->print(F("<h1>404 Not found</h1><p>"));
	printErrnoStr(client, ERRNO);
	client->print(F("</p>"));
	sendHtmlFooter(client);
}

void PageMaker::sendHttp500WithBody(EthernetClient* client, int16_t ERRNO /*= ERROR_UNKNOWN*/)
{
	client->println(F("HTTP/1.1 500 Internal error"));
	client->println(F("Content-Type: text/html"));
	//client->println(F("Content-Length: 19"));
	client->println(F("Connection: close"));  // the connection will be closed after completion of the response
	client->println();
	sendHtmlHeader(client, 0, false, false);
	client->print(F("<h1>500 Internal error</h1><p>"));
	printErrnoStr(client, ERRNO);
	client->print(F("</p>"));
	sendHtmlFooter(client);
}

void PageMaker::printErrnoStr(EthernetClient* client, int16_t ERRNO)
{
	if(ERRNO >= 0)
		return;

	if(ERRNO > -20) {
		client->print(F("INTERNAL ERROR"));
		return;
	}
	if(ERRNO > -50) {
		client->print(F("SD CARD ERROR"));
		return;
	}
	if(ERRNO > -70) {
		client->print(F("HARDWARE ERROR"));
		return;
	}
	if(ERRNO > -80) {
		client->print(F("NODEINFO ERROR"));
		return;
	}
	if(ERRNO > -100) {
		client->print(F("WEBSERVER ERROR"));
		return;
	}

	client->print(F("NETWORK ERROR"));
}

void PageMaker::sendHttpOk(EthernetClient* client, uint32_t cacheTime, MIME_TYPES mime, char* filenameForDownload, uint32_t len)
{
	client->println(F("HTTP/1.1 200 OK"));

	if(filenameForDownload != NULL) {
		client->print(F("Content-Disposition: attachment; filename=\""));
		client->print(filenameForDownload);
		client->println('"');
	}

	if(len > 0) {
		client->print(F("Content-Length: "));
		client->println(len);
	}

	client->print(F("Content-Type: "));
	switch(mime) {
		case HTML:
			printP(client, mimeTypes[HTML]);
		break;
		case CSS:
			printP(client, mimeTypes[CSS]);
		break;
		case CSV:
			printP(client, mimeTypes[CSV]);
		break;
		case BINARY:
		default:
			printP(client, mimeTypes[BINARY]);
		break;
	}
	client->println();

	client->println(F("Connection: close"));  // the connection will be closed after completion of the response
	if(cacheTime > 0) {
		client->print(F("Cache-Control: no-transform,public,max-age="));
		client->println(cacheTime);
	}
	client->println();
	client->flush();
}

void PageMaker::sendHttpOk(EthernetClient* client)
{
	sendHttpOk(client, 0, HTML, NULL, 0);
}

void PageMaker::sendHttpOk(EthernetClient* client, uint32_t cacheTime)
{
	sendHttpOk(client, cacheTime, HTML, NULL, 0);
}

void PageMaker::sendHtmlHeader(EthernetClient* client, uint8_t pageId, boolean refresh /*= true*/, boolean printTitle /*= true*/)
{
	client->println(F("<!DOCTYPE HTML>\n"));
	client->print(F("<html><head><link rel='stylesheet' href='css' type='text/css'><title>"));
	printP(client, pageTitles[pageId]);
	client->print(F("</title>"));

	if(refresh)
		client->print(F("<meta http-equiv='refresh' content='300'>"));

	client->println(F("</head><body>"));

	sendHtmlMenu(client, pageId);

	client->println(F("<div id='frame' class='frame'>"));
	if(printTitle) {
		client->println(F("<h1>"));
		printP(client, pageTitles[pageId]);
		client->println(F("</h1>"));
	}
	client->println(F("<div>"));

	client->flush();
}

void PageMaker::sendHtmlMenu(EthernetClient* client, uint8_t page)
{
	uint8_t menuPages[] = {PAGE_MAIN, PAGE_NODES, PAGE_LIST_FILES, PAGE_MAINTAIN_NODE_INFO};
	client->print(F("<div class='pos' id='tabs'>"));
	client->print(F("<ul>"));

	for(uint8_t i = 0; i < sizeof(menuPages); i++) {
		client->print(F("<li"));
		if(pageBelongsToMenu[page] == menuPages[i]) {
			client->print(F(" class='here'"));
		}
		client->print(F("><a href='"));
		printP(client, pageAddresses[menuPages[i]]);
		client->print("'>");
		printP(client, pageTitles[menuPages[i]]);
		client->print(F("</a></li>"));
	}

	client->print(F("</ul>"));
	client->print(F("</div>"));
	client->flush();
}

void PageMaker::sendHtmlFooter(EthernetClient* client)
{
	client->print(F("</div><p style='margin-top: 25px; text-align: center;'><a href='javascript:window.history.back();'>&laquo; back</a> &middot; <a href='javascript:location.reload();'>reload</a> &middot; <a href='javascript:window.history.forward();'>forward &raquo;</a></p></div><div class='datum'><a href='http://iotduino.de'>iotduino</a> webserver.<br/>"));
	printDate(client, now());
	client->println(F("</div></body></html>"));
	client->flush();
}

void PageMaker::doPageStart(EthernetClient* client)
{
	#ifdef DEBUG_WEBSERVER_ENABLE
	Serial.print(millis());
	Serial.print(F(": doPageStart() on clientId="));
	Serial.println(client->_sock);
	#endif

	#ifdef DEBUG_WEBSERVER_ENABLE
	Serial.println(F("\tdoHttpHeaderOK"));
	#endif
	sendHttpOk(client);
	sendHtmlHeader(client, PAGE_MAIN);
	client->println(F("Welcome to the <a href='http://iotduino.de'>iotduino</a> gateway system."));
	sendHtmlFooter(client);
}

void PageMaker::doPageCss(EthernetClient* client)
{
	#ifdef DEBUG_WEBSERVER_ENABLE
	Serial.print(millis());
	Serial.print(F(": doPageCss() on client="));
	Serial.println(client->_sock);
	#endif

	//sendHttpOk(client, 300);
	sendHttpOk(client, 300, CSS, NULL, 0);

	client->println(
		F(".warn { font-family: monospace; margin-bottom: 5px; background-color: rgb(255,220,220); border: 1px darkgray dashed; padding: 5px;}"
		".info { font-family: monospace; margin-bottom: 5px; background-color: rgb(220,255,220); border: 1px darkgray dashed; padding: 5px;}"
		"a, a:link, a:visited { color: #5F5F5F; text-decoration: underline; font-weight: normal; }"
		"a:active { font-weight: bold; }"
		"a:hover { text-decoration: none; background-color: #FFD8D8; }"
		".warning { color: red; }"
		".ok { color: green; }"
		"input { font-size: 9pt; height: 20px; margin: 0; padding: 0px; background-color: white; border: 1px solid darkgray; }"
		"input[type='submit'] { width: 50px; }"
		"input[type='text'] { width: 120px }"
		"select { width: 120px }"
		".centered { text-align: center; }"
		".righted { text-align: right; }"
		".inline { border: none; }"
		"tbody tr:nth-child(even) {"
		"  background-color: #eee;"
		"}"
		"tbody tr:nth-child(odd) {"
		"  background-color: #fff;"
		"}"
		"tbody tr:hover {"
		"  background-color: #ffdcdc;"
		"}"
		"body {"
		"  margin-left: 0px;"
		"  margin-bottom: 0px;"
		"  font-family: arial;"
		"  font-size: 12pt;"
		"  color: #000000;"
		"  background-color: #dddddd;"
		"}"
		"div.datum {"
		"  position: absolute;"
		"  top: 15px;"
		"  left: 400px;"
		"  text-align: right;"
		"  font-size: 10px;"
		"}"
		"div.frame {"
		"  position: absolute;"
		"  top: 42px;"
		"  left: 10px;"
		"  border: 1px solid #000;"
		"  background-color: #ffffff;"
		"  padding: 20px 30px 10px;"
		"}"
		"h1 {"
		"  font-size: 15pt;"
		"}"
		"h1:first-letter {"
		"  color: #dd0000;"
		"}"
		"div.content {"
		"  margin: 20px 0px 20px;"
		"  border: 1px dotted #000;"
		"  background-color: #fafafa;"
		"  padding: 10px 10px 10px;"
		"}"
		"div.pos {"
		"  z-index: 10;"
		"  position: absolute;"
		"  top: 20px;"
		"  left: 30px;"
		"}"
		"#tabs ul {"
		"  margin-left: 0px;"
		"  padding-left: 0px;"
		"  display: inline;"
		"}"
		"#tabs ul li {"
		"  margin-left: 0px;"
		"  margin-right: 5px;"
		"  margin-bottom: 0px;"
		"  padding: 2px 6px 5px;"
		"  border: 1px solid #000;"
		"  list-style: none;"
		"  display: inline;"
		"  background-color: #cccccc;"
		"}"
		"#tabs ul li.here {"
		"  border-bottom: 1px solid #ffffff;"
		"  list-style: none;"
		"  display: inline;"
		"  background-color: #ffffff;"
		"}"
		"#tabs ul li:hover {"
		"  background-color: #eeeeee;"
		"}"
		"/*nav-links*/"
		"#tabs a, a:link, a:visited, a:active {"
		"  color: #000000;"
		"  font-weight: bold;"
		"  text-decoration: none;"
		"}"
		"#tabs a:hover {"
		"  color: #dd0000;"
		"  text-decoration: none;"
		"}"
		"/**Responsivetablewithcss*AdeBudiman-art.visuadlesigner@gmail.com*2015*/"
		"table {"
		"  border: 1px solid #ccc;"
		"  width: 100%;"
		"  margin: 0px;"
		"  padding: 0px;"
		"  border-collapse: collapse;"
		"  border-spacing: 0px;"
		"}"
		"table tr {"
		"  border: 1px solid #ddd;"
		"  padding: 5px;"
		"}"
		"table th, table td {"
		"  padding: 10px;"
		"  text-align: center;"
		"}"
		"table th {"
		"  text-transform: uppercase;"
		"  font-size: 14px;"
		"  letter-spacing: 1px;"
		"}"
		"@media screen and (max-width: 1024px) {"
		"  table {"
		"    border: 10px;"
		"  }"
		"  table thead {"
		"    display: none;"
		"  }"
		"  table tr {"
		"    margin-bottom: 10px;"
		"    display: block !important;;"
		"    border-bottom: 2px solid #ddd;"
		"  }"
		"  table td {"
		"    display: block !important;"
		"    text-align: right;"
		"    font-size: 13px;"
		"    border-bottom: 1px dotted #ccc;"
		"  }"
		"  table td:last-child {"
		"    border-bottom: 0px;"
		"  }"
		"  table td:before {"
		"    content: attr(data-label);"
		"    float: left;"
		"    text-transform: uppercase;"
		"    font-weight: bold;"
		"  }"
		"}"));

	client->flush();
}

void PageMaker::printDate(EthernetClient* client, uint32_t t)
{
	client->print(year(t));
	client->print(F("-"));
	trailing0(client, month(t));
	client->print(month(t));
	client->print(F("-"));
	trailing0(client, day(t));
	client->print(day(t));
	client->print(F(" "));
	trailing0(client, hour(t));
	client->print(hour(t));
	client->print(F(":"));
	trailing0(client, minute(t));
	client->print(minute(t));
	client->print(F(":"));
	trailing0(client, second(t));
	client->print(second(t));
}

boolean PageMaker::hwIsReadable(HardwareTypeIdentifier type)
{
	switch(type) {
		case HWType_rcswitch:
		case HWType_tone:
			return false;
		default:
			return true;
	}
}

void PageMaker::printExecutableLinks(EthernetClient* client, l3_address_t remote, HardwareTypeIdentifier type, uint8_t address)
{
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

	//							0				1					2					3					4
	//							remote id		hw address			hw type				var to write		listtype
	const uint8_t numKV = 5;
	const char* keys[numKV] = {variableRemote,	variableHwAddress,	variableHwType,		variableVal,		variableListType};
	const char* vals[numKV] = {buf1,			buf2,				buf3,				bufVal,				bufListType};


	switch(type) {
		case HWType_relay:
		case HWType_DIGITAL:
			//off
			bufVal[4] = '\0';
			strcpy_P(bufListType, linkCmdListTypeUint8);

			printLink(client, pageAddresses[PAGE_WRITE_SENSOR], keys, vals, linkNameOff, numKV);
			client->print(F(" &middot; "));
			//on
			bufVal[3] = '1';
			strcpy_P(bufListType, linkCmdListTypeUint8);
			printLink(client, pageAddresses[PAGE_WRITE_SENSOR], keys, vals, linkNameOn, numKV);
			return;
		case HWType_rcswitch:
			client->print(F("<table class='inline' style='width: 99%'"));
			client->print(F("<tr><td class='centered'>1</td><td class='centered'>2</td><td class='centered'>3</td><td class='centered'>4</td><td class='centered'>all</td></tr>"));
			client->print(F("<tr>"));
			strcpy_P(bufListType, linkCmdListTypeUint8);
			for(uint8_t i = 0; i < 5; i++) {
				client->print(F("<td class='inline centered'>"));
				bufVal[5] = '0'+i+1; //device
				bufVal[3] = '0';//val lower
				printLink(client, pageAddresses[PAGE_WRITE_SENSOR], keys, vals, linkNameOff, numKV);

				client->print(F(" &middot; "));

				bufVal[3] = '1';//val lower
				printLink(client, pageAddresses[PAGE_WRITE_SENSOR], keys, vals, linkNameOn, numKV);
				client->print(F("</td>"));
			}
			client->print(F("</tr>"));
			client->print(F("</table>"));
			return;
		case HWType_ANALOG:
		case HWType_rtc:
		case HWType_tone:
			client->print(F("<form action='"));
			printP(client, pageAddresses[PAGE_WRITE_SENSOR]);
			client->print(F("' method='get'>"));

			client->print(F("<input type='hidden' name='"));
			printP(client, variableHwAddress);
			client->print(F("' value='"));
			client->print(address);
			client->print(F("'>"));

			client->print(F("<input type='hidden' name='"));
			printP(client, variableHwType);
			client->print(F("' value='"));
			client->print(type);
			client->print(F("' style='width: 200px'>"));

			client->print(F("<input type='hidden' name='"));
			printP(client, variableRemote);
			client->print(F("' value='"));
			client->print(remote);
			client->print(F("'>"));

			client->print(F("HexInput (msbf): <input type='text' name='"));
			printP(client, variableVal);
			client->print(F("' value='"));
			printP(client, linkCmdValDefault);
			client->print(F("'/>"));
			client->print(F(" <input type='submit'><br/>"));


			client->print(F(" u8:<input type='radio' name='"));
			printP(client, variableListType);
			client->print(F("' value='"));
			printP(client, linkCmdListTypeUint8);
			client->print(F("'"));
			printP(client, linkCmdListChecked);
			client->print(F("/>"));

			client->print(F(" u16:<input type='radio' name='"));
			printP(client, variableListType);
			client->print(F("' value='"));
			printP(client, linkCmdListTypeUint16);
			client->print(F("'/>"));

			client->print(F(" i8:<input type='radio' name='"));
			printP(client, variableListType);
			client->print(F("' value='"));
			printP(client, linkCmdListTypeInt8);
			client->print(F("'/>"));

			client->print(F(" i16:<input type='radio' name='"));
			printP(client, variableListType);
			client->print(F("' value='"));
			printP(client, linkCmdListTypeInt16);
			client->print(F("'/>"));

			client->print(F("</form>"));
			return;
		default:
			client->print('-');
	}
}

void PageMaker::printLink(EthernetClient* client, const char* baseUrl, const char** keys, const char** vals, const char* name, uint8_t num)
{
	//#ifdef DEBUG_WEBSERVER_ENABLE
	//Serial.print(millis());
	//Serial.print(F(": printLink() baseUrl="));
	//serialPrintP(baseUrl);
	//Serial.println();
	//#endif

	client->print(F("<a href='"));
	printP(client, baseUrl);
	client->print('?');

	for(uint8_t i = 0; i < num; i++) {
		#ifdef DEBUG_WEBSERVER_ENABLE
			//Serial.print(F("\t"));
			//serialPrintP(keys[i]);
			//Serial.print('=');
			//Serial.println(vals[i]);
			//Serial.flush();
		#endif

		if(i > 0)
			client->print('&');
		printP(client, keys[i]);
		client->print('=');
		client->print(vals[i]);
	}

	client->print(F("'>"));
	printP(client, name);
	client->print(F("</a>"));
}

void PageMaker::doPageSensorInfo2(EthernetClient* client, RequestContent* req)
{
	#ifdef DEBUG_WEBSERVER_ENABLE
	Serial.print(millis());
	Serial.print(F(": doPageSensorInfo2() on client="));
	Serial.println(client->_sock);
	#endif

	//yay!
	if(req == NULL) {
		sendHttp500WithBody(client, WEBSERVER_INVALID_PARAMS);
		return;
	}

	String* id = req->getValue(variableRemote);
	uint8_t idInt = id->toInt();

	if(id == NULL) {
		sendHttp500WithBody(client, WEBSERVER_INVALID_PARAMS);
		return;
	}

	sendHttpOk(client);
	sendHtmlHeader(client, PAGE_GETSENSORINFO, true, false);

	//delete info?
	if(req->hasKey(variableHwAddress) >= 0 && req->hasKey(variableHwType) >= 0 && req->hasKey(variableDelete) >= 0) {
		String* hwAddress = req->getValue(variableHwAddress);
		uint8_t hwAddressInt = hwAddress->toInt();
		String* hwType = req->getValue(variableHwType);
		uint8_t hwTypeInt = hwType->toInt();

		boolean success = false;
		if(hwAddressInt > 0 && hwTypeInt > 0) {
			success = discoveryManager.deleteInfo(idInt, hwAddressInt, hwTypeInt);
		}
		
		client->print(F("<div class='"));
		if(success) {
			client->print(F("info"));
		} else {
			client->print(F("warn"));
		}
		client->print(F("'>"));
		client->print(F("Info for NoteID "));
		client->print(idInt);
		client->print(F(": type="));
		printP(client, hardwareTypeStrings[hwTypeInt]);
		client->print(F(" address="));
		client->print(hwAddressInt);
		if(!success) {
			client->print(F(" not"));
		}
		client->print(F(" deleted."));
		client->print(F("</div>"));
	}

	//general info
	NodeInfo::NodeInfoTableEntry_t nodeInfoObj;
	nodeInfo.getNodeInfo((l3_address_t) idInt, &nodeInfoObj);

	boolean neighbourActive = 0;
	uint32_t neighbourLastKeepAlive = 0;
	uint8_t neighbourHops = -1;
	uint8_t neighbourNextHop = 0;
	getRouteInfoForNode(idInt, neighbourActive, neighbourLastKeepAlive, neighbourHops, neighbourNextHop);


	client->print(F("<h1>Sensor Info id="));
	client->print(idInt);
	client->print(F(" ("));
	if(strlen(nodeInfoObj.nodeStr) > 0) {
		client->print(nodeInfoObj.nodeStr);
	} else {
		client->print(F("<i>NA</i>"));
	}
	client->println(F(")</h1>"));

	client->print(F("<p>"));

	if(neighbourActive == 0) {
		client->print(F("<span class='warning'>"));
	} else {
		client->print(F("<span class='ok'>"));
	}
	client->print(F("active="));
	client->print(neighbourActive);
	client->print(F("</span>"));

	client->print(F("<br/>nextHop="));
	client->print(neighbourNextHop);
	client->print(F("<br/>hops="));
	client->print(neighbourHops);
	client->print(F("<br/>lastKeepAliveAge="));
	client->print((millis() - neighbourLastKeepAlive) / 1000);
	client->print(F("s</p>"));

	client->println(F("<table><thead><tr><th>HardwareAddress</th><th>HardwareType</th><th>LastUpdated</th><th>requestSensor</th><th>writeSensor</th><th>delete</th></tr></thead><tbody>"));
	uint8_t numInfos = 0;
	//conversion buffers
	char buf1[3];
	char buf2[3];
	char buf3[3];
	uint8_t num = 0;
	DiscoveryManager::Discovery_nodeDiscoveryInfoTableEntry_t elem;
	for(uint8_t i = 0; i < NUM_INFOS_PER_NODE; i++) {
		discoveryManager.readElemIntoVar(&elem, idInt, i);

		if(elem.hardwareAddress > 0 && elem.hardwareType > 0) {
			//table
			client->print(F("<tr>"));
			num++;

			client->print(F("<td data-label='HwAdrr' class='righted'>"));
			client->print(elem.hardwareAddress);
			client->print(F("</td>"));

			client->print(F("<td data-label='HwType'>"));
			printP(client, hardwareTypeStrings[elem.hardwareType]);
			client->print(F("</td>"));

			client->print(F("<td data-label='lastDiscovery' class='righted'>"));
			printDate(client, elem.rtcTimestamp);
			client->print(F("</td>"));

			client->print(F("<td data-label='Read' class='centered'>"));
			if(hwIsReadable((HardwareTypeIdentifier) elem.hardwareType)) {
				itoa(idInt, buf1, 10);
				itoa(elem.hardwareAddress, buf2, 10);
				itoa(elem.hardwareType, buf3, 10);
				const char* keys[3] = {variableRemote, variableHwAddress, variableHwType};
				const char* vals[3] = {buf1, buf2, buf3};
				printLink(client, pageAddresses[PAGE_REQUEST_SENSOR], keys, vals, linkNameX, 3);
			} else {
				client->print(F("-"));
			}
			client->print(F("</td>"));

			client->print(F("<td data-label='Write' class='centered'>"));
			printExecutableLinks(client, idInt, (HardwareTypeIdentifier) elem.hardwareType, elem.hardwareAddress);
			client->print(F("</td>"));

			client->print(F("<td data-label='delete' class='centered'><a href='"));
			printP(client, pageAddresses[PAGE_GETSENSORINFO]);
			client->print(F("?"));
			printP(client, variableRemote);
			client->print(F("="));
			client->print(idInt);
			client->print(F("&"));
			printP(client, variableHwAddress);
			client->print(F("="));
			client->print(elem.hardwareAddress);
			client->print(F("&"));
			printP(client, variableHwType);
			client->print(F("="));
			client->print(elem.hardwareType);
			client->print(F("&"));
			printP(client, variableDelete);
			client->println(F("' onclick=\"return confirm('This will delete this sensor info. Are you sure?')\">x</a>"));
			client->println(F("</td>"));
			client->print(F("</td>"));

			client->print(F("</tr>"));
			client->print(F("</form>"));

			numInfos++;
			client->flush();
		}
	}
	client->print(F("</tbody><tfoot><tr><th colspan='6'>"));
	client->print(numInfos);
	client->println(F(" entries</th></tr></tfoot></table>"));

	sendHtmlFooter(client);
}

uint8_t PageMaker::hexdigit(char hex)
{
	return (hex <= '9') ? hex - '0' : toupper(hex) - 'A' + 10 ;
}

uint8_t PageMaker::hexbyte(char* hex)
{
	return (hexdigit(*hex) << 4) | hexdigit(*(hex+1)) ;
}

void PageMaker::doPageListFiles(EthernetClient* client, RequestContent* req)
{
	#ifdef DEBUG_WEBSERVER_ENABLE
	Serial.print(millis());
	Serial.print(F(": doPageListFiles() on clientId="));
	Serial.println(client->_sock);
	#endif

	if(req->hasKey(variableFilename) >= 0 && req->hasKey(variableDelete) == -1) {
		const uint8_t bufSize = 13;
		char filename[bufSize];
		req->getValue(variableFilename)->toCharArray(filename, bufSize);

		char filetype[bufSize];
		req->getValue(variableFiletype)->toCharArray(filetype, bufSize);

		#ifdef DEBUG_WEBSERVER_ENABLE
		Serial.print(F("\tlist file="));
		Serial.println(filename);
		#endif

		doPageListFile(client, filename, filetype);
	} else {
		#ifdef DEBUG_WEBSERVER_ENABLE
		Serial.println(F("\tlist files."));
		#endif
		
		doPageListFilesStart(client, req);
	}
}

void PageMaker::doPageListFile(EthernetClient* client, const char* filename, const char* filetype)
{
	if(!SD.exists((char*) filename)) {
		sendHttp500WithBody(client, INTERNAL_ERROR_SD_FILE_NOT_FOUND);
		return;
	}

	File f = SD.open(filename);

	if(!f) {
		sendHttp500WithBody(client, INTERNAL_ERROR_SD_FILE_OPEN_FAILED);
		return;
	}

	//type
	boolean isRaw = true;
	if(strcmp_P(filetype, variableFiletypeCsv) == 0) {
		isRaw = false;
	}

	#ifdef DEBUG_WEBSERVER_ENABLE
		uint32_t t1 = millis();
		Serial.print(t1);
		Serial.print(F(": filesize="));
		Serial.println(f.size());
	#endif


	size_t bytes = 0;
	uint16_t totalBytes = 0;
	//read data and write.
	if(isRaw) {
		sendHttpOk(client, 0, BINARY, f.name(), f.size());
		const uint16_t bufSize = 512; //page size.
		uint8_t buffer[bufSize];
		//RAW
		//client->finishSendPacket();
		while(totalBytes < f.size()) {
			uint32_t remaining = f.size() - totalBytes;
			if(remaining < bufSize) {
				bytes = f.readBytes(buffer, remaining);
			} else {
				bytes = f.readBytes(buffer, bufSize);
			}

			//client->addData(buffer, bytes);
			client->write(buffer, bytes);
			totalBytes += bytes;

			wdt_reset();
		}

		//client->finishSendPacket();
	} else {
		//CSV
		const uint16_t bufSize = sizeUInt8List + 4;

		if(f.size() % bufSize != 0) {
			sendHttp500WithBody(client, INTERNAL_ERROR_SD);
			return;
		}

		char filename2[14];
		strcpy(filename2, f.name());
		char* pos = strstr(filename2, ".");
		*(pos+1) = 'c';
		*(pos+2) = 's';
		*(pos+3) = 'v';
		*(pos+4) = '\0';
		sendHttpOk(client, 0, CSV, filename2, 0);

		client->print(F("timestamp"));
		for(uint8_t i = 0; i < sizeUInt8List; i++) {
			client->print(F(";byte"));
			client->print(i);
		}
		client->println();

		uint8_t buffer[bufSize];
		uint32_t timestamp = now();
		//CSV
		while(totalBytes < f.size()) {
			bytes = f.readBytes(buffer, bufSize);
			timestamp = (uint32_t) buffer[3] << 24;
			timestamp |= (uint32_t) buffer[2] << 16;
			timestamp |= (uint16_t) buffer[1] << 8;
			timestamp |= buffer[0];

			printDate(client, timestamp); //client->print(timestamp); //yyyy-MM-dd HH:mm:ss
			client->print(';');
			for(uint8_t i = 0; i < sizeUInt8List; i++) {
				client->print(buffer[4+i]);
				if(i < sizeUInt8List-1) {
					client->print(';');
				} else {
					client->println();
				}
			}
			totalBytes += bytes;

			wdt_reset();
		}
	}

	#ifdef DEBUG_WEBSERVER_ENABLE
		uint32_t t2 = millis();
		Serial.print(millis());
		Serial.print(F(": total bytes written="));
		Serial.println(totalBytes);
		Serial.print(F("\ttime in ms="));
		Serial.println(t2-t1);
		Serial.flush();
	#endif
}

void PageMaker::doPageListFilesStart(EthernetClient* client, RequestContent* req)
{
	sendHttpOk(client);
	sendHtmlHeader(client, PAGE_LIST_FILES, true, true);

	//delete a file?
	if(req->hasKey(variableFilename) >= 0) {
		const uint8_t bufSize = 13;
		char filename[bufSize];
		req->getValue(variableFilename)->toCharArray(filename, bufSize);

		boolean success = false;
		if(SD.exists((char*) filename)) {
			success = SD.remove(filename);
		}

		//result.
		client->print(F("<div class='"));
		if(success) {
			client->print(F("info"));
			} else {
			client->print(F("warn"));
		}
		client->print(F("'>"));
		client->print(F("File "));
		client->print(filename);
		if(!success) {
			client->print(F(" not"));
		}
		client->print(F(" deleted."));
		client->print(F("</div>"));
	}//end delete file

	client->println(F("<table><thead><tr><th>Remote</th><th>HardwareAddress</th><th>HardwareType</th><th>Filename</th><th>RAW</th><th>Size [b]</th><th>delete</th></tr></thead>"));
	client->flush();

	uint8_t num = 0;


	File test = SD.open("/");
	#ifdef DEBUG_WEBSERVER_ENABLE
	Serial.print(F("isDir="));
	Serial.println(test.isDirectory());
	#endif

	if(test.isDirectory()) {
		test.rewindDirectory();

		File cursor = test.openNextFile();
		NodeInfo::NodeInfoTableEntry_t nodeInfoObj;
		while(cursor) {
			#ifdef DEBUG_WEBSERVER_ENABLE
			Serial.print('\t');
			Serial.println(cursor.name());
			#endif
			if(strstr(cursor.name(), ".LOG") != NULL && strlen(cursor.name())  >= 11) {
				//0  3  6
				//XX_XX_XX.LOG
				uint8_t remote = hexbyte(&cursor.name()[0]);
				uint8_t type = hexbyte(&cursor.name()[3]);
				uint8_t address = hexbyte(&cursor.name()[6]);

				nodeInfo.getNodeInfo((l3_address_t) remote, &nodeInfoObj);

				#ifdef DEBUG_WEBSERVER_ENABLE
				Serial.print(F("\t\t"));
				Serial.print(remote);
				Serial.print(' ');
				Serial.print(type);
				Serial.print(' ');
				Serial.println(address);
				Serial.flush();
				#endif

				client->print(F("<tr>"));
				
				//remote
				client->print(F("<td data-label='Remote'>"));
				client->print(remote);
				if(strlen(nodeInfoObj.nodeStr) > 0) {
					client->print(F(" ("));
					client->print(nodeInfoObj.nodeStr);
					client->print(F(")"));
				}
				client->print(F("</td>"));

				//HWaddress
				client->print(F("<td data-label='HwAddress'>"));
				client->print(address);
				client->print(F("</td>"));

				//HWtypoe
				client->print(F("<td data-label='HwType'>"));
				//client->print(type);
				printP(client, hardwareTypeStrings[type]);
				client->print(F("</td>"));

				//link CSV
				client->print(F("<td data-label='CSV'><a href='"));
				printP(client, pageAddresses[PAGE_LIST_FILES]);
				client->print(F("?"));
				printP(client, variableFilename);
				client->print(F("="));
				client->print(cursor.name());
				client->print(F("&"));
				printP(client, variableFiletype);
				client->print(F("="));
				printP(client, variableFiletypeCsv);
				client->print(F("'>"));
				client->print(cursor.name());
				client->print(F("</a></td>"));

				//link RAW
				client->print(F("<td data-label='RAW'><a href='"));
				printP(client, pageAddresses[PAGE_LIST_FILES]);
				client->print(F("?"));
				printP(client, variableFilename);
				client->print(F("="));
				client->print(cursor.name());
				client->print(F("'>x</a></td>"));

				//size
				client->print(F("<td data-label='bytes'>"));
				client->print(cursor.size());
				client->print(F(" ("));
				client->print(cursor.size() / (sizeUInt8List + 4)); //uint32_t rtc timestamp + sensordata.
				client->print(F(" samples)</td>"));
				client->print(F("</td>"));

				//delete
				client->print(F("<td data-label='delete' class='centered'><a href='"));
				printP(client, pageAddresses[PAGE_LIST_FILES]);
				client->print(F("?"));
				printP(client, variableFilename);
				client->print(F("="));
				client->print(cursor.name());
				client->print(F("&"));
				printP(client, variableDelete);
				client->println(F("' onclick=\"return confirm('This will delete the sensor log. Are you sure?')\">x</a>"));
				client->println(F("</td>"));
			
				client->print(F("</tr>"));
				
				num++;
				client->flush();
			}
			cursor = test.openNextFile();
		}
	}

	client->println(F("<tfoot><tr><th colspan='7'>"));
	client->print(num);
	client->println(F(" Entries</th></tr></tfoot>"));
	client->println(F("</table>"));
	sendHtmlFooter(client);
}

void PageMaker::doPageMaintenanceNodeInfo(EthernetClient* client, RequestContent* req)
{
	boolean headerIsSent = false;

	if(req->hasKey(variableRemote) != -1 && req->hasKey(variableName) != -1) {
		//do update.
		String* id = req->getValue(variableRemote);
		l3_address_t idInt = id->toInt();

		String* name = req->getValue(variableName);
		if(name->length() > NODE_INFO_SIZE) {
			sendHttp500WithBody(client, WEBSERVER_INVALID_PARAMS);
			return;
		}

		uint8_t BUF[NODE_INFO_SIZE];
		name->toCharArray((char*) BUF, NODE_INFO_SIZE);
		if(!nodeInfo.updateString(idInt, BUF, NODE_INFO_SIZE)) {
			sendHttp500WithBody(client, NODEINFO_ERROR);
			return;
		} else {
			sendHttpOk(client);
			sendHtmlHeader(client, PAGE_MAINTAIN_NODE_INFO, false, true);
			headerIsSent = true;
			client->print(F("<div class='info'>Updated NoteID "));
			client->print(idInt);
			client->print(F("</div>"));
		}
	}

	if(!headerIsSent) {
		sendHttpOk(client);
		sendHtmlHeader(client, PAGE_MAINTAIN_NODE_INFO, true, true);
	}

	client->println(F("<div class='info'>This may take a while ;-)</div>"));
	client->println(F("<table><thead><tr><th>Node</th><th>Newinfo</th><th></th></tr></thead>"));
	client->print(F("<tfoot><tr><th colspan='3'>"));
	client->print(NODE_INFO_MAX);
	client->println(F(" Entries</th></tr></tfoot>"));

	client->print(F("<tr><td data-label='ID'><form action='"));
	printP(client, pageAddresses[PAGE_MAINTAIN_NODE_INFO]);
	client->print(F("' method='get'><select size='5' type='hidden' name='"));
	printP(client, variableRemote);
	client->print(F("'>"));

	SPIRamManager::iterator it;
	nodeInfo.getIterator(&it);
	uint8_t i = 0;
	NodeInfo::NodeInfoTableEntry_t* currentItem = NULL;
	while(it.hasNext()) {
		i = it.getIteratorIndex();
		currentItem = (NodeInfo::NodeInfoTableEntry_t*) it.next();
		wdt_reset();

		client->print(F("<option value='"));
		client->print(i);
		client->print(F("'>#"));
		client->print(i);
		client->print(F(" "));
		client->print(currentItem->nodeStr);
		client->println(F("</option>"));
		client->flush();
	}

	client->print(F("</select></td>"));

	client->print(F("<td><input type='text' name='"));
	printP(client, variableName);
	client->print(F("' maxlength='"));
	client->print(NODE_INFO_SIZE-1); //terminating character!
	client->print(F("'/></td><td><input type='submit'/></td></tr></form>"));
	client->flush();

	client->println(F("</table>"));
	sendHtmlFooter(client);
}

void PageMaker::doPageNodes(EthernetClient* client, RequestContent* req)
{
	#ifdef DEBUG_WEBSERVER_ENABLE
	Serial.print(millis());
	Serial.print(F(": doPageNodes() on client="));
	Serial.println(client->_sock);
	#endif

	sendHttpOk(client);
	sendHtmlHeader(client, PAGE_NODES);

	//delete a node?
	if(req->hasKey(variableRemote) >= 0 && req->hasKey(variableDelete) >= 0) {
		String* id = req->getValue(variableRemote);
		l3_address_t idInt = id->toInt();
		if(nodeInfo.deleteInfo(idInt) && discoveryManager.deleteInfo(idInt)) {
			client->print(F("<div class='info'>Info for NoteID "));
			client->print(idInt);
			client->print(F(" deleted.</div>"));
		}
	}

	//table
	client->println(F("<table><thead><tr><th>ID</th><th>NodeInfo</th><th>lastDiscovery</th><th>active</th><th>nextHop</th><th>#hops</th><th>routeAge</th><th>delete</th><th>info</th></tr></thead><tbody>"));
	uint8_t numNodes = 0;
	uint32_t nowSystem = millis();
	//uint32_t rtcTime = now();

	//data to display per node
	NodeInfo::NodeInfoTableEntry_t nodeInfoObj;
	boolean neighbourActive;
	uint32_t neighbourLastKeepAlive;
	uint8_t neighbourHops;
	uint8_t neighbourNextHop;

	////SDcard::SD_nodeDiscoveryInfoTableEntry_t discoveryInfo[SD_DISCOVERY_NUM_INFOS_PER_NODE];
	//ietrate possible nodes
	for(uint16_t i = 1; i < NUM_KNOWN_NODES; i++) {
		wdt_reset();

		//get string info
		if(nodeInfo.getNodeInfo((l3_address_t) i, &nodeInfoObj) == 0)
			continue;

		//reset data
		neighbourActive = 0;
		neighbourLastKeepAlive = 0;
		neighbourHops = -1;
		neighbourNextHop = 0;
		//get route info
		getRouteInfoForNode(i, neighbourActive, neighbourLastKeepAlive, neighbourHops, neighbourNextHop);

		//print node info?
		if(nodeInfoObj.nodeId != 0) {
			numNodes++;

			//node info
			client->print(F("<tr><td data-label='ID' class='righted'>"));
			client->print(i);
			client->print(F("</td><td data-label='InfoStr'>"));
			client->print(nodeInfoObj.nodeStr);
			client->print(F("</td><td data-label='lastDicovery' class='righted'>"));

			uint32_t t = nodeInfoObj.lastDiscoveryRequest;
			printDate(client, t);


			client->print(F("</td><td data-label='Active' class='righted'>"));
			client->print(neighbourActive);
			client->print(F("</td><td data-label='NextHop' class='righted'>"));
			client->print(neighbourNextHop);
			client->print(F("</td><td data-label='Hops' class='righted'>"));
			client->print(neighbourHops);
			client->print(F("</td><td data-label='lastKeepAlive' class='righted'>"));
			if(i != l3.localAddress) {
				if(neighbourLastKeepAlive > 0) {
					client->print((nowSystem - neighbourLastKeepAlive) / 1000);
					client->print(F("s"));
				} else {
					client->print(F("<i>no route to host</i>"));
				}
			} else {
				client->print(F(" <i>loopback</i>"));
			}
			client->print(F("</td>"));

			//delete
			client->print(F("<td data-label='delete' class='centered'><a href='"));
			printP(client, pageAddresses[PAGE_NODES]);
			client->print(F("?"));
			printP(client, variableRemote);
			client->print(F("="));
			client->print(i);
			client->print(F("&"));
			printP(client, variableDelete);
			client->println(F("' onclick=\"return confirm('This will delete all node infos. Are you sure?')\">x</a>"));
			client->println(F("</td>"));

			//discover
			client->print(F("<td  data-label='sensors' class='centered'><a href='"));
			printP(client, pageAddresses[PAGE_GETSENSORINFO]);
			client->print(F("?"));
			printP(client, variableRemote);
			client->print(F("="));
			client->print(i);
			client->println(F("'>x</a>"));
			client->println(F("</td>"));

			//tr end
			client->println(F("</tr>"));
			client->flush();
		}
	}

	//num entries
	client->print(F("</tbody><tfoot><tr><th colspan='9'>"));
	client->print(numNodes);
	client->println(F(" entries</th></tr></tfoot></table>"));

	//footer
	sendHtmlFooter(client);
}

boolean PageMaker::doPageWriteSensor(EthernetClient* client, RequestContent* req, hardwareRequestListener* listenerHardwareRequest)
{
	#ifdef DEBUG_WEBSERVER_ENABLE
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
		sendHttp500WithBody(client, WEBSERVER_INVALID_PARAMS);
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
		sendHttp500WithBody(client, WEBSERVER_INVALID_PARAMS);
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
		sendHttp500WithBody(client, WEBSERVER_INVALID_PARAMS);
		return false;
	}

	Layer3::packet_t p;
	cmd.setAddress(hwaddress);
	cmd.setHardwareType((HardwareTypeIdentifier) hwtype);
	seq_t sequence = pf.generateHardwareCommandWrite(&p, idInt, &cmd);
	listenerHardwareRequest->init(idInt, (HardwareTypeIdentifier) hwtype, hwaddress, webserverListener::AWAITING_ANSWER);

	boolean success = dispatcher.getResponseHandler()->registerListenerBySeq(millis()+WEBSERVER_REQUEST_TIMEOUT_MILLIS, sequence, idInt, listenerHardwareRequest);

	success &= l3.sendPacket(p);

	if(!success) {
		sendHttp500WithBody(client, NETWORK_ERROR);
		dispatcher.getResponseHandler()->unregisterListener(listenerHardwareRequest);
		return false;
	}

	return true;
}

void PageMaker::doPageWriteSensor2(EthernetClient* client, hardwareRequestListener* listener)
{
	#ifdef DEBUG_WEBSERVER_ENABLE
	Serial.print(millis());
	Serial.print(F(": doPageWriteSensor2() on clientId="));
	Serial.println(client->_sock);
	#endif

	sendHttpOk(client);
	sendHtmlHeader(client, PAGE_WRITE_SENSOR, false, false);

	client->print(F("<h1>Sensor Write id="));
	client->print(listener->remote);
	client->println(F("</h1>"));
	client->print(F("<h2>hwaddress="));
	client->print(listener->hwaddress);
	client->print(F(" hwtype="));
	printP(client, hardwareTypeStrings[listener->hwtype]);
	if(listener->cmd.isRead) {
		client->print(F(" read"));
		} else {
		client->print(F(" write"));
	}
	client->println(F("</h2>"));

	sendHtmlFooter(client);
	listener->init(0, HWType_UNKNOWN, 0, webserverListener::START);
}

boolean PageMaker::doPageRequestSensor(EthernetClient* client, RequestContent* req, hardwareRequestListener* hwListener)
{
	#ifdef DEBUG_WEBSERVER_ENABLE
	Serial.print(millis());
	Serial.print(F(": doPageRequestSensor() on client="));
	Serial.println(client->_sock);
	#endif

	String* id = req->getValue(variableRemote);
	String* hwAddressStr = req->getValue(variableHwAddress);
	String* hwtypeStr = req->getValue(variableHwType);
	if(id == NULL || hwAddressStr == NULL || hwtypeStr == NULL) {
		sendHttp500WithBody(client, WEBSERVER_INVALID_PARAMS);
		return false;
	}

	int8_t idInt = id->toInt();
	int8_t hwaddress = hwAddressStr->toInt();
	int8_t hwtype = hwtypeStr->toInt();

	if(idInt == -1 || hwaddress == -1 || hwtype == -1) {
		sendHttp500WithBody(client, WEBSERVER_INVALID_PARAMS);
		return false;
	}

	Layer3::packet_t p;
	seq_t sequence = pf.generateHardwareCommandRead(&p, idInt, hwaddress, (HardwareTypeIdentifier) hwtype);
	hwListener->init(idInt, (HardwareTypeIdentifier) hwtype, hwaddress, webserverListener::AWAITING_ANSWER);

	boolean success = dispatcher.getResponseHandler()->registerListenerBySeq(millis()+WEBSERVER_REQUEST_TIMEOUT_MILLIS, sequence, idInt, hwListener);

	success &= l3.sendPacket(p);

	if(!success) {
		sendHttp500WithBody(client, NETWORK_ERROR);
		dispatcher.getResponseHandler()->unregisterListener(hwListener);
		return false;
	}

	return true;
}

void PageMaker::doPageRequestSensor2(EthernetClient* client, hardwareRequestListener* listener)
{
	#ifdef DEBUG_WEBSERVER_ENABLE
	Serial.print(millis());
	Serial.print(F(": doPageRequestSensor2() on clientId="));
	Serial.println(client->_sock);
	#endif

	sendHttpOk(client);
	sendHtmlHeader(client, PAGE_REQUEST_SENSOR, false);

	client->print(F("<h1>Sensor Info id="));
	client->print(listener->remote);
	client->println(F("</h1>"));
	client->print(F("<h2>hwaddress="));
	client->print(listener->hwaddress);
	client->print(F(" hwtype="));
	printP(client, hardwareTypeStrings[listener->hwtype]);
	if(listener->cmd.isRead) {
		client->print(F(" read"));
	} else {
		client->print(F(" write"));
	}
	client->println(F("</h2>"));

	prettyPrintCommandResult(client, &listener->cmd);

	sendHtmlFooter(client);
	listener->init(0, HWType_UNKNOWN, 0, webserverListener::START);
}

void PageMaker::prettyPrintCommandResultGeneric(EthernetClient* client, command_t* cmd)
{
	//fallback
	client->println(F("<code><pre>"));
	if(cmd->numUint8 > 0) {
		client->print(F("Uint8:\t"));
		for(uint8_t i = 0; i < cmd->numUint8; i++)	 {
			client->print(cmd->uint8list[i]);
			client->print(F("\t"));
		}
		client->println(F("<br/>"));
	}
	if(cmd->numUint16 > 0) {
		client->print(F("Uint16:\t"));
		for(uint8_t i = 0; i < cmd->numUint16; i++) {
			client->print(cmd->uint16list[i]);
			client->print(F("\t"));
		}
		client->println(F("<br/>"));
	}
	if(cmd->numInt8 > 0) {
		client->print(F("Int8:\t"));
		for(uint8_t i = 0; i < cmd->numInt8; i++) {
			client->print(cmd->int8list[i]);
			client->print(F("\t"));
		}
		client->println(F("<br/>"));
	}
	if(cmd->numInt16 > 0) {
		client->print(F("Int16:\t"));
		for(uint8_t i = 0; i < cmd->numInt16; i++) {
			client->print(cmd->int16list[i]);
			client->print(F("\t"));
		}
		client->println(F("<br/>"));
	}
	client->println(F("</pre></code>"));
}

void PageMaker::prettyPrintCommandResult(EthernetClient* client, command_t* cmd)
{
	switch(cmd->type) {
		case HWTYPE_led:
			prettyPrintLed(cmd, client);
			break;

		case HWType_button:
			prettyPrintButton(cmd, client);
			break;

		case HWType_humidity:
			prettyPrintHumidity(client, cmd);
			break;

		case HWType_pressure:
			prettyPrintPressure(cmd, client);
			break;

		case HWType_temprature:
			prettyPrintTemperature(client, cmd);
			break;

		case HWType_rtc:
			prettyLinkRTC(cmd, client);
			break;

		case HWType_motion:
			prettyPrintMotion(cmd, client);
			break;

		case HWType_light:
			prettyPrintLight(cmd, client);
			break;
			
		default:
			prettyPrintCommandResultGeneric(client, cmd);
	}
}

void PageMaker::prettyPrintLed(command_t* cmd, EthernetClient* client)
{
	if(cmd->uint8list[0] == 1) {
		client->print(F("on"));
	} else {
		client->print(F("off"));
	}
}

void PageMaker::prettyPrintButton(command_t* cmd, EthernetClient* client)
{
	if(cmd->uint8list[0] == 1) {
		client->print(F("pressed"));
	} else {
		client->print(F("not pressed"));
	}
}

void PageMaker::prettyPrintMotion(command_t* cmd, EthernetClient* client)
{
	if(cmd->uint8list[0] == 1) {
		client->print(F("motion detected"));
		} else {
		client->print(F("no motion"));
	}
}

void PageMaker::prettyPrintHumidity(EthernetClient* client, command_t* cmd)
{
	client->print(F("<script type='text/javascript' src='https://www.google.com/jsapi'></script>"
		"<script type='text/javascript'>"
		"google.load('visualization', '1', {packages:['gauge']});"
		"google.setOnLoadCallback(drawChart);"
		"function drawChart() {"
			"	var data = google.visualization.arrayToDataTable(["
			"	['Label', 'Value'],"
			"	['Humidity', "));
			client->print(cmd->uint8list[0]);
			client->print(F("]"
			"	]);"
			"	var options = {"
				"		width: 120, height: 120,"
				"		yellowFrom: 0, yellowTo: 30,"
				"		minorTicks: 5,"
				"		min: 0,"
				"		max: 100"
			"	};"
			"	var chart = new google.visualization.Gauge(document.getElementById('chart_div'));"
			"	chart.draw(data, options);"
		"}"
		"</script>"
		"<div id='chart_div' style='width: 120px; height: 120px;'></div>"));
}

void PageMaker::prettyPrintPressure(command_t* cmd, EthernetClient* client)
{
	//prettyPrintCommandResultGeneric(client, cmd);
	uint16_t hPaVal = cmd->uint16list[0];
	client->print(F("<script type='text/javascript' src='https://www.google.com/jsapi'></script>"
	"<script type='text/javascript'>"
	"google.load('visualization', '1', {packages:['gauge']});"
	"google.setOnLoadCallback(drawChart);"
	"function drawChart() {"
		"	var data = google.visualization.arrayToDataTable(["
		"	['Label', 'Value'],"
		"	['Pressure', "));
		client->print(hPaVal);
		client->print(F("]"
		"	]);"
		"	var options = {"
			"		width: 120, height: 120,"
			"		greenFrom: 1013, greenTo:1090,"
			"		yellowFrom: 950, yellowTo: 1013,"
			"		redFrom: 870, redTo: 950,"
			"		minorTicks: 10,"
			"		min: 870,"
			"		max: 1090"
		"	};"
		"	var chart = new google.visualization.Gauge(document.getElementById('chart_div'));"
		"	chart.draw(data, options);"
	"}"
	"</script>"
	"<div id='chart_div' style='width: 120px; height: 120px;'></div>"));
}

void PageMaker::prettyPrintTemperature(EthernetClient* client, command_t* cmd)
{
	//prettyPrintCommandResultGeneric(client, cmd);
	client->print(F("<script type='text/javascript' src='https://www.google.com/jsapi'></script>"
		"<script type='text/javascript'>"
		"google.load('visualization', '1', {packages:['gauge']});"
		"google.setOnLoadCallback(drawChart);"
		"function drawChart() {"
		"	var data = google.visualization.arrayToDataTable(["
		"	['Label', 'Value'],"
		"	['Temperature', "));
	client->print(cmd->uint8list[0]);
	client->print(F("]"
		"	]);"
		"	var options = {"
		"		width: 120, height: 120,"
		"		greenFrom: 18, greenTo:25,"
		"		yellowFrom: 25, yellowTo: 30,"
		"		redFrom: 30, redTo: 50,"
		"		minorTicks: 5,"
		"		min: -10,"
		"		max: 40"
		"	};"
		"	var chart = new google.visualization.Gauge(document.getElementById('chart_div'));"
		"	chart.draw(data, options);"
		"}"
		"</script>"
		"<div id='chart_div' style='width: 120px; height: 120px;'></div>"));
}

void PageMaker::prettyLinkRTC(command_t* cmd, EthernetClient* client)
{
	uint32_t tmp = 0;
	tmp = (uint32_t) cmd->uint8list[0] << 24;
	tmp |= (uint32_t) cmd->uint8list[1] << 16;
	tmp |= (uint16_t) cmd->uint8list[2] << 8;
	tmp |= cmd->uint8list[3];
	printDate(client, tmp);
}

void PageMaker::prettyPrintLight(command_t* cmd, EthernetClient* client)
{
	int32_t tmp = 0;
	tmp = cmd->int16list[0];

	uint8_t level = 100 - tmp * 100 / 1024;

	client->print(level);
	client->print(F(" %"));
}

boolean PageMaker::getRouteInfoForNode(uint8_t nodeId, boolean &neighbourActive, uint32_t &neighbourLastKeepAlive, uint8_t &neighbourHops, uint8_t &neighbourNextHop) {
#ifdef ENABLE_EXTERNAL_RAM
	SPIRamManager::iterator it;
	l3.getNeighbourManager()->getIterator(&it);
	while(it.hasNext()) {
		NeighbourManager::neighbourData_t* currentItem = (NeighbourManager::neighbourData_t*) it.next();
#else
	for(uint8_t j = 0; j < CONFIG_L3_NUM_NEIGHBOURS; j++) {
		NeighbourManager::neighbourData_t* currentItem = &l3.getNeighbourManager()->neighbours[j];
#endif
		if(currentItem->nodeId == nodeId) {
			neighbourActive = 1;
			neighbourLastKeepAlive = currentItem->timestamp;
			neighbourHops = currentItem->hopCount;
			neighbourNextHop = currentItem->hopNextNodeId;
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
