#include <windows.h>
#include <cstdio>
#include <iostream>
#include "SDLTimer.h"


Timer::Timer()
{   
	PCFreq = 0.0;
	CounterStart = 0;
	countdownTime = 0;
}


void Timer::start()
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);

	PCFreq = double(li.QuadPart)/1000.0;

	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
}

void Timer::countdown(double ticks)
{
	start();
	countdownTime = ticks;
}

bool Timer::elapsed()
{
	if (countdownTime == 0)
		return true;

	return millis() > countdownTime;
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
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart-CounterStart)/PCFreq;
}


double Timer::getTimestamp()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart);
}