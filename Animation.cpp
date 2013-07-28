#include "Animation.h"



DoubleAnimation::DoubleAnimation()
{
	state = AnimatorState::New;
}


DoubleAnimation::DoubleAnimation(double start, double end, long duration, Interpolator * interpolator, bool autoReverse, bool fillAfter)
{
	this->delta = (end - start);
	this->duration = (double)duration;
	this->autoReverse = autoReverse;
	this->fillAfter = fillAfter;

	startValue = start;
	endValue = end;
	state = AnimatorState::New;

	if (interpolator != NULL)
	{
		this->interpolator = interpolator;
	}
	else
	{
		this->interpolator = new AccelerateDecelerateInterpolator();
	}

}

void DoubleAnimation::stop()
{
	state = AnimatorState::New;
}

void DoubleAnimation::start()
{
	if (delta != 0)
	{
		timer.start();
		state = AnimatorState::Forward;	
	}
}

double DoubleAnimation::getElapsedTime()
{
	return timer.millis();
}

bool DoubleAnimation::isRunning()
{
	getValue();
	return state == AnimatorState::Forward || state == AnimatorState::Reverse || state == AnimatorState::FinishedHolding;
}

void DoubleAnimation::setInterpolator(Interpolator * interpolator)
{
	this->interpolator = interpolator;
}

void DoubleAnimation::setAutoReverse(bool autoReverse)
{
	this->autoReverse = autoReverse;
}

void DoubleAnimation::setFillAfter(bool fillAfter)
{
	this->fillAfter = fillAfter;
}

double DoubleAnimation::getValue()
{
	if (autoReverse)
	{
		if (state == AnimatorState::New|| state == AnimatorState::Finished || state == AnimatorState::FinishedHolding)
			return startValue;
		else if (state == AnimatorState::Forward)
		{
			double position = timer.get_ticks()/duration;
			double nextValue = startValue + interpolator->getValue(position)*delta;

			if (position >= 1.0 || (delta > 0 && nextValue >= endValue) || (delta < 0 && nextValue <= endValue))
			{
				state = AnimatorState::Reverse;
				timer.start();
				return endValue;
			}
			else
				return nextValue;
		}
		else if (state == AnimatorState::Reverse)
		{			
			double position = timer.get_ticks()/duration;
			double nextValue = endValue - interpolator->getValue(position)*delta;

			if (position >= 1.0 || (delta > 0 && nextValue <= startValue) || (delta < 0 && nextValue >= startValue))
			{
				if (fillAfter)
					state = AnimatorState::FinishedHolding;
				else
					state = AnimatorState::Finished;

				return startValue;
			}
			else
				return nextValue;
		}
		else
		{
			state = AnimatorState::New;
			return *(&state + 20);
		}
	}
	else
	{
		if (state == AnimatorState::New)
			return startValue;
		else if (state == AnimatorState::Finished || state == AnimatorState::FinishedHolding)
			return endValue;
		else
		{
			double position = timer.get_ticks()/duration;
			double nextValue = startValue + interpolator->getValue(position)*delta;

			if (position >= 1.0 || (delta > 0 && nextValue >= endValue) || (delta < 0 && nextValue <= endValue))
			{
				if (fillAfter)
					state = AnimatorState::FinishedHolding;
				else
					state = AnimatorState::Finished;

				return endValue;
			}
			else
				return nextValue;
		}	
	}
}



