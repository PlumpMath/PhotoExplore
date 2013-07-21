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

	bool DrawingEnabled;
	int sampleCount;

	Frame lastFrame;
	Timer lastActiveTime;
	View * infoText;		

	FlyWheel * flyWheel;

public:
	void onFrame(const Controller & controller);
	void draw();

	void setFlyWheel(FlyWheel * flyWheel);
	void setSwipeDetectedListener(boost::function<void(Hand swipingHand, Vector swipeVector)>);

};

#endif