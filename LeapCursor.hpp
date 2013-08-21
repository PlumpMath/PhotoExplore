#ifndef LEAPIMAGE_OVERLAY_LEAPCURSOR_HPP_
#define LEAPIMAGE_OVERLAY_LEAPCURSOR_HPP_

#include <Leap.h>
#include "GLImport.h"
#include "Types.h"

using namespace Leap;

class LeapDebugVisual {
		
private:
	Vector drawPoint;

public:	
	int trackPointableId;
	bool trackMouseCursor;

	Color fillColor,lineColor;
	double size;
	float depth, lineWidth;

	LeapDebugVisual(Pointable _trackPointable, double size, Color fillColor)
	{
		this->trackPointableId = _trackPointable.id();
		this->size = size;
		this->fillColor = fillColor;
		this->depth= 10;
		this->lineWidth = 1;
		trackMouseCursor = false;
	}

	LeapDebugVisual(double size, Color fillColor)
	{
		trackPointableId = -1;
		this->size = size;
		this->fillColor = fillColor;
		this->depth= 10;
		this->lineWidth = 1;
		trackMouseCursor = false;
	}

	void onFrame(const Controller & controller);
	void update();
	void draw();
};

#endif