
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
	std::map<int,LeapDebugVisual> pointableList;
	std::vector<LeapDebugVisual*> persistentVisuals;
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

public:
	LeapDebug();

	void onFrame(const Controller&  controller);
	void draw();
	void showValue(string key, string value);	
	void showValue(string key, double value);
	void plotValue(string key, Color color, float value);
	void addDebugVisual(LeapDebugVisual * ldv);

	void setTutorialImages(vector<string> names);

	static LeapDebug * instance;


};


#endif