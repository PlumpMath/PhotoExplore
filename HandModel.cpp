#include "HandModel.h"
#include "GlobalConfig.hpp"

HandProcessor * HandProcessor::getInstance()
{
	if (instance == NULL)
		instance = new HandProcessor();

	return instance;
}

HandModel * HandProcessor::LastModel(int handId)
{
	if (instance == NULL)
		instance = new HandProcessor();

	return instance->lastModel(handId);
}

HandModel * HandProcessor::LastModel()
{
	if (instance == NULL)
		instance = new HandProcessor();

	return instance->lastModel();
}


HandProcessor::HandProcessor()
{
	historyLength = 20;
	lastDominantHandId = -1;
	lastThumbs = new int[historyLength];
	lastIntent = new int[historyLength];
}


HandModel * HandProcessor::lastModel()
{
	if (modelMap.size() == 0 || modelMap.count(lastDominantHandId) == 0)
		return &defaultModel;

	return &modelMap[lastDominantHandId];
}


HandModel * HandProcessor::lastModel(int id)
{
	if (modelMap.size() == 0 || modelMap.count(id) == 0)
		return &defaultModel;

	return &modelMap[id];
}

void HandProcessor::processFrame(Frame frame)
{
	if (frame.id() == lastFrameId)
		return;

	if (frame.hands().count() == 0)
	{
		if (frame.pointables().count() > 0)
		{
			Pointable intent;
			if (frame.tools().count() > 0)
				intent = frame.tools().frontmost();
			else
				intent = frame.fingers().frontmost();

			int * fingerArray = new int[5];
			for (int i=0;i<5;i++)
			{
				fingerArray[i] = -1;
			}
			fingerArray[1] = intent.id();

			modelMap[-2] = HandModel(-2,fingerArray,fingerArray[1]);
			lastDominantHandId = -2;			
		}
	}
	else
	{
		Hand dominantHand;
		if (GlobalConfig::LeftHanded)
			dominantHand = frame.hands().leftmost();
		else
			dominantHand = frame.hands().rightmost();

		lastDominantHandId = dominantHand.id();	

		for (int i=0;i<frame.hands().count();i++)
		{
			Hand hand = frame.hands()[i];

			bool isLeft = frame.hands().count() >= 2 && frame.hands().leftmost().id() == hand.id();

			modelMap[hand.id()] = buildModel(hand, isLeft);
		}
	}
	lastFrameId = frame.id();
}

int HandProcessor::selectIndexFinger(Hand hand, vector<int> * fingerIndexes, int thumbIndex)
{
	if (strongDeterminedIndex != -1)
	{
		Finger strongFinger = hand.finger(strongDeterminedIndex);

		if (strongFinger.isValid())
		{
			std::vector<int>::iterator it = std::find(fingerIndexes->begin(), fingerIndexes->end(), strongDeterminedIndex);
			if (it != fingerIndexes->end())
				return (*it);				
		}
		else
		{
			purgeFromHistory(lastIntent, historyLength, strongDeterminedIndex);
			strongDeterminedIndex = -1;
		}
	}
	else
	{
		int lastIndexFinger = lastIntent[historyLength - 1];

		if (hand.finger(lastIndexFinger).isValid())
		{
			if (getContigousHistory(lastIntent, historyLength, lastIndexFinger) >= .95f)
			{
				strongDeterminedIndex = lastIndexFinger;
				std::vector<int>::iterator it = std::find(fingerIndexes->begin(), fingerIndexes->end(), strongDeterminedIndex);
				if (it != fingerIndexes->end())
					return (*it);	
			}
		}
	}

	int intentId, index;
	if (thumbIndex == 0)
	{
		index = 1;
		intentId = fingerIndexes->at(index);
		//pushHistory(lastIntent, intentId);
	}
	else
	{
		index = 0;
		intentId = fingerIndexes->at(index);
		//pushHistory(lastIntent, -1);
	}

	return index;
}

bool HandProcessor::hasThumb(Hand hand, vector<int> * fingerIndexes, bool invert)
{
	float score = 0;

	Finger possibleThumb;
	if (invert)
		possibleThumb = hand.finger(*(--fingerIndexes->end()));
	else
		possibleThumb = hand.finger(*(fingerIndexes->begin()));


	float palmDist = LeapHelper::GetDistanceFromHand(possibleThumb);

	if (palmDist > 0) score += .30f;

	float fingerTipAngle = LeapHelper::GetFingerTipAngle(hand, possibleThumb);
	if (invert)
		fingerTipAngle *= -1;

	score += .7f * (float)(1.0 - abs((fingerTipAngle + PI / 2.0)) / (PI / 2.0));

	//score += .1f * getContigousHistory(lastThumbs, i);

	return score > .5f;
}



HandModel HandProcessor::buildModel(Hand hand, bool invert)
{
	HandModel handModel;

	if (hand.tools().count() > 0)
	{
		Pointable intent = hand.tools().frontmost();

		int * fingerArray = new int[5];
		for (int i=0;i<5;i++)
		{
			fingerArray[i] = -1;
		}
		fingerArray[1] = intent.id();

		handModel = HandModel(hand.id(),fingerArray,fingerArray[1]);
	}
	else if (hand.fingers().count() == 0)
	{
		handModel = HandModel(hand.id());
	}
	else if (hand.fingers().count() == 1)
	{
		Pointable intent = hand.fingers()[0];	
		int * fingerArray = new int[5];
		for (int i=0;i<5;i++)
		{
			fingerArray[i] = -1;
		}
		fingerArray[1] = intent.id();

		handModel = HandModel(hand.id(),fingerArray,fingerArray[1]);
	}
	else
	{
		vector<int> * fingerIndexes = LeapHelper::SortFingers(hand);

		int thumbPosition, indexFingerPosition;
		int size = fingerIndexes->size();

		if (size == 5)
		{
			if (invert)
			{
				thumbPosition = size - 1;
				indexFingerPosition = size - 2;
			}
			else
			{
				thumbPosition = 0;
				indexFingerPosition = 1;		
			}
		}
		else
		{		
			bool thumbVisible = hasThumb(hand, fingerIndexes, invert);
			if (invert)
			{
				thumbPosition = (thumbVisible) ? (size-1) : -1;				
				indexFingerPosition = (thumbVisible) ? (size-2) : (size-1);
			}
			else
			{
				thumbPosition = (thumbVisible) ? 0 : -1;				
				indexFingerPosition = (thumbPosition == 0) ? 1 : 0; // selectIndexFinger(hand, fingerIndexes, thumbPosition);
			}
		}

		int * fingerArray = new int[5];

		for (int i=0;i<5;i++)
		{
			fingerArray[i] = -1;
		}

		if (thumbPosition >= 0)
		{
			fingerArray[0] = fingerIndexes->at(thumbPosition);
		}

		if (indexFingerPosition >= 0)
		{
			fingerArray[1] = fingerIndexes->at(indexFingerPosition);

			for (int i = indexFingerPosition + 1, j = 2; i < fingerIndexes->size() && j < 5; i++, j++)
			{
				int id = fingerIndexes->at(i);
				fingerArray[j] = id;
			}
		}

		handModel = HandModel(hand.id(),fingerArray,fingerArray[1]);
	}
	return handModel;
}

float HandProcessor::getContigousHistory(int * history, int length, int id)
{
	int maxCount = 0;
	int count = 1;
	for (int i = 1; i < length; i++)
	{
		if (history[i] == id && history[i - 1] == id)
		{
			count++;
		}
		else
		{
			maxCount = max(maxCount, count);
			count = 1;
		}
	}
	maxCount = max(maxCount, count);
	return (float)maxCount / (float)length;
}

void HandProcessor::pushHistory(int *history, int length, int id)
{
	for (int i = 0; i < length - 1; i++)
	{
		history[i] = history[i + 1];
	}
	history[length - 1] = id;
}

void HandProcessor::purgeFromHistory(int * history, int length, int id)
{
	for (int i = 0; i < length - 1; i++)
	{
		if (history[i] == id)
			history[i] = -1;
	}
}