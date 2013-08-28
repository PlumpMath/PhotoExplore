#include "LeapInput.hpp"
#include "LeapHelper.h"
#include "HandModel.h"
#include "LeapDebug.h"
#include "GLImport.h"
#include "ShakeGestureDetector.hpp"
#include "GraphicContext.hpp"
#include "InputEventHandler.hpp"

LeapInput::LeapInput()
{
	shakeGestureState = -1;
	pointingGestureState = 0;
	hitLastFrame = NULL;
	topElement = NULL;
	mouseHitLast = NULL;
	drawNonDominant = false;
	cursorDrawEnabled = true;
}

void LeapInput::setCursorDrawEnabled(bool drawCursors)
{
	this->cursorDrawEnabled = drawCursors;
}

bool LeapInput::isCursorDrawEnabled()
{
	return this->cursorDrawEnabled;
}


InteractionState LeapInput::getInteractionState()
{
	return this->currentState;
}

void LeapInput::processInputEvents()
{
	static PointableCursor * mouseVisual = NULL;
	static Color clickColor = Color(GlobalConfig::tree()->get_child("Leap.MouseInput.CursorVisual.LineColor"));
	static Color baseColor = Color(GlobalConfig::tree()->get_child("Leap.MouseInput.CursorVisual.FillColor"));

	if (mouseVisual == NULL)
	{
		float cursorDimension = (((float)GlobalConfig::ScreenWidth) / 2560.0f) *  GlobalConfig::tree()->get<float>("Overlay.BaseCursorSize");	

		mouseVisual = new MouseCursor(cursorDimension,baseColor);
		//mouseVisual->lineColor = Color(GlobalConfig::tree()->get_child("Leap.MouseInput.CursorVisual.LineColor"));
		mouseVisual->lineWidth =GlobalConfig::tree()->get<float>("Leap.MouseInput.CursorVisual.LineWidth");
		
		LeapDebug::getInstance().addDebugVisual(mouseVisual);
		
		
		
		InputEventHandler::getInstance().addMouseButtonCallback([this](GLFWwindow*window,int button,int action, int mods) -> bool {
			
			if (this->mouseButtonState[button] != GLFW_PRESS && action == GLFW_PRESS)
				this->mouseButtonState[button] = action;
			
			return true;
		});
	}
	
	double x,y;
	glfwGetCursorPos(GraphicsContext::getInstance().MainWindow,&x,&y);


	Pointable fakePointable;
	LeapElement * hit = NULL;
	Vector screenPoint = Vector(x,y,0);
	int flags = 0;

	View * topView = (View*)topElement; //dynamic_cast<View*>(globalGestureListenerStack.top());
	hit = topView->elementAtPoint((int)screenPoint.x,(int)screenPoint.y,flags);
	
	if (hit != mouseHitLast)
	{
		if (hit != NULL)	
		{
			hit->pointableEnter(fakePointable);
		}

		if (mouseHitLast != NULL)
			mouseHitLast->pointableExit(fakePointable);
	}

	mouseHitLast = hit;


	int mouseState = mouseButtonState[GLFW_MOUSE_BUTTON_1];
	
	//if (mouseState == GLFW_PRESS || mouseState == GLFW_REPEAT)
	//{
	//	mouseVisual->fillColor = clickColor;
	//}
	//else
	//{		
	//	mouseVisual->fillColor = baseColor;
	//}

	if (hit != NULL && hit->isClickable() && mouseState == GLFW_PRESS)
	{
		hit->elementClicked();
	}
	
	int newMouseState = glfwGetMouseButton(GraphicsContext::getInstance().MainWindow,GLFW_MOUSE_BUTTON_1);

	if (newMouseState == GLFW_PRESS)
		mouseButtonState[GLFW_MOUSE_BUTTON_1] = GLFW_REPEAT;
	else
		mouseButtonState[GLFW_MOUSE_BUTTON_1] = GLFW_RELEASE;

}

void LeapInput::enableNonDominantCursor(bool _enable)
{
	this->drawNonDominant = _enable;
}

static int determineHandState(const Controller & controller, Frame frame, Hand hand)
{		
	static float maxSpreadHandInterfingerAngle = GlobalConfig::tree()->get<float>("Leap.HandPoseControl.SpreadHand.MaxInterfingerAngle");
	
	HandModel * hm = HandProcessor::getInstance()->lastModel();

	return hm->Pose;
}

void LeapInput::processFrame(const Controller & controller, Frame frame)
{	
	static float maxRadialClickVelocity = GlobalConfig::tree()->get<float>("Leap.TouchDistance.MaxRadialClickVelocity");
	static float maxLongitudalClickVelocity = GlobalConfig::tree()->get<float>("Leap.TouchDistance.MaxLongitudalClickVelocity");
	static float clickDistanceThreshold = GlobalConfig::tree()->get<float>("Leap.TouchDistance.ClickDistanceThreshold");
	static float minimumDrawDistance = GlobalConfig::tree()->get<float>("Leap.TouchDistance.MinimumDrawDistance");
	static bool drawIntentOnly = GlobalConfig::tree()->get<bool>("Leap.TouchDistance.DrawIntentOnly");
	
	bool clickOnRelease = true;
	
	if (frame.id() == lastFrameId || !frame.isValid())
		return;

	HandModel * hm = HandProcessor::getInstance()->lastModel();

	Pointable testPointable = frame.pointable(hm->IntentFinger);
	Vector screenPoint;
	LeapElement * hit = NULL;

	Hand hand = controller.frame().hand(hm->HandId);

	currentState.ActiveHandId = hand.id();
	currentState.HandState = determineHandState(controller,frame,hand);

	int elementStateFlags = 0;
	if (testPointable.isValid() && topElement != NULL)
	{				
		screenPoint = LeapHelper::FindScreenPoint(controller,testPointable);

		LeapElement * topView = topElement;
		hit = topView->elementAtPoint((int)screenPoint.x,(int)screenPoint.y,elementStateFlags);
		
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
	if (currentState.HandState == HandModel::Pointing)
		canPointableClick = true;
	else
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
				for (int i = 1; i < 100; i++)
				{
					f0 = controller.frame(i-1);
					f1 = controller.frame(i);

					if (!f1.isValid())
						break;

					p0 = f0.pointable(testPointable.id());
					p1 = f1.pointable(testPointable.id());
				
					
					if (p0.touchDistance() >= clickDistanceThreshold && p1.touchDistance() < clickDistanceThreshold)
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

	currentState.ClickingPointableId = -1;		
	if (!clicked && canPointableClick && testPointable.touchDistance() < clickDistanceThreshold)
	{
		currentState.ClickingPointableId = testPointable.id();
	}

	static PointableTouchCursor * ldvIntent = NULL;

	if (ldvIntent == NULL)
	{
		float cursorDimension = PointableCursor::getDefaultSize();
		ldvIntent =new PointableTouchCursor(cursorDimension,cursorDimension*2.0f,Color(GlobalConfig::tree()->get_child("Leap.IntentControl.ScrollVisual.Color")));
		LeapDebug::getInstance().addDebugVisual(ldvIntent);
	}

	if (canPointableClick)
	{
		ldvIntent->setColor(Color(GlobalConfig::tree()->get_child("Leap.IntentControl.SelectVisual.Color")));
	}
	else
	{		
		ldvIntent->setColor(Color(GlobalConfig::tree()->get_child("Leap.IntentControl.ScrollVisual.Color")));
	}
	
	ldvIntent->setPointable(testPointable);


	handleGlobalGestures(controller);
	
	lastFrame = frame;
	lastFrameId = frame.id();

	ldvIntent->setVisible(cursorDrawEnabled);
}


void LeapInput::handleGlobalGestures(const Controller & controller)
{	
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

void LeapInput::requestGlobalGestureFocus(ActivityView * globalListener)
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
		LeapDebug::getInstance().setTutorialImages(tutorial);
		globalGestureListenerStack.top()->onGlobalFocusChanged(true);
	}
}

void LeapInput::releaseGlobalGestureFocus(ActivityView * globalListener)
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
			LeapDebug::getInstance().setTutorialImages(tutorial);
		}
	}
}

void LeapInput::setTopLevelElement(LeapElement * _topElement)
{
	this->topElement = _topElement;
}