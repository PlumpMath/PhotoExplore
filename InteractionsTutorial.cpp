#include "InteractionsTutorial.hpp"
#include "SwipeGestureDetector.hpp"
#include "LeapInput.hpp"
#include "TouchScrollSteps.hpp"
#include "SwipeTutorialSteps.hpp"
#include "ShakeTutorialSteps.hpp"
#include "PointTutorialSteps.hpp"
#include "CustomGrid.hpp"

InteractionsTutorial::InteractionsTutorial()
{
	selectedView = NULL;
	indicatorPanel = new FloatingPanel(0,0,Vector());
	indicatorPanel->setAnimateOnLayout(false);
	indicatorPanel->setBackgroundColor(Color(GlobalConfig::tree()->get_child("InteractiveTutorial.FinishedStepColor")));

	activeStepPanel = new FloatingPanel(0,0,Vector());
	activeStepPanel->setAnimateOnLayout(false);
	activeStepPanel->setBackgroundColor(Color(GlobalConfig::tree()->get_child("InteractiveTutorial.ActiveStepColor")));

	exitButton = new Button(" ");
	exitButton->setStyle(GlobalConfig::tree()->get_child("InteractiveTutorial.ExitButton"));
	exitButton->elementClickedCallback = [this](LeapElement * e){
		this->viewFinishedCallback("done");
	};

	init();

	loadTutorial("intro");
}

void InteractionsTutorial::loadTutorial(string name)
{	
	//if (!completionDelay.elapsed())
	//	return;

	currentTutorial.clear();
	
	startTutorialButton->setText("Begin");	
	startTutorialButton->elementClickedCallback = [this](LeapElement * clicked){
		this->state = Tutorial;
		this->startCurrentTutorial();
		this->layoutDirty = true;
	};
	nextTutorialButton->setText("Skip");

	vector<string> tutorialImages;
	if (name.compare("intro") == 0)
	{
		titlePanel->setText("PhotoExplore Tutorial");
		infoPanel->setText(GlobalConfig::tree()->get<string>("InteractiveTutorial.InfoStrings.Intro"));
		
		startTutorialButton->elementClickedCallback = [this](LeapElement * clicked){
			this->loadTutorial("touchscroll");
		};
		nextTutorialButton->elementClickedCallback = [this](LeapElement * clicked){
			this->loadTutorial("touchscroll");
		};
	}
	else if (name.compare("touchscroll") == 0)
	{
		titlePanel->setText("Touch Scrolling");
		infoPanel->setText(GlobalConfig::tree()->get<string>("InteractiveTutorial.InfoStrings.TouchScroll"));

		currentTutorial.push_back(new TouchScrollTutorial::SpreadHand());
		currentTutorial.push_back(new TouchScrollTutorial::Step2());
		currentTutorial.push_back(new TouchScrollTutorial::Step3());
		currentTutorial.push_back(new TouchScrollTutorial::Step4());

		tutorialImages.push_back("swipe");

		nextTutorialButton->elementClickedCallback = [this](LeapElement * clicked){
			this->loadTutorial("swipe");
		};
	}
	else if (name.compare("swipe") == 0)
	{		
		titlePanel->setText("Swiping");
		infoPanel->setText(GlobalConfig::tree()->get<string>("InteractiveTutorial.InfoStrings.Swipe"));		

		currentTutorial.push_back(new TouchScrollTutorial::SpreadHand());
		currentTutorial.push_back(new SwipeTutorial::SwipeLeft());
		currentTutorial.push_back(new SwipeTutorial::SwipeRight());
		currentTutorial.push_back(new SwipeTutorial::SwipeLeftFast());
				
		tutorialImages.push_back("swipe");

		nextTutorialButton->elementClickedCallback = [this](LeapElement * clicked){
			this->loadTutorial("shake");
		};
	}	
	//else if (name.compare("select") == 0)
	//{		
	//	titlePanel->setText("Selecting");
	//	currentTutorial.push_back(new PointTutorial::PointHand());
	//	currentTutorial.push_back(new PointTutorial::Press());
	//	currentTutorial.push_back(new PointTutorial::Release());
	//			
	//	vector<string> tutorialImages;
	//	tutorialImages.push_back("point");
	//	LeapDebug::instance->setTutorialImages(tutorialImages);

	//	nextTutorialButton->elementClickedCallback = [this](LeapElement * clicked){
	//		this->loadTutorial("touchscroll");
	//	};
	//}
	else if (name.compare("shake") == 0)
	{		
		titlePanel->setText("Shake");
		infoPanel->setText(GlobalConfig::tree()->get<string>("InteractiveTutorial.InfoStrings.Shake"));

		currentTutorial.push_back(new TouchScrollTutorial::SpreadHand());
		currentTutorial.push_back(new ShakeTutorial::PullBack());
		currentTutorial.push_back(new ShakeTutorial::Shake());
				
		tutorialImages.push_back("shake");
		
		nextTutorialButton->elementClickedCallback = [this](LeapElement * clicked){
			this->loadTutorial("complete");
		};
	}
	else if (name.compare("complete") == 0)
	{
		infoPanel->setText(GlobalConfig::tree()->get<string>("InteractiveTutorial.InfoStrings.Complete"));
		startTutorialButton->setText("Exit Tutorial");
		nextTutorialButton->setText("Repeat Tutorial");
		nextTutorialButton->elementClickedCallback = [this](LeapElement * clicked){
			this->loadTutorial("touchscroll");
		};

		startTutorialButton->elementClickedCallback = [this](LeapElement * clicked){
			this->viewFinishedCallback("done");
		};
	}
	else
	{
		throw new std::runtime_error("Invalid tutorial name " + name);
	}

	LeapDebug::instance->setTutorialImages(tutorialImages);
	state = Dialog;
	this->layoutDirty = true;
}

void InteractionsTutorial::init()
{
	generatePanels(cv::Size2i(0,4));
	LeapInput::getInstance()->requestGlobalGestureFocus(this);
	
	titlePanel = new TextPanel();
	titlePanel->setStyle(GlobalConfig::tree()->get_child("InteractiveTutorial.TitlePanel"));
	addChild(titlePanel);

	infoPanel = new TextPanel("");
	infoPanel->setStyle(GlobalConfig::tree()->get_child("InteractiveTutorial.InfoPanel"));
	//infoPanel->setLayoutParams(LayoutParams(cv::Size2f(0,0),cv::Vec4f(5,5,5,5)));

	startTutorialButton = new Button("");
	startTutorialButton->setStyle(GlobalConfig::tree()->get_child("InteractiveTutorial.StartButton"));
	startTutorialButton->elementClickedCallback = [this](LeapElement * clicked){
		this->state = Tutorial;
		this->startCurrentTutorial();
		this->layoutDirty = true;
	};
	startTutorialButton->setLayoutParams(LayoutParams(cv::Size2f(0,0),cv::Vec4f(2,2,2,2)));
	
	vector<RowDefinition> gridDefinition;	
	gridDefinition.push_back(RowDefinition(.7f));
	gridDefinition.push_back(RowDefinition(.3f));
	gridDefinition[0].ColumnWidths.push_back(1);
	gridDefinition[1].ColumnWidths.push_back(1);

	infoDialog = new CustomGrid(gridDefinition);

	infoDialog->addChild(infoPanel);
	infoDialog->addChild(startTutorialButton);
	
	addChild(infoDialog);
	
	nextTutorialButton = new Button("");
	nextTutorialButton->setStyle(GlobalConfig::tree()->get_child("InteractiveTutorial.NextButton"));

	tutorialStepGrid = new UniformGrid(cv::Size2i(1,5));
		
	addChild(indicatorPanel);
	addChild(activeStepPanel);
	addChild(exitButton);
	addChild(nextTutorialButton);

	addChild(tutorialStepGrid);
	
	float menuHeight = GlobalConfig::tree()->get<float>("Menu.Height");
	float tutorialHeight = GlobalConfig::tree()->get<float>("Tutorial.Height");

	this->layout(Vector(0,menuHeight,0),cv::Size2f(GlobalConfig::ScreenWidth,GlobalConfig::ScreenHeight-(menuHeight+tutorialHeight)));
}


void InteractionsTutorial::generatePanels(cv::Size2i gridSize)
{
	float numPanels = gridSize.height;
	FixedAspectGrid * floatLayout = new FixedAspectGrid(cv::Size2i(0,(int)numPanels),1.0f);
	float totalWidth = GlobalConfig::ScreenWidth*10.0f;
	
	float menuHeight = GlobalConfig::tree()->get<float>("Menu.Height");
	float tutorialHeight = GlobalConfig::tree()->get<float>("Tutorial.Height");

	float totalHeight = GlobalConfig::ScreenHeight-(menuHeight+tutorialHeight);

	floatLayout->setLayoutParams(cv::Size2f(totalWidth,totalHeight));

	float panelWidth =totalHeight/numPanels;
	
	int itemCount = ceilf(totalWidth/panelWidth) * numPanels;
	float spacing = 5;

	float anglePerRow = 360.0f / ceilf(totalWidth/panelWidth);

	for (int i =0; i < itemCount;i++)
	{
		PanelBase * fp;
		fp = new FloatingPanel(100,100,Vector());

		float angle = anglePerRow * (float)(i/((int)numPanels));


		float r = 50; //180 + (20.0f*abs(sinf(angle*2.0f*GeomConstants::DegToRad)));
		float g = 50;// 180 + (20.0f*abs(sinf(angle*4.0f*GeomConstants::DegToRad)));
		float b = 150 + 105.0f*abs(sinf(angle*2.0f*GeomConstants::DegToRad));
		float a = 100 + 155.0f*abs(sinf(angle*.5f*GeomConstants::DegToRad));
		fp->setBackgroundColor(Color((int)r,(int)g,(int)b,(int)a));

		fp->setLayoutParams(LayoutParams(cv::Size2f(0,0),cv::Vec4f(spacing,spacing,spacing,spacing)));
		fp->setBorderThickness(0);
		fp->NudgeAnimationEnabled = true;

		floatLayout->addChild(fp);
	}	
	scrollView = new ScrollingView(floatLayout);
	scrollView->layout(Vector(0,menuHeight,0),cv::Size2f(GlobalConfig::ScreenWidth,totalHeight));
	scrollView->getFlyWheel()->setBoundaryMode(FlyWheel::WrapAround);
	scrollView->getFlyWheel()->overrideValue(scrollView->getFlyWheel()->getMinValue()/2.0f);
	addChild(scrollView);
}

void InteractionsTutorial::layout(Vector position, cv::Size2f size)
{	
	
	float menuHeight = GlobalConfig::tree()->get<float>("Menu.Height");
	float progressBarRC = GlobalConfig::tree()->get<float>("InteractiveTutorial.ProgressBarRC");

	lastPosition = position;
	lastSize = size;
	
	cv::Size2f titleSize = cv::Size2f(size.width*.4f,menuHeight);
	titlePanel->layout(position + Vector((size.width-titleSize.width)*.5f,-menuHeight+10,0),titleSize);

	cv::Size2f exitButtonSize = cv::Size2f(size.width*.2f,size.height*.2f);
	exitButton->layout(position + Vector(0,size.height - exitButtonSize.height,1),exitButtonSize);

	exitButtonSize = cv::Size2f(size.width*.2f,size.height*.2f);
	nextTutorialButton->layout(position + Vector(size.width - exitButtonSize.width,size.height - exitButtonSize.height,1),exitButtonSize);

	cv::Size2f infoPanelSize = cv::Size2f(size.width*.3f,size.height*.5f);
	if (state == Dialog)
	{
		infoDialog->layout(position + Vector(size.width*.5f - infoPanelSize.width*.5f,size.height*.2,10),infoPanelSize);
		tutorialStepGrid->layout(position + Vector(size.width*1.2f,size.height*.2f,10),cv::Size2f(size.width*.4f,size.height*.6f));
		indicatorPanel->setVisible(false);
		activeStepPanel->setVisible(false);
	}
	else
	{
		infoDialog->layout(position + Vector(-infoPanelSize.width*1.2f,size.height*.2,10),infoPanelSize);		
		tutorialStepGrid->layout(position + Vector(size.width*.5f,size.height*.2f,10),cv::Size2f(size.width*.4f,size.height*.6f));

		if (stepNum >= 0 && stepNum < currentTutorial.size() && stepNum < tutorialStepGrid->getChildren()->size())
		{
			PanelBase * selectedPanel  = dynamic_cast<PanelBase*>(tutorialStepGrid->getChildren()->at(stepNum));
			if (selectedPanel != NULL)
			{
				TutorialStep * activeStep = currentTutorial.at(stepNum);
				activeStepPanel->setVisible(true);
				indicatorPanel->setVisible(true);

				cv::Size2f indicatorSize = selectedPanel->getMeasuredSize();

				((TextPanel*)selectedPanel)->setStyle(GlobalConfig::tree()->get_child("InteractiveTutorial.ActiveStepStyle"));
				selectedPanel->setBackgroundColor(Colors::Transparent);

				float targetWidth = indicatorSize.width;

				indicatorSize.width *= min<float>(1.0f,activeStep->getProgress());
				indicatorSize.width = max<float>(0.0f,indicatorSize.width);

				LeapHelper::lowpass(activeStepPanel->getWidth(),indicatorSize.width,progressBarRC,layoutTime.millis());

				activeStepPanel->layout(selectedPanel->getLastPosition()+Vector(0,0,0),indicatorSize);

				cv::Size2f backgroundSize = indicatorSize;
				backgroundSize.width = targetWidth - indicatorSize.width;

				indicatorPanel->setBackgroundColor(Color(GlobalConfig::tree()->get_child("InteractiveTutorial.ActiveStepStyle.BackgroundColor")));
				indicatorPanel->layout(selectedPanel->getLastPosition()+Vector(indicatorSize.width,0,0),backgroundSize);
			}
		}
		else
		{
			indicatorPanel->setVisible(false);
			activeStepPanel->setVisible(false);
		}
	}
	layoutTime.start();
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
	else if (completionDelay.elapsed() && state == Tutorial)
	{		
		nextTutorialButton->elementClicked();
	}
}

void InteractionsTutorial::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	if (stepNum < currentTutorial.size())
	{
		TutorialStep * activeStep = currentTutorial.at(stepNum);
		activeStep->onGlobalGesture(gestureType);
	}
}

void InteractionsTutorial::getTutorialDescriptor(vector<string> & tutorial)
{

}

void InteractionsTutorial::onGlobalFocusChanged(bool focused)
{
	if (focused)
		SwipeGestureDetector::getInstance().setFlyWheel(scrollView->getFlyWheel());
	else
	{
		SwipeGestureDetector::getInstance().setSwipeScrollingEnabled(true);
		SwipeGestureDetector::getInstance().setTouchScrollingEnabled(true);
	}
}


void InteractionsTutorial::startCurrentTutorial()
{	
	scrollView->getFlyWheel()->overrideValue(scrollView->getFlyWheel()->getMinValue()/2.0f);
	tutorialStepGrid->clearChildren();
	float b = 10;
	for (auto it = currentTutorial.begin(); it != currentTutorial.end();it++)
	{		
		TextPanel * stepView = new TextPanel((*it)->getDescription());
		stepView->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(b,b,b,b)));
		stepView->setStyle(GlobalConfig::tree()->get_child("InteractiveTutorial.StepStyle"));
		tutorialStepGrid->addChild(stepView);
	}
	
	nextTutorialButton->setStyle(GlobalConfig::tree()->get_child("InteractiveTutorial.NextButton"));
	//tutorialStepGrid->addChild(nextTutorialButton);
	stepNum = -1;
	nextStep();
}

void InteractionsTutorial::setActiveStep(int index)
{
	stepNum = index;
	TutorialStep * currentStep = currentTutorial.at(stepNum);
	currentStep->reset();
	for (int i=0;i<currentTutorial.size(); i++)
	{
		TextPanel * stepView = (TextPanel*)tutorialStepGrid->getChildren()->at(i);
		if (i<stepNum)
			stepView->setStyle(GlobalConfig::tree()->get_child("InteractiveTutorial.CompletedStepStyle"));
		else if (i==stepNum)
			stepView->setStyle(GlobalConfig::tree()->get_child("InteractiveTutorial.ActiveStepStyle"));
		else
			stepView->setStyle(GlobalConfig::tree()->get_child("InteractiveTutorial.StepStyle"));
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
		for (int i=0;i<=stepNum && i<currentTutorial.size() && i < tutorialStepGrid->getChildren()->size(); i++)
		{
			TextPanel * stepView = (TextPanel*)tutorialStepGrid->getChildren()->at(i);
			stepView->setStyle(GlobalConfig::tree()->get_child("InteractiveTutorial.CompletedStepStyle"));
		}
		completionDelay.countdown(1000);
	}
	this->layoutDirty = true;
}