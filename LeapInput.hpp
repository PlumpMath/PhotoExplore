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
#include "HandModel.h"

using namespace Leap;
using namespace std;



struct InteractionState {
			
	int ActiveHandId;
	int HandState;
	
	int ClickingPointableId;
	
	InteractionState() :
		ActiveHandId(-1),
		HandState(HandModel::Invalid),
		ClickingPointableId(-1)		
	{

	}

	InteractionState(int _handId, bool _clickingPointableId) :
		ActiveHandId(_handId),
		HandState(0),
		ClickingPointableId(_clickingPointableId)
	{

	}
	
};

class LeapInput {
	
private:
	stack<ActivityView*> globalGestureListenerStack;
	
	map<int,int> mouseButtonState;

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
	
	bool cursorDrawEnabled, clickEnabled;
	
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

	void setCursorDrawEnabled(bool drawCursors);
	bool isCursorDrawEnabled();
	
	InteractionState getInteractionState();

	void setClickEnabled(bool clickEnabled);
	bool isClickEnabled();


};

#endif