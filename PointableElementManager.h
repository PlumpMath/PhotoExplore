#ifndef PointableElementManager_H
#define PointableElementManager_H

#include <Leap.h>
#include <LeapMath.h>
#include <list>
#include <map>
#include <stack>
#include "HandModel.h"
#include <boost/function.hpp>
#include "SDLTimer.h"

#include "LeapElement.hpp"

using namespace Leap;
using namespace std;

class GlobalGestureListener {

public:
	virtual bool onLeapGesture(const Controller & controller, const Gesture & gesture) = 0;	
	virtual void onGlobalGesture(const Controller & controller, std::string gestureType) = 0;
	
	virtual void getTutorialDescriptor(vector<string> & tutorial) = 0;

};


class PointableElementManager {
	
private:
	HandProcessor * handProcessor;
	std::list<LeapElement*> testElements;
	LeapElement * gestureFocusElement;
	stack<GlobalGestureListener*> globalGestureListenerStack;

	set<int> processedGestures;
	
	map<int,bool> mouseButtonState;
	map<int,bool> keyState;

	static PointableElementManager * instance;

	PointableElementManager();

	const static int EnterEvent = 0;
	const static int ExitEvent = 1;

	void getRoutingStack(LeapElement * element, stack<LeapElement*> & result);
	void callEvent(LeapElement * element, Pointable & pointable, int eventType);	
	
	Timer pointingGestureTimer;
	int shakeGestureState, pointingGestureState;
	void handleGlobalGestures(const Controller & controller);

	long lastFrameId;
	Frame lastFrame;

	bool hoverClickEnabled;

	int hoverClickState;
	Timer hoverClickTimer;

	LeapElement * hitLastFrame;

	
public:
	static PointableElementManager * getInstance()
	{
		if (instance == NULL)
			instance = new PointableElementManager();
		return instance;
	}

	void registerElement(LeapElement * element);
	void unregisterElement(LeapElement * element);

	void processFrame(const Controller & controller, Frame frame);
	void processInputEvents();
	
	void requestGlobalGestureFocus(GlobalGestureListener * globalListener);
	void releaseGlobalGestureFocus(GlobalGestureListener * globalListener);

	void setHoverClickEnabled(bool enableHoverClick);
	bool isHoverClickEnabled();

	LeapElement * findElementAt(Vector screenPoint);

	
	std::map<int,LeapElement*> lastHit;

};

#endif