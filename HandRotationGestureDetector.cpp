#include "HandRotationGestureDetector.h"

HandRotationGestureDetector::HandRotationGestureDetector()
{
	state = SettlingState;
	frameTimer.start();
}

bool HandRotationGestureDetector::isActive()
{
	return state == MovingState;
}

int HandRotationGestureDetector::getState()
{
	return state;
}

float HandRotationGestureDetector::getRotation()
{
	return lastAngle;
}

float HandRotationGestureDetector::getStartAngle()
{
	return startAngle;
}

Vector HandRotationGestureDetector::getOffset()
{
	return lastPosition - startPosition;
}

void HandRotationGestureDetector::onFrame(const Controller & controller, HandModel * handModel)
{
	Frame frame = controller.frame();
	Hand hand = frame.hand(handModel->HandId);

	lastAngle = LeapHelper::lowpass(lastAngle,hand.palmNormal().roll(),100,frameTimer.millis());
	frameTimer.start();

	switch (state)
	{
	case SettlingState:
		if (hand.isValid() && hand.palmVelocity().magnitude() < 20.0f && abs(lastAngle) < PI/4.0f)
		{
			state = RotatingState;
			startAngle = lastAngle;
		}
		break;
	case RotatingState:
		if (hand.isValid())
		{
			if (abs(lastAngle) > PI/4.0f)
			{
				state = MovingState;
				startAngle = lastAngle;
				startPosition = hand.palmPosition();
			}
		}
		else
		{
			state = SettlingState;
		}
		break;		
	case MovingState:
		if (hand.isValid() && abs(lastAngle) > PI/8.0f)
		{
			lastPosition = hand.palmPosition();
		}
		else
		{
			lastPosition = startPosition;
			state = RotatingState;
		}
		break;
	}
}
