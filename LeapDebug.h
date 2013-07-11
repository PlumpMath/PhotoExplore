
#include "GLImport.h"
#include "LeapHelper.h"
#include "Types.h"
#include <vector>
#include <map>
#include "HandModel.h"
#include <sstream>
#include "TextPanel.h"


#ifndef LeapDebug_H
#define LeapDebug_H

using namespace Leap;

class LeapDebugVisual {

	
public:
	LeapDebugVisual()
	{
	}



	LeapDebugVisual(Point2f screenPoint, Pointable pointable, double size = 20, Color fillColor = Colors::OrangeRed)
	{
		this->screenPoint = screenPoint;
		this->size = size;
		this->fillColor = fillColor;
		this->timeToLive = -1;
		this->wayOfLife = LiveBySize;
		this->depth= 10;
	}


	LeapDebugVisual(Vector screenPoint, int timeToLive,  int wayOfLife, double size, Color fillColor)
	{
		this->screenPoint = Point2f(screenPoint.x,screenPoint.y);
		this->size = size;
		this->fillColor = fillColor;
		this->timeToLive = timeToLive;
		this->wayOfLife = wayOfLife;
		this->depth= 10;
	}


	LeapDebugVisual(Point2f screenPoint, int timeToLive,  int wayOfLife, double size, Color fillColor)
	{
		this->screenPoint = screenPoint;
		this->size = size;
		this->fillColor = fillColor;
		this->timeToLive = timeToLive;
		this->wayOfLife = wayOfLife;
		this->depth= 10;
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

	Point2f screenPoint;
	Color fillColor,lineColor;
	double size;
	int timeToLive, wayOfLife;
	float depth;

	const static int LiveForever = 0;
	const static int LiveBySize = 1;
	const static int LiveByTime = 2;

};

class LeapDebug : public Listener {

private:
	void drawPointer(LeapDebugVisual * position);
	Color backgroundColor;
	std::map<int,LeapDebugVisual> pointableList;
	std::vector<LeapDebugVisual*> persistentVisuals;
	HandProcessor * handProcessor;

	Frame lastFrame;
	TextPanel * leapNotFocusedPanel;
	TextPanel * leapDisconnectedPanel;

	bool isControllerConnected, isControllerFocused;

public:
	LeapDebug(HandProcessor * handProcessor);

	void onFrame(const Controller&  controller);
	void draw();
	void showValue(string key, double value);	
	void addDebugVisual(LeapDebugVisual * ldv);

	static LeapDebug * instance;
};


#endif