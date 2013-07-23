#ifndef LEAPIMAGE_EVENTHANDLING_LEAP_ELEMENT_HPP_
#define LEAPIMAGE_EVENTHANDLING_LEAP_ELEMENT_HPP_

#include <Leap.h>
#include <boost/function.hpp>
#include <opencv2/opencv.hpp>

using namespace Leap;

class LeapElement {

private:
	LeapElement * parent;

protected:
	bool clickable, enabled;
	
public:
	const static int Flags_ElementNonClickable = 0x0001;
	const static int Flags_None = 0;


	boost::function<void(LeapElement * node)> elementClickedCallback;
	boost::function<void(LeapElement * node, Pointable & pointable)> pointableEnterCallback;
	boost::function<void(LeapElement * node, Pointable & pointable)> pointableExitCallback;

	LeapElement();
	
	void setParent(LeapElement * _parent);

	LeapElement * getParent();

	virtual cv::Rect_<int> getHitRect();

	virtual float getZValue();
	
	virtual bool onGesture(const Gesture & gesture);

	virtual void gestureFocusLost(LeapElement * lostTo);

	virtual void OnPointableEnter(Pointable & pointable);
	
	virtual void setClickable(bool _clickable);

	virtual bool isClickable();

	virtual void setEnabled(bool _enabled);

	virtual bool isEnabled();
	void pointableEnter(Pointable & pointable);

	virtual void OnPointableExit(Pointable & pointable);

	bool interceptPointableExit(LeapElement * origin, Pointable & pointable);

	bool interceptPointableEnter(LeapElement * origin, Pointable & pointable);

	virtual bool OnInterceptPointableExit(LeapElement * origin, Pointable & pointable);

	virtual bool OnInterceptPointableEnter(LeapElement * origin, Pointable & pointable);

	void pointableExit(Pointable & pointable);
	
	void elementClicked();

	virtual void OnElementClicked(Pointable & pointable);

	virtual void onFrame(const Controller & controller);

	virtual LeapElement * elementAtPoint(int x, int y, int & elementStateFlags);
};

#endif