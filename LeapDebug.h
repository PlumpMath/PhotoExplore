
#include "GLImport.h"
#include "LeapHelper.h"
#include "Types.h"
#include <vector>
#include <map>
#include "HandModel.h"
#include <sstream>
#include "TextPanel.h"
#include "ContentPanel.hpp"


#ifndef LeapDebug_H
#define LeapDebug_H

using namespace Leap;

class LeapDebugVisual {

	
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

	map<string,View*> tutorialPanels;
	ViewGroup * tutorialLayout;
	ContentPanel * tutorialPanel;

public:
	LeapDebug(HandProcessor * handProcessor);

	void onFrame(const Controller&  controller);
	void draw();
	void showValue(string key, double value);	
	void addDebugVisual(LeapDebugVisual * ldv);

	void setTutorialImages(vector<string> names);

	static LeapDebug * instance;
};


#endif