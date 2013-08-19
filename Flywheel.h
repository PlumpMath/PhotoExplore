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
	volatile double tmpFriction;
	BoundaryMode boundaryMode;

	Timer wheelTimer;
	DoubleAnimation scrollAnimation;
	double currentTarget;

protected:
	volatile double position,velocity, minValue, maxValue, minVelocity, maxVelocity, friction;
	volatile bool isDragging;
	
	virtual void update(double deltaTicks);
	virtual void boundPosition();

public:
	
	FlyWheel(double startPosition = 0);
	void impartVelocity(double velocity);
	void applyFrictionUntilStopped(double frictionCoefficient);
	
	double getPosition();

	double getVelocity();
	void setVelocity(double velocity);
	
	BoundaryMode getBoundaryMode();
	void setBoundaryMode(BoundaryMode _mode);

	double getFriction();
	void setFriction(double friction);

	void setMaxValue(double maxValue);
	double getMaxValue();
	void setMinValue(double minValue);
	double getMinValue();

	virtual void overrideValue(double value);
	double getCurrentPosition();

	virtual void setDraggingState(bool isDragging);

	virtual void flingWheel(double flingSpeed);


};


#endif