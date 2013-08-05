#ifndef LEAPIMAGE_SHAKE_TUTORIAL_STEPS_HPP_
#define LEAPIMAGE_SHAKE_TUTORIAL_STEPS_HPP_

#include "InteractionsTutorial.hpp"

namespace ShakeTutorial 
{	
	struct PullBack : public TutorialStep
	{	
		
		float clickProgress;

		void reset()
		{			
			SwipeGestureDetector::getInstance().setTouchScrollingEnabled(false);
			SwipeGestureDetector::getInstance().setSwipeScrollingEnabled(false);
		}

		float getProgress()
		{
			return clickProgress;
		}

		int getFirstDependent() {return 0;}


		bool onFrame(const Controller & controller)
		{	
			HandModel * hm = HandProcessor::getInstance()->lastModel();
			if (hm->HandId >= 0)
			{				
				Pointable clicking = controller.frame().pointable(LeapHelper::FindPointableClosestToTouchZone(controller,controller.frame().hand(hm->HandId)));
				clickProgress = 0.8f + clicking.touchDistance();
				if (clicking.isValid() && clicking.touchDistance() < 0.2f)
				{
					return false;
				}
			}
			return true;
		}

		
		bool isComplete(const Controller & controller)
		{
			return onFrame(controller);
		}

		string getDescription()
		{
			return "Step 2 - Pull your fingers back";
		}

	};

	struct Shake : public TutorialStep
	{	
		bool gestureComplete;

		void reset()
		{
			gestureComplete = false;
		}

		int getFirstDependent() {return 0;}
		float getProgress() { return 0; }

		bool onFrame(const Controller & controller)
		{
			return gestureComplete;
		}
		
		bool isComplete(const Controller & controller)
		{
			return gestureComplete;
		}

		string getDescription()
		{
			return "Step 3 - Gently but quickly shake your fingers";
		}

		void onGlobalGesture(string gestureType)
		{
			if (gestureType.compare("shake") == 0)
			{
				gestureComplete = true;
			}
		}

	};

}

#endif