#include <boost/date_time/posix_time/posix_time.hpp>


#ifndef WinTimer_H
#define WinTimer_H

#ifdef USE_WINDOWS_TIMER

#include <windows.h>

class Timer
{
private:	
	double PCFreq;
	__int64 CounterStart;

	double countdownTime;

public:

	Timer()
	{   
		PCFreq = 0.0;
		CounterStart = 0;
		countdownTime = 0;
	}

	void start()
	{
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);

		PCFreq = double(li.QuadPart)/1000.0;

		QueryPerformanceCounter(&li);
		CounterStart = li.QuadPart;
	}

	void countdown(double ticks)
	{
		start();
		countdownTime = ticks;
	}

	bool elapsed()
	{
		if (countdownTime == 0)
			return true;

		return millis() > countdownTime;
	}

	double seconds()
	{
		return get_ticks()/1000.0;
	}

	double millis()
	{
		return get_ticks();
	}

	double get_ticks()
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return double(li.QuadPart-CounterStart)/PCFreq;
	}

	double getTimestamp()
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return double(li.QuadPart);
	}
};
#endif

class Timer
{
private:	
	double countdownTime;
	boost::posix_time::ptime startTime;
public:

	Timer()
	{   
		countdownTime = 0;
	}

	void start()
	{
		startTime = boost::posix_time::microsec_clock::local_time(); 
	}

	void countdown(double ticks)
	{
		start();
		countdownTime = ticks;
	}

	bool elapsed()
	{
		if (countdownTime == 0)
			return true;

		return millis() > countdownTime;
	}

	double seconds()
	{
		return get_ticks()/1000.0;
	}

	double millis()
	{
		return get_ticks();
	}

	double get_ticks()
	{
		return (boost::posix_time::microsec_clock::local_time() - startTime).total_nanoseconds()/1000000.0;
	}
};

#endif