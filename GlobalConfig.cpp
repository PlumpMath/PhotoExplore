#include "GlobalConfig.hpp"
#include <fstream>



GlobalConfig::GlobalConfig()
{
	propertyFileLoaded = false;
}

GlobalConfig& GlobalConfig::getInstance()
{
	static GlobalConfig instance;
	return instance;
}

bool GlobalConfig::isLoaded()
{
	return propertyFileLoaded;
}


void GlobalConfig::loadConfigFile(string path)
{
	using boost::property_tree::ptree;
	
	std::ifstream inf;
	inf.open(path,std::ios::in);
	
	boost::property_tree::read_json(inf,getInstance().propertyTree);
	propertyFileLoaded = true;
	inf.close();

	if (tree()->get<bool>("Leap.PreferLeftHand"))
	{
		this->LeftHanded = true;
	}
}

boost::property_tree::ptree * GlobalConfig::tree()
{
	if (!getInstance().isLoaded())
		throw new std::runtime_error("uh oh");
	return &(getInstance().propertyTree);
}