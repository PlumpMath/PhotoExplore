#ifndef LEAPIMAGE_CIRCULAR_ACTION_HPP_
#define LEAPIMAGE_CIRCULAR_ACTION_HPP_

#include <Leap.h>
#include "LeapCursor.hpp"
#include <boost/property_tree/ptree.hpp>
#include "SDLTimer.h"
#include <boost/thread.hpp>

using namespace std;
using namespace Leap;

class CircularAction : public OverlayVisual {


private:
	Color circleColor;
	
	enum State
	{		
		Steadying = 0,
		Rotating = 1,
		Active = 2		
	};

	int state;
	Timer sphereRadiusTimer;

	float drawRadius,drawPitch, averageRadius;
	Vector drawPoint;
	
	bool grasped;

	map<int,float> fingerAngleMap, drawFingerAngles;
	map<int,float> fingerErrorMap;

	boost::mutex mutey;


	boost::property_tree::ptree config;

	vector<PointableCursor*> cursors;
	int handId;


	Vector getHandFingerDelta(const Controller & c, Finger f);
	float getHandFingerPitch(const Controller & c, Finger f);

	void updateErrorMap(const Controller & c, Hand hand);
	void updateRotateMap(const Controller & c, Hand hand);

	void setNewHand(Hand newHand);
	void updateCursors(const Controller & controller);

	void drawFingerOffsets(Vector center, Color lineColor, float radius, vector<pair<float,float> > & offsets);
	void drawPolygon(Vector center, Color lineColor, Color fillColor, float radius, float angle);

public:
	CircularAction();

	void draw();
	void onFrame(const Controller & controller);	

};


#endif