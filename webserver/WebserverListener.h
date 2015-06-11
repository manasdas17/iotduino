/*
 * WebserverListener.h
 *
 * Created: 11.06.2015 21:23:27
 *  Author: helge
 */


#ifndef WEBSERVERLISTENER_H_
#define WEBSERVERLISTENER_H_

#include <dispatcher/EventCallbackInterface.h>

/**
 * generic webserver listener class - includes a state.
 */
class webserverListener : public EventCallbackInterface {
	public:
	enum STATE {START, AWAITING_ANSWER, FINISHED, FAILED};
	STATE state;
};

#endif /* WEBSERVERLISTENER_H_ */