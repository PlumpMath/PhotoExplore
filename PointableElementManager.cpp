#include "PointableElementManager.h"
#include "LeapHelper.h"
#include "HandModel.h"
#include "LeapDebug.h"
#include "GLImport.h"
#include "ShakeGestureDetector.hpp"

PointableElementManager::PointableElementManager()
{
	this->handProcessor = HandProcessor::getInstance();
	gestureFocusElement = NULL;
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
		mouseButtonState[GLFW_MOUSE_BUTTON_1] = mouseState;
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


//void PointableElementManager::getRoutingStack(LeapElement * element, stack<LeapElement*> & result)
//{
//	while (element->getParent() != NULL)
//	{
//		result.push(element->getParent());
//		element = element->getParent();
//	}
//}
//
//void PointableElementManager::callEvent(LeapElement * element, Pointable & pointable, int eventType)
//{
//	stack<LeapElement*> routingStack;
//	getRoutingStack(element,routingStack);
//
//	while (!routingStack.empty())
//	{
//		bool intercepted = false;
//		switch (eventType)
//		{
//		case EnterEvent:
//			intercepted = routingStack.top()->interceptPointableEnter(element, pointable);
//			break;
//		case ExitEvent:
//			intercepted = routingStack.top()->interceptPointableExit(element,pointable);
//			break;
//		}
//		routingStack.pop();
//		
//		if (intercepted)
//			break;		
//	}
//}

void PointableElementManager::processFrame(const Controller & controller, Frame frame)
{
	if (frame.id() == lastFrameId || !frame.isValid())
		return;

	if (this->globalGestureListenerStack.size() > 0)
	{
		View * topView = dynamic_cast<View*>(globalGestureListenerStack.top());
		topView->onFrame(controller);
	}

	if (this->globalGestureListenerStack.size() > 0)
	{
		GestureList gestureList = frame.gestures(lastFrame);
		for (int i=0;i< gestureList.count(); i++)
		{
			Gesture g = gestureList[i];

			if (processedGestures.count(g.id()) == 0)
			{
				if (globalGestureListenerStack.top()->onLeapGesture(controller, g))
				{
					processedGestures.insert(g.id());
				}
			}
		}
	
	}

	if (processedGestures.size() > 0)
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
		

	
	//Hand hand;
	//if (GlobalConfig::LeftHanded)
	//	hand = frame.hands().leftmost();
	//else
	//	hand = frame.hands().rightmost();
	
	HandModel * hm = handProcessor->lastModel();
	
	Pointable testPointable = frame.pointable(hm->IntentFinger);
	Vector screenPoint;
	LeapElement * hit = NULL;

	Hand hand = controller.frame().hand(hm->HandId);
	
	bool canPointableClick = (!hand.isValid() || hand.pointables().count() == 1);
	int elementStateFlags = 0;
	if (testPointable.isValid() && globalGestureListenerStack.size() > 0)
	{
		Screen screen;
		screenPoint = LeapHelper::FindScreenPoint(controller,testPointable,screen);
		
		View * topView = dynamic_cast<View*>(globalGestureListenerStack.top());
		hit = topView->elementAtPoint((int)screenPoint.x,(int)screenPoint.y,elementStateFlags);

		if (hit != hitLastFrame)
		{
			if (hit != NULL)	
			{	
				hit->pointableEnter(testPointable);		
				if (elementStateFlags == 0 && hoverClickEnabled && hit->isClickable() && canPointableClick)
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
			if (hit != NULL && elementStateFlags == 0 && hoverClickEnabled && hit->isClickable())
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
		&& (canPointableClick)) //|| hoverClickState == 1))
	{
		hit->onFrame(controller);

		//Check for hover-click completion
		Pointable lastFramePointable = controller.frame(1).pointable(testPointable.id());		
		if (hoverClickState == 1 && hoverClickTimer.seconds() >= GlobalConfig::getInstance().getFloat("HoverClickTime"))
		{
			hoverClickState = 0;
			hit->elementClicked();
		}		
		//Check for touchzone events
		else if (canPointableClick && lastFramePointable.isValid())
		{
			if (lastFramePointable.touchDistance() >= 0 && testPointable.touchDistance() < 0)
			{
				//hoverClickTimer.start();
				hoverClickState = 0;
				hit->elementClicked();
			}
		}
	}
	else
	{
		//hoverClickTimer.start();
		hoverClickState = 0;
	}
	

	static LeapDebugVisual * ldvHover = NULL, * ldvIntent = NULL, * ldvNonDominant = NULL;

	float cursorDimension = (((float)GlobalConfig::ScreenWidth) / 2560.0f) * 41;

	if (ldvHover == NULL)
	{
		ldvHover =new LeapDebugVisual(Point2f(screenPoint.x,screenPoint.y),1,LeapDebugVisual::LiveForever,0,Colors::HoloBlueBright);
		ldvHover->depth=11;
		LeapDebug::instance->addDebugVisual(ldvHover);

		ldvIntent =new LeapDebugVisual(Point2f(screenPoint.x,screenPoint.y),1,LeapDebugVisual::LiveForever,cursorDimension,Colors::Black.withAlpha(.5f));
		ldvIntent->depth=10;
		LeapDebug::instance->addDebugVisual(ldvIntent);

		ldvNonDominant = new LeapDebugVisual(Point2f(screenPoint.x,screenPoint.y),1,LeapDebugVisual::LiveForever,0,Colors::DarkGreen.withAlpha(.5f));
		ldvNonDominant->depth=10;
		LeapDebug::instance->addDebugVisual(ldvNonDominant);

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
		}
	}
	else
		ldvNonDominant->size = 0;


	if (canPointableClick)
		ldvIntent->lineColor = Colors::HoloBlueBright;
	else
		ldvIntent->lineColor = Colors::OrangeRed;
	
	ldvIntent->screenPoint.x =screenPoint.x;
	ldvIntent->screenPoint.y =screenPoint.y;
	ldvHover->screenPoint.x =screenPoint.x;
	ldvHover->screenPoint.y =screenPoint.y;

	if (hoverClickState == 1)
	{
		double size = cursorDimension*(hoverClickTimer.seconds()/2.0);
		ldvHover->size = size;
		//LeapDebugVisual * ldv =new LeapDebugVisual(Point2f(hitScreenPoint.x,hitScreenPoint.y),2,2,size,Colors::HoloBlueBright);
	}
	else		
		ldvHover->size = 0;

	handleGlobalGestures(controller);
	
	lastFrame = frame;
	lastFrameId = frame.id();

}


void PointableElementManager::handleGlobalGestures(const Controller & controller)
{
	if (globalGestureListenerStack.size() > 0)
	{
		if (shakeGestureState == -1)
		{
			if (ShakeGestureDetector::getInstance().shakeGestureOccured())
			{
				globalGestureListenerStack.top()->onGlobalGesture(controller, "shake");
				ShakeGestureDetector::getInstance().resetDetector();
			}
		}

		HandModel * hm = handProcessor->lastModel();
		Pointable intentPointable = controller.frame().pointable(hm->IntentFinger);
		if (intentPointable.isValid() && intentPointable.tipVelocity().magnitude() < GlobalConfig::getInstance().getFloat("PointGestureMinVelocity")
			&& intentPointable.direction().angleTo(Vector::forward()) < Leap::PI/4.0f 
			&& (!intentPointable.hand().isValid() || intentPointable.hand().pointables().count() == 1))
		{
			if (pointingGestureState == 0)
			{			
				pointingGestureState = 1;
				pointingGestureTimer.start();				
			}
			else if (pointingGestureState == 1)
			{
				if (intentPointable.tipVelocity().magnitude() < GlobalConfig::getInstance().getFloat("PointGestureMinVelocity") 
					&& intentPointable.direction().angleTo(Vector::forward()) < Leap::PI/3.0f)
				{
					if (pointingGestureTimer.millis() > 50)
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
		globalGestureListenerStack.push(globalListener);
}

void PointableElementManager::releaseGlobalGestureFocus(GlobalGestureListener * globalListener)
{
	if (globalGestureListenerStack.size() > 0 && globalGestureListenerStack.top() == globalListener)
		globalGestureListenerStack.pop();
}