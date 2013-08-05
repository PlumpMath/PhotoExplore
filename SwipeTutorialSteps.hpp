#ifndef LEAPIMAGE_SWIPE_TUTORIAL_STEPS_HPP_
#define LEAPIMAGE_SWIPE_TUTORIAL_STEPS_HPP_

#include "InteractionsTutorial.hpp"
#include "SwipeGestureDetector.hpp"
#include "Flywheel.h"

namespace SwipeTutorial {

	struct SwipeLeft : public TutorialStep
	{	
		FlyWheel * fly;

		void reset()
		{
			fly = SwipeGestureDetector::getInstance().getFlyWheel();
			SwipeGestureDetector::getInstance().setTouchScrollingEnabled(false);
			SwipeGestureDetector::getInstance().setSwipeScrollingEnabled(true);
		}

		int getFirstDependent() {return 1;}
		float getProgress() { return 0; }

		bool onFrame(const Controller & controller)
		{
			if (fly->getVelocity() < -500)
				return true;
			return false;
		}
		
		bool isComplete(const Controller & controller)
		{
			return true;
		}

		string getDescription()
		{
			return "Step 2 - Swipe to the left, gently extending your fingers as you swipe";
		}

	};

	struct SwipeRight : public TutorialStep
	{	
		FlyWheel * fly;

		void reset()
		{
			fly = SwipeGestureDetector::getInstance().getFlyWheel();
			SwipeGestureDetector::getInstance().setTouchScrollingEnabled(false);
			SwipeGestureDetector::getInstance().setSwipeScrollingEnabled(true);
		}

		int getFirstDependent() {return 1;}
		float getProgress() { return 0; }

		bool onFrame(const Controller & controller)
		{
			if (fly->getVelocity() > 500)
				return true;
			return false;
		}
		
		bool isComplete(const Controller & controller)
		{
			return true;
		}

		string getDescription()
		{
			return "Step 3 - Swipe to the right, gently extending your fingers as you swipe";
		}

	};

	struct SwipeLeftFast : public TutorialStep
	{	
		FlyWheel * fly;

		void reset()
		{
			fly = SwipeGestureDetector::getInstance().getFlyWheel();
			SwipeGestureDetector::getInstance().setTouchScrollingEnabled(false);
			SwipeGestureDetector::getInstance().setSwipeScrollingEnabled(true);
		}

		int getFirstDependent() {return 2;}
		float getProgress() { return fly->getVelocity()/-2500; }

		bool onFrame(const Controller & controller)
		{
			if (fly->getVelocity() < -2500)
				return true;
			return false;
		}
		
		bool isComplete(const Controller & controller)
		{
			return true;
		}

		string getDescription()
		{
			return "Step 4 - Now swipe left again, this time quickly";
		}

	};




}

#endif