#ifndef LEAPIMAGE_LEAP_STATUS_OVERLAY_HPP_
#define LEAPIMAGE_LEAP_STATUS_OVERLAY_HPP_

#include "View.hpp"
#include "TextPanel.h"
#include "ActivityView.hpp"

#include <Leap.h>

using namespace Leap;

class LeapStatusOverlay : public ActivityView, public Listener {

private:
	TextPanel * notFocusedView;
	TextPanel * notConnectedView;

	//Accessible by both Leap thread and render thread
	volatile bool isConnected,isFocused;

public:
	LeapStatusOverlay();

	void onFramePoll(const Controller & controller);
		
	void layout(Vector position, cv::Size2f size);

	void draw();
	
	void onGlobalGesture(const Controller & controller, std::string gestureType);
	void getTutorialDescriptor(vector<string> & tutorial);

};


#endif