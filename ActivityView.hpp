#ifndef LEAPIMAGE_ACTIVITY_VIEW_HPP_
#define LEAPIMAGE_ACTIVITY_VIEW_HPP_

#include "View.hpp"

class ActivityView : public ViewGroup {

public:
	virtual bool onLeapGesture(const Controller & controller, const Gesture & gesture) = 0;	
	virtual void onGlobalGesture(const Controller & controller, std::string gestureType) = 0;
	
	virtual void getTutorialDescriptor(vector<string> & tutorial) = 0;

	virtual void onGlobalFocusChanged(bool isFocused) {}	
};

#endif