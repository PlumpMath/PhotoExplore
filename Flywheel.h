#include <cstdio>
#include <math.h>
#include "Types.h"
#include <limits.h>
#include "SDLTimer.h"
#include "Animation.h"

#ifndef FlyWheelH
#define FlyWheelH

class FlyWheel {

public:
	enum BoundaryMode
	{
		BounceBack,
		WrapAround
	};

private:
	volatile double position,velocity, minValue, maxValue, minVelocity, maxVelocity, friction;
	volatile double tmpFriction;
	BoundaryMode boundaryMode;

	Timer wheelTimer;
	void update(double deltaTicks);
	DoubleAnimation scrollAnimation;
	double currentTarget;

public:
	
	FlyWheel(double startPosition = 0);
	void impartVelocity(double velocity);
	void moveTowards(double targetPosition, double deltaTime);
	void applyFrictionUntilStopped(double frictionCoefficient);
	
	double getPosition();

	double getVelocity();
	void setVelocity(double velocity);
	
	BoundaryMode getBoundaryMode();
	void setBoundaryMode(BoundaryMode _mode);

	double getFriction();
	void setFriction(double friction);

	void animateTo(double targetPosition, long duration = 0);
	void spinTo(double targetPosition);

	void setMaxValue(double maxValue);
	double getMaxValue();
	void setMinValue(double minValue);
	double getMinValue();

	void overrideValue(double value);
	double getCurrentPosition();


};


#endif