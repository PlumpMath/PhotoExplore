#include "PointableElementManager.h"
#include "LeapHelper.h"
#include "HandModel.h"
#include "LeapDebug.h"
#include "GLImport.h"
#include "ShakeGestureDetector.hpp"
#include "RadialMenu.hpp"

PointableElementManager::PointableElementManager()
{
	shakeGestureState = -1;
	pointingGestureState = 0;
	hitLastFrame = NULL;
	drawNonDominant = false;
}

void PointableElementManager::processInputEvents()
{
	return;
	int x,y;
	glfwGetMousePos(&x,&y);
	
	Pointable fakePointable;
	

	LeapElement * hit = NULL;

	Vector screenPoint = Vector(x,y,0);

	int flags = 0;
	View * topView = dynamic_cast<View*>(globalGestureListenerStack.top());
	hit = topView->elementAtPoint((int)screenPoint.x,(int)screenPoint.y,flags);
	
	if (hit != hitLastFrame)
	{
		if (hit != NULL)	
			hit->pointableEnter(fakePointable);
	
		if (hitLastFrame != NULL)
			hitLastFrame->pointableExit(fakePointable);
	}

	hitLastFrame = hit;

	int mouseState = glfwGetMouseButton(GLFW_MOUSE_BUTTON_1);

	if (mouseButtonState[GLFW_MOUSE_BUTTON_1] != mouseState)
	{
		mouseButtonState[GLFW_MOUSE_BUTTON_1] = (bool)mouseState;
		if (hit != NULL && hit->isClickable() && mouseState == GLFW_PRESS)
		{
			hit->elementClicked();
		}
	}


	if (glfwGetKey(GLFW_KEY_BACKSPACE) == GLFW_PRESS)
	{
		keyState[GLFW_KEY_BACKSPACE] = true;
	}
	else if (keyState[GLFW_KEY_BACKSPACE])
	{
		keyState[GLFW_KEY_BACKSPACE] = false;
		if (globalGestureListenerStack.size() > 0)
			globalGestureListenerStack.top()->onGlobalGesture(Leap::Controller(), "shake");
	}

	if (glfwGetKey(GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		keyState[GLFW_KEY_LEFT] = true;
	}
	else if (keyState[GLFW_KEY_LEFT])
	{
		keyState[GLFW_KEY_LEFT] = false;
		if (globalGestureListenerStack.size() > 0)
			globalGestureListenerStack.top()->onGlobalGesture(Leap::Controller(), "left_key");
	}
	else if (glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		keyState[GLFW_KEY_RIGHT] = true;
	}
	else if (keyState[GLFW_KEY_RIGHT])
	{
		keyState[GLFW_KEY_RIGHT] = false;
		if (globalGestureListenerStack.size() > 0)
			globalGestureListenerStack.top()->onGlobalGesture(Leap::Controller(), "right_key");
	}

	if (glfwGetKey(GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		keyState[GLFW_KEY_DOWN] = true;
	}
	else if (keyState[GLFW_KEY_DOWN])
	{
		keyState[GLFW_KEY_DOWN] = false;
		if (globalGestureListenerStack.size() > 0)
			globalGestureListenerStack.top()->onGlobalGesture(Leap::Controller(), "down_key");
	}
}

void PointableElementManager::enableNonDominantCursor(bool _enable)
{
	this->drawNonDominant = _enable;
}

static int determineHandState(const Controller & controller, Frame frame, Hand hand)
{		
	static float maxSpreadHandInterfingerAngle = GlobalConfig::tree()->get<float>("Leap.HandPoseControl.SpreadHand.MaxInterfingerAngle");
	
	HandModel * hm = HandProcessor::getInstance()->lastModel();
	int handState;

	if (!hand.isValid() || hand.pointables().count() == 1)
	{
		handState = InteractionState::Pointing;
	}
	else
	{
		if (hand.pointables().count() == 2 && hm->ThumbId > -1)
		{
			//float angle0 = LeapHelper::GetFingerTipAngle(hand,hand.fingers()[0]);
			//float angle1 = LeapHelper::GetFingerTipAngle(hand,hand.fingers()[1]);

			//if (abs(angle0 - angle1) < (GeomConstants::DegToRad*maxSpreadHandInterfingerAngle))
			//	handState = InteractionState::Spread;
			//else
				handState = InteractionState::Pointing;
		}
		else
		{
			handState = InteractionState::Spread;
		}
	}

	return handState;
}

void PointableElementManager::processFrame(const Controller & controller, Frame frame)
{	
	static float maxRadialClickVelocity = GlobalConfig::tree()->get<float>("Leap.TouchDistance.MaxRadialClickVelocity");
	static float maxLongitudalClickVelocity = GlobalConfig::tree()->get<float>("Leap.TouchDistance.MaxLongitudalClickVelocity");
	static float cursorDimension = (((float)GlobalConfig::ScreenWidth) / 2560.0f) *  GlobalConfig::tree()->get<float>("Overlay.BaseCursorSize");
	static float clickDistanceThreshold = GlobalConfig::tree()->get<float>("Leap.TouchDistance.ClickDistanceThreshold");
	static float minimumDrawDistance = GlobalConfig::tree()->get<float>("Leap.TouchDistance.MinimumDrawDistance");
	static bool drawIntentOnly = GlobalConfig::tree()->get<bool>("Leap.TouchDistance.DrawIntentOnly");
	
	bool clickOnRelease = GlobalConfig::tree()->get<bool>("Leap.TouchDistance.ClickOnRelease");
	
	if (frame.id() == lastFrameId || !frame.isValid())
		return;

	if (this->globalGestureListenerStack.size() > 0)
	{
		View * topView = dynamic_cast<View*>(globalGestureListenerStack.top());
		topView->onFrame(controller);
	}

	//if (this->globalGestureListenerStack.size() > 0)
	//{
	//	GestureList gestureList = frame.gestures(lastFrame);
	//	for (int i=0;i< gestureList.count(); i++)
	//	{
	//		Gesture g = gestureList[i];

	//		if (processedGestures.count(g.id()) == 0)
	//		{
	//			if (globalGestureListenerStack.top()->onLeapGesture(controller, g))
	//			{
	//				processedGestures.insert(g.id());
	//			}
	//		}
	//	}	
	//}

	if (processedGestures.size() > 0)
	{	
		for (auto it = processedGestures.begin(); it != processedGestures.end();it++)
		{
			Gesture g = frame.gesture(*it);
			if (!g.isValid())
			{
				it = processedGestures.erase(it);
				if (it == processedGestures.end())
					break;
			}
		}
	}

	HandModel * hm = HandProcessor::getInstance()->lastModel();

	Pointable testPointable = frame.pointable(hm->IntentFinger);
	Vector screenPoint;
	LeapElement * hit = NULL;

	Hand hand = controller.frame().hand(hm->HandId);

	currentState.ActiveHandId = hand.id();
	currentState.HandState = determineHandState(controller,frame,hand);

	int elementStateFlags = 0;
	if (testPointable.isValid() && globalGestureListenerStack.size() > 0)
	{				
		screenPoint = LeapHelper::FindScreenPoint(controller,testPointable);

		hit = RadialMenu::instance->elementAtPoint((int)screenPoint.x,(int)screenPoint.y,elementStateFlags);

		if (hit == NULL)
		{
			View * topView = dynamic_cast<View*>(globalGestureListenerStack.top());
			hit = topView->elementAtPoint((int)screenPoint.x,(int)screenPoint.y,elementStateFlags);
		}

		if (hit != hitLastFrame)
		{
			if (hitLastFrame != NULL)
				hitLastFrame->pointableExit(testPointable);

			if (hit != NULL)	
				hit->pointableEnter(testPointable);			
		}
		hitLastFrame = hit;
	}


	bool canPointableClick = false;
	if (currentState.HandState == InteractionState::Pointing)
		canPointableClick = true;
	else if (clickOnRelease)
	{
		Pointable clicking = frame.pointable(currentState.ClickingPointableId);
		if (clicking.isValid())
			canPointableClick = true;
	}

	bool clicked = false;

	if (hit != NULL )
	{		
		hit->onFrame(controller);
	
		if (elementStateFlags == 0 && canPointableClick && hit->isClickable())
		{
			float radialVelocity = testPointable.direction().cross(testPointable.tipVelocity()).magnitude();
			float longitudalVelocity = testPointable.direction().dot(testPointable.tipVelocity());

			if (radialVelocity < maxRadialClickVelocity && longitudalVelocity < maxLongitudalClickVelocity)
			{
				Frame f1, f0 = controller.frame(0), fN = lastFrame;
				Pointable p0,p1;

				int count = 0;
				for (int i = 1; i < 10; i++)
				{
					f0 = controller.frame(i-1);
					f1 = controller.frame(i);

					if (!f1.isValid())
						break;

					p0 = f0.pointable(testPointable.id());
					p1 = f1.pointable(testPointable.id());
				
					if (clickOnRelease)
					{
						if (p0.touchDistance() >= clickDistanceThreshold && p1.touchDistance() < clickDistanceThreshold)
						{
							clicked = true;
							hit->elementClicked();
							break;		
						}
					}
					else if (p1.touchDistance() >= clickDistanceThreshold && p0.touchDistance() < clickDistanceThreshold)
					{
						clicked = true;
						hit->elementClicked();
						break;					
					}

					if (f1.id() == lastFrame.id())
						break;
				}
			}
		}
	}

	if (clickOnRelease)
	{
		currentState.ClickingPointableId = -1;		
		if (!clicked && canPointableClick && testPointable.touchDistance() < clickDistanceThreshold)
		{
			currentState.ClickingPointableId = testPointable.id();
		}
	}

	static LeapDebugVisual * ldvIntent = NULL, * ldvNonDominant = NULL, * ldvNonDominantTD = NULL;
	static vector<LeapDebugVisual*> touchDistanceVisuals;


	if (ldvIntent == NULL)
	{
		ldvIntent =new LeapDebugVisual(screenPoint,1,LeapDebugVisual::LiveForever,cursorDimension,GlobalConfig::tree()->get_child("Leap.IntentControl.ScrollVisual.Color"));
		ldvIntent->depth=10;
		LeapDebug::instance->addDebugVisual(ldvIntent);

		ldvNonDominant = new LeapDebugVisual(screenPoint,1,LeapDebugVisual::LiveForever,0,GlobalConfig::tree()->get_child("Leap.IntentControl.NonDominant.FillColor"));
		ldvNonDominant->depth=10;
		ldvNonDominant->lineWidth =GlobalConfig::tree()->get<float>("Leap.IntentControl.NonDominant.LineWidth");
		ldvNonDominant->lineColor = Color(GlobalConfig::tree()->get_child("Leap.IntentControl.NonDominant.LineColor"));
		LeapDebug::instance->addDebugVisual(ldvNonDominant);
		
		ldvNonDominantTD = new LeapDebugVisual(screenPoint,1,LeapDebugVisual::LiveForever,0,GlobalConfig::tree()->get_child("Leap.IntentControl.NonDominantTD.FillColor"));
		ldvNonDominantTD->depth=10;
		ldvNonDominantTD->lineWidth =GlobalConfig::tree()->get<float>("Leap.IntentControl.NonDominantTD.LineWidth");
		ldvNonDominantTD->lineColor = Color(GlobalConfig::tree()->get_child("Leap.IntentControl.NonDominantTD.LineColor"));
		LeapDebug::instance->addDebugVisual(ldvNonDominantTD);
	}

	if (frame.hands().count() > 1 && drawNonDominant)
	{
		HandModel * hm2 = HandProcessor::LastModel(frame.hands().leftmost().id());
		Pointable nonDomPnt = frame.pointable(hm2->IntentFinger);

		if (nonDomPnt.isValid())
		{
			Vector ndPt = LeapHelper::FindScreenPoint(controller,nonDomPnt);
			ldvNonDominant->size = cursorDimension;
			ldvNonDominant->screenPoint.x = ndPt.x;
			ldvNonDominant->screenPoint.y = ndPt.y;
			
			ldvNonDominantTD->size =  cursorDimension*(1.0f + max<float>(minimumDrawDistance,nonDomPnt.touchDistance()));
			ldvNonDominantTD->screenPoint.x = ndPt.x;
			ldvNonDominantTD->screenPoint.y = ndPt.y;
						
			Color c = Color(GlobalConfig::tree()->get_child("Leap.IntentControl.NonDominantTD.FillColor"));
			float alphaMod = 0;
			if (nonDomPnt.touchDistance() <= minimumDrawDistance)
			{
				alphaMod = min<float>(1.0f,-2.0f * nonDomPnt.touchDistance());
			}
			c.setAlpha(c.colorArray[3] * alphaMod);
			ldvNonDominantTD->fillColor = c;
		}
	}
	else
	{
		ldvNonDominant->size = 0;
		ldvNonDominantTD->size = 0;
	}


	if (canPointableClick)
	{
		ldvIntent->fillColor = Color(GlobalConfig::tree()->get_child("Leap.IntentControl.SelectVisual.Color"));
		ldvIntent->lineColor = Color(GlobalConfig::tree()->get_child("Leap.IntentControl.SelectVisual.LineColor"));
	}
	else
	{		
		ldvIntent->fillColor = Color(GlobalConfig::tree()->get_child("Leap.IntentControl.ScrollVisual.Color"));
		ldvIntent->lineColor = Color(GlobalConfig::tree()->get_child("Leap.IntentControl.ScrollVisual.LineColor"));
	}
	
	ldvIntent->screenPoint.x =screenPoint.x;
	ldvIntent->screenPoint.y =screenPoint.y;
	


	for (int i=0;i<touchDistanceVisuals.size();i++)
		touchDistanceVisuals.at(i)->size = 0;

	//for (int i=0;i<hand.pointables().count();i++)
	//{
	//	Finger f = hand.fingers()[i];


	int i = 0;
	if (touchDistanceVisuals.size() <= i)
	{
		LeapDebugVisual * distanceVisual =new LeapDebugVisual(Vector(-100,-100,0),1,LeapDebugVisual::LiveForever,0,Color(GlobalConfig::tree()->get_child("Leap.TouchDistance.Visual.FillColor")));
		distanceVisual->lineColor = Color(GlobalConfig::tree()->get_child("Leap.TouchDistance.Visual.LineColor"));
		distanceVisual->lineWidth = GlobalConfig::tree()->get<float>("Leap.TouchDistance.Visual.LineWidth");
		distanceVisual->depth=11;
		LeapDebug::instance->addDebugVisual(distanceVisual);
		touchDistanceVisuals.push_back(distanceVisual);
	}

	//if (drawIntentOnly && f.id() != testPointable.id()) continue;

	float drawTD = min<float>(1.0f,testPointable.touchDistance());
	touchDistanceVisuals.at(i)->size = cursorDimension*(1.0f + max<float>(minimumDrawDistance,drawTD));

	touchDistanceVisuals.at(i)->screenPoint = LeapHelper::FindScreenPoint(controller,testPointable);

	if (canPointableClick)
	{				
		touchDistanceVisuals.at(i)->lineColor = Color(GlobalConfig::tree()->get_child("Leap.TouchDistance.ClickVisual.LineColor"));
		touchDistanceVisuals.at(i)->lineWidth = GlobalConfig::tree()->get<float>("Leap.TouchDistance.ClickVisual.LineWidth");

		float alphaMod = 0;
		Color c = Color(GlobalConfig::tree()->get_child("Leap.TouchDistance.ClickVisual.FillColor"));
		if (testPointable.touchDistance() <= minimumDrawDistance)
		{
			alphaMod = min<float>(1.0f,-2.0f * testPointable.touchDistance());
		}
		c.setAlpha(c.colorArray[3] * alphaMod);
		touchDistanceVisuals.at(i)->fillColor = c;
	}
	else
	{
		touchDistanceVisuals.at(i)->fillColor = Color(GlobalConfig::tree()->get_child("Leap.TouchDistance.Visual.FillColor"));
		touchDistanceVisuals.at(i)->lineColor = Color(GlobalConfig::tree()->get_child("Leap.TouchDistance.Visual.LineColor"));
		touchDistanceVisuals.at(i)->lineWidth = GlobalConfig::tree()->get<float>("Leap.TouchDistance.Visual.LineWidth");
	}
	//}



	handleGlobalGestures(controller);
	
	lastFrame = frame;
	lastFrameId = frame.id();

}


void PointableElementManager::handleGlobalGestures(const Controller & controller)
{
	int nigger = 0;
	if (globalGestureListenerStack.size() > 0)
	{
		if (!GlobalConfig::tree()->get<bool>("Shake.Disabled"))
		{
			ShakeGesture * latestShake = ShakeGestureDetector::getInstance().getLatestShakeGesture();

			if (latestShake != NULL)
			{
				globalGestureListenerStack.top()->onGlobalGesture(controller, "shake");
				ShakeGestureDetector::getInstance().resetDetector();
			}
		}

		HandModel * hm = HandProcessor::getInstance()->lastModel();
		Pointable intentPointable = controller.frame().pointable(hm->IntentFinger);
		if (intentPointable.isValid() && intentPointable.tipVelocity().magnitude() < GlobalConfig::tree()->get<float>("Leap.CustomGesture.Point.Trigger.MaxTipVelocity") 
			&& intentPointable.direction().angleTo(Vector::forward()) < GeomConstants::DegToRad*GlobalConfig::tree()->get<float>("Leap.CustomGesture.Point.Trigger.MaxAngleOffsetToZAxis")
			&& (!intentPointable.hand().isValid() || intentPointable.hand().pointables().count() == 1))
		{
			if (pointingGestureState == 0)
			{			
				pointingGestureState = 1;
				pointingGestureTimer.start();				
			}
			else if (pointingGestureState == 1)
			{
				if (intentPointable.tipVelocity().magnitude() < GlobalConfig::tree()->get<float>("Leap.CustomGesture.Point.Maintain.MaxTipVelocity") 
					&& intentPointable.direction().angleTo(Vector::forward()) < GeomConstants::DegToRad*GlobalConfig::tree()->get<float>("Leap.CustomGesture.Point.Maintain.MaxAngleOffsetToZAxis"))
				{
					if (pointingGestureTimer.millis() > GlobalConfig::tree()->get<float>("Leap.CustomGesture.Point.TimeToComplete"))
					{
						globalGestureListenerStack.top()->onGlobalGesture(controller, "pointing");
						pointingGestureState = 0;
					}			
				}
				else
				{
					pointingGestureState = 0;
				}
			}
		}
	}
}

void PointableElementManager::requestGlobalGestureFocus(ActivityView * globalListener)
{
	if (globalGestureListenerStack.empty() || globalGestureListenerStack.top() != globalListener)
	{
		if (globalGestureListenerStack.size() > 0)
		{
			globalGestureListenerStack.top()->onGlobalFocusChanged(false);
		}
		globalGestureListenerStack.push(globalListener);
		vector<string> tutorial;
		globalListener->getTutorialDescriptor(tutorial);
		LeapDebug::instance->setTutorialImages(tutorial);
		globalGestureListenerStack.top()->onGlobalFocusChanged(true);
	}
}

void PointableElementManager::releaseGlobalGestureFocus(ActivityView * globalListener)
{
	if (globalGestureListenerStack.size() > 0 && globalGestureListenerStack.top() == globalListener)
	{
		globalGestureListenerStack.top()->onGlobalFocusChanged(false);
		globalGestureListenerStack.pop();
		if (globalGestureListenerStack.size() > 0)
		{
			vector<string> tutorial;
			globalGestureListenerStack.top()->onGlobalFocusChanged(true);
			globalGestureListenerStack.top()->getTutorialDescriptor(tutorial);
			LeapDebug::instance->setTutorialImages(tutorial);
		}
	}
}