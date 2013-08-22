#ifndef LEAPIMAGE_TUTORIAL_SWIPE_HPP_
#define LEAPIMAGE_TUTORIAL_SWIPE_HPP_

#include "InteractionsTutorial.hpp"
#include "LeapHelper.h" 

namespace TouchScrollTutorial
{
	struct SpreadHand : public TutorialStep
	{
		int state;
		Timer spreadTimer;
		
		void reset() { state = 0;}
		int getFirstDependent() {return 0;}
		float getProgress()
		{
			if (state == 0)
				return 0;
			else
				return (float)(spreadTimer.millis()/100.0);
		}

		bool onFrame(const Controller & controller)
		{
			if (LeapInput::getInstance()->getInteractionState().HandState == HandModel::Spread)
			{
				if (state == 0)
				{
					spreadTimer.start();
					state = 1;
				}
				else if (state == 1 && spreadTimer.millis() > 100)
					return true;
			}
			else
			{
				state = 0;
			}

			return false;
		}

		bool isComplete(const Controller & controller)
		{
			//Hand h = controller.frame().hand(LeapInput::getInstance()->getInteractionState().ActiveHandId);
			return LeapInput::getInstance()->getInteractionState().HandState == HandModel::Spread;			
		}
		
		string getDescription()
		{
			return "Step 1 - Hold out your hand to begin; spread your fingers but keep them relaxed. The cursor will turn orange.";
		}

	};

	struct Step2 : public TutorialStep
	{	
		float clickProgress;

		void reset()
		{
			SwipeGestureDetector::getInstance().setTouchScrollingEnabled(true);
			SwipeGestureDetector::getInstance().setSwipeScrollingEnabled(false);
			clickProgress = 0;
		}

		int getFirstDependent() {return 0;}
		float getProgress() { return clickProgress;}

		bool onFrame(const Controller & controller)
		{
			int id = LeapInput::getInstance()->getInteractionState().ActiveHandId;
			
			HandModel * hm = HandProcessor::getInstance()->lastModel();

			if (hm->HandId >= 0)
			{
				Pointable clicking = controller.frame().pointable(LeapHelper::FindPointableClosestToTouchZone(controller,controller.frame().hand(hm->HandId)));
				
				if (clicking.isValid())
				{
					clickProgress = 1.0f - clicking.touchDistance();

					if (clicking.touchDistance() < 0.0f)
					{
						return true;
					}
				}
			}
			return false;
		}
		
		bool isComplete(const Controller & controller)
		{
			return onFrame(controller);
		}

		string getDescription()
		{
			return "Step 2 - To grab the screen, move your fingers forward. The cursor will fill in.";
		}

	};

	struct Step3 : public TutorialStep
	{	
		float startPosition, totalDistance, travelDistanceToComplete;

		int state;

		void reset() {state = 0;}		
		int getFirstDependent() {return 0;}
		float getProgress() { return totalDistance/travelDistanceToComplete;}


		Step3()
		{
			travelDistanceToComplete = GlobalConfig::tree()->get<float>("InteractiveTutorial.Swipe.Step3.TravelDistance");
			reset();
		}

		bool onFrame(const Controller & controller)
		{
			FlyWheel * fly = SwipeGestureDetector::getInstance().getFlyWheel();
			if (state == 0)
			{
				totalDistance = 0;
				startPosition = fly->getCurrentPosition();
				state = 1;
			}
			else if (state == 1)
			{
				totalDistance +=  abs(startPosition - fly->getCurrentPosition());
				startPosition = fly->getCurrentPosition();
				
				if (totalDistance >= travelDistanceToComplete)
					return true;
			}
			return false;
		}
		
		bool isComplete(const Controller & controller)
		{
			return true;
		}

		string getDescription()
		{
			return "Step 3 - Keeping the cursor filled, move your hand left and right to drag the screen.";
		}

	};

	struct Step4 : public TutorialStep
	{	
		
		float clickProgress;

		void reset()
		{

		}

		float getProgress()
		{
			return clickProgress;
		}

		int getFirstDependent() {return 2;}


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
			return true;
		}

		string getDescription()
		{
			return "Step 4 - To release the screen, just pull your fingers back.";
		}

	};
}

#endif