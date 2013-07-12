#include "GlobalConfig.hpp"



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
	boost::property_tree::read_json(path,getInstance().propertyTree);
	propertyFileLoaded = true;

	if (tree()->get<bool>("Leap.PreferLeftHand"))
	{
		this->LeftHanded = true;
	}
}

int GlobalConfig::PreferredScreenIndex()
{
	return -1;         
}

float GlobalConfig::getFloat(string key)
{
	if (!isLoaded())
		throw new std::exception("uh oh");
	return propertyTree.get<float>(key,0);
}

int GlobalConfig::getInt(string key)
{	
	if (!isLoaded())
		throw new std::exception("uh oh");
	return propertyTree.get<int>(key,0);
}

void GlobalConfig::putValue(string key, float value)
{
	if (!isLoaded())
		throw new std::exception("uh oh");
	propertyTree.put(key,value);
}

void GlobalConfig::putValue(string key, int value)
{
	if (!isLoaded())
		throw new std::exception("uh oh");
	propertyTree.put(key,value);
}

boost::property_tree::ptree * GlobalConfig::tree()
{
	if (!getInstance().isLoaded())
		throw new std::exception("uh oh");
	return &(getInstance().propertyTree);
}