#include "FacebookIntroView.hpp"
#include "CustomGrid.hpp"
#include "GraphicContext.hpp"

using namespace cv;

FacebookIntroView::FacebookIntroView()
{
	friendPhotoGrid = new UniformGrid(Size2i(4,4));	
	friendPhotoGrid->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(0,0,10,0)));

	myPhotoGrid = new UniformGrid(Size2i(4,4));
	myPhotoGrid->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(10,0,0,0)));

	
	vector<RowDefinition> gridDefinition;	
	gridDefinition.push_back(RowDefinition(1));	
	gridDefinition[0].ColumnWidths.push_back(.5f);
	gridDefinition[0].ColumnWidths.push_back(.5f);

	mainLayout = new CustomGrid(gridDefinition);
		
	layoutDirty = true;
	lastPosition = Leap::Vector(0,0,0);
	lastSize = cv::Size2f(0,0);

	Color buttonColor = Colors::DarkBlue.withAlpha(.9f);

	photoButton = new Button("My Photos");
	photoButton->panelId = "my_photo_button";
	photoButton->setTextSize(12);
	photoButton->setTextColor(Colors::White);
	photoButton->setBackgroundColor(buttonColor);
	photoButton->setBorderThickness(1);
	photoButton->setBorderColor(Colors::HoloBlueBright);
	photoButton->elementClickedCallback = boost::bind(&FacebookIntroView::buttonClicked,this,_1);
	photoButton->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(50,50,50,50)));

	friendButton= new Button("Photos from my Friends");
	friendButton->panelId = "friend_button";
	friendButton->setTextSize(10);
	friendButton->setTextColor(Colors::White);
	friendButton->setBackgroundColor(buttonColor);	
	//friendButton->setBorderColor(Colors::HoloBlueBright);
	friendButton->setBorderThickness(0);
	friendButton->elementClickedCallback = boost::bind(&FacebookIntroView::buttonClicked,this,_1);
	friendButton->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(50,50,50,50)));
	
	friendListButton = new Button("Photos from my Friends");
	friendListButton->setTextSize(10);
	friendListButton->setTextColor(Colors::White);
	friendListButton->setBackgroundColor(buttonColor);	
	//friendListButton->setBorderColor(Colors::HoloBlueBright);
	friendListButton->setBorderThickness(0);
	friendListButton->elementClickedCallback = boost::bind(&FacebookIntroView::buttonClicked,this,_1);
	friendListButton->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(50,50,50,50)));
	
	
	
	mainLayout->addChild(friendPhotoGrid);
	mainLayout->addChild(myPhotoGrid);

	addChild(mainLayout);

	addChild(friendListButton);
	//addChild(friendButton);
	addChild(photoButton);

	vector<RadialMenuItem> menuItems;
	//menuItems.push_back(RadialMenuItem("Fullscreen Mode","full"));
	menuItems.push_back(RadialMenuItem("Exit Photo Explorer","exit",Colors::DarkRed));
	menuItems.push_back(RadialMenuItem("Cancel","cancel",Colors::OrangeRed));
	radialMenu = new RadialMenu(menuItems);
	radialMenu->setVisible(false);
	radialMenu->ItemClickedCallback = [this](string id) -> bool{

		if (id.compare("exit") == 0)
		{
			GraphicsContext::getInstance().invokeApplicationExitCallback();
		}
		else if (id.compare("full") == 0)
			GraphicsContext::getInstance().invokeGlobalAction("full");
		return true;
	};
	addChild(radialMenu);

}

void FacebookIntroView::setFinishedCallback(const boost::function<void(std::string)> & callback)
{
	finishedCallback = callback;
}

void FacebookIntroView::buttonClicked(LeapElement * element)
{
	for (auto it =  friendPhotoGrid->getChildren()->begin(); it != friendPhotoGrid->getChildren()->end();it++)
	{
		Panel * p = dynamic_cast<Panel*>(*it);

		if (p != NULL)
			p->setDataPriority(10);
	}
	friendPhotoGrid->clearChildren();

	for (auto it =  myPhotoGrid->getChildren()->begin(); it != myPhotoGrid->getChildren()->end();it++)
	{
		Panel * p = dynamic_cast<Panel*>(*it);

		if (p != NULL)
			p->setDataPriority(10);
	}
	myPhotoGrid->clearChildren();


	if (element == photoButton)
	{
		finishedCallback("my_photos");
	}
	else if (element == friendButton)
	{
		finishedCallback("friends_view");
	}
	else if (element == friendListButton)
	{
		finishedCallback("friend_list_view");
	}
}



void FacebookIntroView::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	;
}

bool FacebookIntroView::onLeapGesture(const Controller & controller, const Gesture & gesture)
{
	if (radialMenu->checkMenuOpenGesture(gesture))
		radialMenu->show();
	else
		return false;
}

void FacebookIntroView::show(FBNode * node)
{	
	PointableElementManager::getInstance()->requestGlobalGestureFocus(this);	
	friendPhotoGrid->clearChildren();
	myPhotoGrid->clearChildren();

	NodeQuerySpec friendConfig(2);	
	friendConfig.layers[0].insert(make_pair("friends",SelectionConfig(40)));
	friendConfig.layers[1].insert(make_pair("photos",SelectionConfig(1)));

	bool isLoading;
	viewChanged("friend_photos",DataViewGenerator::getInstance()->getDataView(node,friendConfig,[this](vector<FBNode*> & data2){
		this->viewChanged("friend_photos",data2);},isLoading));
		
	NodeQuerySpec myConfig(2);
	myConfig.layers[0].insert(make_pair("albums",SelectionConfig(10,0,false)));
	myConfig.layers[0].insert(make_pair("photos",SelectionConfig(5)));
	myConfig.layers[1].insert(make_pair("photos",SelectionConfig(5)));
	viewChanged("my_photos",DataViewGenerator::getInstance()->getDataView(node,myConfig,[this](vector<FBNode*> & data1){this->viewChanged("my_photos",data1);},isLoading));
}

void FacebookIntroView::viewChanged(string viewIdentifier, vector<FBNode*> & viewData)
{
	bool friendPhotos = viewIdentifier.compare("friend_photos") == 0;

	//if (friendPhotos)
	//	friendPhotoGrid->clearChildren();
	//else
	//	myPhotoGrid->clearChildren();


	std::random_shuffle(viewData.begin(),viewData.end());

	
	for (auto it = viewData.begin(); it != viewData.end(); it++)
	{
		if (friendPhotos && friendPhotoGrid->getChildren()->size() >= 16)
			break;
		else if (!friendPhotos && myPhotoGrid->getChildren()->size() >= 16)
			break;
		
		FBNode * node = (*it);
		if (node->getNodeType().compare(NodeType::FacebookImage) == 0)
		{
			View * item = ViewOrchestrator::getInstance()->requestView((*it)->getId(), NULL);

			if (item == NULL)
			{
				Panel * p = new Panel(0,0);
				p->setNode(node);
				p->setVisible(true);
				item = p;				
				p->setLayoutParams(LayoutParams(cv::Size2f(500,500)));
				ViewOrchestrator::getInstance()->registerView(node->getId(),item, NULL);
			}

			((Panel*)item)->setClickable(false);
			((Panel*)item)->setDataPriority(0);
			if (friendPhotos)
			{
 				friendPhotoGrid->addChild(item);
			}
			else
			{
				myPhotoGrid->addChild(item);
			}

		}
		//else if (node->getNodeType().compare(NodeType::FacebookFriend) == 0)
		//{
		//	NodeQuerySpec friendPhotos(1);
		//	friendPhotos.layers[0].insert(make_pair("photos",SelectionConfig(5)));
		//	bool isLoading;
		//	DataViewGenerator::getInstance()->getDataView(node,friendPhotos,[this](vector<FBNode*> & data){this->viewChanged("friend_photos",data);},isLoading);
		//}

	}

	layoutDirty = true;
}

void FacebookIntroView::layout(Leap::Vector position, Size2f size)
{
	lastPosition = position;
	lastSize = size;

	mainLayout->layout(position,size);
	radialMenu->layout(position,size);

	cv::Size2f buttonSize = cv::Size2f(size.width * .25f, size.height * .35f);
	friendListButton->layout(position + Leap::Vector(size.width*.125f, (size.height-buttonSize.height)*.5f,1), buttonSize);
	photoButton->layout(position + Leap::Vector(size.width*.625f,(size.height-buttonSize.height)*.5f,1), buttonSize);
	//friendListButton->layout(position + Leap::Vector(0,buttonSize.height + (size.height*.4f),1), buttonSize);
		
	layoutDirty = false;
}

//Note: This is a hack.
void FacebookIntroView::onFrame(const Controller & controller)
{	
	//HandModel * hm = HandProcessor::LastModel();	
	//Pointable testPointable = controller.frame().pointable(hm->IntentFinger);
	//
	//if (testPointable.isValid())
	//{
	//	Leap::Vector screenPoint = LeapHelper::FindScreenPoint(controller,testPointable);

	//	if (friendPhotoGrid->getHitRect().contains(cv::Point_<float>(screenPoint.x,screenPoint.y)))
	//	{
	//		friendScroll->OnPointableEnter(testPointable);
	//	}
	//	else if (myPhotoGrid->getHitRect().contains(cv::Point_<float>(screenPoint.x,screenPoint.y)))
	//	{
	//		photoScroll->OnPointableEnter(testPointable);
	//	}
	//}
}
