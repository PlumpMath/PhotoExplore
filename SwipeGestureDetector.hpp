#ifndef LEAPIMAGE_LEAPUTIL_SWIPE_GESTURE_DETECTOR_HPP_
#define LEAPIMAGE_LEAPUTIL_SWIPE_GESTURE_DETECTOR_HPP_

#include <Leap.h>
#include <list>
#include "GLImport.h"
#include "Types.h"
#include "SDLTimer.h"
#include "View.hpp"
#include <boost/function.hpp>
#include "Flywheel.h"
#include "LeapDebug.h"
#include <boost/thread/mutex.hpp>
#include "CircularAction.hpp"

using namespace Leap;

using namespace std;

struct Sample
{
	Sample(float _palmRadius, float _gestureSpeed, Vector _gestureDirection) : 
		palmRadius(_palmRadius),
		gestureSpeed(_gestureSpeed),
		gestureDirection(_gestureDirection)
	{}

	Sample(float _palmRadius) : 
		palmRadius(_palmRadius),
		gestureSpeed(0),
		gestureDirection(0,0,0)
	{}

	float palmRadius;

	Vector gestureDirection;
	float gestureSpeed;	
};

struct TrackedSwipe 
{
	int id;
	float minTouchDistance;
	float maxSphereGrowth, minSphereGrowth;

	
	list<float> speedList;
	list<float> distanceList;
	list<float> touchDistanceList;

	TrackedSwipe() : 
		id(-1),
		minTouchDistance(1),
		maxSphereGrowth(0),
		minSphereGrowth(0)
	{}

	TrackedSwipe(int _id) : 
		id(_id),
		minTouchDistance(1),
		maxSphereGrowth(0),
		minSphereGrowth(0)
	{}

};

class SwipeGestureDetector : public Listener
{
	
public:
	static SwipeGestureDetector& getInstance();
	
private:
	SwipeGestureDetector();
	SwipeGestureDetector(SwipeGestureDetector const&);
	void operator=(SwipeGestureDetector const&); 
	~SwipeGestureDetector();

private:
	boost::function<void(Hand swipingHand, Vector swipeVector)> swipeDetectedListener;
	Vector directionSum, directionSpeedSum;
	list<Sample> sampleList;

	map<int,TrackedSwipe> swipeMap;

	bool DrawingEnabled, touchScrollingEnabled, swipeScrollingEnabled, knobScrollingEnabled;
	int sampleCount;
	int state;

	Frame lastFrame;
	Timer lastActiveTime;
	View * infoText;		

	FlyWheel * flyWheel;

	void doScrollWheelScrolling(double,double);
	void doTouchZoneScrolling(const Controller & controller);
	void doGestureScrolling(const Controller & controller);
	void doKnobScrolling(const Controller & controller);

	Vector startScrollScreenPoint;
	double startScrollPos, currentScrollVelocity;
	int scrollingPointableId, scrollingHandId;
	
	vector<PointableCursor*> scrollPointVisuals;

	boost::mutex flyWheelMutex;

	Timer newWheelCooldown;

	CircularAction * scrollKnob;



public:
	const static int IdleState = 0;
	const static int GestureScrolling = 1;
	const static int TouchScrolling = 2;
	const static int MouseScrolling = 3;
	const static int KnobScrolling = 4;

	void onFrame(const Controller & controller);
	void draw();

	void setFlyWheel(FlyWheel * flyWheel);
	FlyWheel * getFlyWheel();

	void setTouchScrollingEnabled(bool touchScrollingEnabled);
	void setSwipeScrollingEnabled(bool swipeScrolllingEnabled);

	void setSwipeDetectedListener(boost::function<void(Hand swipingHand, Vector swipeVector)>);

};

#endif