#ifndef LEAPIMAGE_OVERLAY_LEAPCURSOR_HPP_
#define LEAPIMAGE_OVERLAY_LEAPCURSOR_HPP_

#include <Leap.h>
#include "GLImport.h"

class LeapDebugVisual {

private:
	void drawPointer(LeapDebugVisual * ldv);
	
public:
	LeapDebugVisual()
	{
	}

	
	LeapDebugVisual(Vector _screenPoint, int timeToLive,  int wayOfLife, double size, Color fillColor)
	{
		this->screenPoint = _screenPoint;
		this->size = size;
		this->fillColor = fillColor;
		this->timeToLive = timeToLive;
		this->wayOfLife = wayOfLife;
		this->depth= 10;
		this->lineWidth = 1;
	}


	void iterateLife(int count = 1)
	{
		switch (wayOfLife)
		{
		case LiveForever:
			break;
		case LiveBySize:
			size -= 1.0*count;
			break;
		case LiveByTime:
			timeToLive -= count;
			break;
		}
	}

	bool isAlive()
	{
		switch (wayOfLife)
		{
		case LiveForever:
			return true;
		case LiveBySize:
			return (size > 0);			
		case LiveByTime:
			return timeToLive > 0;
		} 
	}

	Leap::Vector screenPoint;
	Color fillColor,lineColor;
	double size;
	int timeToLive, wayOfLife;
	float depth, lineWidth;

	const static int LiveForever = 0;
	const static int LiveBySize = 1;
	const static int LiveByTime = 2;


	void draw();

};

#endif