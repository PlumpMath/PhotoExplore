#include "CircularAction.hpp"
#include "GlobalConfig.hpp"
#include "LeapHelper.h"
#include "HandModel.h"
#include "DrawingUtil.hpp"


CircularAction::CircularAction()
{
	state = Idle;
	config = GlobalConfig::tree()->get_child("Leap.CircularAction");
	handId = -1;
	trackedFingerId = -1;
	trackedOffset = 0;
	drawPitch = 0;
	numGrasped = 0;
	
	knobHomePosition = Vector(100,100,50);

	circleColor = config.get_child("Color");

	for (int i=0; i < 5; i++)
	{
		cursors.push_back(new PointableTouchCursor(20,40,Colors::PrettyPurple));
	}

	flyWheel = new FlyWheel();
	
	minAngle = config.get<float>("MinAngle")/Leap::RAD_TO_DEG;
	maxAngle = config.get<float>("MaxAngle")/Leap::RAD_TO_DEG;
	
	flyWheel->setMaxValue(maxAngle*1000.0);
	flyWheel->setMinValue(minAngle*1000.0);
	flyWheel->setTargetActive(false);
	flyWheel->setTargetPosition(0);
}


void CircularAction::draw()
{
	//if (mutey.try_lock())
	//{

	mutey.lock();
	
	static float offset = -Leap::PI;
	static float lineWidth = 2;
	
	float angularWidth = (2.0f*Leap::PI) - (maxAngle-minAngle);
	float startAngle = offset + (Leap::PI-angularWidth)*0.5f;
	float endAngle = startAngle + angularWidth;
	
	float innerRad =config.get<float>("InnerDrawRadius");
	float outerRad =config.get<float>("OuterDrawRadius");
	
	float knobAngle = 0;
	float nonGraspedRatio = 0;
	
	vector<pair<float,float> > drawData;
	
	if (handId >= 0)
	{		
		nonGraspedRatio =  grasped*(5.0f - (float)numGrasped)/5.0f;
		
		for (auto it = cursors.begin(); it != cursors.end(); it++)
		{
			(*it)->draw();
		}
	
		for (auto it = orderedFingers.begin(); it != orderedFingers.end(); it++)
		{
			if (fingerErrorMap.count(it->first.id()))
			{
				float errorVal = fingerErrorMap[it->first.id()];
				drawData.push_back(make_pair(0.0f,errorVal));
			}
		}

		knobAngle = flyWheel->getPosition()/1000.0;
		
		knobAngle = max<float>(knobAngle,minAngle);
		knobAngle = min<float>(knobAngle,maxAngle);
				
		float range = (maxAngle - minAngle);
		float errorRange = range * 0.1f;
		float minErrorAlpha = 0, maxErrorAlpha = 0;
		
		if (maxAngle - knobAngle < errorRange)
		{
			maxErrorAlpha = 1.0f - ((maxAngle - knobAngle)/errorRange);
		}
		
		if (knobAngle - minAngle < errorRange)
		{
			minErrorAlpha = 1.0f - ((knobAngle - minAngle)/errorRange);
		}
	}	
	
	float fillAlpha = (grasped) ? 0.9f : 0.1f + (numGrasped*0.1f);
	
	
	Color drawWheelColor = circleColor.blendRGB(Colors::Red,nonGraspedRatio);
	
	DrawingUtil::drawCircleFill(drawPoint,circleColor.withAlpha(fillAlpha),innerRad,outerRad,startAngle+knobAngle,endAngle+knobAngle);
	DrawingUtil::drawCircleFill(drawPoint+Vector(0,0,-1.0f),circleColor.withAlpha(0.1f),innerRad,outerRad,startAngle+minAngle,endAngle+maxAngle);
	DrawingUtil::drawCircleLine(drawPoint,circleColor,lineWidth,outerRad,0,Leap::PI*2.0f);
	DrawingUtil::drawCircleLine(drawPoint,drawWheelColor,lineWidth+(6.0f*nonGraspedRatio),innerRad,0,Leap::PI*2.0f);

	if (!grasped && !drawData.empty())
		drawFingerOffsets(drawPoint+Vector(0,0,0.5f),circleColor.withAlpha(0.6f),outerRad,drawData);
	
	mutey.unlock();
}



void CircularAction::drawFingerOffsets(Vector center, Color lineColor,float radius, vector<pair<float, float> > & offsets)
{
	float upperBound = (grasped) ? config.get<float>("Grasped.MaxRadialError") : config.get<float>("Open.MaxRadialError");
	float lowerBound = (grasped) ? config.get<float>("Grasped.MinRadialError") : config.get<float>("Open.MinRadialError");
	float maxTouchDist = (grasped) ? config.get<float>("Grasped.MaxTouchDist") : config.get<float>("Open.MaxTouchDist");
	
	float anglePerFinger = Leap::PI*0.2f;
	
	float margin = 0;
	float angle = Leap::PI + anglePerFinger*0.5f;
	for (auto it = offsets.begin(); it != offsets.end(); it++)
	{
		float errorVal = it->second;
		if (errorVal >= upperBound || errorVal <= lowerBound)
		{
			float fingerAlpha = 0.2f + abs(errorVal/100.0f);
			fingerAlpha = min<float>(1.0f,fingerAlpha);
			fingerAlpha = max<float>(0.2f,fingerAlpha);
			
			float length = radius + errorVal;
			float start = radius;
			Color color = lineColor.withAlpha(fingerAlpha);
			
			DrawingUtil::drawCircleFill(center,color,start,length,angle-Leap::PI*0.1f,angle+anglePerFinger);
		}
		
		angle += anglePerFinger;
	}
}

void CircularAction::setNewHand(Hand newHand)
{
	handId = newHand.id();
	state = Idle;
	sphereRadiusTimer.start();
	drawRadius = 0;
	drawPitch = 0;
	trackedFingerId = -1;
	trackedOffset = 0;
	grasped = false;

}

float CircularAction::getValue()
{
	return flyWheel->getCurrentPosition()/1000.0f;
}

bool CircularAction::isGrasped()
{
	return grasped;
}

float CircularAction::getHandFingerPitch(const Controller & controller, Finger f)
{
	Vector delta = getHandFingerDelta(controller,f);
	return atan2(delta.x,delta.y);
}

Vector CircularAction::getHandFingerDelta(const Controller & controller, Finger f)
{
	Vector p1 = LeapHelper::FindNormalizedScreenPoint(controller,f.stabilizedTipPosition());
	Vector p2 = LeapHelper::FindNormalizedScreenPoint(controller,f.hand().stabilizedPalmPosition());
	Vector delta = p1 - p2;
	return delta;
}

void CircularAction::updateCursors(const Controller & controller)
{	
	Frame frame = controller.frame();

	Hand hand = frame.hand(handId);
	
	if (hand.isValid())
	{
		for (int f = 0; f < hand.fingers().count() && f < 5; f++)
		{
			Finger finger = hand.fingers()[f];
			int fingerId = finger.id();
			if (fingerId == trackedFingerId)
				cursors.at(f)->setColor(Colors::Red);
			else
				cursors.at(f)->setColor(Colors::PrettyPurple);
				
			cursors.at(f)->setPointable(finger);
		}
	}

	for (auto it = cursors.begin(); it != cursors.end(); it++)
	{
		(*it)->onFrame(controller);
	}
}


void CircularAction::updateErrorMap(const Controller & controller, Hand hand)
{	
	Frame frame = controller.frame();
	
	fingerErrorMap.clear();
	
	bool usePalmPosition = config.get<bool>("UsePalmCenter");
	
	Vector centroid = (usePalmPosition) ? hand.stabilizedPalmPosition() : hand.sphereCenter();
	
	
	//centroid.z = 0;
	averageRadius = config.get<float>("TargetSphereRadius"); 

	float upperBound = (grasped) ? config.get<float>("Grasped.MaxRadialError") : config.get<float>("Open.MaxRadialError");
	float lowerBound = (grasped) ? config.get<float>("Grasped.MinRadialError") : config.get<float>("Open.MinRadialError");
	float maxTouchDist = (grasped) ? config.get<float>("Grasped.MaxTouchDist") : config.get<float>("Open.MaxTouchDist");

	int minFingerCount = (grasped) ? config.get<int>("Grasped.MinFingerCount") : config.get<int>("Open.MinFingerCount");
	int maxNonGraspedFingerCount = (grasped) ? config.get<int>("Grasped.MaxNonGraspedCount") : config.get<int>("Open.MaxNonGraspedCount");

	int nonGraspedCount = 0;
	numGrasped = 0;
	
	for (int f = 0; f < hand.fingers().count(); f++)
	{
		Finger finger = hand.fingers()[f];
		Vector p = finger.stabilizedTipPosition(); //LeapHelper::FindNormalizedScreenPoint(controller,finger.stabilizedTipPosition());
		//p.z = 0;
		
		float errorVal = p.distanceTo(centroid) - averageRadius;
		
		fingerErrorMap.insert(make_pair(finger.id(),errorVal));
		
		if (finger.touchDistance() < maxTouchDist && errorVal < upperBound && errorVal > lowerBound)
		{
			numGrasped++;
		}
		else
		{
			nonGraspedCount++;
		}
	}

	grasped = numGrasped >= minFingerCount && nonGraspedCount < maxNonGraspedFingerCount;
		
	if (grasped)
	{
		flyWheel->setTargetActive(false);
		flyWheel->overrideValue(drawPitch*1000.0);
	}
	else
	{
		flyWheel->setTargetActive(true);
		drawPitch = flyWheel->getCurrentPosition() / 1000.0f;
		flyWheel->setTargetPosition(0);
		rotationStartFrame = frame;
		trackedFingerId = -1;
	}
}

void CircularAction::updateRotation(const Controller & controller, Hand hand)
{
	Finger trackedFinger = hand.finger(trackedFingerId);
	
	if (trackedFinger.isValid())
	{
		drawPitch = getHandFingerPitch(controller, trackedFinger) - trackedOffset;		
		drawPitch = min<float>(maxAngle,drawPitch);
		drawPitch = max<float>(minAngle,drawPitch);
	}
	else
	{
		vector<Finger> fv;
		
		for (int f = 0;f<hand.fingers().count();f++)
		{
			fv.push_back(hand.fingers()[f]);
		}
		
		auto topMost = std::max_element(fv.begin(),fv.end(),[hand](const Finger & f0, const Finger & f1) -> bool {
			return f0.stabilizedTipPosition().y < f1.stabilizedTipPosition().y;
		});
		
		if (topMost != fv.end())
		{
			trackedFinger = *topMost;
			trackedFingerId = trackedFinger.id();
			trackedOffset = getHandFingerPitch(controller, trackedFinger) - drawPitch;
		}
	}
	
}

void CircularAction::updateRotateMap(const Controller & controller, Hand hand)
{	
	Frame frame = controller.frame();

	for (auto it = fingerAngleMap.begin(); it != fingerAngleMap.end(); it++)
	{
		Finger f = hand.finger(it->first);
		if (!f.isValid())
		{
			it = fingerAngleMap.erase(it);
			
			if (it == fingerAngleMap.end())
				break;
		}
	}
	
	auto oldest = std::max_element(fingerAngleMap.begin(),fingerAngleMap.end(),[hand](const pair<int,float> & p0, const pair<int,float> & p1) -> bool {		
		Finger f0 = hand.finger(p0.first);
		Finger f1 = hand.finger(p1.first);		
		return f0.stabilizedTipPosition().y < f1.stabilizedTipPosition().y;
	});

	if (oldest != fingerAngleMap.end())
	{
		int oldestId = oldest->first;
		Finger oldFinger = hand.finger(oldest->first);
		drawPitch = getHandFingerPitch(controller, oldFinger) - oldest->second;

		for (auto it = fingerAngleMap.begin(); it != fingerAngleMap.end(); it++)
		{
			Finger f = hand.finger(it->first);
			if (f.id() != oldestId)
			{
				it = fingerAngleMap.erase(it);
				if (it == fingerAngleMap.end())
					break;
			}
		}
	}
	
	drawPitch = min<float>(maxAngle,drawPitch);
	drawPitch = max<float>(minAngle,drawPitch);
		
	drawFingerAngles.clear();
	for (int f = 0; f < hand.fingers().count(); f++)
	{
		Finger finger = hand.fingers()[f];
				
		if (fingerAngleMap.count(finger.id()) == 0)
		{
			auto adjustedVal = make_pair(finger.id(),getHandFingerPitch(controller,finger) - drawPitch);
			fingerAngleMap.insert(adjustedVal);
		}
		auto val = make_pair(finger.id(),getHandFingerPitch(controller,finger));
		drawFingerAngles.insert(val);
	}
}


void CircularAction::onFrame(const Controller & controller)
{
	if (mutey.try_lock())
	{

		Frame frame = controller.frame();

		Hand hand = frame.hand(handId);
	
		if (!hand.isValid())
		{
			hand = frame.hands().rightmost();
			if (hand.isValid())
			{
				setNewHand(hand);	
			}
			else
				handId = -1;
		}
		
		
		if (hand.isValid())
		{	
			updateErrorMap(controller,hand);
			updateRotation(controller,hand);
			
			orderedFingers = LeapHelper::GetOrderedFingers(hand);
			
			float newDrawRadius = hand.sphereRadius() - config.get<float>("SphereRadiusThreshold");
			drawPoint = LeapHelper::FindScreenPoint(controller,hand.stabilizedPalmPosition()) + Vector(0,0,50);

			drawRadius = LeapHelper::lowpass(drawRadius,newDrawRadius,config.get<float>("SphereRadiusRC"),sphereRadiusTimer.seconds());

			drawRadius = max<float>(-config.get<float>("SphereRadiusMaxOffset"),drawRadius);
			drawRadius = min<float>(config.get<float>("SphereRadiusMaxOffset"),drawRadius);
			
			updateCursors(controller);

			switch (state)
			{
			case Idle:
				if (grasped) //hand.palmVelocity().magnitude() < config.get<float>("SteadyVelocity"))
				{
					state = Rotating;
					fingerAngleMap.clear();
					rotationStartFrame = frame;
				}
				else
				{					
					break;
				}
			case Rotating:
				if (!grasped)
				{
					//drawPitch = 0;
					fingerAngleMap.clear();
				}
				break;
			default:
				break;
			}
		}
		else
			state = Idle;

		mutey.unlock();
	}
}

