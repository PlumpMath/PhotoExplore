#ifndef LEAPIAMGE_TUTORIAL_FRICTION_HPP_
#define LEAPIAMGE_TUTORIAL_FRICTION_HPP_

#include "InteractionsTutorial.hpp"
#include "SwipeGestureDetector.hpp"


namespace FrictionTutorial
{
	struct WithdrawHandStep : public TutorialStep
	{			
		float clickProgress;

		void reset()
		{

		}

		float getProgress()
		{
			return clickProgress;
		}

		int getFirstDependent() {return 0;}


		bool onFrame(const Controller & controller)
		{
			int id = LeapInput::getInstance()->getInteractionState().ActiveHandId;
			
			HandModel * hm = HandProcessor::getInstance()->lastModel();

			if (hm->HandId >= 0)
			{
				Pointable clicking = controller.frame().pointable(hm->IntentFinger);
				clickProgress = max<float>(0.0f,clicking.touchDistance()*2.0f);
				if (clicking.isValid() && clicking.touchDistance() < 0.5f)
				{
					return false;
				}
			}
			return true;
		}
		
		bool isComplete(const Controller & controller)
		{
			return true;
		}

		string getDescription()
		{
			return "Step 2 - Pull your fingers back to stop scrolling";
		}

	};


	struct SlowWheel : public TutorialStep
	{
		float progress, velocityThreshold, startSpeed;
		FlyWheel * fly;
		
		void reset()
		{			
			fly->overrideValue(0);
			fly->setFriction(0);
			fly->setVelocity(-(velocityThreshold + startSpeed));
			progress = 0;		
		}

		SlowWheel(FlyWheel * _fly)
		{
			velocityThreshold = GlobalConfig::tree()->get<float>("Leap.TouchScroll.FlyWheelVelocityThreshold");
			startSpeed = GlobalConfig::tree()->get<float>("InteractiveTutorial.Friction.SlowWheel.StartSpeed");
			this->fly = _fly;
		}

		int getFirstDependent() {return 0;}
		float getProgress() { return progress;}

		bool onFrame(const Controller & controller)
		{			
			progress =  (startSpeed - (abs(fly->getVelocity()) - velocityThreshold))/startSpeed;
			return abs(fly->getVelocity()) < velocityThreshold;
		}

		bool isComplete(const Controller & controller)
		{
			return true;
		}
		
		string getDescription()
		{
			return "Step 3 - Gently move your fingers forward to slow the scrolling";
		}

	};

}

#endif