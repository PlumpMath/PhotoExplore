#ifndef LEAPIMAGE_TUTORIAL_POINT_HPP_
#define LEAPIMAGE_TUTORIAL_POINT_HPP_

#include "InteractionsTutorial.hpp"

namespace PointTutorial
{
	struct PointHand : public TutorialStep
	{
		int state;
		Timer pointTimer;
		
		void reset() { state = 0;}
		int getFirstDependent() {return 0;}
		float getProgress() { return (float)(pointTimer.millis()/100.0);}

		bool onFrame(const Controller & controller)
		{
			if (LeapInput::getInstance()->getInteractionState().HandState == InteractionState::Pointing)
			{
				if (state == 0)
				{
					pointTimer.start();
					state = 1;
				}
				else if (state == 1 && pointTimer.millis() > 100)
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
			return (LeapInput::getInstance()->getInteractionState().HandState == InteractionState::Pointing);			
		}
		
		string getDescription()
		{
			return "Step 1 - Point with your index finger";
		}

	};
}