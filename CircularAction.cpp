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

//	for (int i=0; i < 5; i++)
//	{
//		cursors.push_back(new PointableTouchCursor(20,40,Colors::PrettyPurple));
//	}

	flyWheel = new FlyWheel();
	
	releaseCountdown.start();
	
	minAngle = config.get<float>("MinAngle")/Leap::RAD_TO_DEG;
	maxAngle = config.get<float>("MaxAngle")/Leap::RAD_TO_DEG;
	
	flyWheel->setMaxValue(maxAngle*1000.0);
	flyWheel->setMinValue(minAngle*1000.0);
	flyWheel->setTargetActive(false);
	flyWheel->setTargetPosition(0);
}


void CircularAction::draw()
{
	mutey.lock();
	
	static float offset = -Leap::PI;
	
	float lowerBound = config.get<float>("Open.MinRadialError");
	
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
		
	if (state != Hidden || (innerRadiusAnimation.isRunning() && outerRadiusAnimation.isRunning()))
	{		
		DrawingUtil::drawCircleFill(drawPoint,circleColor.withAlpha(0.65f),circleColor.withAlpha(0.85f),innerRad,outerRad-outerLineWidth,startAngle+knobAngle,endAngle+knobAngle);
		DrawingUtil::drawCircleFill(drawPoint,circleColor.withAlpha(0.85f),circleColor.withAlpha(0.0f),outerRad-outerLineWidth,outerRad+1,startAngle+knobAngle,endAngle+knobAngle);
		
		DrawingUtil::drawCircleFill(drawPoint+Vector(0,0,-1.0f),circleColor.withAlpha(0.1f),innerRad,outerRad,startAngle+minAngle,endAngle+maxAngle);
				
		if (!grasped)
			drawFingerOffsets(drawPoint+Vector(0,0,0.5f),circleColor.withAlpha(0.6f),outerRad-outerLineWidth,drawData);
		
		DrawingUtil::drawCircleLine(drawPoint+Vector(0,0,0.5f),circleInnerLineColor,innerLineWidth,innerRad,startAngle+minAngle,endAngle+maxAngle);
		DrawingUtil::drawCircleLine(drawPoint+Vector(0,0,1.5f),circleInnerLineColor2,innerLineWidth,innerRad+innerLineWidth*0.5f,0,Leap::PI*2.0f);
		
		if (grasped)
		{
			float ringStart = Leap::PI*0.5f, ringEnd = Leap::PI*2.5f;
			
			if (outerRingAnimation.isRunning())
			{
				ringStart = Leap::PI - outerRingAnimation.getValue();
				ringEnd = Leap::PI*2.0f + outerRingAnimation.getValue();			}
			
			DrawingUtil::drawCircleLine(drawPoint+Vector(0,0,1.5f),circleOuterLineColor,outerLineWidth,outerRad,ringStart,ringEnd);
		}	
	}
	
	mutey.unlock();
}



void CircularAction::drawFingerOffsets(Vector center, Color lineColor,float radius, vector<pair<int,float> > & offsets)
{
	float anglePerFinger = Leap::PI*0.2f;
	float offset = Leap::PI;
	float targetRad = config.get<float>("TargetSphereRadius");
		
	int fingerCount = 0;
	for (auto it = orderedFingers.begin(); it != orderedFingers.end(); it++)
	{
		if (fingerErrorMap.count(it->first.id()))
		{
			int fingerIndex = fingerCount;
			float errorVal = fingerErrorMap[it->first.id()];
				
			float angle = offset + anglePerFinger*((float)fingerCount);
			
			float fingerAlpha = abs(errorVal/120.0f);
			fingerAlpha = min<float>(0.8f,fingerAlpha);
			fingerAlpha = max<float>(0.2f,fingerAlpha);
			
			float length = max<float>(radius,radius + errorVal);
			float start = radius;
			Color color = lineColor.withAlpha(fingerAlpha);
			
			float lineWidth = outerLineWidth;
			
			if (errorVal < 0)
				lineWidth += errorVal*-0.2f;
						
			Vector tipPoint = LeapHelper::GetXYCoords(it->first.stabilizedTipPosition() - knobCenterPoint,knobPlaneNormal);
			
			tipPoint *= (radius/targetRad);
			tipPoint += center;
			DrawingUtil::drawCircleLine(tipPoint+Vector(0,0,1.5f),Colors::Red,3,30,0,Leap::PI*2.0f);
			
			DrawingUtil::drawCircleFill(center,color,start,length,angle,angle+anglePerFinger);
			DrawingUtil::drawCircleLine(center+Vector(0,0,0.5f),circleOuterLineColor,lineWidth,length,angle,angle+anglePerFinger);
		}
		
		fingerCount++;
	}
}

void CircularAction::setNewHand(Hand newHand)
{
	handId = newHand.id();
	drawPitch = 0;
	trackedFingerId = -1;
	trackedOffset = 0;
	grasped = false;
	flyWheel->overrideValue(0);
}

float CircularAction::getValue()
{
	return flyWheel->getCurrentPosition()/1000.0f;
}

bool CircularAction::isGrasped()
{
	return state == Rotating || (state == PendingHide && grasped);
}

float CircularAction::getHandFingerPitch(const Controller & controller, Finger f)
{
//	Vector delta = getHandFingerDelta(controller,f);
//	return atan2(delta.x,delta.y);
	Vector _knobCenterPoint = f.hand().stabilizedPalmPosition();
	//Vector _knobPlaneNormal = Vector(0,0,-1);
	return LeapHelper::GetAngleOnPlane(f.stabilizedTipPosition() - _knobCenterPoint,Vector(0,1,1),knobPlaneNormal,Vector(0,0,0));
}

Vector CircularAction::getHandFingerDelta(const Controller & controller, Finger f)
{
	Vector p1 = LeapHelper::FindNormalizedScreenPoint(controller,f.stabilizedTipPosition());
	Vector p2 = LeapHelper::FindNormalizedScreenPoint(controller,f.hand().stabilizedPalmPosition());
	Vector delta = p1 - p2;
	return delta;
}

void CircularAction::updateCursors(const Controller & controller)
{}


void CircularAction::updateErrorMap(const Controller & controller, Hand hand)
{	
	Frame frame = controller.frame();
	
	fingerErrorMap.clear();
	
	//bool usePalmPosition = config.get<bool>("UsePalmCenter");
	
	//Vector centroid = (usePalmPosition) ? hand.stabilizedPalmPosition() : hand.sphereCenter();
	
	Vector centroid = knobCenterPoint;
	
	
	//centroid.z = 0;
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
		newGraspState = numGrasped >= minFingerCount && nonGraspedCount < maxNonGraspedFingerCount;
	}
	else
		newGraspState = false;
	
	if (grasped != newGraspState)
	{
		grasped = newGraspState;
		
		outerRingAnimation = DoubleAnimation(0,Leap::PI/2.0,config.get<int>("OuterRingGraspAnimDuration"),NULL,false,false);
		outerRingAnimation.start();
		
		knobPlaneNormal = hand.palmNormal();
		knobCenterPoint = hand.stabilizedPalmPosition();
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
		drawPitch = boundAngle(getHandFingerPitch(controller, trackedFinger) - trackedOffset);
		
		drawPitch = min<float>(maxAngle,drawPitch);
		drawPitch = max<float>(minAngle,drawPitch);
		
//		Logger::stream("CircularAction","INFO") << "Angle = " << getHandFingerPitch(controller, trackedFinger) * Leap::RAD_TO_DEG
//		<< " DP = " << drawPitch*Leap::RAD_TO_DEG
//		<< " pID = " << trackedFinger.id() << endl;
	}
	else
	{
		if (intentFinger.isValid())
			trackedFingerId = intentFinger.id();
		else
			trackedFingerId = hand.fingers().leftmost().id();
		
		if (trackedFingerId >= 0)
		{		
			Finger newTrackedPrev = controller.frame(1).finger(trackedFingerId);
			Finger newTracked = controller.frame().finger(trackedFingerId);
							
			if (!newTrackedPrev.isValid())
				newTrackedPrev = newTracked;
				
			trackedOffset = getHandFingerPitch(controller, newTrackedPrev) - drawPitch;
			trackedOffset = boundAngle(trackedOffset);
			
//			Logger::stream("CircularAction","INFO") << "Transitioning pointable: DP = " << drawPitch*Leap::RAD_TO_DEG
//			<< " TrackedOffset=" << trackedOffset*Leap::RAD_TO_DEG
//			<< " OldAngle = " << getHandFingerPitch(controller, newTrackedPrev)*Leap::RAD_TO_DEG
//			<< " NewAngle=" << getHandFingerPitch(controller, newTracked)*Leap::RAD_TO_DEG
//			<< " pID = " << newTracked.id() << endl;
			
			drawPitch = getHandFingerPitch(controller, newTracked) - trackedOffset;
//			Logger::stream("CircularAction","INFO") << "Transitioning pointable: NewDP = " << drawPitch*Leap::RAD_TO_DEG << endl;
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

static bool checkClicked(const Controller & controller, Pointable p)
{
	Pointable pThis = controller.frame().pointable(p.id());
	Pointable pLast = controller.frame(1).pointable(p.id());
	
	if (pLast.isValid())
	{
		if (pThis.touchDistance() > 0 && pLast.touchDistance() <= 0)
		{
			return true;
		}
	}
	return false;
}

static bool checkClicked(const Controller & controller, int pId)
{
	Pointable pThis = controller.frame().pointable(pId);
	Pointable pLast = controller.frame(1).pointable(pId);
	
	if (pThis.isValid() && pLast.isValid())
	{
		if (pThis.touchDistance() > 0 && pLast.touchDistance() <= 0)
		{
			return true;
		}
	}
	return false;
}

static Vector constrainVector(Vector origin, Vector point, float maxDistance)
{
	if (origin.distanceTo(point) > maxDistance)
	{
		return origin + (point.normalized() * maxDistance);
	}
	else
		return point;
}

bool CircularAction::checkShowGesture(const Controller & controller, Hand hand)
{
	if (config.get<bool>("Show.CircleGesture.Enabled"))
	{
		GestureList gList = controller.frame().gestures();
		for (int g = 0; g < gList.count(); g++)
		{
			Gesture gesture = gList[g];
			
			if (gesture.type() == Gesture::TYPE_CIRCLE && gesture.state() != Gesture::STATE_INVALID)
			{
				CircleGesture cg = CircleGesture(gesture);
				
				if (cg.progress() > config.get<float>("Show.CircleGesture.MinProgress") &&
					cg.radius() > config.get<float>("Show.CircleGesture.MinRadius") &&
					cg.radius() < config.get<float>("Show.CircleGesture.MaxRadius") &&
					cg.normal().angleTo(Vector(0,0,-1)) < config.get<float>("Show.CircleGesture.MaxZAngleOffset")/Leap::RAD_TO_DEG
				)
				{
					knobCenterPoint = cg.center();
					knobPlaneNormal = cg.normal();
					
					knobHomePosition = constrainVector(Vector(GlobalConfig::ScreenWidth/2.0f,GlobalConfig::ScreenHeight/2.0f,0),
													   LeapHelper::FindScreenPoint(controller,cg.center()),800) + Vector(0,0,50);
					return true;
				}
			}
		}
	}
	
	if (config.get<bool>("Show.MultiTap.Enabled"))
	{	
		Hand lastHand = controller.frame(1).hand(hand.id());
		int tapCount = 0;
		for (int f = 0; f < hand.fingers().count(); f++)
		{
			if (checkClicked(controller,hand.fingers()[f]))
				tapCount++;
		}
		
		if (tapCount >= config.get<int>("Show.MultiTap.MinTapCount") &&
				hand.fingers().count() >= config.get<int>("Show.MultiTap.MinFingerCount"))
		{			
			knobHomePosition = constrainVector(Vector(GlobalConfig::ScreenWidth/2.0f,GlobalConfig::ScreenHeight/2.0f,0),
											   LeapHelper::FindScreenPoint(controller,hand.stabilizedPalmPosition()),800) + Vector(0,0,50);
			return true;
		}
	}	
	return false;
}

void CircularAction::setState(int newState)
{
	static float startRad = config.get<float>("InnerDrawRadius") + (config.get<float>("OuterDrawRadius") - config.get<float>("InnerDrawRadius"))*0.5f;
	
	if (state != newState)
	{
		switch (newState)
		{
			case Idle:
				releaseCountdown.start();
				if (state == Hidden)
				{				
					innerRadiusAnimation = DoubleAnimation(0,config.get<float>("InnerDrawRadius"),config.get<int>("Inner.ShowAnimDuration"),NULL,false,false);
					innerRadiusAnimation.start();
					
					outerRadiusAnimation = DoubleAnimation(0,config.get<float>("OuterDrawRadius"),config.get<int>("ShowAnimDuration"),NULL,false,false);
					outerRadiusAnimation.start();
				}
				else if (state == Rotating)
				{
					releaseCountdown.countdown(config.get<int>("PointingReleaseTimeout"));
				}
				grasped = false;
				break;
			case Hidden:				
				innerRadiusAnimation = DoubleAnimation(config.get<float>("InnerDrawRadius"),startRad,config.get<int>("HideAnimDuration"),NULL,false,false);
				innerRadiusAnimation.start();
				
				outerRadiusAnimation = DoubleAnimation(config.get<float>("OuterDrawRadius"),startRad,config.get<int>("HideAnimDuration"),NULL,false,true);
				outerRadiusAnimation.start();
				break;
			case PendingHide:
				hideCountdown.countdown(config.get<int>("HideTimeout"));
				break;
			case PendingRelease:
				releaseCountdown.countdown(config.get<int>("PointingReleaseTimeout"));
				break;
			default:
				break;			
		}

		state = newState;
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

			if (grasped)
				updateRotation(controller,hand);
			
			orderedFingers = LeapHelper::GetOrderedFingers(hand);
			
		}
		else
		{
			if (state != Hidden)
				setState(PendingHide);
			
			flyWheel->setTargetActive(true);
			drawPitch = flyWheel->getCurrentPosition() / 1000.0f;
			flyWheel->setTargetPosition(0);
		}
		
		
		
		HandModel * hm = HandProcessor::LastModel(handId);
		
		switch (state)
		{
		case Hidden:
			if (hand.isValid() && checkShowGesture(controller, hand))
			{					
				setState(Idle);
			}
			break;
		case Idle:
			if (releaseCountdown.elapsed())
			{
				setState(Hidden);
			}
			else if (hm->Pose == HandModel::Pointing && checkClicked(controller,hm->IntentFinger))
			{
				setState(PendingHide);
			}
			else if (grasped)
			{
				setState(Rotating);
			}
			break;
		case PendingRelease:
			if (!grasped || releaseCountdown.elapsed())
			{
				setState(Idle);
			}
			else if (grasped && hm->Pose != HandModel::Pointing)
			{
				setState(Rotating);
			}
			break;
		case Rotating:
			if (!grasped)
				setState(Idle);
			else if (hm->Pose == HandModel::Pointing && checkClicked(controller,hm->IntentFinger))
			{
				setState(PendingHide);
			}
//			else if (hm->Pose == HandModel::Pointing)
//			{
//				//setState(PendingRelease);
//			}
			break;
		case PendingHide:
			if (hideCountdown.elapsed())
				setState(Hidden);
			else if (grasped)
				setState(Rotating);
			else if (hand.isValid())
				setState(Idle);
			break;
		default:
			break;
		}
		
		
		drawPoint = knobHomePosition;
				
		mutey.unlock();
	}
}

