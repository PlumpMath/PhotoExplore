#ifndef LEAPIMAGE_SCROLLING_VIEW_HPP_
#define LEAPIMAGE_SCROLLING_VIEW_HPP_

#include <Leap.h>

#include "GLImport.h"

#include "View.hpp"
#include "Flywheel.h"
#include "PointableElementManager.h"


typedef int ScrollOrientation;

class ScrollingView : public View {

private:
	View * content;
	FlyWheel * scrollingFlywheel;
	bool hasGestureFocus;
	ScrollOrientation scrollOrientation; 

	void notifyScrollChanged(float newScroll);

	bool drawNegativeLoad, drawPositiveLoad;
	Timer loadAnimTimer;
	cv::Size2f lastContentSize;

	void drawLoadIndicator(float offset, float thickness);
	Color loadIndicatorColor;
	int loadIndicatorMode;

public:	
	const static ScrollOrientation Horizontal = 2;
	const static ScrollOrientation Vertical = 3;

	boost::function<void(Vector,cv::Size2f)> visibleRectChangedListener;

	ScrollingView(View * contentView, ScrollOrientation scrollOrientation = Horizontal);
	
	void setContent(View * contentView);
	View * getContent();

	void setOrientation(ScrollOrientation orientation);
	ScrollOrientation getOrientation();
	
	void measure(cv::Size2f & measuredSize);
	void layout(Vector position, cv::Size2f size);

	void update();
	void draw();

	FlyWheel * getFlyWheel();

	//Leap
	LeapElement * elementAtPoint(int x, int y, int & elementStateFlags);

	bool onLeapGesture(const Controller & controller, const Gesture & gesture);	
	void onGlobalGesture(const Controller & controller, std::string gestureType);

	void setDrawLoadingIndicator(int mode, Color indicatorColor);

};

#endif