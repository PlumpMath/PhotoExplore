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

class LeapInput {
	
private:
	stack<ActivityView*> globalGestureListenerStack;

	set<int> processedGestures;
	
	map<int,bool> mouseButtonState;
	map<int,bool> keyState;

	static LeapInput * instance;

	LeapInput();

	const static int EnterEvent = 0;
	const static int ExitEvent = 1;

	void callEvent(LeapElement * element, Pointable & pointable, int eventType);	
	
	Timer pointingGestureTimer;
	int shakeGestureState, pointingGestureState;
	void handleGlobalGestures(const Controller & controller);

	long lastFrameId;
	Frame lastFrame;

	LeapElement * hitLastFrame;
	LeapElement * mouseHitLast;
		
	InteractionState currentState;
	
	bool drawNonDominant;
	
	LeapElement * topElement;
	
public:
	static LeapInput * getInstance()
	{
		if (instance == NULL)
			instance = new LeapInput();
		return instance;
	}

	void processFrame(const Controller & controller, Frame frame);
	void processInputEvents();
	
	void setTopLevelElement(LeapElement * top);

	void requestGlobalGestureFocus(ActivityView * globalListener);
	void releaseGlobalGestureFocus(ActivityView * globalListener);

	void enableNonDominantCursor(bool enable);

	InteractionState getInteractionState();



};

#endif