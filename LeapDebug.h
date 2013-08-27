
#include "GLImport.h"
#include "LeapHelper.h"
#include "Types.h"
#include <vector>
#include <map>
#include "HandModel.h"
#include <sstream>
#include "ContentPanel.hpp"
#include "LeapCursor.hpp"
#include "WinHelper.hpp"
#include <list>


#ifndef LeapDebug_H
#define LeapDebug_H

#include "LeapCursor.hpp"

using namespace Leap;

class LeapDebug : public Listener {

	struct PlotValue {

		list<float> values;
		Color color;
		float lineWidth;

		PlotValue() :
			lineWidth(0),
			color(Colors::HoloBlueBright)
		{

		}

	};

private:
	Color backgroundColor;
	std::map<int,PointableCursor> pointableList;
	std::vector<OverlayVisual*> persistentVisuals;
	HandProcessor * handProcessor;

	bool debugPlotEnabled;

	Frame lastFrame;

	bool isControllerConnected, isControllerFocused;

	map<string,View*> tutorialPanels;
	ViewGroup * tutorialLayout;
	ContentPanel * tutorialPanel;
	
	map<string,string> debugValues;

	map<string,PlotValue> plots;

	View * plotLegend;

#ifdef _WIN32
	void updateDebugBox();
	void initDebugBox();
	HWND debugTextBox;
#endif
	
	LeapDebug();
	LeapDebug(LeapDebug const&);
	void operator=(LeapDebug const&); 

public:
	static LeapDebug& getInstance()
	{
		static LeapDebug instance; 
		return instance;
	}

	void onFrame(const Controller&  controller);
	
	void showValue(string key, string value);	
	void showValue(string key, double value);
	void plotValue(string key, Color color, float value);
	void addDebugVisual(OverlayVisual * ldv);

	void setTutorialImages(vector<string> names);
	void layoutTutorial();

	void draw();
	void update();
};


#endif