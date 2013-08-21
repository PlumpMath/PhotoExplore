#include "NotchedWheel.hpp"
#include "GlobalConfig.hpp"



NotchedWheel::NotchedWheel(double _notchOffset, double _notchSpacing) :
	notchOffset(_notchOffset),
	notchSpacing(_notchSpacing)
{
	currentNotchIndex = 0;
	hasTarget = false;
}


void NotchedWheel::setNotches(double offset, double spacing)
{
	this->notchOffset = offset;
	this->notchSpacing = spacing;
	hasTarget = false;
}

#ifdef _WIN32

static double round(double number) {
    return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}

#endif

void NotchedWheel::flingWheel(double flingVelocity)
{
	if (!hasTarget)
	{
		targetNotchIndex = currentNotchIndex + sgn(flingVelocity);		
		hasTarget = true;
	}
}

void NotchedWheel::overrideValue(double value)
{
	hasTarget = false;
	FlyWheel::overrideValue(value);
}

void NotchedWheel::update(double elapsedSeconds)
{
	float max_Kp = GlobalConfig::tree()->get<float>("ScrollView.NotchedWheel.NotchTracker.Proportional");
	float max_alpha = GlobalConfig::tree()->get<float>("ScrollView.NotchedWheel.NotchTracker.VelocityComponent");
	float notchChangeThreshold = GlobalConfig::tree()->get<float>("ScrollView.NotchedWheel.NotchChangeThreshold");
	
	if (abs(velocity) > maxVelocity)
	{
		velocity = sgn(velocity) * maxVelocity;
	}
	
	if (velocity != 0)
	{
		double frictionAccel;
		frictionAccel = (-velocity * friction);

		velocity += frictionAccel * elapsedSeconds;
		position += (velocity * elapsedSeconds);
	}
	
	double closestNotchIndex = round((position-notchOffset)/notchSpacing);
	double closestNotch =  closestNotchIndex*notchSpacing;
	closestNotch += notchOffset;
		
	if (!isDragging)
	{
		if (abs(closestNotch - position) < notchSpacing*notchChangeThreshold)
		{
			if ((int)closestNotchIndex != (int)currentNotchIndex)
			{
				int lastNotch = (int)currentNotchIndex;
				currentNotchIndex = (int)closestNotchIndex;
				
				if (currentNotchIndex == targetNotchIndex)
					hasTarget = false;

				if (!notchChangedListener.empty())
				{
					notchChangedListener(lastNotch,(int)currentNotchIndex);
				}
			}
		}

		if (hasTarget)
		{
			closestNotch = (targetNotchIndex*notchSpacing)+notchOffset;
		}

		double alpha = elapsedSeconds/(max_alpha + elapsedSeconds);
		velocity =  velocity*alpha +  (max_Kp  * (closestNotch-position))*(1.0f - alpha);
	}

	boundPosition();
}

void NotchedWheel::setNotchChangedListener(boost::function<void(int,int)> _notchChangedListener)
{
	this->notchChangedListener = _notchChangedListener;
}

double NotchedWheel::getNotchingSpacing()
{
	return this->notchSpacing;
}

double NotchedWheel::getNotchOffset()
{
	return this->notchOffset;
}

void NotchedWheel::setCurrentNotchIndex(int _notchIndex)
{
	this->currentNotchIndex = _notchIndex;
}

void NotchedWheel::setTargetNotchIndex(int _targetNotchIndex)
{
	this->targetNotchIndex = _targetNotchIndex;
	hasTarget = true;
}