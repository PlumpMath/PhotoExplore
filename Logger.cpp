#include "Logger.hpp"



Logger::Logger()
{
	if (GlobalConfig::tree()->get<bool>("Logging.Enabled"))
	{
		std::string logFilePath = GlobalConfig::tree()->get<std::string>("Logging.File","photo_explorer.log");
		logTimer.start();
		logstream.open(logFilePath);
		logstream.precision(3);
		logstream.setf(std::ios_base::fixed);
	}
}

Logger::~Logger()
{
	if (GlobalConfig::tree()->get<bool>("Logging.Enabled"))
	{
		logstream.close();
	}
}


Logger& Logger::getInstance()
{
	static Logger instance; 		
	return instance;
}


void Logger::log(std::string tag, std::string message)
{
	getInstance().logstream << "[" << tag << "] - " << message << std::endl;
}

std::ofstream & Logger::stream(std::string tag, const char * severity)
{
	if (std::string(severity).compare("TIME") == 0)
		return getInstance().nostream;
	
	getInstance().logstream <<  (long)getInstance().logTimer.millis() << " : " << severity << "- [" << tag << "] - " << " ";
	return getInstance().logstream;
}

