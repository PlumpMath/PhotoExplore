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

boost::property_tree::ptree * GlobalConfig::tree()
{
	if (!getInstance().isLoaded())
		throw new std::runtime_error("uh oh");
	return &(getInstance().propertyTree);
}