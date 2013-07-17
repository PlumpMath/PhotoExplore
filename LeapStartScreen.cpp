#include "LeapStartScreen.h"
#include "FacebookBrowser.hpp"
#include "LinearLayout.hpp"
#include "FixedAspectGrid.hpp"
#include "GraphicContext.hpp"
#include "AbsoluteLayout.hpp"


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
			//cout << "I have HWND: " << hnd << endl;
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
		rootView = new FacebookBrowser(new FBNode("human"));
		state = FinishedState;
	}
	else {
		init();
		doPreCheck();
	}
}

void LeapStartScreen::doPreCheck()
{
	//if (state == StartState)
	//{
	//	facebookLoginButton->setDrawLoadAnimation(true);
	//	state = PreCheckState;

	//	facebookPreCheckClient = CefRefPtr<Cefalopod>(new Cefalopod());

	//	CefBrowserSettings browserSettings;

	//	CefWindowInfo info;
	//	float windowWidth = GlobalConfig::ScreenWidth;
	//	float windowHeight = GlobalConfig::ScreenHeight;
	//	
	//	info.SetAsOffScreen(0);

	//	string fbURL = "https://www.facebook.com/dialog/oauth?client_id=144263362431439&redirect_uri=http://144263362431439.com&scope=user_photos,friends_photos&response_type=token";
	//	CefBrowserHost::CreateBrowser(info, facebookPreCheckClient.get(),fbURL, browserSettings);
	//}
}

void LeapStartScreen::setFinishedCallback(boost::function<void()>  _finishedCallback)
{
	this->finishedCallback = _finishedCallback;
}

void LeapStartScreen::shutdown()
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
	else 
	{		
		hit = facebookLoginButton->elementAtPoint(x,y,elementStateFlags);
		if (hit != NULL)return hit;
		
		return mainLayout->elementAtPoint(x,y,elementStateFlags);		
	} 
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
	menuItems.push_back(RadialMenuItem("Exit and Logout","logout", Colors::DarkRed));
	menuItems.push_back(RadialMenuItem("Exit Photo Explorer","exit",Colors::OrangeRed));
	menuItems.push_back(RadialMenuItem("Hide Tutorial","hide_tutorial", Colors::DarkTurquoise));
	menuItems.push_back(RadialMenuItem("Cancel","cancel",Colors::SkyBlue));
	radialMenu = new RadialMenu(menuItems);
	radialMenu->ItemClickedCallback = [this](string id) -> bool{

		if (id.compare("cancel") != 0)
		{
			GraphicsContext::getInstance().invokeGlobalAction(id);
		}
		return true;
	};

	radialMenu->layout(Vector(),cv::Size2f(GlobalConfig::ScreenWidth,GlobalConfig::ScreenHeight));


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
		
	
	facebookLoginButton = new Button("Start Exploring");		
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

	
	PointableElementManager::getInstance()->registerElement(mainLayout);

	this->layout(Vector(),cv::Size2f(GlobalConfig::ScreenWidth, GlobalConfig::ScreenHeight));
		

	FixedAspectGrid * floatLayout = new FixedAspectGrid(cv::Size2i(0,6),1.0f);
	floatLayout->setLayoutParams(cv::Size2f(GlobalConfig::ScreenWidth*2,GlobalConfig::ScreenHeight));

	int itemCount = (int)(((float)GlobalConfig::ScreenWidth/(GlobalConfig::ScreenHeight/6.0f))*1.5f*6.0f);
		
	//string panelText = "Photo Explorer";
	std::srand(10);

	//int letterIndex = 0;

	for (int i =0; i < itemCount;i++)
	{
		PanelBase * fp;
		//if (letterIndex < panelText.size() && i > (itemCount/24) && i % 6 == 1)
		//{
		//	fp = new TextPanel(panelText.substr(letterIndex++,1));
		//	((TextPanel*)fp)->setTextSize(50.0f);
		//}
		//else
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
}	

void FloatingPanel::update(double deltaTime)
{		
	//Vector velocity = Vector();

	//bool gravityWins = false;

	//if (gravityWellPresent)
	//{		
	//	Vector delta = gravityWellPosition - (position + Vector(getWidth(),getHeight(),0));

	//	float newSize = (pow(startSize*2.0f,2)/ delta.magnitudeSquared());

	//	this->backgroundColor.setAlpha(min<float>(1,max<float>(newSize,.5f)));
	//}
	//else
	//	this->backgroundColor.setAlpha(.5f);
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
		Vector pt = LeapHelper::FindScreenPoint(controller,frame.pointable(handModel->IntentFinger));
		int flags = 0;
		LeapElement * element = floatingPanelsView->elementAtPoint((int)pt.x,(int)pt.y,flags);
		if (element != NULL)
			element->OnPointableEnter(frame.pointable(handModel->IntentFinger));

	}

}

void LeapStartScreen::layout(Leap::Vector pos, cv::Size2f size)
{
	this->lastPosition = pos;
	this->lastSize = size;

	mainLayout->layout(pos,size);

	cv::Size2f buttonSize = cv::Size2f(size.width*.4f,size.height*.35f);
	facebookLoginButton->layout(Vector((size.width-buttonSize.width)*.5f,size.height*.30f,0)+pos,buttonSize);
	
	//buttonSize = cv::Size2f(400,200);
	//if (logoutButton->isVisible())
	//	logoutButton->layout(Vector((size.width*.25f)-(buttonSize.width*.5f),size.height*.3f,0)+pos,buttonSize);
	//else
	//	logoutButton->layout(Vector((size.width*.25f)-(buttonSize.width*.5f),size.height*.3f,0)+pos,cv::Size2f(0,0));
}

void LeapStartScreen::launchBrowser()
{		
	if (state != BrowserOpenState)
	{		
		facebookLoginButton->setDrawLoadAnimation(true);
		facebookLoginButton->setText("Please login using the browser popup.");
		facebookLoginButton->reloadText();

		GlobalConfig::TestingToken = "";
		state = BrowserOpenState;

		facebookClient = new Cefalopod();

		CefBrowserSettings browserSettings;

		CefWindowInfo info;
		//float windowWidth = GlobalConfig::ScreenWidth;
		//float windowHeight = GlobalConfig::ScreenHeight;


	#if defined(_WIN32) // Windows
		vector<HWND> myHandles = getToplevelWindows();			
		info.SetAsPopup(0,"Login");
		//info.width = 1000;
		//info.height = 600;
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
	else
	{
		return floatingPanelsView->onLeapGesture(controller,gesture);	
	}
}

void LeapStartScreen::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	if (gestureType.compare("shake") == 0)
	{
		ViewGroup * panels = (ViewGroup*)floatingPanelsView->getContent();
		std::random_shuffle(panels->getChildren()->begin(), panels->getChildren()->end());

		floatingPanelsView->layout(Vector(),cv::Size2f(GlobalConfig::ScreenWidth,GlobalConfig::ScreenHeight));

		//float padding = 10;
		//int gridSpace = ((GlobalConfig::ScreenWidth - (padding*2))/10);
		//float floatingPanelWidth = gridSpace;
		//float count =0;
		//for (int x=padding;x<GlobalConfig::ScreenWidth-padding;x += gridSpace)
		//{
		//	for (int y=padding;y<GlobalConfig::ScreenHeight-padding;y += gridSpace)
		//	{
		//		panels->getChildren()->at(count++)->layout(Vector(x+padding/2.0f,y+padding/2.0f,-1), cv::Size2f(floatingPanelWidth-padding,floatingPanelWidth-padding));				
		//	}
		//}	
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

		if (loggedInNode != NULL)
			loggedInNode->update();

		View::update();
		//double dT = updateTimer.millis();
		//for (int i=0;i<panelList.size();i++)
		//{
		//	panelList.at(i)->update(dT);
		//}
		//updateTimer.start();

		//if (fileBrowserCallback.get() != NULL)
		//{
		//	if (fileBrowserCallback->isDone())
		//	{
		//		startDir = fileBrowserCallback->result.at(0);

		//		boost::filesystem::path startPath = startDir;
		//		if (!boost::filesystem::is_directory(startPath))
		//			startPath = startPath.parent_path();

		//		//fileBrowser = new LeapFileBrowser(new FileNode(startPath));	
		//		//elementManager->unregisterElement(mainLayout);
		//		state = FinishedState;
		//	}
		//}
		if (state == PreCheckState)		
		{
			//if (facebookPreCheckClient->done || facebookPreCheckClient->loadedEnded)
			//{	
			//	facebookLoginButton->setDrawLoadAnimation(false);
			//	state = StartState;
			//	string tokenResult = facebookPreCheckClient->token;
			//	if (tokenResult.size() > 0)
			//	{
			//		loggedInNode = new FBNode("me");
			//		//facebookLoginButton->setText("Logged in as ...");

			//		loggedInNode->setLoadCompleteDelegate([&,tokenResult](){
			//			
			//			this->facebookLoginButton->setDrawLoadAnimation(false);
			//			
			//			LeapStartScreen * me = this;
			//			string nodeName = this->loggedInNode->getAttribute("name");
			//			if (nodeName.size() > 0)
			//			{
			//				string token2 = tokenResult;
			//				//me->statusPanel->setText("Logged in as " + nodeName ". );
			//				//me->statusPanel->reloadText();
			//				
			//				me->facebookLoginButton->setText("Logged in as " + nodeName + ". Tap here to Explore!");
			//				me->facebookLoginButton->reloadText();
			//				
			//				me->logoutButton->setText("Logout and Exit");
			//				me->logoutButton->setVisible(true);

			//				me->layoutDirty=true;

			//				me->facebookLoginButton->elementClickedCallback = [me,token2](LeapElement * element){
			//					GlobalConfig::TestingToken = token2;
			//					me->rootView = new FacebookBrowser(me->loggedInNode);
			//					me->state = me->FinishedState;
			//				};
			//			}
			//			me->loggedInNode->setLoadCompleteDelegate([](){});
			//		});
			//		
			//		facebookLoginButton->setDrawLoadAnimation(true);
			//		GlobalConfig::TestingToken = tokenResult;
			//		FacebookLoader::instance->loadField(loggedInNode ,"me?fields=id,name");
			//	}
			//	else
			//	{

			//		//this->statusPanel->setText("Tap or hover over the Login button to begin");
			//		//this->statusPanel->reloadText();

			//		this->logoutButton->setVisible(false);
			//		this->facebookLoginButton->setText("Tap or hover here to Login");
			//		this->facebookLoginButton->reloadText();
			//		this->facebookLoginButton->elementClickedCallback = [this](LeapElement * element){
			//			this->launchBrowser();
			//		};
			//	}
			//}
		}
		else if (state == BrowserOpenState)
		{
			if (facebookClient->quit)
			{
				this->finishedCallback();
			}
			else if (facebookClient->done)
			{		
				GlobalConfig::TestingToken = facebookClient->token;
				if (GlobalConfig::TestingToken.length() > 0)
				{
					//FBNode * myNode =;
					//FacebookLoader::instance->loadQuery(myNode ,"fql?q=SELECT%20uid%2C%20name%20FROM%20user%20WHERE%20uid%20%3D%20me()%0AOR%20uid%20IN%20(SELECT%20uid2%20FROM%20friend%20WHERE%20uid1%20%3D%20me())%20ORDER%20BY%20likes_count", "friends");
					//FacebookLoader::instance->load(myNode ,"?fields=albums.fields(id),friends.fields(id,name), photos.fields(id)");
					rootView = new FacebookBrowser( new FBNode("me"));
					
					//facebookClient->
					//facebookPreCheckClient->Release();
					state = FinishedState;
				}
				else
				{					
					//this->logoutButton->setVisible(false);
					this->facebookLoginButton->setText("Login error! Tap or hover here to retry.");
					this->facebookLoginButton->reloadText();
					this->facebookLoginButton->elementClickedCallback = [this](LeapElement * element){
						this->launchBrowser();
					};
					//this->doPreCheck();
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
		if (facebookLoginButton->isVisible())
			facebookLoginButton->draw();
	}

	radialMenu->draw();
}

