#ifndef WinTimer_H
#define WinTimer_H

#include <boost/date_time/posix_time/posix_time.hpp>

class Timer
{
private:	
	double countdownTime;
	boost::posix_time::ptime startTime;
public:

	Timer();
	void start();
	void countdown(double ticks);
	bool elapsed();
	double seconds();
	double millis();
	double get_ticks();
};

#endif