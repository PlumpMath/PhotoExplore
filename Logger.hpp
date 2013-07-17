#ifndef LEAPIMAGE_LOGGER_HPP_
#define LEAPIMAGE_LOGGER_HPP_

#include <iostream>
#include <fstream>
#include "SDLTimer.h"

//namespace Severity
//{
//	
//	static char * ERROR = "ERROR";
//	static char *  WARN = "WARN";
//	static char *  INFO = "INFO";
//	static char *  DEBUG = "DEBUG";
//
//}

class Logger {
	
	
private:	


	Logger()
	{
		logTimer.start();
		logstream.open("photo_explorer.log");
		logstream.precision(3);
		logstream.setf(std::ios_base::fixed);
	}

	~Logger()
	{
		logstream.close();
	}


	Logger(Logger const&);
	void operator=(Logger const&); 

	Timer logTimer;

public:		
	std::ofstream logstream;

	static Logger& getInstance()
	{
		static Logger instance; 		
		return instance;
	}
	

	static void log(std::string tag, std::string message)
	{
		getInstance().logstream << "[" << tag << "] - " << message << std::endl;
	}

	static std::ofstream & stream(std::string tag, const char * severity = "INFO")
	{
		getInstance().logstream <<  (long)getInstance().logTimer.millis() << " : " << severity << "- [" << tag << "] - " << " ";
		return getInstance().logstream;
	}


};

#endif