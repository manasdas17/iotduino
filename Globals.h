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

DHT11 dht11;
RCSwitchTevionFSI07 rcsw;
MotionDetector motion;
Light light;

#ifndef PRODUCTIVE_MEGA328P
	#include <interfaces/output/MyTone.h>
	MyTone mytone;
#endif

#endif /* GLOBALS_H_ */