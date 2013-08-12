#ifndef LEAPIMAGE_LEAP_STATUS_OVERLAY_HPP_
#define LEAPIMAGE_LEAP_STATUS_OVERLAY_HPP_

#include "View.hpp"
#include "TextPanel.h"

#include <Leap.h>

using namespace Leap;

class LeapStatusOverlay : public ViewGroup, public Listener {

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

};


#endif