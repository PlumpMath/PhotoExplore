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


static double round(double number) {
    return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}

void NotchedWheel::flingWheel(double flingVelocity)
{
	if (!hasTarget)
	{
		double currentNotchIndex = round((position-notchOffset)/notchSpacing);

		currentNotchIndex += sgn(flingVelocity);

		targetNotch = currentNotchIndex*notchSpacing;
		targetNotch += notchOffset;

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

		if (abs(closestNotch - position) < notchSpacing*0.1f)
		{
			if ((int)closestNotchIndex != (int)currentNotchIndex)
			{
				int lastNotch = (int)currentNotchIndex;
				currentNotchIndex = (int)closestNotchIndex;
				
				if (currentNotchIndex == targetNotch)
					hasTarget = false;

				if (!notchChangedListener.empty())
				{
					notchChangedListener(lastNotch,(int)currentNotchIndex);
				}
			}
		}

		if (hasTarget)
		{
			closestNotch = targetNotch;
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