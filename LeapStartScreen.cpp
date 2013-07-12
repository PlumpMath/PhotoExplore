#include "LeapStartScreen.h"
#include "FacebookBrowser.hpp"
#include "LinearLayout.hpp"
#include "FixedAspectGrid.hpp"
#include "GraphicContext.hpp"


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
		rootView = new FacebookBrowser(createMockData());
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
	if (state == FinishedState)
	{
		((FacebookBrowser*)rootView)->elementAtPoint(x,y,elementStateFlags);
	}
	else // if (state == StartState)
	{
		//LeapElement * hit = logoutButton->elementAtPoint(x,y);
		//if (hit == NULL)
		//{
		LeapElement * hit = facebookLoginButton->elementAtPoint(x,y,elementStateFlags);
		if (hit == NULL)
		{
			return mainLayout->elementAtPoint(x,y,elementStateFlags);
		}
		return hit;
		//}
		//return hit;
	} 
}

FBNode * LeapStartScreen::createMockData()
{
	FBNode * human = new FBNode("fake");
	ImageManager::getInstance()->loadMockImage("","");

	vector<path> dirContents;
	boost::filesystem::path testPath_mine = boost::filesystem::path("C:\\Users\\Adam\\Pictures\\pixiv-win\\LeapTestImages");
	copy(directory_iterator(testPath_mine), directory_iterator(), back_inserter(dirContents));

	stringstream ss;
	ss << "fake_MY_ALBUM"; 

	FBNode * n1 = new FBNode(ss.str());
	n1->setNodeType("albums");
	human->Edges.insert(Edge("albums",n1));

	int photoIndex = 0;
	for (int j=0;j<1;j++)
	{				
		stringstream ss2;
		ss2 << "fake_MY_IMG_" << j; 

		FBNode * n2 = new FBNode(ss2.str());
		n2->setNodeType("photos");
		n1->Edges.insert(Edge("photos",n2));

		while ((is_directory(dirContents.at(photoIndex)) ||  
			dirContents.at(photoIndex).extension().string().size() < 3 || (
			dirContents.at(photoIndex).extension().string().find("jpg") == string::npos && 
			dirContents.at(photoIndex).extension().string().find("png") == string::npos)
			) && photoIndex++ < dirContents.size());


		if (j < dirContents.size())
			ImageManager::getInstance()->setImageRelevance(ss2.str(),16,1,0,dirContents.at(photoIndex%dirContents.size()).string(), cv::Size2i(500,500));
		photoIndex++;
	}

	int friendCount = 0, friendImageCount = 5, friendAlbumCount = 0;
	for (int i=0;i<friendCount;i++)
	{
		if (photoIndex >= dirContents.size())
		{
			cout << "Error! Ran out of fake images." << endl;
			break;
		}

		stringstream ss;
		ss << "fake_Friend " << i;
		FBNode * n2 = new FBNode(ss.str());
		n2->setNodeType("friends");
		human->Edges.insert(Edge("friends", n2));

		n2->Edges.insert(Edge("name",ss.str() + " Name"));
		bool profileLoaded = false;

		for (int a=0;a<friendAlbumCount+1;a++)
		{
			if (photoIndex >= dirContents.size())
			{
				cout << "Error! Ran out of fake images." << endl;
				break;
			}

			FBNode * n1 = NULL;
			if (a > 0)			
			{
				stringstream ss3;
				ss3 << ss.str() << " Album " << a;
				n1 =new FBNode(ss3.str());
				n1->setNodeType("albums");
				n2->Edges.insert(Edge("albums",n1));
			}			

			for (int j=0;j<friendImageCount;j++)
			{
				if (photoIndex >= dirContents.size())
				{
					cout << "Error! Ran out of fake images." << endl;
					break;
				}


				stringstream ss2;
				ss2 << "fake_IMG_" << photoIndex;

				FBNode * n3 = new FBNode(ss2.str());
				n3->setNodeType("photos");

				n3->Edges.insert(Edge("name",ss2.str() + "-name"));

				if (n1 == NULL)
					n2->Edges.insert(Edge("photos",n3));
				else
					n1->Edges.insert(Edge("photos",n3));
						
				
				while ((is_directory(dirContents.at(photoIndex)) ||  
						dirContents.at(photoIndex).extension().string().size() < 3 || (
							dirContents.at(photoIndex).extension().string().find("jpg") == string::npos && 
							dirContents.at(photoIndex).extension().string().find("png") == string::npos)
					) && photoIndex++ < dirContents.size());

				if (photoIndex < dirContents.size())
				{
					if (a == 0 && j == 0 && !profileLoaded)
					{
						profileLoaded = true;
						j--;
						ImageManager::getInstance()->setImageRelevance(ss.str(),16,1,0,dirContents.at(photoIndex%dirContents.size()).string(),cv::Size2i(500,500));
						//ImageManager::getInstance()->loadMockImage(ss.str(),dirContents.at(photoIndex%dirContents.size()).string());
					}
					else
					{
						ImageManager::getInstance()->setImageRelevance(ss2.str(),16,1,0,dirContents.at(photoIndex%dirContents.size()).string(),cv::Size2i(500,500));
						//ImageManager::getInstance()->loadMockImage(ss2.str(),dirContents.at(photoIndex%dirContents.size()).string());
					}
					
				}
				else
				{
					cout << "Error! Ran out of fake images." << endl;
					break;
				}
				photoIndex++;
			}
		}
	}

	return human;
}


void LeapStartScreen::init()
{
	vector<RowDefinition> gridDefinition;
	
	gridDefinition.push_back(RowDefinition(.01f));
	gridDefinition.push_back(RowDefinition(.15f));
	gridDefinition.push_back(RowDefinition(.05f));
	gridDefinition.push_back(RowDefinition(.45f));
	gridDefinition.push_back(RowDefinition(.05f));
	gridDefinition.push_back(RowDefinition(.05f));
	gridDefinition.push_back(RowDefinition(.05f));
	gridDefinition.push_back(RowDefinition(.05f));
	gridDefinition.push_back(RowDefinition(.05f));
	//gridDefinition.push_back(RowDefinition(.04f));

	gridDefinition[0].ColumnWidths.push_back(1);	
	gridDefinition[1].ColumnWidths.push_back(1);
	gridDefinition[2].ColumnWidths.push_back(1);
	gridDefinition[3].ColumnWidths.push_back(1);
	gridDefinition[4].ColumnWidths.push_back(1);
	gridDefinition[5].ColumnWidths.push_back(1);
	gridDefinition[6].ColumnWidths.push_back(1);
	gridDefinition[7].ColumnWidths.push_back(1);
	gridDefinition[8].ColumnWidths.push_back(1);
	//gridDefinition[9].ColumnWidths.push_back(1);
	
	mainLayout = new CustomGrid(gridDefinition);	
	
	vector<RadialMenuItem> menuItems;
	//menuItems.push_back(RadialMenuItem("Fullscreen Mode","full"));
	menuItems.push_back(RadialMenuItem("Exit Photo Explorer","exit",Colors::DarkRed));
	menuItems.push_back(RadialMenuItem("Cancel","cancel",Colors::OrangeRed));
	radialMenu = new RadialMenu(menuItems);
	radialMenu->ItemClickedCallback = [this](string id) -> bool{

		if (id.compare("exit") == 0)
		{
			//this->onShakeGesture();
			this->finishedCallback();
		}
		else if (id.compare("full") == 0)
			GraphicsContext::getInstance().invokeGlobalAction("full");
		return true;
	};

	
	radialMenu->setVisible(false);
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
		
	//ViewGroup * buttonLayout = new LinearLayout(true);//cv::Size2i(0,1),1,true);


//	Button * localImageButton = new Button("Local Images");
//	localImageButton->setBackgroundColor(Colors::HoloBlueBright.withAlpha(.5f));
//	localImageButton->setBorderThickness(1);
//	localImageButton->setTextSize(20);
//	localImageButton->setTextColor(Colors::White);
//	localImageButton->setLayoutParams(LayoutParams(cv::Size2f(600,600),cv::Vec4f(50,50,50,50)));
//
//	localImageButton->elementClickedCallback = [this](LeapElement * element){
//		if (fileBrowserCallback.get() == NULL)
//		{
//			this->rootView = new FacebookBrowser(createMockData());
//			this->state = FinishedState;
////			this->fileBrowserCallback = new CefFileBrowser();
////			CefBrowserSettings browserSettings;
////			CefWindowInfo info;
////
////#if defined(_WIN32) // Windows
////			vector<HWND> myHandles = getToplevelWindows();
////			info.SetAsPopup(myHandles.at(0),"Choose Directory");
////#endif
////			CefBrowserHost::CreateBrowser(info, fileBrowserCallback.get(), "http://www.google.com", browserSettings);
//		}
//	};
	
	//facebookLoginButton = new Button("Tap or hover here to Login test test test test long short of cats moew for ioajwdoi 12i09j awodi wi");		
	facebookLoginButton = new TextPanel("Tap or hover here to Login");		
	facebookLoginButton->setBackgroundColor(Colors::SteelBlue.withAlpha(.8f));
	facebookLoginButton->setTextColor(Colors::White);
	facebookLoginButton->setTextFitPadding(20);
	facebookLoginButton->setTextSize(14);	
	facebookLoginButton->NudgeAnimationEnabled = true;
	facebookLoginButton->setDrawLoadAnimation(true);
	facebookLoginButton->loadAnimationColor = Colors::SteelBlue.withAlpha(.3f);
	facebookLoginButton->elementClickedCallback = [this](LeapElement * element){
		this->launchBrowser();	
	};
	facebookLoginButton->setLayoutParams(LayoutParams(cv::Size2f(600,600),cv::Vec4f(50,50,50,50)));

	//logoutButton = new Button("Logout and Exit");		
	//logoutButton ->setBackgroundColor(Colors::OrangeRed.withAlpha(.9f));
	//logoutButton ->setBorderThickness(2);
	////logoutButton ->setBorderColor(Colors::HoloBlueBright);
	//logoutButton ->setTextColor(Colors::White);
	//logoutButton->setTextSize(10);	
	//logoutButton->setVisible(false);
	//logoutButton->elementClickedCallback = [this](LeapElement * element){
	//	this->deleteCookies();
	//	this->doPreCheck();
	//	//this->state = StartState;
	//	//this->facebookLoginButton->setText("Login to Facebook");
	//	//this->facebookLoginButton->reloadText();
	//	//this->logoutButton->setVisible(false);

	//	//LeapStartScreen * me = this;
	//	//facebookLoginButton->elementClickedCallback = [me](LeapElement * element){
	//	//	me->launchBrowser();
	//	//	me->state = BrowserOpenState;
	//	//};

	//};
	//logoutButton->setLayoutParams(LayoutParams(cv::Size2f(600,600),cv::Vec4f(150,50,50,50)));

	
	TextPanel * helpPanel_1 = new TextPanel("1. Make a quick circle gesture to show the menu");
	helpPanel_1->setTextSize(8);
	helpPanel_1->setTextColor(Colors::Black);
	helpPanel_1->setBackgroundColor(Colors::WhiteSmoke.withAlpha(0));

	TextPanel * helpPanel_2 = new TextPanel("2. Swipe left and right with a spread hand to scroll. The cursor will be outlined in orange when a spread hand is detected.");
	helpPanel_2->setTextSize(8);
	helpPanel_2->setTextColor(Colors::Black);
	helpPanel_2->setBackgroundColor(Colors::WhiteSmoke.withAlpha(0));
	
	TextPanel * helpPanel_3 = new TextPanel("3. Use a pointed finger to select a photo or a button, and to stop scrolling. The cursor will be outlined in blue when a pointed finger is detected.");
	helpPanel_3->setTextSize(8);
	helpPanel_3->setTextColor(Colors::Black);
	helpPanel_3->setBackgroundColor(Colors::WhiteSmoke.withAlpha(0));
		
	TextPanel * helpPanel_4 = new TextPanel("4. Shake your hand or finger to go back");
	helpPanel_4->setTextSize(8);
	helpPanel_4->setTextColor(Colors::Black);
	helpPanel_4->setBackgroundColor(Colors::WhiteSmoke.withAlpha(0));

	TextPanel * helpPanel_6 = new TextPanel("5. Point with two hands to stretch selected photos");
	helpPanel_6->setTextSize(8);
	helpPanel_6->setTextColor(Colors::Black);
	helpPanel_6->setBackgroundColor(Colors::WhiteSmoke.withAlpha(0));

	
	mainLayout->addChild(helpPanel_1);
	mainLayout->addChild(helpPanel_2);
	//mainLayout->addChild(helpPanel_5);
	mainLayout->addChild(helpPanel_3);
	mainLayout->addChild(helpPanel_4);
	mainLayout->addChild(helpPanel_6);
	
	PointableElementManager::getInstance()->registerElement(mainLayout);

	this->layout(Vector(),cv::Size2f(GlobalConfig::ScreenWidth, GlobalConfig::ScreenHeight));
		
	float padding = 10;
	int gridSpace = ((GlobalConfig::ScreenWidth - (padding*2))/10);
	float floatingPanelWidth = gridSpace;
	float count =0;
	for (int x=padding;x<GlobalConfig::ScreenWidth-padding;x += gridSpace)
	{
		for (int y=padding;y<GlobalConfig::ScreenHeight-padding;y += gridSpace)
		{
			count++;
			FloatingPanel * fp = new FloatingPanel(floatingPanelWidth-padding,floatingPanelWidth-padding,Vector(x+padding/2.0f,y+padding/2.0f,-1));

			Color cb = Colors::White;
			cb.setAlpha(1);
			fp->setBackgroundColor(cb);		
			fp->setBorderColor(Colors::LightSteelBlue);
			fp->setBorderThickness(.6f);
			fp->NudgeAnimationEnabled = true;
			
			panelList.push_back(fp);
		}
	}	
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
		for (int j=0;j<panelList.size();j++)
		{			
			Vector pt = LeapHelper::FindScreenPoint(controller,frame.pointable(handModel->IntentFinger));
			int flags;
			LeapElement * e = panelList[j]->elementAtPoint((int)pt.x,(int)pt.y,flags);
			if (e != NULL)
				e->OnPointableEnter(frame.pointable(handModel->IntentFinger));
		}

	}

}

void LeapStartScreen::layout(Leap::Vector pos, cv::Size2f size)
{
	this->lastPosition = pos;
	this->lastSize = size;


	mainLayout->layout(pos,size);

	cv::Size2f buttonSize = cv::Size2f(size.width*.4f,size.height*.35f);
	facebookLoginButton->layout(Vector((size.width-buttonSize.width)*.5f,size.height*.25f,0)+pos,buttonSize);
	
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
	return false;
}

void LeapStartScreen::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	;
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
		double dT = updateTimer.millis();
		for (int i=0;i<panelList.size();i++)
		{
			panelList.at(i)->update(dT);
		}
		updateTimer.start();

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
		//if (facebookClient != NULL)
		//	facebookClient->draw();
				
		for (int i=0;i<panelList.size();i++)
		{
			panelList.at(i)->draw();
		}
		mainLayout->draw();
		
		if (facebookLoginButton->isVisible())
			facebookLoginButton->draw();
		
		//if (logoutButton->isVisible())
		//	logoutButton->draw();
		
		if (radialMenu->isVisible())
			radialMenu->draw();
	}
}

