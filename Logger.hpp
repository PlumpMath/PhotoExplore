#ifndef LEAPIMAGE_LOGGER_HPP_
#define LEAPIMAGE_LOGGER_HPP_

#include <iostream>
#include <fstream>

class Logger {
	
	
private:	


	Logger()
	{
		logstream.open("photo_explorer.log");
	}

	~Logger()
	{
		logstream.close();
	}


	Logger(Logger const&);
	void operator=(Logger const&); 

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


};

#endif