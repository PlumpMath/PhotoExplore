#ifndef LEAPIMAGE_TUTORIAL_POINT_HPP_
#define LEAPIMAGE_TUTORIAL_POINT_HPP_

#include "InteractionsTutorial.hpp"

namespace PointTutorial
{
	struct PointHand : public TutorialStep
	{
		int state;
		Timer pointTimer;
		
		void reset()
		{	
			//SwipeGestureDetector::getInstance().getFlyWheel()->setFriction(0);
			//SwipeGestureDetector::getInstance().getFlyWheel()->setVelocity(-500);

			SwipeGestureDetector::getInstance().setTouchScrollingEnabled(false);
			SwipeGestureDetector::getInstance().setSwipeScrollingEnabled(false);			
			state = 0;
		}

		int getFirstDependent() {return 0;}
		float getProgress() { return (float)(pointTimer.millis()/100.0);}

		bool onFrame(const Controller & controller)
		{
			if (LeapInput::getInstance()->getInteractionState().HandState == HandModel::Pointing)
			{
				if (state == 0)
				{
					pointTimer.start();
					state = 1;
				}
				else if (state == 1 && pointTimer.millis() > 100)
				{
					SwipeGestureDetector::getInstance().getFlyWheel()->setVelocity(0);					
					return true;
				}
			}
			else
			{
				state = 0;
			}

			return false;
		}

		bool isComplete(const Controller & controller)
		{
			return (LeapInput::getInstance()->getInteractionState().HandState == HandModel::Pointing);			
		}
		
		string getDescription()
		{
			return "Step 1 - Point with your index finger";
		}

	};

	struct Press : public TutorialStep
	{	
		Timer pressTimer;
		float clickProgress;

		void reset()
		{
			SwipeGestureDetector::getInstance().setTouchScrollingEnabled(false);
			SwipeGestureDetector::getInstance().setSwipeScrollingEnabled(false);
			clickProgress = 0;
		}

		int getFirstDependent() {return 0;}
		float getProgress() { return clickProgress;}

		bool onFrame(const Controller & controller)
		{			
			HandModel * hm = HandProcessor::getInstance()->lastModel();
			if (hm->HandId >= 0)
			{
				Pointable clicking = controller.frame().pointable(hm->IntentFinger);				
				if (clicking.isValid())
				{
					clickProgress = 0.8f - clicking.touchDistance();
					if (clicking.touchDistance() < -0.2f)
					{
						return true;
					}
				}
			}
			return false;
		}
		
		bool isComplete(const Controller & controller)
		{
			return true;
		}

		string getDescription()
		{
			return "Step 2 - Tap and hold.";
		}
	};

	struct Release : public TutorialStep
	{	
		Timer pressTimer;
		float clickProgress;

		void reset()
		{
			SwipeGestureDetector::getInstance().setTouchScrollingEnabled(false);
			SwipeGestureDetector::getInstance().setSwipeScrollingEnabled(false);
			clickProgress = 0;
		}

		int getFirstDependent() {return 2;}
		float getProgress() { return clickProgress;}

		bool onFrame(const Controller & controller)
		{			
			HandModel * hm = HandProcessor::getInstance()->lastModel();
			if (hm->HandId >= 0)
			{
				Pointable clicking = controller.frame().pointable(hm->IntentFinger);				
				if (clicking.isValid())
				{
					clickProgress = 0.8f + clicking.touchDistance();
					if (clicking.touchDistance() > 0.2f)
					{
						return true;
					}
				}
			}
			return false;
		}
		
		bool isComplete(const Controller & controller)
		{
			return true;
		}

		string getDescription()
		{
			return "Step 3 - Release to select.";
		}
	};
}

#endif