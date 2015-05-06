//
//
//  Generated by StarUML(tm) C++ Add-In
//
//  @ Project : Untitled
//  @ File Name : Gyroscope.h
//  @ Date : 20.10.2014
//  @ Author :
//
//


#if !defined(_GYROSCOPE_H)
#define _GYROSCOPE_H

#include <utils/Triple.h>
#include <drivers/HardwareID.h>
#include <dispatcher/Commands.h>


class Gyroscope {
	public:
		virtual Triple<int16_t> readGyro()=0;
		virtual void readGyro(HardwareCommandResult* hwresult)=0;
};

#endif  //_GYROSCOPE_H
