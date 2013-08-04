#include "InteractionsTutorial.hpp"
#include "SwipeGestureDetector.hpp"
#include "LeapInput.hpp"
#include "SwipeTutorialSteps.hpp"
#include "FrictionTutorialSteps.hpp"

InteractionsTutorial::InteractionsTutorial()
{
	selectedView = NULL;
	indicatorPanel = new FloatingPanel(0,0,Vector());
	indicatorPanel->setBackgroundColor(Color(GlobalConfig::tree()->get_child("InteractiveTutorial.FinishedStepColor")));
	addChild(indicatorPanel);

	activeStepPanel = new FloatingPanel(0,0,Vector());
	activeStepPanel->setBackgroundColor(Color(GlobalConfig::tree()->get_child("InteractiveTutorial.ActiveStepColor")));
	addChild(activeStepPanel);

	init();
	loadTutorial("tap");
}

void InteractionsTutorial::loadTutorial(string name)
{	
	currentTutorial.clear();

	if (name.compare("swipe") == 0)
	{
		currentTutorial.push_back(new SwipeTutorial::SpreadHand());
		currentTutorial.push_back(new SwipeTutorial::Step2());
		currentTutorial.push_back(new SwipeTutorial::Step3());
		currentTutorial.push_back(new SwipeTutorial::Step4());

		nextTutorialButton->elementClickedCallback = [this](LeapElement * clicked){
			this->loadTutorial("tap");
		};
	}
	else if (name.compare("tap") == 0)
	{
		nextTutorialButton->elementClickedCallback = [this](LeapElement * clicked){
			this->loadTutorial("swipe");
		};
	}
	//else if (name.compare("friction") == 0)
	//{

	//	currentTutorial.push_back(new SwipeTutorial::SpreadHand());
	//	currentTutorial.push_back(new FrictionTutorial::WithdrawHandStep());
	//	currentTutorial.push_back(new FrictionTutorial::SlowWheel(scrollView->getFlyWheel()));

	//	nextTutorialButton->elementClickedCallback = [this](LeapElement * clicked){
	//		this->loadTutorial("swipe");
	//	};
	//}
	else
	{
		throw new std::runtime_error("Invalid tutorial name " + name);
	}
	startCurrentTutorial();
}

void InteractionsTutorial::init()
{
	generatePanels(cv::Size2i(0,4));
	LeapInput::getInstance()->requestGlobalGestureFocus(this);
		
	nextTutorialButton = new Button("");
	nextTutorialButton->setStyle(GlobalConfig::tree()->get_child("InteractiveTutorial.NextButton"));

	tutorialStepGrid = new UniformGrid(cv::Size2i(1,5));
	addChild(tutorialStepGrid);

	this->layout(Vector(),cv::Size2f(GlobalConfig::ScreenWidth,GlobalConfig::ScreenHeight));
}


void InteractionsTutorial::generatePanels(cv::Size2i gridSize)
{
	float numPanels = gridSize.height;
	FixedAspectGrid * floatLayout = new FixedAspectGrid(cv::Size2i(0,(int)numPanels),1.0f);
	float totalWidth = GlobalConfig::ScreenWidth*10.0f;

	floatLayout->setLayoutParams(cv::Size2f(totalWidth,GlobalConfig::ScreenHeight));

	float panelWidth =((float)GlobalConfig::ScreenHeight)/numPanels;
	
	int itemCount = ceilf(totalWidth/panelWidth) * numPanels;
	float spacing = 5;

	std::srand(10);
	
	for (int i =0; i < itemCount;i++)
	{
		PanelBase * fp;
		fp = new FloatingPanel(100,100,Vector());


		float r = 25.0f*abs(sinf(i*2.0f)/PI);
		float g = 25.0f*abs(sinf(i*4.0f)/PI);
		float b = 255.0f*abs(sinf(i)/PI);
		fp->setBackgroundColor(Color(255,(int)r,(int)g,(int)b));

		fp->setLayoutParams(LayoutParams(cv::Size2f(0,0),cv::Vec4f(spacing,spacing,spacing,spacing)));
		fp->setBorderThickness(0);
		fp->NudgeAnimationEnabled = true;

		floatLayout->addChild(fp);
	}	
	scrollView = new ScrollingView(floatLayout);
	scrollView->layout(Vector(),cv::Size2f(GlobalConfig::ScreenWidth,GlobalConfig::ScreenHeight));
	
	addChild(scrollView);
}

void InteractionsTutorial::layout(Vector position, cv::Size2f size)
{	
	lastPosition = position;
	lastSize = size;
	tutorialStepGrid->layout(position + Vector(size.width*.5f,size.height*.2f,0),cv::Size2f(size.width*.4f,size.height*.6f));
		
	if (stepNum > 0 && stepNum < currentTutorial.size()+1)
	{
		PanelBase * selectedPanel = dynamic_cast<PanelBase*>(tutorialStepGrid->getChildren()->at(stepNum-1));
		if (selectedPanel != NULL)
		{
			indicatorPanel->setVisible(true);
			indicatorPanel->setAnimateOnLayout(false);
			cv::Size2f indicatorSize = selectedPanel->getMeasuredSize();
			//indicatorSize.width *= .06f;
			indicatorSize.height += (selectedPanel->getLastPosition().y - tutorialStepGrid->getLastPosition().y);

			//indicatorPanel->layout(tutorialStepGrid->getLastPosition() - Vector(indicatorSize.width,0,1),indicatorSize);
			indicatorPanel->layout(tutorialStepGrid->getLastPosition(),indicatorSize);
		}
	}
	else
	{
		indicatorPanel->setVisible(false);
	}

	if (stepNum >= 0 && stepNum < currentTutorial.size())
	{
		PanelBase * selectedPanel  = dynamic_cast<PanelBase*>(tutorialStepGrid->getChildren()->at(stepNum));
		if (selectedPanel != NULL)
		{
			TutorialStep * activeStep = currentTutorial.at(stepNum);
			activeStepPanel->setVisible(true);
			cv::Size2f indicatorSize = selectedPanel->getMeasuredSize();
			activeStepPanel->setAnimateOnLayout(false);
			//activeStepPanel->setBackgroundColor(activeStepPanel->getBackgroundColor().withAlpha(activeStep->getProgress()));
			indicatorSize.width *= min<float>(1.0f,activeStep->getProgress());
			//activeStepPanel->layout(selectedPanel->getLastPosition() - Vector(indicatorSize.width,0,1),indicatorSize);
			activeStepPanel->layout(selectedPanel->getLastPosition(),indicatorSize);
		}
	}
	else
	{
		activeStepPanel->setVisible(false);
	}
}

void InteractionsTutorial::onFrame(const Controller & controller)
{
	if (stepNum >= 0 && stepNum < currentTutorial.size())
	{
		int i = currentTutorial.at(stepNum)->getFirstDependent();
		for (;i<=stepNum && i < currentTutorial.size();i++)
		{
			if (i == stepNum)
			{
				if (currentTutorial.at(i)->onFrame(controller))
				{
					nextStep();
					break;
				}
				else
					this->layoutDirty = true;
			}
			else
			{
				if (!currentTutorial.at(i)->isComplete(controller))
				{
					setActiveStep(i);
					break;
				}
			}
		}
	}
}

void InteractionsTutorial::onGlobalGesture(const Controller & controller, std::string gestureType)
{

}

void InteractionsTutorial::getTutorialDescriptor(vector<string> & tutorial)
{

}

void InteractionsTutorial::onGlobalFocusChanged(bool focused)
{
	if (focused)
		SwipeGestureDetector::getInstance().setFlyWheel(scrollView->getFlyWheel());
}


void InteractionsTutorial::startCurrentTutorial()
{	
	tutorialStepGrid->clearChildren();
	for (auto it = currentTutorial.begin(); it != currentTutorial.end();it++)
	{		
		TextPanel * stepView = new TextPanel((*it)->getDescription());
		stepView->setStyle(GlobalConfig::tree()->get_child("InteractiveTutorial.StepStyle"));
		tutorialStepGrid->addChild(stepView);
	}
	tutorialStepGrid->addChild(nextTutorialButton);
	stepNum = -1;
	nextStep();
}

void InteractionsTutorial::setActiveStep(int index)
{
	stepNum = index;
	TutorialStep * currentStep = currentTutorial.at(stepNum);
	currentStep->reset();
	TextPanel * stepView;
	if (stepNum < tutorialStepGrid->getChildren()->size())
	{
		stepView = (TextPanel*)tutorialStepGrid->getChildren()->at(stepNum);
	}
	this->layoutDirty = true;
}

void InteractionsTutorial::nextStep()
{
	stepNum++;
	if (stepNum < currentTutorial.size())
	{
		setActiveStep(stepNum);
	}
	else
	{
		stepNum = currentTutorial.size();
	}
	this->layoutDirty = true;
}