#include "LeapStartScreen.h"
#include "FacebookBrowser.hpp"
#include "LinearLayout.hpp"
#include "FixedAspectGrid.hpp"
#include "GraphicContext.hpp"
#include "AbsoluteLayout.hpp"
#include "SwipeGestureDetector.hpp"
#include "GLImport.h"
#include "InteractionsTutorial.hpp"
#include "WinHelper.hpp"
#include "FileBrowser.hpp"

#include <boost/thread.hpp>

LeapStartScreen::LeapStartScreen()
{
	state = StartState;
	tutorialButton = NULL;	
}


void LeapStartScreen::setFinishedCallback(boost::function<void()>  _finishedCallback)
{
	this->finishedCallback = _finishedCallback;
}

LeapStartScreen::~LeapStartScreen()
{

}

LeapElement * LeapStartScreen::elementAtPoint(int x, int y, int & elementStateFlags)
{
	if (state == FinishedState)
	{
		if (rootView != NULL)
			return rootView->elementAtPoint(x,y,elementStateFlags);
		else
			return NULL;
	}
	else
		return ViewGroup::elementAtPoint(x,y,elementStateFlags);	
}

void LeapStartScreen::generateBackgroundPanels()
{	

	float numPanels = 6;
	FixedAspectGrid * floatLayout = new FixedAspectGrid(cv::Size2i(0,(int)numPanels),1.0f);
	float totalWidth = GlobalConfig::ScreenWidth*10.0f;

	floatLayout->setLayoutParams(cv::Size2f(totalWidth,GlobalConfig::ScreenHeight));

	float panelWidth =((float)GlobalConfig::ScreenHeight)/numPanels;
	
	int itemCount = ceilf(totalWidth/panelWidth) * numPanels;
		
	std::srand(10);

	for (int i =0; i < itemCount;i++)
	{
		PanelBase * fp;
		fp = new FloatingPanel(100,100,Vector());
		if (std::rand() % 5 == 0)
		{
			fp->setBackgroundColor(Colors::LightSteelBlue);
			fp->setBorderColor(Colors::White);
		}
		else
		{
			fp->setBackgroundColor(Colors::White);
			fp->setBorderColor(Colors::LightSteelBlue);
		}
		
		float border = 0;
		fp->setLayoutParams(LayoutParams(cv::Size2f(0,0),cv::Vec4f(border,border,border,border)));
		fp->setBorderThickness(0.6f);
		fp->setUseLineBorder(true);
		fp->NudgeAnimationEnabled = true;

		floatLayout->addChild(fp);
	}	
	
	floatingPanelsView = new ScrollingView(floatLayout);
	floatingPanelsView->layout(Vector(),cv::Size2f(GlobalConfig::ScreenWidth,GlobalConfig::ScreenHeight));
	floatingPanelsView->getFlyWheel()->overrideValue(-GlobalConfig::ScreenWidth*.5f);
}

void LeapStartScreen::init()
{


	vector<RowDefinition> gridDefinition;
	
	gridDefinition.push_back(RowDefinition(.01f));
	gridDefinition.push_back(RowDefinition(.15f));
	gridDefinition.push_back(RowDefinition(.05f));
	gridDefinition.push_back(RowDefinition(.50f));

	gridDefinition[0].ColumnWidths.push_back(1);	
	gridDefinition[1].ColumnWidths.push_back(1);
	gridDefinition[2].ColumnWidths.push_back(1);
	gridDefinition[3].ColumnWidths.push_back(1);
	
	mainLayout = new CustomGrid(gridDefinition);	
	mainLayout->setEnabled(false);
	
	vector<RadialMenuItem> menuItems;
	radialMenu = new RadialMenu(menuItems);
	radialMenu->ItemClickedCallback = [this](string id) -> bool{

		if (id.compare("cancel") != 0)
		{
			GraphicsContext::getInstance().invokeGlobalAction(id);
		}
		return true;
	};
	
	TextPanel * title = new TextPanel("PhotoExplore");
	title->setTextFitPadding(10);
	title->setTextSize(30);
	title->setTextColor(Colors::Black);
	title->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(10,50,10,10)));

	TextPanel * subTitle = new TextPanel("for Facebook");
	subTitle->setTextSize(20);
	subTitle->setTextColor(Colors::Black);

	
	mainLayout->addChild(new TextPanel(""));
	mainLayout->addChild(title);
	mainLayout->addChild(subTitle);
	mainLayout->addChild(new TextPanel(""));
	
	facebookLoginButton = new Button(GlobalConfig::tree()->get<string>("LeapStartScreen.StartButtonText"));		
	facebookLoginButton->setBackgroundColor(Colors::White); 
	facebookLoginButton->setTextColor(Colors::Black);
	facebookLoginButton->setTextFitPadding(20);
	facebookLoginButton->setTextSize(18);	
	facebookLoginButton->setBorderThickness(1.0f);
	facebookLoginButton->setBorderColor(Colors::SteelBlue);	
	facebookLoginButton->NudgeAnimationEnabled = true;
	facebookLoginButton->elementClickedCallback = [this](LeapElement * element){
		this->launchBrowser();	
	};

	if (GlobalConfig::tree()->get<bool>("InteractiveTutorial.Enabled"))
	{
		tutorialButton = new Button("x");
		tutorialButton->NudgeAnimationEnabled = true;
		tutorialButton->setStyle(GlobalConfig::tree()->get_child("LeapStartScreen.TutorialButton"));
		tutorialButton->elementClickedCallback = [this](LeapElement * element){
			this->launchTutorial();	
		};
	}

	noticePanel = new TextPanel(GlobalConfig::tree()->get<string>("LeapStartScreen.InternetNotice"));		
	noticePanel->setTextColor(Colors::White);
	noticePanel->setBackgroundColor(Colors::SteelBlue);
	noticePanel->setTextFitPadding(20);
	noticePanel->setTextSize(6);	

	generateBackgroundPanels();
		
	addChild(radialMenu);
	addChild(floatingPanelsView);
	addChild(mainLayout);
	addChild(noticePanel);
	addChild(facebookLoginButton);
	
	if (tutorialButton != NULL)
		addChild(tutorialButton);

	this->layout(Vector(),cv::Size2f(GlobalConfig::ScreenWidth, GlobalConfig::ScreenHeight));
	
	if (GlobalConfig::tree()->get<bool>("FakeDataMode.FileBrowser")) {

		LeapInput::getInstance()->requestGlobalGestureFocus(this);

		rootView = new FileSystem::FileBrowser();
		state = FinishedState;
	}
	else if (GlobalConfig::tree()->get<bool>("FakeDataMode.Enable")) {

		LeapInput::getInstance()->requestGlobalGestureFocus(this);

		FBNode * root =new FBNode("human");
		root->setNodeType("me");
		FacebookDataDisplay::getInstance()->displayNode(root,"");
		rootView = (FacebookBrowser*) FacebookDataDisplay::getInstance();
		state = FinishedState;
	}
	else {
		LeapInput::getInstance()->requestGlobalGestureFocus(this);
	}	
}	

void LeapStartScreen::launchTutorial()
{
	state = FinishedState;
	InteractionsTutorial * t = new InteractionsTutorial();
	t->setViewFinishedCallback([this](string s){
		rootView = NULL;
		this->state = StartState;
		LeapInput::getInstance()->requestGlobalGestureFocus(this);
	});
	rootView = t;
}

void LeapStartScreen::onGlobalFocusChanged(bool focused)
{
	if (focused && floatingPanelsView != NULL)
	{
		SwipeGestureDetector::getInstance().setFlyWheel(floatingPanelsView->getFlyWheel());
	}
	else if (!focused)
	{
		SwipeGestureDetector::getInstance().setFlyWheel(NULL);
	}
}

void LeapStartScreen::onFrame(const Controller & controller)
{
	if (state == FinishedState)
	{
		((FacebookBrowser*)rootView)->onFrame(controller);
	}
}

void LeapStartScreen::layout(Leap::Vector pos, cv::Size2f size)
{
	this->lastPosition = pos;
	this->lastSize = size;

	if (state != FinishedState)
	{
		mainLayout->layout(pos+Vector(0,0,2),size);

		cv::Size2f buttonSize = cv::Size2f(size.width*.4f,size.height*.35f);
		facebookLoginButton->layout(Vector((size.width-buttonSize.width)*.5f,size.height*.30f,2)+pos,buttonSize);

	
		if (tutorialButton != NULL)
		{
			float rightEdge = (size.width + buttonSize.width)*.5f;
			buttonSize.width = (size.width-rightEdge) *.5f; 
			float x = rightEdge + (size.width-rightEdge)*.5f - buttonSize.width*.5f;
			tutorialButton->layout(Vector(x,size.height*.30f,2)+pos,buttonSize);
		}

		buttonSize = cv::Size2f(size.width*.3f,size.height*.15f);
		noticePanel->layout(Vector((size.width-buttonSize.width)*.5f,size.height*.70f,2)+pos,buttonSize);
	}
	else if (rootView != NULL)
	{
		rootView->layout(Vector(),size);
	}
	
	radialMenu->layout(Vector(0,0,50),cv::Size2f(GlobalConfig::ScreenWidth,GlobalConfig::ScreenHeight));
}

void LeapStartScreen::launchBrowser()
{
	string testToken = GlobalConfig::tree()->get<string>("FacebookAPI.DebugToken","");
	
	if (testToken.length() > 0)
	{
		startApplication(testToken);
	}
	else if (state != BrowserOpenState && state != FinishedState)
	{	
		facebookLoginButton->setText(GlobalConfig::tree()->get<string>("LeapStartScreen.BrowserLoginPrompt"));
		facebookLoginButton->reloadText();

		GlobalConfig::TestingToken = "";
		state = BrowserOpenState;
		
#ifdef _WIN32
		facebookClient = new Cefalopod();
		
		glfwIconifyWindow(GraphicsContext::getInstance().MainWindow);		

		CefBrowserSettings browserSettings;

		CefWindowInfo info;
		float windowWidth = GlobalConfig::ScreenWidth;
		float windowHeight = GlobalConfig::ScreenHeight;

		info.SetAsPopup(0,"Login");
		info.width = (int)windowWidth;
		info.height = (int)windowHeight;

		string fbURL = "https://www.facebook.com/dialog/oauth?client_id=144263362431439&redirect_uri=http://144263362431439.com&scope=user_photos,friends_photos,user_likes,publish_stream&response_type=token";
		CefBrowserHost::CreateBrowser(info, facebookClient.get(),fbURL, browserSettings);	
		
#else		
		//glFinish();
		//GraphicsContext::getInstance().invokeGlobalAction("hide");
		//glfwIconifyWindow(GraphicsContext::getInstance().MainWindow);
		
		new boost::thread([this](){
			
			string cefPath = GlobalConfig::tree()->get<string>("ExternalCef.CefClientPath");
			int code = system(cefPath.c_str());
					
			string tokenPath = GlobalConfig::tree()->get<string>("ExternalCef.TokenFilePath");
			ifstream readStream;
			readStream.open(tokenPath);
			
			string tok;
			readStream >> tok;
			
			readStream.close();
			std::remove(tokenPath.c_str());
			
			LeapStartScreen * me = this;
			this->postTask([me,tok](){
				
				GraphicsContext::getInstance().invokeGlobalAction("focus");
				//glfwRestoreWindow(GraphicsContext::getInstance().MainWindow);
				if (tok.length() > 0)
				{
					me->startApplication(tok);
				}
				else
				{
					me->facebookLoginButton->setText("Login failed, tap to retry");
					me->facebookLoginButton->reloadText();
					me->state = StartState;	
				}
			});
		});
#endif
	}
}

void LeapStartScreen::startApplication(std::string token)
{	
	GlobalConfig::TestingToken = token;
	FBNode * root = new FBNode("me");
	root->setNodeType("me");
	
	rootView = (FacebookBrowser*) FacebookDataDisplay::getInstance();
	FacebookBrowser::getInstance()->displayNode(root,"");
	state = FinishedState;
}


bool LeapStartScreen::onLeapGesture(const Controller & controller, const Gesture & gesture)
{
	return false;
}

void LeapStartScreen::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	if (gestureType.compare("shake") == 0)
	{
		ViewGroup * panels = (ViewGroup*)floatingPanelsView->getContent();
		std::random_shuffle(panels->getChildren()->begin(), panels->getChildren()->end());

		floatingPanelsView->layout(Vector(0,0,0),cv::Size2f(GlobalConfig::ScreenWidth,GlobalConfig::ScreenHeight));
	} 
	else if (gestureType.compare("pointing") == 0)
	{
		floatingPanelsView->getFlyWheel()->impartVelocity(0);
	}
}

void LeapStartScreen::getTutorialDescriptor(vector<string> & tutorial)
{	
	tutorial.push_back("point");
}

void LeapStartScreen::update(double delta)
{
	if (state == FinishedState)
	{
		if (rootView != NULL)
		{
			rootView->update();
		}
	}
	else
	{
		ViewGroup::update();
		
		if (state == BrowserOpenState)
		{
		
#ifdef _WIN32
			if (facebookClient->quit && !facebookClient->done)
			{					
				glfwRestoreWindow(GraphicsContext::getInstance().MainWindow);
				//GraphicsContext::getInstance().invokeGlobalAction("show");

				this->facebookLoginButton->setText("Login window closed, tap to retry.");
				this->facebookLoginButton->reloadText();
				this->state = StartState;
				this->facebookLoginButton->elementClickedCallback = [this](LeapElement * element){
					this->launchBrowser();
				};
			}
			else if (facebookClient->done)
			{
				glfwRestoreWindow(GraphicsContext::getInstance().MainWindow);				
				//GraphicsContext::getInstance().invokeGlobalAction("show");

				string token = facebookClient->token;
				if (token.length() > 0)
				{
					startApplication(token);
				}
				else
				{					
					this->facebookLoginButton->setText("Login failed. Tap to retry.");
					this->facebookLoginButton->reloadText();
					this->facebookLoginButton->elementClickedCallback = [this](LeapElement * element){
						this->launchBrowser();
					};
				}
			}
#endif
		}
	}
}

void LeapStartScreen::draw()
{
	if (state == FinishedState)
	{
		if (rootView != NULL)
			rootView->draw();
	}
	else
	{
		ViewGroup::draw();
	}

	radialMenu->draw();
}

