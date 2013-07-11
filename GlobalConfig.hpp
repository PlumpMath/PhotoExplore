#ifndef LEAPIMAGE_CONFIGURATION_HPP_
#define LEAPIMAGE_CONFIGURATION_HPP_

#include <string>
#include <map>

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
	static GlobalConfig& getInstance()
	{
		static GlobalConfig instance;
		return instance;
	}

private:
	GlobalConfig();

	GlobalConfig(GlobalConfig const&);
	void operator=(GlobalConfig const&); 

	map<string,ConfigValue> configMap;

public:
	static int PreferredScreenIndex();

	static std::string TestingToken;
	static int ScreenWidth,ScreenHeight;	
	static float SteadyVelocity, SelectCircleMinRadius, MinimumInteractionScreenDistance;
	static bool LeftHanded,AllowSingleHandInteraction;

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