#include "Flywheel.h"
#include "LeapHelper.h"



FlyWheel::FlyWheel(double startPosition)
{
	minVelocity = INT_MIN;
	minValue = INT_MIN;
	maxVelocity = INT_MAX;
	maxValue = INT_MAX;
	friction = .5;
	velocity = 0;
	position = startPosition;
	tmpFriction = 0;
	wheelTimer.start();
}

double FlyWheel::getMaxValue()
{
	return maxValue;
}

void FlyWheel::setMaxValue(double maxValue)
{
	this->maxValue = maxValue;
}

double FlyWheel::getMinValue()
{
	return minValue;
}

void FlyWheel::setMinValue(double minValue)
{
	if (minValue < maxValue)
		this->minValue = minValue;
	else
		this->minValue = maxValue;
}

void FlyWheel::overrideValue(double newValue)
{	
	velocity = 0;
	this->position = newValue;
}


void FlyWheel::setFriction(double f)
{
	friction = f;
}

double FlyWheel::getFriction()
{
	return friction;
}


void FlyWheel::update(double delta)
{
	double elapsedSeconds = ((double)delta)/1000.0;

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
		
	
	if (position > maxValue)
	{
		position = maxValue;
		if (velocity > 0)
			velocity = 0;
	}
	else if (position <= minValue)
	{
		position = minValue;
		if (velocity < 0)
			velocity = 0;
	}        

}

void FlyWheel::impartVelocity(double velocity)
{
	this->velocity = velocity;
}

void FlyWheel::moveTowards(double targetPosition, double deltaMilliseconds)
{
	position = LeapHelper::lowpass(position,targetPosition,.5,(double)deltaMilliseconds / 1000.0);
}

void FlyWheel::spinTo(double targetPosition)
{
	if (targetPosition != currentTarget)
		currentTarget = targetPosition;

	double displacement = (targetPosition - position);	
	double requiredVelocity = displacement / ( friction * ( 1.0 / ( 1.0 - friction ) ) );
	this->velocity = requiredVelocity;
}

void FlyWheel::animateTo(double targetPosition, long duration)
{
	//if (targetPosition != currentTarget && !scrollAnimation.isRunning())
	//	currentTarget = targetPosition;

	//if (duration <= 0)
	//	duration = abs(targetPosition-getPosition());	

	//if (duration == 0)
	//	return;

	//scrollAnimation = DoubleAnimation(getPosition(),targetPosition,duration);
	//scrollAnimation.start();
}


double FlyWheel::getVelocity()
{
	return velocity;
}


void FlyWheel::setVelocity(double _velocity)
{
	this->velocity = _velocity;
}

double FlyWheel::getCurrentPosition()
{
	return position;
}

double FlyWheel::getPosition()
{
	//if (scrollAnimation.isRunning())
	//{
	//	position = scrollAnimation.getValue();
	//	return position;
	//}
	//else
	//{
	double delta = wheelTimer.get_ticks();
	wheelTimer.start();
	update(delta);
	return position;
	//}
}