#include "LeapHelper.h"
#include <vector>
#include <map>
#include <math.h>
#include <algorithm>

#ifndef HandModel_H_
#define HandModel_H_

using namespace Leap;
using namespace std;


struct HandModel {

	enum HandStates {
		
		Invalid = -1,
		Spread = 1,
		Pointing = 2
	};


	int HandId;
	int IntentFinger;
	int ThumbId;

	bool isLeftHand;

	int Pose;

	HandModel() :
		HandId(-1),
		IntentFinger(-1),
		ThumbId(-1),
		Pose(HandStates::Invalid)
	{}

		
	HandModel(int _handId) :
		HandId(_handId),
		IntentFinger(-1),
		ThumbId(-1),
		Pose(HandStates::Invalid)
	{}

	HandModel(int _handId, int _intentFinger) :
		HandId(_handId),
		IntentFinger(_intentFinger),
		Pose(HandStates::Invalid)
	{
	}

};


class HandProcessor
{

	
public:
	static HandProcessor *getInstance();
	static HandModel * LastModel(int handId);
	static HandModel * LastModel();
		
	HandModel * lastModel();
	HandModel * lastModel(int id);
	void processFrame(Frame frame);
	
	Hand drawHand;

	void draw();

private:
	static HandProcessor * instance;

	HandModel defaultModel;

	map<int,HandModel> modelMap;

	int lastDominantHandId;

	long lastFrameId;

	
	HandProcessor();

	HandModel buildModel(Hand hand, bool invert);
	HandModel buildComplexModel(Hand hand, bool invert);

	int determineHandPose(Hand hand, HandModel * model,vector<pair<Finger,float> > & fingerAngles);
};

#endif
