#include "LeapStartScreen.h"
#include "FacebookBrowser.hpp"
#include "LinearLayout.hpp"
#include "FixedAspectGrid.hpp"
#include "GraphicContext.hpp"
#include "AbsoluteLayout.hpp"
#include "SwipeGestureDetector.hpp"


#if defined(_WIN32) 
	struct EnumWindowsCallbackArgs {
		EnumWindowsCallbackArgs( DWORD p ) : pid( p ) { }
		const DWORD pid;
		std::vector<HWND> handles;
	};

	static BOOL CALLBACK EnumWindowsCallback( HWND hnd, LPARAM lParam )
	{
		EnumWindowsCallbackArgs *args = (EnumWindowsCallbackArgs *)lParam;

		DWORD windowPID;
		(void)::GetWindowThreadProcessId( hnd, &windowPID );
		if ( windowPID == args->pid ) {
			args->handles.push_back( hnd );
		}

		return TRUE;
	}

	std::vector<HWND> getToplevelWindows()
	{
		EnumWindowsCallbackArgs args( ::GetCurrentProcessId() );
		if ( ::EnumWindows( &EnumWindowsCallback, (LPARAM) &args ) == FALSE ) {
			// XXX Log error here
			return std::vector<HWND>();
		}
		return args.handles;
	}
#endif

LeapStartScreen::LeapStartScreen(std::string startDir)
{
	state = StartState;
	this->startDir = startDir;
	loggedInNode = NULL;
	PointableElementManager::getInstance()->requestGlobalGestureFocus(this);

	updateTimer.start();
	lastHit = NULL;

	if (GlobalConfig::tree()->get<bool>("FakeDataMode.Enable")) {
		
		init();
		FBNode * root =new FBNode("human");
		root->setNodeType("me");
		FacebookDataDisplay::getInstance()->displayNode(root,"");
		rootView = (FacebookBrowser*) FacebookDataDisplay::getInstance();
		state = FinishedState;
	}
	else {
		init();
	}
}


void LeapStartScreen::setFinishedCallback(boost::function<void()>  _finishedCallback)
{
	this->finishedCallback = _finishedCallback;
}

void LeapStartScreen::shutdown()
{

}

LeapStartScreen::~LeapStartScreen()
{

}

LeapElement * LeapStartScreen::elementAtPoint(int x, int y, int & elementStateFlags)
{	
	LeapElement * hit = RadialMenu::instance->elementAtPoint(x,y,elementStateFlags);
	if (hit != NULL)
		return hit;

	if (state == FinishedState)
	{
		return ((FacebookBrowser*)rootView)->elementAtPoint(x,y,elementStateFlags);
	}
	else if (!GlobalConfig::tree()->get<bool>("LeapStartScreen.TrainingMode"))
	{		
		hit = facebookLoginButton->elementAtPoint(x,y,elementStateFlags);
		if (hit != NULL)return hit;
		
		return mainLayout->elementAtPoint(x,y,elementStateFlags);		
	} 
	return NULL;
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
	
	vector<RadialMenuItem> menuItems;
	radialMenu = new RadialMenu(menuItems);
	radialMenu->ItemClickedCallback = [this](string id) -> bool{

		if (id.compare("cancel") != 0)
		{
			GraphicsContext::getInstance().invokeGlobalAction(id);
		}
		return true;
	};

	radialMenu->layout(Vector(0,0,50),cv::Size2f(GlobalConfig::ScreenWidth,GlobalConfig::ScreenHeight));


	TextPanel * title = new TextPanel("Photo Explorer");
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
		
	if (!GlobalConfig::tree()->get<bool>("LeapStartScreen.TrainingMode"))
	{

		facebookLoginButton = new Button(GlobalConfig::tree()->get<string>("LeapStartScreen.StartButtonText"));		
		facebookLoginButton->setBackgroundColor(Colors::White); //Colors::SteelBlue.withAlpha(.8f));
		facebookLoginButton->setTextColor(Colors::Black);
		facebookLoginButton->setTextFitPadding(20);
		facebookLoginButton->setTextSize(18);	
		facebookLoginButton->setBorderThickness(1.0f);
		facebookLoginButton->setBorderColor(Colors::SteelBlue);	
		facebookLoginButton->NudgeAnimationEnabled = true;
		//facebookLoginButton->setDrawLoadAnimation(true);
		facebookLoginButton->loadAnimationColor = Colors::SteelBlue.withAlpha(.3f);
		facebookLoginButton->elementClickedCallback = [this](LeapElement * element){
			this->launchBrowser();	
		};
		facebookLoginButton->setLayoutParams(LayoutParams(cv::Size2f(600,600),cv::Vec4f(50,50,50,50)));

		noticePanel = new TextPanel(GlobalConfig::tree()->get<string>("LeapStartScreen.InternetNotice"));		
		noticePanel->setTextColor(Colors::White);
		noticePanel->setBackgroundColor(Colors::SteelBlue);
		noticePanel->setTextFitPadding(20);
		noticePanel->setTextSize(6);	
	}
	

	this->layout(Vector(),cv::Size2f(GlobalConfig::ScreenWidth, GlobalConfig::ScreenHeight));
		

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

		fp->setBorderThickness(.6f);
		fp->NudgeAnimationEnabled = true;

		floatLayout->addChild(fp);
	}	
	
	floatingPanelsView = new ScrollingView(floatLayout);
	floatingPanelsView->layout(Vector(),cv::Size2f(GlobalConfig::ScreenWidth,GlobalConfig::ScreenHeight));
	floatingPanelsView->getFlyWheel()->overrideValue(-GlobalConfig::ScreenWidth*.5f);

	SwipeGestureDetector::getInstance().setFlyWheel(floatingPanelsView->getFlyWheel());
	//	.setSwipeDetectedListener([this](Hand swipingHand, Vector swipeVector){

	//	float currentVelocity = floatingPanelsView->getFlyWheel()->getVelocity();

	//	if (abs(swipeVector.x) > abs(currentVelocity))
	//		floatingPanelsView->getFlyWheel()->setVelocity(swipeVector.x);

	//});
}	

void FloatingPanel::update(double deltaTime)
{		
}

void FloatingPanel::drawContent(Vector pos, float w, float h)
{

}

void FloatingPanel::draw()
{
	PanelBase::draw();
}
	
class MyVisitor : public CefCookieVisitor
{
public:
	bool Visit( const CefCookie& cookie, int count, int total, bool& deleteCookie )
	{
		deleteCookie = true;
		//cout << "Cookie[" << count << "]: Value = " << CefString(cookie.value.str).ToString() << " Domain = " << CefString(cookie.path.str).ToString() << "  Path = " << CefString(cookie.path.str).ToString() <<  "\n";
		return true;
	}

	IMPLEMENT_REFCOUNTING(MyVisitor);
};


void LeapStartScreen::deleteCookies()
{
	CefRefPtr<MyVisitor> v = new MyVisitor();
	CefCookieManager::GetGlobalManager()->VisitAllCookies(v.get());

}

void LeapStartScreen::onFrame(const Controller & controller)
{
	if (state == FinishedState)
	{
		((FacebookBrowser*)rootView)->onFrame(controller);
	}
	else if (state == StartState)
	{
		Frame frame = controller.frame();
		HandModel * handModel = HandProcessor::LastModel();
		
		Pointable intentPointable = frame.pointable(handModel->IntentFinger);
		
		Vector pt = LeapHelper::FindScreenPoint(controller,intentPointable);
		int flags = 0;
		LeapElement * element = floatingPanelsView->elementAtPoint((int)pt.x,(int)pt.y,flags);
		if (element != NULL)
			element->OnPointableEnter(intentPointable);


	}

}

void LeapStartScreen::layout(Leap::Vector pos, cv::Size2f size)
{
	this->lastPosition = pos;
	this->lastSize = size;

	mainLayout->layout(pos+Vector(0,0,2),size);

	if (!GlobalConfig::tree()->get<bool>("LeapStartScreen.TrainingMode"))
	{
		cv::Size2f buttonSize = cv::Size2f(size.width*.4f,size.height*.35f);
		facebookLoginButton->layout(Vector((size.width-buttonSize.width)*.5f,size.height*.30f,2)+pos,buttonSize);

		buttonSize = cv::Size2f(size.width*.3f,size.height*.15f);
		noticePanel->layout(Vector((size.width-buttonSize.width)*.5f,size.height*.70f,2)+pos,buttonSize);
	}
}

void LeapStartScreen::launchBrowser()
{		
	if (state != BrowserOpenState)
	{	
		facebookLoginButton->setText(GlobalConfig::tree()->get<string>("LeapStartScreen.BrowserLoginPrompt"));
		facebookLoginButton->reloadText();

		GlobalConfig::TestingToken = "";
		state = BrowserOpenState;

		facebookClient = new Cefalopod();

		CefBrowserSettings browserSettings;

		CefWindowInfo info;
		float windowWidth = GlobalConfig::ScreenWidth;
		float windowHeight = GlobalConfig::ScreenHeight;


	#if defined(_WIN32) // Windows
		vector<HWND> myHandles = getToplevelWindows();			
		info.SetAsPopup(0,"Login");
		info.width = (int)windowWidth;
		info.height = (int)windowHeight;
	#else			
		info.SetAsOffScreen(0);
	#endif

		string fbURL = "https://www.facebook.com/dialog/oauth?client_id=144263362431439&redirect_uri=http://144263362431439.com&scope=user_photos,friends_photos,user_likes,publish_stream&response_type=token";
		CefBrowserHost::CreateBrowser(info, facebookClient.get(),fbURL, browserSettings);
	}
}

void LeapStartScreen::elementClicked(LeapElement * element)
{
}

bool LeapStartScreen::onLeapGesture(const Controller & controller, const Gesture & gesture)
{
	if (radialMenu->checkMenuOpenGesture(gesture))
	{
		radialMenu->show();
		return true;
	}
	//else
	//{
	//	return floatingPanelsView->onLeapGesture(controller,gesture);	
	//}
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
			if (facebookClient->quit && !facebookClient->done)
			{	
				this->facebookLoginButton->setText("Login window closed, tap to retry.");
				this->facebookLoginButton->reloadText();
				this->state = StartState;
				this->facebookLoginButton->elementClickedCallback = [this](LeapElement * element){
					this->launchBrowser();
				};
			}
			else if (facebookClient->done)
			{		
				GlobalConfig::TestingToken = facebookClient->token;
				if (GlobalConfig::TestingToken.length() > 0)
				{
					if (!DestroyWindow(facebookClient->browserHandle))
					{
						Logger::stream("Cefalopod","ERROR") << "Couldn't destroy. " << GetLastError() << endl;
					}
					FBNode * root = new FBNode("me");
					root->setNodeType("me");
					
					rootView = (FacebookBrowser*) FacebookDataDisplay::getInstance();
					FacebookBrowser::getInstance()->displayNode(root,"");
					state = FinishedState;
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
		}

		mainLayout->update();
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
		floatingPanelsView->draw();
		mainLayout->draw();		
		if (!GlobalConfig::tree()->get<bool>("LeapStartScreen.TrainingMode") && facebookLoginButton->isVisible())
		{
			facebookLoginButton->draw();
			noticePanel->draw();
		}

	}

	radialMenu->draw();
}

