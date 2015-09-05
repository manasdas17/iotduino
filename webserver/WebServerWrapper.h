/*
* WebServerWrapper.h
*
* Created: 05.09.2015 14:16:40
* Author: helge
*/


#ifndef __WEBSERVERWRAPPER_H__
#define __WEBSERVERWRAPPER_H__

#include<webserver/WebServer.h>

class WebServerWrapper
{
	private:
	WebServer webserver;

	public:
	WebServerWrapper() {

	}

	void loop() {
		webserver.loop();
	}

	void init() {
		webserver.init();
	}

}; //WebServerWrapper

#endif //__WEBSERVERWRAPPER_H__
