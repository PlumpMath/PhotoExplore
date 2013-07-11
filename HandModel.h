#include "LeapHelper.h"
#include <vector>
#include <map>
#include <math.h>
#include <algorithm>

#ifndef HandModel_H_
#define HandModel_H_

using namespace Leap;
using namespace std;


class HandModel
{
public:
	HandModel()
	{
		this->HandId= -1;
		this->IndexedFingers = NULL;
		this->IntentFinger = -1;
	}

	
	HandModel(int handId)
	{
		this->HandId= handId;
		this->IndexedFingers = NULL;
		this->IntentFinger = -1;
	}


	HandModel(int handId, int * indexedFingers, int intentFinger)
	{
		this->HandId = handId;
		this->IndexedFingers = indexedFingers;
		this->IntentFinger = intentFinger;
	}

	int HandId;
	int * IndexedFingers;
	int IntentFinger;

	vector<int> SelectFingers(vector<int> fingers)
	{
		vector<int> fingerList;
		if (IndexedFingers != NULL)
		{
			for (int i = 0; i < fingers.size(); i++)
			{
				int id = IndexedFingers[fingers[i]];
				if (id != -1)
				{
					fingerList.push_back(id);
				}
			}
		}
		return fingerList;
	}
};


class HandProcessor
{

	
public:
	static HandProcessor * getInstance();
	static HandModel * LastModel(int handId);
	static HandModel * LastModel();
		
	HandModel * lastModel();
	HandModel * lastModel(int id);
	void processFrame(Frame frame);
	

private:
	int strongDeterminedIndex;
	int * lastThumbs, * lastIntent;
	int historyLength;
	static HandProcessor * instance;

	HandModel defaultModel;

	map<int,HandModel> modelMap;

	int lastDominantHandId;

	long lastFrameId;

	
	HandProcessor();

	int selectIndexFinger(Hand hand, vector<int> * fingerIndexes, int thumbIndex);
	bool hasThumb(Hand hand, vector<int> * fingerIndexes, bool invert);

	HandModel buildModel(Hand hand, bool invert);

	static float getContigousHistory(int * history, int length, int id);
	static void pushHistory(int *history, int length, int id);
	static void purgeFromHistory(int * history, int length, int id);
};

#endif
