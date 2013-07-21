#include "SwipeGestureDetector.hpp"
#include "TextPanel.h"
#include "GlobalConfig.hpp"
#include <boost/property_tree/ptree.hpp>

using namespace boost::property_tree;

SwipeGestureDetector::SwipeGestureDetector()
{
	DrawingEnabled = GlobalConfig::tree()->get<bool>("Leap.CustomGesture.Swipe.DebugDraw");
	sampleCount = 300;
	infoText = new TextPanel("0____0");
	((TextPanel*)infoText)->setTextAlignment(1);
}

SwipeGestureDetector::~SwipeGestureDetector()
{
}

SwipeGestureDetector& SwipeGestureDetector::getInstance()
{
	static SwipeGestureDetector instance; 
	return instance;
}

void SwipeGestureDetector::setSwipeDetectedListener(boost::function<void(Hand swipingHand, Vector swipeVector)> _swipeDetectedListener)
{
	this->swipeDetectedListener = _swipeDetectedListener;
}

void SwipeGestureDetector::setFlyWheel(FlyWheel * _flyWheel)
{
	this->flyWheel = _flyWheel;
	swipeMap.clear();
}

void SwipeGestureDetector::onFrame(const Controller & controller)
{
	ptree swipeConfig = GlobalConfig::tree()->get_child("Leap.CustomGesture.Swipe");

	Vector gestureVector;		
	Frame frame = controller.frame();
	if (frame.id() == lastFrame.id())
		return;
	if (flyWheel == NULL)
	{
		lastFrame = frame;
		return;
	}

	GestureList gestures; 
	if (lastFrame.isValid())
		gestures = frame.gestures(lastFrame);
	else
		gestures = frame.gestures();

	Vector _directionSum = Vector();
	Vector _directionSpeedSum = Vector();
	
	float count = 0;


	Hand gestureHand;
	for (auto it = gestures.begin(); it != gestures.end(); it++)
	{
		Gesture g = *it;

		if (g.state() == Gesture::STATE_INVALID || g.state() == Gesture::STATE_STOP)
			continue;
		if (g.type() != Gesture::TYPE_SWIPE)
			continue;		

		SwipeGesture swipe(g);


		if (!gestureHand.isValid())
			gestureHand = swipe.hands()[0];

		if (lastFrame.isValid() && gestureHand.isValid())
		{
			if (lastActiveTime.millis() > 100)
			{
				swipeMap.clear();
			}
			lastActiveTime.start();
						
			if (swipeMap.find(swipe.id()) == swipeMap.end())
			{
				swipeMap.insert(make_pair(swipe.id(),TrackedSwipe(swipe.id())));	
			}		

			swipeMap[swipe.id()].distanceList.push_back((swipe.startPosition()-swipe.position()).magnitude());
			if (swipeMap[swipe.id()].distanceList.size() > sampleCount)
				swipeMap[swipe.id()].distanceList.pop_front();

			Hand h1 = gestureHand;
			Hand h2 = lastFrame.hand(h1.id());

			if (h2.isValid())
			{
				float sphereAccel = h1.sphereRadius() -  h2.sphereRadius();

				if (sphereAccel > swipeMap[swipe.id()].maxSphereGrowth) 
				{
					swipeMap[swipe.id()].maxSphereGrowth = sphereAccel;
				}
				else if (sphereAccel < swipeMap[swipe.id()].minSphereGrowth) 
				{
					swipeMap[swipe.id()].minSphereGrowth  = sphereAccel;
				}
			}

			float pointableDistance = swipe.pointable().touchDistance();
			swipeMap[swipe.id()].touchDistanceList.push_back(pointableDistance*50.0f);
			if (swipeMap[swipe.id()].touchDistanceList.size() > sampleCount)
				swipeMap[swipe.id()].touchDistanceList.pop_front();
			if (pointableDistance < swipeMap[swipe.id()].minTouchDistance) 
			{
				swipeMap[swipe.id()].minTouchDistance = pointableDistance;
			}		

			
			swipeMap[swipe.id()].speedList.push_back(swipe.speed());
			if (swipeMap[swipe.id()].speedList.size() > sampleCount)
				swipeMap[swipe.id()].speedList.pop_front();
			
			float swipeSpeed = 0, sampleCount = 0, numSpeedSamples = 10;

			for (auto it = swipeMap[swipe.id()].speedList.begin(); it != swipeMap[swipe.id()].speedList.end() && sampleCount < numSpeedSamples; it++)
			{
				sampleCount++;
				swipeSpeed = max<float>(swipeSpeed,*it);
			}
			//swipeSpeed /= sampleCount;
			swipeSpeed *= swipeConfig.get<float>("SpeedScale");

			bool triggered = false;
			if (swipeConfig.get<bool>("SphereGrowth.Enabled"))
			{
				if (swipeMap[swipe.id()].maxSphereGrowth > swipeConfig.get<float>("SphereGrowth.MinPositive"))
				{
					triggered = true;
				}
				else if (swipeMap[swipe.id()].minSphereGrowth < swipeConfig.get<float>("SphereGrowth.MaxNegative"))
				{						
					triggered = true;
				}
			}

			if (!triggered && swipeConfig.get<bool>("TouchZoneDistance.Enabled"))
			{
				if (swipeMap[swipe.id()].minTouchDistance < swipeConfig.get<float>("TouchZoneDistance.MaxTouchDistance"))
				{						
					triggered = true;
				}
			}

			if (triggered)
			{
				flyWheel->setFriction(.5f);
				if (abs(swipeSpeed) > abs(flyWheel->getVelocity()))
					flyWheel->setVelocity((swipeSpeed*swipe.direction()).x);
			}
		}

	}
	
	if (gestureHand.isValid() && DrawingEnabled)
	{
		stringstream ss;
		for (auto it = swipeMap.begin(); it != swipeMap.end(); it++)
		{		
			ss << it->first << "_Touch[" << it->second.minTouchDistance << "]_Sphere-[" << it->second.minSphereGrowth << "]_Sphere+[" << it->second.maxSphereGrowth << "]   ";		
		}
		((TextPanel*)infoText)->setText(ss.str());
	}

	static float maxFrictionInductionSpeed = swipeConfig.get<float>("FrictionInduce.MaxTipVelocity");
	static float frictionScale = swipeConfig.get<float>("FrictionInduce.FrictionScale");
	static float touchDistanceScale = swipeConfig.get<float>("FrictionInduce.TouchDistanceScale");

	if (maxFrictionInductionSpeed > 0 && !gestureHand.isValid() && frame.hands().count() > 0)
	{
		Hand h1 = frame.hands().rightmost(); //TEMP
		if (h1.isValid())
		{
			float avgTipVelocity = 0;
			float minTouchDistance = 1;
			for (int i=0;i<h1.fingers().count(); i++)
			{
				Finger f = h1.fingers()[i];
				avgTipVelocity += f.tipVelocity().magnitude();
				if (f.touchDistance() < minTouchDistance)
					minTouchDistance = f.touchDistance();
			}
			avgTipVelocity /= ((float)h1.fingers().count());

			if (abs(flyWheel->getVelocity()) > 200 && avgTipVelocity < maxFrictionInductionSpeed)
			{
				float tipVelocityFriction = (maxFrictionInductionSpeed - avgTipVelocity)/maxFrictionInductionSpeed;
				tipVelocityFriction *= frictionScale;
				tipVelocityFriction = max<float>(tipVelocityFriction,.5f);

				tipVelocityFriction += powf(1.0f - minTouchDistance,2.0f)*touchDistanceScale;				

				flyWheel->setFriction(tipVelocityFriction);
			}
		}
	}

	
	lastFrame = frame;
}

void SwipeGestureDetector::draw()
{
	if (DrawingEnabled) 
	{
		static float maxV = GlobalConfig::ScreenHeight*.2f;
		
		static Color colors [] = {Colors::Red,Colors::Blue,Colors::Orange,Colors::HoloBlueBright,Colors::LeapGreen};

		glBindTexture(GL_TEXTURE_2D,NULL);

		float originY = 200;

		glColor4fv(Colors::Red.getFloat());
		glLineWidth(1);
		glBegin(GL_LINE_STRIP);
		glVertex3f(0,originY,11);
		glVertex3f(GlobalConfig::ScreenWidth,originY,11);
		glEnd();
		
		int count = 0;
		for (auto it = swipeMap.begin(); it != swipeMap.end();it++)
		{
			glColor4fv(colors[it->first%5].getFloat());
			glLineWidth(3);
			float x = 0;

			glBegin(GL_LINE_STRIP);
			for (auto point = it->second.distanceList.begin(); point != it->second.distanceList.end(); point++)
			{
				float xD = ((x++)*(GlobalConfig::ScreenWidth/sampleCount)*.4f) + 200;
				float yD = (*point)+originY+count;
				glVertex3f(xD,yD,10);
			}
			glEnd();

			x = (GlobalConfig::ScreenWidth/sampleCount)*.45f;

			glBegin(GL_LINE_STRIP);
			for (auto point = it->second.touchDistanceList.begin(); point != it->second.touchDistanceList.end(); point++)
			{
				float xD = ((x++)*(GlobalConfig::ScreenWidth/sampleCount)*.4f) + 200;
				float yD = (*point)+originY+count;
				glVertex3f(xD,yD,10);
			}
			glEnd();

			if (x != 0)
			{
				count+= GlobalConfig::ScreenHeight*.22f;
				if (count > GlobalConfig::ScreenHeight*.7f)
					count = 0;
			}
		}
		infoText->layout(Vector(300,GlobalConfig::ScreenHeight*.6f,10), cv::Size2f(600,400));
		infoText->draw();
	}

}
