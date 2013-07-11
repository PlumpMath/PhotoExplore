#ifndef HandRotationGestureDetector_H_
#define HandRotationGestureDetector_H_

#include <Leap.h>
#include <LeapMath.h>
#include "HandModel.h"
#include "SDLTimer.h"

using namespace Leap;

class HandRotationGestureDetector {
	

private:
	int state;
	float startAngle, lastAngle;
	Vector startPosition, lastPosition;
	Timer frameTimer;


public:
	static const int SettlingState = 0;
	static const int RotatingState = 1;
	static const int MovingState = 2;

	HandRotationGestureDetector();

	bool isActive();
	int getState();

	float getRotation();
	float getStartAngle();
	Vector getOffset();

	void onFrame(const Controller & controller, HandModel * handModel);



};

#endif