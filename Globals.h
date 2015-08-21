/*
 * Globals.h
 *
 * Created: 26.05.2015 15:03:41
 *  Author: helge
 */


#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <DebugConfig.h>

#include <networking/Layer3.h>
#include <drivers/HardwareDriver.h>
#include <drivers/HardwareID.h>
#include <dispatcher/HardwareInterface.h>
#include <dispatcher/PacketDispatcher.h>
#include <dispatcher/PacketFactory.h>
#include <utils/SimpleTimer.h>

uint16_t address_local;
#define PIN_CE A0
#define PIN_CSN SS
Layer2rf24 l2;
Layer3 l3;
PacketDispatcher dispatcher;
HardwareInterface hwInterface;
PacketFactory pf;
SimpleTimer timer;

#include <drivers/digitalio/DHT11.h>
#include <interfaces/output/RCSwitchTevionFSI07.h>
#include <interfaces/input/MotionDetector.h>
#include <interfaces/input/Light.h>

#ifdef RTC_ENABLE
	#include <interfaces/input/RTC.h>
	RTC rtc;
#endif

#ifdef SDCARD_ENABLE
	#include <sdcard/SDcard.h>
	SDcard sdcard;

	#ifdef SDCARD_LOGGER_ENABLE
		#include <sdcard/SDHardwareResponseListener.h>
		SDHardwareRequestListener sdlistener;
	#endif

	#include <sdcard/SubscriptionManager.h>
	SubscriptionManager subscriptionManager;
#endif

#ifdef WEBSERVER_ENABLE
	#include <webserver/WebServer.h>
	WebServer webServer;
#endif


DHT11 dht11;
RCSwitchTevionFSI07 rcsw;
MotionDetector motion;
Light light;

///** error codes, see https://en.wikipedia.org/wiki/List_of_HTTP_status_codes */
//typedef enum errorCode_t {
//n2xx_Success, //This class of status codes indicates the action requested by the client was received, understood, accepted and processed successfully.
	//n201_Created, //The request has been fulfilled and resulted in a new resource being created.
	//n202_Accepted, //The request has been accepted for processing, but the processing has not been completed. The request might or might not eventually be acted upon, as it might be disallowed when processing actually takes place.
	//n204_No_Content, //The server successfully processed the request, but is not returning any content.
//n3xx_Redirection, //This class of status code indicates the client must take additional action to complete the request. Many of these status codes are used in URL redirection.
	//n300_Multiple_Choices, //Indicates multiple options for the resource that the client may follow. It, for instance, could be used to present different format options for video, list files with different extensions, or word sense disambiguation.
	//n301_Moved_Permanently, //This and all future requests should be directed to the given URI.
	//n307_Temporary_Redirect, //In this case, the request should be repeated with another URI; however, future requests should still use the original URI. In contrast to how 302 was historically implemented, the request method is not allowed to be changed when reissuing the original request. For instance, a POST request should be repeated using another POST request.[12]
	//n308_Permanent_Redirect, //The request, and all future requests should be repeated using another URI. 307 and 308 (as proposed) parallel the behaviours of 302 and 301, but do not allow the HTTP method to change. So, for example, submitting a form to a permanently redirected resource may continue smoothly.[13]
//n4xx_Client_Error, //The 4xx class of status code is intended for cases in which the client seems to have erred. Except when responding to a HEAD request, the server should include an entity containing an explanation of the error situation, and whether it is a temporary or permanent condition. These status codes are applicable to any request method. User agents should display any included entity to the user.
	//n400_Bad_Request, //The server cannot or will not process the request due to something that is perceived to be a client error (e.g., malformed request syntax, invalid request message framing, or deceptive request routing).[15]
	//n401_Unauthorized, //Similar to 403 Forbidden, but specifically for use when authentication is required and has failed or has not yet been provided. The response must include a WWW-Authenticate header field containing a challenge applicable to the requested resource. See Basic access authentication and Digest access authentication.
	//n403_Forbidden, //The request was a valid request, but the server is refusing to respond to it. Unlike a 401 Unauthorized response, authenticating will make no difference.
	//n404_Not_Found, //The requested resource could not be found but may be available again in the future. Subsequent requests by the client are permissible.
	//n405_Method_Not_Allowed, //A request was made of a resource using a request method not supported by that resource; for example, using GET on a form which requires data to be presented via POST, or using PUT on a read-only resource.
	//n408_Request_Timeout, //The server timed out waiting for the request. According to HTTP specifications: "The client did not produce a request within the time that the server was prepared to wait. The client MAY repeat the request without modifications at any later time."
	//n409_Conflict, //Indicates that the request could not be processed because of conflict in the request, such as an edit conflict in the case of multiple updates.
	//n410_Gone, //Indicates that the resource requested is no longer available and will not be available again. This should be used when a resource has been intentionally removed and the resource should be purged. Upon receiving a 410 status code, the client should not request the resource again in the future. Clients such as search engines should remove the resource from their indices.[16] Most use cases do not require clients and search engines to purge the resource, and a "404 Not Found" may be used instead.
	//n411_Length_Required, //The request did not specify the length of its content, which is required by the requested resource.
	//n413_Payload_Too_Large, //The request is larger than the server is willing or able to process. Called "Request Entity Too Large " previously.
	//n414_Request_URI_Too_Long, //The URI provided was too long for the server to process. Often the result of too much data being encoded as a query-string of a GET request, in which case it should be converted to a POST request.
	//n417_Expectation_Failed, //The server cannot meet the requirements of the Expect request-header field.
	//n423_Locked, //The resource that is being accessed is locked.[4]
	//n428_Precondition_Required, //The origin server requires the request to be conditional. Intended to prevent "the 'lost update' problem, where a client GETs a resource's state, modifies it, and PUTs it back to the server, when meanwhile a third party has modified the state on the server, leading to a conflict."[19]
	//n429_Too_Many_Requests, //The user has sent too many requests in a given amount of time. Intended for use with rate limiting schemes.[19]
	//n431_Request_Header_Fields_Too_Large, //The server is unwilling to process the request because either an individual header field, or all the header fields collectively, are too large.[19]
	//n440_Login_Timeout, //A Microsoft extension. Indicates that your session has expired.[20]
//n5xx_Server_Error, //The server failed to fulfill an apparently valid request., //The server failed to fulfill an apparently valid request.
	//n500_Internal_Server_Error, //A generic error message, given when an unexpected condition was encountered and no more specific message is suitable.
	//n501_Not_Implemented, //The server either does not recognize the request method, or it lacks the ability to fulfill the request. Usually this implies future availability (e.g., a new feature of a web-service API).
	//n502_Bad_Gateway, //The server was acting as a gateway or proxy and received an invalid response from the upstream server.
	//n503_Service_Unavailable, //The server is currently unavailable (because it is overloaded or down for maintenance). Generally, this is a temporary state.
	//n504_Gateway_Timeout, //The server was acting as a gateway or proxy and did not receive a timely response from the upstream server.
	//n505_HTTP_Version_Not_Supported, //The server does not support the HTTP protocol version used in the request.
	//n507_Insufficient_Storage, //The server is unable to store the representation needed to complete the request.[4]
	//n520_Unknown_Error, //This status code is not specified in any RFC and is returned by certain services, for instance Microsoft Azure and CloudFlare servers: "The 520 error is essentially a “catch-all” response for when the origin server returns something unexpected or something that is not tolerated/interpreted (protocol violation or empty response)."[33]
	//n522_Origin_Connection_Time_out, //This status code is not specified in any RFCs, but is used by CloudFlare's reverse proxies to signal that a server connection timed out.
//};
//
//volatile errorCode_t ERRNO;
#endif /* GLOBALS_H_ */