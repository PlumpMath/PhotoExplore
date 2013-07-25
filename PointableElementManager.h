#ifndef PointableElementManager_H
#define PointableElementManager_H

#include <Leap.h>
#include <list>
#include <map>
#include <stack>
#include <boost/function.hpp>
#include "SDLTimer.h"
#include "LeapElement.hpp"
#include "ActivityView.hpp"

using namespace Leap;
using namespace std;



struct InteractionState {
		
	enum HandStates {
		
		Invalid = -1,
		Spread = 1,
		Pointing = 2
	};

	//const static int Invalid = -1;
	//const static int Spread = 1;
	//const static int Pointing = 2;

	int ActiveHandId;
	int HandState;
	
	int ClickingPointableId;
	
	InteractionState() :
		ActiveHandId(-1),
		HandState(HandStates::Invalid),
		ClickingPointableId(-1)		
	{

	}

	InteractionState(int _handId, HandStates _handState, bool _clickingPointableId) :
		ActiveHandId(_handId),
		HandState(_handState),
		ClickingPointableId(_clickingPointableId)
	{

	}
	
};

class PointableElementManager {
	
private:
	stack<ActivityView*> globalGestureListenerStack;

	set<int> processedGestures;
	
	map<int,bool> mouseButtonState;
	map<int,bool> keyState;

	static PointableElementManager * instance;

	PointableElementManager();

	const static int EnterEvent = 0;
	const static int ExitEvent = 1;

	void callEvent(LeapElement * element, Pointable & pointable, int eventType);	
	
	Timer pointingGestureTimer;
	int shakeGestureState, pointingGestureState;
	void handleGlobalGestures(const Controller & controller);

	long lastFrameId;
	Frame lastFrame;

	LeapElement * hitLastFrame;
		
	InteractionState currentState;
	
	bool drawNonDominant;
	
	
public:
	static PointableElementManager * getInstance()
	{
		if (instance == NULL)
			instance = new PointableElementManager();
		return instance;
	}

	void processFrame(const Controller & controller, Frame frame);
	void processInputEvents();
	
	void requestGlobalGestureFocus(ActivityView * globalListener);
	void releaseGlobalGestureFocus(ActivityView * globalListener);

	void enableNonDominantCursor(bool enable);

	InteractionState getInteractionState();

	//LeapElement * findElementAt(Vector screenPoint);
		
	//std::map<int,LeapElement*> lastHit;



};

#endif