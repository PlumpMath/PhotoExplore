#include "CircularAction.hpp"
#include "GlobalConfig.hpp"
#include "LeapHelper.h"
#include "HandModel.h"
#include "DrawingUtil.hpp"


CircularAction::CircularAction()
{
	state = Hidden;
	config = GlobalConfig::tree()->get_child("Leap.CircularAction");
	handId = -1;
	trackedFingerId = -1;
	trackedOffset = 0;
	drawPitch = 0;
	numGrasped = 0;
	
	knobHomePosition = Vector(GlobalConfig::ScreenWidth-160,GlobalConfig::ScreenHeight-150,50);

	circleColor = config.get_child("Color");
	circleInnerLineColor = config.get_child("InnerLineColor");
	circleOuterLineColor = config.get_child("OuterLineColor");
	
	innerLineWidth = config.get<float>("InnerLineWidth");
	outerLineWidth = config.get<float>("OuterLineWidth");

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
	
	float angularWidth = (config.get<float>("KnobAngleRange")/Leap::RAD_TO_DEG) - (maxAngle-minAngle);
	float startAngle = offset + (Leap::PI-angularWidth)*0.5f;
	float endAngle = startAngle + angularWidth;
	
	float innerRad = (innerRadiusAnimation.isRunning()) ? innerRadiusAnimation.getValue() : config.get<float>("InnerDrawRadius");
	float outerRad = (outerRadiusAnimation.isRunning()) ? outerRadiusAnimation.getValue() : config.get<float>("OuterDrawRadius");
	
	if (innerRad > outerRad)
		outerRad = innerRad;
	
	Color circleOuterLineColor2 = Color(config.get_child("OuterLineColor2"));
	Color circleInnerLineColor2 = Color(config.get_child("InnerLineColor2"));
	
	float knobAngle = 0;
	float nonGraspedRatio = 0;
	
	vector<pair<int,float> > drawData;
	
	if (handId >= 0)
	{		
		nonGraspedRatio =  grasped*(5.0f - (float)numGrasped)/5.0f;
				
		HandModel * hm = HandProcessor::LastModel(handId);
	
		int fingerCount = 0;
		for (auto it = orderedFingers.begin(); it != orderedFingers.end(); it++)
		{
			if (fingerErrorMap.count(it->first.id()))
			{
				int fingerIndex = fingerCount;
				float errorVal = fingerErrorMap[it->first.id()];
				drawData.push_back(make_pair(fingerIndex,errorVal));
			}
			fingerCount++;
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
		
	if (state != Hidden)
	{		
		DrawingUtil::drawCircleFill(drawPoint,circleColor,innerRad,outerRad,startAngle+knobAngle,endAngle+knobAngle);
		DrawingUtil::drawCircleFill(drawPoint+Vector(0,0,-1.0f),circleColor.withAlpha(0.1f),innerRad,outerRad,startAngle+minAngle,endAngle+maxAngle);
				
		if (!grasped && !drawData.empty())
			drawFingerOffsets(drawPoint,circleColor.withAlpha(0.6f),outerRad,drawData);
		
		DrawingUtil::drawCircleLine(drawPoint+Vector(0,0,0.5f),circleInnerLineColor,innerLineWidth,innerRad,startAngle+minAngle,endAngle+maxAngle);
		DrawingUtil::drawCircleLine(drawPoint+Vector(0,0,1.5f),circleInnerLineColor2,innerLineWidth,innerRad+innerLineWidth*0.5f,0,Leap::PI*2.0f);
		
		if (grasped)
		{
			float ringStart = Leap::PI*0.5f, ringEnd = Leap::PI*2.5f;
			
			if (outerRingAnimation.isRunning())
			{
				ringStart = Leap::PI - outerRingAnimation.getValue();
				ringEnd = Leap::PI*2.0f + outerRingAnimation.getValue();
				DrawingUtil::drawCircleLine(drawPoint+Vector(0,0,0.5f),circleOuterLineColor,2,outerRad,0,Leap::PI*2.0f);
			}
			
			DrawingUtil::drawCircleLine(drawPoint+Vector(0,0,0.5f),circleOuterLineColor,outerLineWidth,outerRad,ringStart,ringEnd);
		}
		else
		{
			DrawingUtil::drawCircleLine(drawPoint+Vector(0,0,0.5f),circleOuterLineColor,2,outerRad,0,Leap::PI*2.0f);
		}		
	}
	
	mutey.unlock();
}



void CircularAction::drawFingerOffsets(Vector center, Color lineColor,float radius, vector<pair<int,float> > & offsets)
{
	float anglePerFinger = Leap::PI*0.2f;
	float offset = Leap::PI;
	
	for (auto it = offsets.begin(); it != offsets.end(); it++)
	{
		float errorVal = it->second;
		float angle = offset + anglePerFinger*((float)it->first);
		
		float fingerAlpha = abs(errorVal/120.0f);
		fingerAlpha = min<float>(0.8f,fingerAlpha);
		fingerAlpha = max<float>(0.2f,fingerAlpha);
		
		float length = radius + errorVal;
		float start = radius;
		Color color = lineColor.withAlpha(fingerAlpha);
		
		DrawingUtil::drawCircleFill(center,color,start,length,angle,angle+anglePerFinger);
		DrawingUtil::drawCircleLine(center+Vector(0,0,0.5f),circleOuterLineColor,outerLineWidth,length,angle,angle+anglePerFinger);
		
	}
}

void CircularAction::setNewHand(Hand newHand)
{
	handId = newHand.id();
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
//	Frame frame = controller.frame();
//
//	Hand hand = frame.hand(handId);
//	
//	if (hand.isValid())
//	{
//		for (int f = 0; f < hand.fingers().count() && f < 5; f++)
//		{
//			Finger finger = hand.fingers()[f];
//			int fingerId = finger.id();
//			if (fingerId == trackedFingerId)
//				cursors.at(f)->setColor(Colors::Red);
//			else
//				cursors.at(f)->setColor(Colors::PrettyPurple);
//				
//			cursors.at(f)->setPointable(finger);
//		}
//	}
//
//	for (auto it = cursors.begin(); it != cursors.end(); it++)
//	{
//		(*it)->onFrame(controller);
//	}
}


void CircularAction::updateErrorMap(const Controller & controller, Hand hand)
{	
	Frame frame = controller.frame();
	
	fingerErrorMap.clear();
	
	bool usePalmPosition = config.get<bool>("UsePalmCenter");
	
	Vector centroid = (usePalmPosition) ? hand.stabilizedPalmPosition() : hand.sphereCenter();
	
	
	centroid.z = 0;
	averageRadius = config.get<float>("TargetSphereRadius"); 

	boost::property_tree::ptree stateConfig = (grasped) ? config.get_child("Grasped") : config.get_child("Open");
	
	
	float upperBound = stateConfig.get<float>("MaxRadialError");
	float lowerBound = stateConfig.get<float>("MinRadialError");
	float maxTouchDist = stateConfig.get<float>("MaxTouchDist");
	int minFingerCount = stateConfig.get<int>("MinFingerCount");
	int maxNonGraspedFingerCount = stateConfig.get<int>("MaxNonGraspedCount");
	
	float maxHandSphere = stateConfig.get<float>("Hand.MaxSphereRadius");
		
	int nonGraspedCount = 0;
	numGrasped = 0;
	
	bool newGraspState;
	if (hand.sphereRadius() < maxHandSphere)
	{		
		for (int f = 0; f < hand.fingers().count(); f++)
		{
			Finger finger = hand.fingers()[f];
			Vector p = finger.stabilizedTipPosition();
			p.z = 0;
			
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
		newGraspState = numGrasped >= minFingerCount && nonGraspedCount < maxNonGraspedFingerCount;
	}
	else
		newGraspState = false;
	
	if (grasped != newGraspState)
	{
		grasped = newGraspState;
		
		outerRingAnimation = DoubleAnimation(0,Leap::PI/2.0,config.get<int>("OuterRingGraspAnimDuration"),NULL,false,false);
		outerRingAnimation.start();
	}
		
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

static Pointable getTopMost(Hand hand)
{
	Finger topMostFinger;
	
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
		topMostFinger = Finger(*topMost);
	}
	return topMostFinger;
}

static float boundAngle(float angle)
{
	if (angle < -Leap::PI)
		angle += Leap::PI * 2.0f;
	else if (angle > Leap::PI)
		angle -= Leap::PI * 2.0f;
	
	return angle;
}

void CircularAction::updateRotation(const Controller & controller, Hand hand)
{
	Finger intentFinger = hand.finger(HandProcessor::LastModel(hand.id())->IntentFinger);
	Finger trackedFinger = hand.finger(trackedFingerId);
	
	if ((trackedFinger.isValid() && !intentFinger.isValid()) ||
		(trackedFinger.isValid() && trackedFinger.id() == intentFinger.id()))
	{
		drawPitch = getHandFingerPitch(controller, trackedFinger);
		
		drawPitch = boundAngle(drawPitch) - trackedOffset;
		
		drawPitch = min<float>(maxAngle,drawPitch);
		drawPitch = max<float>(minAngle,drawPitch);
		
		//Logger::stream("CircularAction","INFO") << "Angle = " << getHandFingerPitch(controller, trackedFinger) * Leap::RAD_TO_DEG << " DP = " << drawPitch*Leap::RAD_TO_DEG << endl;
	}
	else
	{
		if (intentFinger.isValid())
			trackedFingerId = intentFinger.id();
		else
			trackedFingerId = hand.fingers().leftmost().id();
		
		if (trackedFingerId >= 0)
		{		
			Finger lastFrameIntent = controller.frame(1).finger(intentFinger.id());
					
			if (!lastFrameIntent.isValid())
				lastFrameIntent = intentFinger;
				
			trackedOffset = getHandFingerPitch(controller, lastFrameIntent) - drawPitch;
			trackedOffset = boundAngle(trackedOffset);
			
			//Logger::stream("CircularAction","INFO") << "Transitioning pointable: DP = " << drawPitch*Leap::RAD_TO_DEG << " TrackedOffset=" << trackedOffset*Leap::RAD_TO_DEG << " NewAngle=" << getHandFingerPitch(controller, intentFinger)*Leap::RAD_TO_DEG << endl;
			drawPitch = getHandFingerPitch(controller, intentFinger) - trackedOffset;
			//Logger::stream("CircularAction","INFO") << "Transitioning pointable: NewDP = " << drawPitch*Leap::RAD_TO_DEG << endl;
			drawPitch = min<float>(maxAngle,drawPitch);
			drawPitch = max<float>(minAngle,drawPitch);
		}
//		else
//		{
//			Logger::stream("CircularAction","ERROR") << "Invalid hand, has " << hand.pointables().count() << " pointables" << endl;
//		}
	}
}

float CircularAction::getAverageKnobDistance(const Controller & controller, vector<pair<Finger,float> > & orderedFingers, Vector knobPosition)
{
	float averageDistance = 0, count = 0;
	for (auto it = orderedFingers.begin(); it != orderedFingers.end(); it++)
	{
		Vector fingerPos = LeapHelper::FindScreenPoint(controller,it->first);
		
		averageDistance += fingerPos.distanceTo(knobPosition);
		count++;
	}
	averageDistance /= count;
	
	return averageDistance;
}

bool CircularAction::checkShowGesture(const Controller & controller, Hand hand)
{
	Hand lastHand = controller.frame(1).hand(hand.id());
	int tapCount = 0;
	for (int f = 0; f < hand.fingers().count(); f++)
	{
		Finger finger = hand.fingers()[f];
		Finger lastFinger = lastHand.finger(finger.id());
		
		if (lastFinger.isValid())
		{
			if (finger.touchDistance() > 0 && lastFinger.touchDistance() <= 0)
			{
				tapCount++;
			}
		}
	}
	
	return tapCount >= 1 && hand.fingers().count() >= 4;
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

			if (grasped)
				updateRotation(controller,hand);
			
			orderedFingers = LeapHelper::GetOrderedFingers(hand);
		
			switch (state)
			{
			case Hidden:
				if (checkShowGesture(controller, hand))
				{
					knobHomePosition = LeapHelper::FindScreenPoint(controller,hand.stabilizedPalmPosition()) + Vector(0,0,50);
					drawPoint = knobHomePosition;
					
					innerRadiusAnimation = DoubleAnimation(10,config.get<float>("InnerDrawRadius"),config.get<int>("Inner.ShowAnimDuration"),NULL,false,true);
					innerRadiusAnimation.start();
					
					float startRad = 0;
					
					outerRadiusAnimation = DoubleAnimation(startRad,config.get<float>("OuterDrawRadius"),config.get<int>("ShowAnimDuration"),NULL,false,true);
					outerRadiusAnimation.start();
					
					state = Idle;
				}
				break;
			case Idle:
				if (grasped)
				{
					state = Rotating;
				}
				else
				{
					drawPoint = knobHomePosition;
					break;
				}
			case Rotating:
				if (grasped)
					drawPoint =  knobHomePosition;
				else
					state = Idle;
				break;
			default:
				break;
			}
		}
		else
			state = Hidden;
		
		mutey.unlock();
	}
}

