#include "PunchGestureDetector.hpp"
#include "GlobalConfig.hpp"
#include "Types.h"

PunchGestureDetector::PunchGestureDetector()
{

}

PunchGestureDetector::~PunchGestureDetector()
{

}


PunchGestureDetector& PunchGestureDetector::getInstance()
{
	static PunchGestureDetector instance; 
	return instance;
}

void PunchGestureDetector::onFrame(const Controller & controller)
{
	static bool enabled = GlobalConfig::tree()->get<bool>("Leap.CustomGesture.Punch.Enabled");
	static float minPunchVelocity= GlobalConfig::tree()->get<float>("Leap.CustomGesture.Punch.MinPunchVelocity");
	static float maxAngleDeviation = GeomConstants::DegToRad*GlobalConfig::tree()->get<float>("Leap.CustomGesture.Punch.MaxPunchAngleDeviationFromZAxis");

	if (!enabled) return;

	if (punchingHand.isValid() )
	{

	}


}