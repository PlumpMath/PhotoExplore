#include <cstdio>
#include <iostream>
#include "SDLTimer.h"

Timer::Timer()
{
	countdownTime = 0;
}

void Timer::start()
{
	countdownTime = 0;
	startTime = boost::posix_time::microsec_clock::local_time();
}

void Timer::countdown(double ticks)
{
	start();
	countdownTime = ticks;
}

bool Timer::elapsed()
{
	if (countdownTime == 0)
		return false;
	
	return millis() > countdownTime;
//	{
//		countdownTime = 0;
//		return true;
//	}
//	else
//	{
//		return false;
//	}
}

double Timer::seconds()
{
	return get_ticks()/1000.0;
}

double Timer::millis()
{
	return get_ticks();
}

double Timer::get_ticks()
{
	return (boost::posix_time::microsec_clock::local_time() - startTime).total_nanoseconds()/1000000.0;
}

bool Timer::counting()
{
	return countdownTime != 0;
}