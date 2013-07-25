#include "HandModel.h"
#include "GlobalConfig.hpp"
#include "GLImport.h"

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
			
			modelMap[-2] = HandModel(-2,intent.id());
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

		drawHand = dominantHand;

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


HandModel HandProcessor::buildComplexModel(Hand hand, bool isLeft)
{
	static float minThumbFingerSpacing = GlobalConfig::tree()->get<float>("Leap.HandModel.MinThumbFingerSpacing")*GeomConstants::DegToRad;
	static float minThumbFingerAngle = GlobalConfig::tree()->get<float>("Leap.HandModel.MinThumbFingerAngle")*GeomConstants::DegToRad;
	static float minThumbFingerDirOffset = GlobalConfig::tree()->get<float>("Leap.HandModel.MinThumbFingerDirectionOffset")*GeomConstants::DegToRad;
	static bool enableAutoLeftDetection = GlobalConfig::tree()->get<bool>("Leap.HandModel.EnableAutoLeftDetection");

	vector<pair<Finger,float> > fingerAngles;

	for (int i=0;i<hand.fingers().count();i++)
	{
		Finger f = hand.fingers()[i];
		fingerAngles.push_back(make_pair(f,LeapHelper::GetFingerTipAngle(hand,f)));
	}

	std::sort(fingerAngles.begin(),fingerAngles.end(),[isLeft](pair<Finger,float> & v0, pair<Finger,float> & v1) -> bool {
		if (isLeft)
			return v1.second < v0.second;
		else
			return v0.second < v1.second;
	});


	HandModel handModel;

	if (hand.fingers().count() < 5)
	{
		//Check for thumb-finger case

		auto f0 = fingerAngles.at(0);
		auto f1 = fingerAngles.at(1);

		float angleSpacing = abs(f0.second - f1.second);
		
		handModel.IntentFinger = f0.first.id(); //Assume thumb is not visible
		if (angleSpacing > minThumbFingerAngle )
		{
			handModel.ThumbId = f0.first.id();
			handModel.IntentFinger = f1.first.id();
		}
		else if (f0.first.direction().angleTo(f1.first.direction()) > minThumbFingerDirOffset)
		{			
			handModel.ThumbId = f0.first.id();
			handModel.IntentFinger = f1.first.id();
		}


		if (handModel.ThumbId >= 0 && hand.fingers().count() == 2)
		{
			if (handModel.IntentFinger != hand.fingers().frontmost().id())
			{
				handModel.ThumbId = f1.first.id();
				handModel.IntentFinger = f0.first.id();
			}
		}

		//else
		//{
		//	float interFingerSpacing = f0.first.stabilizedTipPosition().distanceTo(f1.first.stabilizedTipPosition());
		//	if (interFingerSpacing > minThumbFingerSpacing)
		//	{
		//		handModel.ThumbId = f0.first.id();
		//		handModel.IntentFinger = f1.first.id();
		//	}
		//}
	}	
	else
	{				
		if (!isLeft && enableAutoLeftDetection) //Might be a sole left hand
		{
			auto f3 = fingerAngles.at(3);
			auto f4 = fingerAngles.at(4); //Left thumb
			
			float angleSpacing = abs(f3.second - f4.second);
		
			if (angleSpacing > minThumbFingerAngle)
			{
				handModel.ThumbId = f4.first.id();
				handModel.IntentFinger = f3.first.id();
			}
			else
			{
				handModel.ThumbId = fingerAngles.at(0).first.id();
				handModel.IntentFinger = fingerAngles.at(1).first.id();
			}
		}
		else
		{
			handModel.ThumbId = fingerAngles.at(0).first.id();
			handModel.IntentFinger = fingerAngles.at(1).first.id();
		}
	}
	return handModel;
}

HandModel HandProcessor::buildModel(Hand hand, bool invert)
{
	HandModel handModel;

	if (hand.pointables().count() == 0)
	{
		handModel = HandModel(hand.id());
	}
	else if (hand.tools().count() > 0)
	{
		Pointable intent = hand.tools().frontmost();
		handModel = HandModel(hand.id(),intent.id());
	}
	else if (hand.fingers().count() == 1)
	{
		Pointable intent = hand.fingers()[0];	
		handModel = HandModel(hand.id(),intent.id());
	}
	else
	{
		handModel = buildComplexModel(hand,invert);
	}
	handModel.HandId = hand.id();
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

void HandProcessor::draw()
{
	Vector handCenter = Vector(300,300,100);
	glBindTexture(GL_TEXTURE_2D,NULL);
	glLineWidth(2);
	for (int i=0;i< drawHand.fingers().count();i++)
	{
		Finger f = drawHand.fingers()[i];
		
		float angle = LeapHelper::GetFingerTipAngle(drawHand,f);
		Vector drawFinger = Vector(200 * cos(angle), 200 * sin(angle), 0) + handCenter;

		if (lastModel()->IntentFinger == f.id())
			glColor4fv(Colors::Lime.getFloat());
		else 
			glColor4fv(Colors::OrangeRed.getFloat());

		glBegin(GL_LINES);
			glVertex3f(handCenter.x, handCenter.y, handCenter.z);
			glVertex3f(drawFinger.x, drawFinger.y, drawFinger.z);
		glEnd();
	}
}


		//vector<int> * fingerIndexes = LeapHelper::SortFingers(hand);

		//int thumbPosition, indexFingerPosition;
		//int size = fingerIndexes->size();

		//if (size == 5)
		//{
		//	if (invert)
		//	{
		//		thumbPosition = size - 1;
		//		indexFingerPosition = size - 2;
		//	}
		//	else
		//	{
		//		thumbPosition = 0;
		//		indexFingerPosition = 1;		
		//	}
		//}
		//else
		//{		
		//	bool thumbVisible = hasThumb(hand, fingerIndexes, invert);
		//	if (invert)
		//	{
		//		thumbPosition = (thumbVisible) ? (size-1) : -1;				
		//		indexFingerPosition = (thumbVisible) ? (size-2) : (size-1);
		//	}
		//	else
		//	{
		//		thumbPosition = (thumbVisible) ? 0 : -1;				
		//		indexFingerPosition = (thumbPosition == 0) ? 1 : 0; // selectIndexFinger(hand, fingerIndexes, thumbPosition);
		//	}
		//}

		//int * fingerArray = new int[5];

		//for (int i=0;i<5;i++)
		//{
		//	fingerArray[i] = -1;
		//}

		//if (thumbPosition >= 0)
		//{
		//	fingerArray[0] = fingerIndexes->at(thumbPosition);
		//}

		//if (indexFingerPosition >= 0)
		//{
		//	fingerArray[1] = fingerIndexes->at(indexFingerPosition);

		//	for (int i = indexFingerPosition + 1, j = 2; i < fingerIndexes->size() && j < 5; i++, j++)
		//	{
		//		int id = fingerIndexes->at(i);
		//		fingerArray[j] = id;
		//	}
		//}