#ifndef LEAPIMAGE_CONFIGURATION_HPP_
#define LEAPIMAGE_CONFIGURATION_HPP_

#include <string>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace std;

class GlobalConfig
{

	struct ConfigValue {

		float floatValue;
		string stringValue;
		bool boolValue;
		int intValue;

		ConfigValue(float _value) :
			floatValue(_value)
		{}

	};

public:
	static GlobalConfig& getInstance();

	static boost::property_tree::ptree * tree();

private:
	GlobalConfig();

	GlobalConfig(GlobalConfig const&);
	void operator=(GlobalConfig const&); 

	//map<string,ConfigValue> configMap;
	boost::property_tree::ptree propertyTree;

	bool propertyFileLoaded;

public:
	static int PreferredScreenIndex();

	bool isLoaded();

	static std::string TestingToken;
	static int ScreenWidth,ScreenHeight;	
	static float SteadyVelocity, SelectCircleMinRadius, MinimumInteractionScreenDistance;
	static bool LeftHanded,AllowSingleHandInteraction;

	void loadConfigFile(string configFilePath);

	float getFloat(string key);
	string getString(string key);
	bool getBool(string key);
	int getInt(string key);

	void putValue(string key, float value);
	void putValue(string key, bool value);
	void putValue(string key, int value);
	void putValue(string key, string value);

};


#endif