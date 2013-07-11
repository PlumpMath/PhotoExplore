#include "GlobalConfig.hpp"


GlobalConfig::GlobalConfig()
{
	putValue("MenuCircleRadius",30.0f);
	putValue("MaxCommentLength",180);
	putValue("PointGestureMinVelocity",30.0f);
	putValue("HoverClickTime",2.0f);
}

int GlobalConfig::PreferredScreenIndex()
{
	return -1;         
}

float GlobalConfig::getFloat(string key)
{
	auto it = configMap.find(key);
	if (it != configMap.end())
		return it->second.floatValue;
	else
		return 0;
}

int GlobalConfig::getInt(string key)
{	
	auto it = configMap.find(key);
	if (it != configMap.end())
		return it->second.intValue;
	else
		return 0;
}

void GlobalConfig::putValue(string key, float value)
{
	configMap.insert(make_pair(key,ConfigValue(value)));
}

void GlobalConfig::putValue(string key, int value)
{
	ConfigValue cv(0);
	cv.intValue = value;
	configMap.insert(make_pair(key,cv));
}
