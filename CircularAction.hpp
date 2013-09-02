#ifndef LEAPIMAGE_CIRCULAR_ACTION_HPP_
#define LEAPIMAGE_CIRCULAR_ACTION_HPP_

#include <Leap.h>
#include "LeapCursor.hpp"
#include <boost/property_tree/ptree.hpp>
#include "SDLTimer.h"
#include <boost/thread.hpp>
#include "Flywheel.h"

using namespace std;
using namespace Leap;

class CircularAction : public OverlayVisual {


private:
//	struct GraspedFinger {
//		
//		int fingerId;
//		
//		float radialError;
//		bool isGrasped;
//		
//		
//	};
	
	Color circleColor;
	
	enum State
	{
		Idle = 0,
		Rotating = 1,
		Active = 2		
	};

	int state;
	Timer sphereRadiusTimer;

	float drawRadius,drawPitch, averageRadius;
	Vector drawPoint;
	
	float minAngle, maxAngle;
	
	bool grasped;
	int numGrasped;
	int trackedFingerId;
	float trackedOffset;
	
	Vector knobHomePosition;
	
	map<int,float> fingerAngleMap, drawFingerAngles;
	map<int,float> fingerErrorMap;
	
	vector<pair<Finger,float> > orderedFingers;

	Frame rotationStartFrame;
	float startPitch;

	boost::mutex mutey;
	
	boost::property_tree::ptree config;

	vector<PointableTouchCursor*> cursors;
	int handId;

	FlyWheel * flyWheel;


	Vector getHandFingerDelta(const Controller & c, Finger f);
	float getHandFingerPitch(const Controller & c, Finger f);

	void updateErrorMap(const Controller & c, Hand hand);
	void updateRotateMap(const Controller & c, Hand hand);
	void updateRotation(const Controller & c, Hand hand);

	void setNewHand(Hand newHand);
	void updateCursors(const Controller & controller);

	void drawFingerOffsets(Vector center, Color lineColor, float radius, vector<pair<float,float> > & offsets);
	void drawPolygon(Vector drawPoint,Color fillColor, float innerRadius, float outerRadius, float startAngle, float endAngle);

public:
	CircularAction();

	void draw();
	void onFrame(const Controller & controller);	

	float getValue();
	bool isGrasped();

};


#endif