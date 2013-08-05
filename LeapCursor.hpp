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
	}

	LeapDebugVisual(double size, Color fillColor)
	{
		this->size = size;
		this->fillColor = fillColor;
		this->depth= 10;
		this->lineWidth = 1;
	}

	void onFrame(const Controller & controller);
	void draw();
};

#endif