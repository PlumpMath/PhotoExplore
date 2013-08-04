
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

#include "LeapCursor.hpp"

using namespace Leap;

class LeapDebug : public Listener {

private:
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