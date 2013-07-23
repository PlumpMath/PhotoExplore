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
	hoverClickEnabled = true;
}


void PointableElementManager::registerElement(LeapElement * element)
{
	if (std::find(testElements.begin(),testElements.end(),element) == testElements.end())
		this->testElements.push_back(element);
}

void PointableElementManager::unregisterElement(LeapElement * element)
{
	this->testElements.remove(element);
}

LeapElement * PointableElementManager::findElementAt(Vector screenPoint)
{
	int flags = 0;
	LeapElement * hit = NULL;
	for (std::list<LeapElement*>::iterator it = testElements.begin();it != testElements.end();it++)
	{
		hit = (*it)->elementAtPoint((int)screenPoint.x,(int)screenPoint.y,flags);
		if (hit != NULL)
		{
			return hit;
		}
	}
	return hit;
}

void PointableElementManager::setHoverClickEnabled(bool _hoverClickEnabled)
{
	this->hoverClickEnabled = _hoverClickEnabled;
	
}

bool PointableElementManager::isHoverClickEnabled()
{
	return this->hoverClickEnabled;
}


void PointableElementManager::processInputEvents()
{
	int x,y;
	glfwGetMousePos(&x,&y);
	
	Pointable fakePointable;
	

	LeapElement * hit = NULL;

	Vector screenPoint = Vector(x,y,0);

	int flags = 0;
	View * topView = dynamic_cast<View*>(globalGestureListenerStack.top());
	hit = topView->elementAtPoint((int)screenPoint.x,(int)screenPoint.y,flags);

	LeapElement * hitLastFrame = NULL;
	auto it = lastHit.find(fakePointable.id());
	if (it != lastHit.end())
	{
		hitLastFrame = it->second;		
	}

	if (hit != hitLastFrame)
	{
		if (hit != NULL)	
			hit->pointableEnter(fakePointable);
	
		if (hitLastFrame != NULL)
			hitLastFrame->pointableExit(fakePointable);
	}

	lastHit[fakePointable.id()] = hit;


	for (auto it = lastHit.begin(); it != lastHit.end();it++)
	{
		if (it->first != fakePointable.id() && it->second != NULL)
		{
			it->second->pointableExit(fakePointable);
			lastHit[it->first] = NULL;
		}
	}

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

void PointableElementManager::processFrame(const Controller & controller, Frame frame)
{	
	static bool hoverClickEnabled = GlobalConfig::tree()->get<bool>("Leap.HoverSelect.Enabled");
	static float hoverClickTimeLimit = GlobalConfig::tree()->get<double>("Leap.HoverSelect.SelectTime");
	static bool tapClickEnabled = GlobalConfig::tree()->get<bool>("Leap.TouchDistance.ClickEnabled");
	static float maxRadialClickVelocity = GlobalConfig::tree()->get<float>("Leap.TouchDistance.MaxRadialClickVelocity");
	static float maxLongitudalClickVelocity = GlobalConfig::tree()->get<float>("Leap.TouchDistance.MaxLongitudalClickVelocity");
	static float cursorDimension = (((float)GlobalConfig::ScreenWidth) / 2560.0f) *  GlobalConfig::tree()->get<float>("Overlay.BaseCursorSize");
	static float clickDistanceThreshold = GlobalConfig::tree()->get<float>("Leap.TouchDistance.ClickDistanceThreshold");
	static float minimumDrawDistance = GlobalConfig::tree()->get<float>("Leap.TouchDistance.MinimumDrawDistance");
	static bool drawIntentOnly = GlobalConfig::tree()->get<bool>("Leap.TouchDistance.DrawIntentOnly");
	
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

			
	bool canPointableClick = (!hand.isValid() || hand.pointables().count() == 1);
	bool overSelectableElement = false;

	int elementStateFlags = 0;
	if (testPointable.isValid() && globalGestureListenerStack.size() > 0)
	{				
		Screen screen = controller.locatedScreens()[0];
		screenPoint = LeapHelper::FindScreenPoint(controller,testPointable.stabilizedTipPosition(),filteredScreenPoint,screen);

		hit = RadialMenu::instance->elementAtPoint((int)screenPoint.x,(int)screenPoint.y,elementStateFlags);

		if (hit == NULL)
		{
			View * topView = dynamic_cast<View*>(globalGestureListenerStack.top());
			hit = topView->elementAtPoint((int)screenPoint.x,(int)screenPoint.y,elementStateFlags);
		}

		if (hit != hitLastFrame)
		{
			if (hit != NULL)	
			{	
				hit->pointableEnter(testPointable);		
				if (hoverClickEnabled && elementStateFlags == 0 && hoverClickEnabled && hit->isClickable() && canPointableClick)
				{
					hoverClickState = 1;
					hoverClickTimer.start();
				}
				else
				{
					hoverClickState = 0;
				}
			}
			else 
				hoverClickState = 0;

			if (hitLastFrame != NULL)
				hitLastFrame->pointableExit(testPointable);
		}
		else if (canPointableClick && hoverClickState == 0)
		{
			if (hoverClickEnabled && hit != NULL && elementStateFlags == 0 && hoverClickEnabled && hit->isClickable())
			{
				hoverClickState = 1;
				hoverClickTimer.start();
			}
		}
		hitLastFrame = hit;
	}
	



	if (elementStateFlags == 0 
		&& hit != NULL 
		&& hit->isClickable() 
		&& canPointableClick)
	{
		hit->onFrame(controller);
		overSelectableElement = true;

		bool clicked = false; 
		if (hoverClickEnabled)
		{
			if (hoverClickState == 1 && hoverClickTimer.seconds() >= hoverClickTimeLimit)
			{
				hoverClickState = 0;
				hit->elementClicked();
				clicked = true;
			}		
		}

		if (tapClickEnabled && !clicked)
		{
			float radialVelocity = testPointable.direction().cross(testPointable.tipVelocity()).magnitude();
			float longitudalVelocity = testPointable.direction().dot(testPointable.tipVelocity());

			if (radialVelocity < maxRadialClickVelocity && longitudalVelocity < maxLongitudalClickVelocity)
			{
				Pointable lastFramePointable = controller.frame(1).pointable(testPointable.id());		
				if (lastFramePointable.isValid())
				{
					if (lastFramePointable.touchDistance() >= clickDistanceThreshold && testPointable.touchDistance() < clickDistanceThreshold)
					{
						//hoverClickTimer.start();
						hoverClickState = 0;
						hit->elementClicked();
					}
				}
			}
		}
	}
	else
	{
		//hoverClickTimer.start();
		hoverClickState = 0;
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

	if (frame.hands().count() > 1)
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

	for (int i=0;i<hand.fingers().count();i++)
	{
		Finger f = hand.fingers()[i];


		if (touchDistanceVisuals.size() <= i)
		{
			LeapDebugVisual * distanceVisual =new LeapDebugVisual(Vector(-100,-100,0),1,LeapDebugVisual::LiveForever,0,Color(GlobalConfig::tree()->get_child("Leap.TouchDistance.Visual.FillColor")));
			distanceVisual->lineColor = Color(GlobalConfig::tree()->get_child("Leap.TouchDistance.Visual.LineColor"));
			distanceVisual->lineWidth = GlobalConfig::tree()->get<float>("Leap.TouchDistance.Visual.LineWidth");
			distanceVisual->depth=11;
			LeapDebug::instance->addDebugVisual(distanceVisual);
			touchDistanceVisuals.push_back(distanceVisual);
		}

		if (drawIntentOnly && f.id() != testPointable.id()) continue;

		touchDistanceVisuals.at(i)->size = cursorDimension*(1.0f + max<float>(minimumDrawDistance,f.touchDistance()));

		touchDistanceVisuals.at(i)->screenPoint = LeapHelper::FindScreenPoint(controller,f);

		if (canPointableClick)
		{				
			touchDistanceVisuals.at(i)->lineColor = Color(GlobalConfig::tree()->get_child("Leap.TouchDistance.ClickVisual.LineColor"));
			touchDistanceVisuals.at(i)->lineWidth = GlobalConfig::tree()->get<float>("Leap.TouchDistance.ClickVisual.LineWidth");

			float alphaMod = 0;
			Color c = Color(GlobalConfig::tree()->get_child("Leap.TouchDistance.ClickVisual.FillColor"));
			if (f.touchDistance() <= minimumDrawDistance)
			{
				alphaMod = min<float>(1.0f,-2.0f * f.touchDistance());
			}
			c.setAlpha(c.colorArray[3] * alphaMod);
			touchDistanceVisuals.at(i)->fillColor = c;
		}
		else
		{
			touchDistanceVisuals.at(i)->lineColor = Color(GlobalConfig::tree()->get_child("Leap.TouchDistance.Visual.FillColor"));
			touchDistanceVisuals.at(i)->lineColor = Color(GlobalConfig::tree()->get_child("Leap.TouchDistance.Visual.LineColor"));
			touchDistanceVisuals.at(i)->lineWidth = GlobalConfig::tree()->get<float>("Leap.TouchDistance.Visual.LineWidth");
		}
	}



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

void PointableElementManager::requestGlobalGestureFocus(GlobalGestureListener * globalListener)
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

void PointableElementManager::releaseGlobalGestureFocus(GlobalGestureListener * globalListener)
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