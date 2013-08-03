#ifndef LEAPIMAGE_LOGGER_HPP_
#define LEAPIMAGE_LOGGER_HPP_

#include <iostream>
#include <fstream>
#include "SDLTimer.h"
#include "GlobalConfig.hpp"


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


	Logger();

	~Logger();

	Logger(Logger const&);
	void operator=(Logger const&); 

	Timer logTimer;

public:		
	std::ofstream logstream;
	std::ofstream nostream;

	static Logger& getInstance();
	

	static void log(std::string tag, std::string message);

	static std::ofstream & stream(std::string tag, const char * severity = "INFO");


};

#endif