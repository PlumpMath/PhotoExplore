#ifndef LEAPIMAGE_INTERACTION_TUTORIAL_HPP_
#define LEAPIMAGE_INTERACTION_TUTORIAL_HPP_

#include "ActivityView.hpp"
#include "ColorPanel.hpp"
#include "FixedAspectGrid.hpp"
#include "ScrollingView.hpp"
#include <Leap.h>
#include <boost/function.hpp>
#include "UniformGrid.hpp"
#include "Button.hpp"

using namespace Leap;


struct TutorialStep
{
	virtual float getProgress() = 0;
	virtual int getFirstDependent() = 0;
	virtual void reset() = 0;
	virtual string getDescription() = 0;	
	virtual bool onFrame(const Controller & controller)  = 0;
	virtual bool isComplete(const Controller & controller) = 0;
};


class InteractionsTutorial : public ActivityView 
{
private:	
	
	ScrollingView * scrollView;
	UniformGrid * tutorialStepGrid;

	void generatePanels(cv::Size2i gridSize);

	void init();

	void startCurrentTutorial();
	void loadTutorial(string name);

	vector<TutorialStep*> currentTutorial;

	void nextStep();
	int stepNum;

	FloatingPanel * indicatorPanel, * activeStepPanel;

	View * selectedView;
	void setActiveStep(int index);

	Button * nextTutorialButton;

public:
	InteractionsTutorial();

	void layout(Vector position, cv::Size2f size);
		
	void onGlobalGesture(const Controller & controller, std::string gestureType);
	void getTutorialDescriptor(vector<string> & tutorial);
	void onGlobalFocusChanged(bool focused);

	void onFrame(const Controller & controller);

};

#endif