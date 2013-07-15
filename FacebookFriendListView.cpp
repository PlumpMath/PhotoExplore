#include "FacebookFriendListView.hpp"
#include "FixedAspectGrid.hpp"
#include "GraphicContext.hpp"


FacebookFriendListView::FacebookFriendListView()
{
	vector<RowDefinition> gridDefinition;	
	gridDefinition.push_back(RowDefinition(.02f));
	gridDefinition.push_back(RowDefinition(.98f));
	gridDefinition[0].ColumnWidths.push_back(1);
	gridDefinition[1].ColumnWidths.push_back(1);
	mainLayout = new CustomGrid(gridDefinition);

	rowCount = GlobalConfig::tree()->get<int>("FriendListView.RowCount");

	friendGroup = new FixedAspectGrid(cv::Size2i(0,rowCount),1.6f, true);
	((FixedAspectGrid*)friendGroup)->setInteriorMarginsOnly(true);
		
	itemScroll = new ScrollingView(friendGroup);

	itemScroll->getFlyWheel()->overrideValue(0);
	mainLayout->addChild(new TextPanel(""));
	mainLayout->addChild(itemScroll);

	friendDetail = new FriendDetailView();	
	friendDetail->setVisible(false);

	vector<RadialMenuItem> menuItems;
	menuItems.push_back(RadialMenuItem("Exit Photo Explorer","exit", Colors::DarkRed));
	menuItems.push_back(RadialMenuItem("Exit and Logout","logout", Colors::DarkRed));
	menuItems.push_back(RadialMenuItem("Cancel","cancel",Colors::OrangeRed));
	radialMenu = new RadialMenu(menuItems);
	radialMenu->ItemClickedCallback = [this](string id) -> bool{

		if (id.compare("exit") == 0)
		{
			GraphicsContext::getInstance().invokeApplicationExitCallback();
		}
		else if (id.compare("logout") == 0)
		{
			GraphicsContext::getInstance().invokeGlobalAction("logout");
		}
		return true;
	};
	addChild(friendDetail);	
	addChild(mainLayout);
	addChild(radialMenu);


}

void FacebookFriendListView::show(FBNode * root)
{	
	PointableElementManager::getInstance()->requestGlobalGestureFocus(this);
	
	currentRightBoundary = 0;
	lastUpdatePos = 100000;
	items.clear();
	friendGroup->clearChildren();
	activeNode = root;
	//updateLoading();
}

void FacebookFriendListView::addNode(FBNode * node)//, vector<FBNode*> & viewData)
{
	if (items.count(node->getId()) == 0)
	{
		items.insert(make_pair(node->getId(),node));

		View * item = ViewOrchestrator::getInstance()->requestView(node->getId(), this);

		if (item == NULL)
		{
			item = new FriendPanel(cv::Size2f(600,400));					
			ViewOrchestrator::getInstance()->registerView(node->getId(),item, this);
		}

		item->elementClickedCallback = [this,node](LeapElement*element){
			this->friendPanelClicked((FriendPanel*)element, node);
		};
		item->setLayoutParams(LayoutParams(cv::Size2f(600,400),cv::Vec4f(5,5,5,5)));
		item->setVisible(true);
		((FriendPanel*)item)->show(node,[this,item](){

			float itemWidth = 1.6f * (lastSize.height/((float)rowCount));
			friendGroup->addChild(item);
			currentRightBoundary =  (itemWidth * ceilf((float)(friendGroup->getChildren()->size())/(float)rowCount));	
			this->layoutDirty = true;
		});

		
	}
}



void FacebookFriendListView::loadItems(int friends)
{
	int requestedFriends = activeNode->loadState["friends"].requestedCount;
	
	stringstream loadstr;
	loadstr << activeNode->getId() << "?fields=";

	if (friends > requestedFriends)
	{
		activeNode->loadState["friends"].requestedCount = friends;
		loadstr << "friends.offset(" << requestedFriends << ").limit(" << friends << ").fields(id,name)";
	}

	FacebookFriendListView * v = this;
	FBDataSource::instance->loadField(activeNode,loadstr.str(),"",[v](FBNode * _node){
			
		FacebookFriendListView * v2= v;
		v->postTask([v2,_node](){
			
			if (v2->activeNode == _node)
				v2->updateLoading();
		});		
	});
}



void FacebookFriendListView::updateLoading()
{
	float scrollPosition = itemScroll->getFlyWheel()->getPosition();
	cv::Size2f visibleSize = itemScroll->getMeasuredSize();

	lastUpdatePos = -scrollPosition;

	float leftBound = -scrollPosition;
	float rightBound = -scrollPosition + visibleSize.width;

	float itemWidth = 400;
		
	//cv::Size2f s = friendGroup->getMeasusredSize();
	Timer loadingTimer;
	loadingTimer.start();

	int loadMore = 0;
	float remainingPixels =  currentRightBoundary - rightBound;
	if (remainingPixels < (itemWidth * 1.0f))
	{
		loadMore = rowCount * 2;
		if (remainingPixels < 0)
			loadMore += ceilf(-remainingPixels/itemWidth)*rowCount;

		int itemCount = friendGroup->getChildren()->size() + loadMore;

		int availableFriends = activeNode->Edges.get<EdgeTypeIndex>().count("friends");

		auto friendNodes = activeNode->Edges.get<EdgeTypeIndex>().equal_range("friends");
		
		while (items.size() < itemCount)
		{
			if (friendNodes .first != friendNodes .second)
			{
				addNode(friendNodes .first->Node);
				friendNodes .first++;
			} else 
			{
				if (!activeNode->edgeLimitReached("photos"))
				{
					int loadFriends =  GlobalConfig::tree()->get<int>("FriendListView.FriendsPerRequest") + availableFriends;	
					loadItems(loadFriends);
					itemScroll->setDrawLoadingIndicator(2,Colors::HoloBlueBright);
				}
				else
					itemScroll->setDrawLoadingIndicator(1,Colors::DarkRed);

				break;
			}
		}						
	}
	
	loadingTimer.start();
	
	float peakPriority = (itemScroll->getMeasuredSize().width*.5f) - itemScroll->getFlyWheel()->getPosition();
	peakPriority -= itemScroll->getFlyWheel()->getVelocity() * GlobalConfig::tree()->get<float>("FriendListView.ScrollAheadTime");
	
	for (auto it = friendGroup->getChildren()->begin(); it != friendGroup->getChildren()->end();it++)
	{
		PanelBase * imagePanel = (PanelBase*)(*it);
		float targetPriority;

		float distance = abs((imagePanel->getPosition().x + imagePanel->getWidth()/2.0f) - peakPriority);

		targetPriority = min<float>(10,distance/1000.0f);

		if (dynamic_cast<FriendPanel*>(imagePanel) != NULL)
			((FriendPanel*)imagePanel)->setDataPriority(targetPriority);	
	}	
}

//void FacebookFriendListView::viewChanged(vector<FBNode*> & viewData)
//{
//	//itemScroll->setDrawLoadingIndicator(false,false);
//	for (auto it = viewData.begin(); it != viewData.end(); it++)
//	{
//		FBNode * node = (*it);
//		if (node->getNodeType().compare(NodeType::FacebookFriend) == 0)
//		{	
//			FBNode * node = (*it);
//			NodeQuerySpec friendConfig(2,3);
//			friendConfig.layers[0].insert(make_pair("photos",SelectionConfig(2)));
//			friendConfig.layers[0].insert(make_pair("albums",SelectionConfig(4,0,false)));
//			friendConfig.layers[1].insert(make_pair("photos",SelectionConfig(1)));
//			bool isLoading;
//			friendViewLoaded(node,DataViewGenerator::getInstance()->getDataView(node,friendConfig,[this,node](vector<FBNode*> & photoData){ this->friendViewLoaded(node,photoData);},isLoading));	
//		}
//	}
//	layoutDirty = true;
//}
//
//void FacebookFriendListView::updateLoading(Vector newPos,cv::Size2f visibleSize)
//{
//	float leftBound = -newPos.x;
//	float rightBound = -newPos.x + visibleSize.width;
//		
//
//	cv::Size2f s = friendGroup->getMeasuredSize();
//	float newDataPixels = rightBound - s.width;
//	if (newDataPixels > 0)
//	{
//		bool load = false;
//		NodeQuerySpec querySpec(2);				
//		
//		float itemWidth = 400.0f;
//		int loadMore = 3 * (newDataPixels/itemWidth);
//		if (friendLoadCount + loadMore > friendLoadTarget)
//		{
//			friendLoadTarget = friendLoadCount+loadMore;
//			//cout << "Loading friends to " << friendLoadTarget << endl;
//			load = true;
//			querySpec.layers[0].insert(make_pair("friends",SelectionConfig(friendLoadTarget,friendLoadCount,true)));
//		}
//
//		bool isLoading = false;
//		if (load)
//			viewChanged(DataViewGenerator::getInstance()->getDataView(activeNode,querySpec,[this](vector<FBNode*> & viewData){ this->viewChanged(viewData);},isLoading));	
//
//		//if (isLoading)
//		//	itemScroll->setDrawLoadingIndicator(true,false);
//	}
//
//	for (auto it = friendGroup->getChildren()->begin(); it != friendGroup->getChildren()->end();it++)
//	{
//		PanelBase * imagePanel = (PanelBase*)(*it);
//		float targetPriority;
//		float leftDist = leftBound - (imagePanel->getPosition().x + imagePanel->getWidth());
//		float rightDist = imagePanel->getPosition().x - rightBound;
//
//		if (itemScroll->getFlyWheel()->getVelocity() < 0)
//		{				
//			if (leftDist > 2000)
//			{
//				targetPriority = leftDist/4000.0f;
//			}
//			else if (rightDist < 2000)
//				targetPriority = 0;
//			else
//				targetPriority = rightDist/4000.0f;
//
//		}
//		else
//		{
//			if (rightDist > 2000)
//			{
//				targetPriority = rightDist/4000.0f;
//			}
//			else if (leftDist < 2000)
//				targetPriority = 0;
//			else
//				targetPriority = leftDist/4000.0f;
//
//		}
//
//		if (dynamic_cast<FriendPanel*>(imagePanel) != NULL)
//			((FriendPanel*)imagePanel)->setDataPriority(targetPriority);			
//	}
//}

void FacebookFriendListView::friendPanelClicked(FriendPanel * panel, FBNode * clicked)
{	
	float targetPriority = 10;
	for (auto it = friendGroup->getChildren()->begin(); it != friendGroup->getChildren()->end();it++)
	{		
		FriendPanel * imagePanel = (FriendPanel*)*it;	
		imagePanel->setDataPriority(targetPriority);	
	}
	
	mainLayout->setVisible(false);

	friendDetail->setFinishedCallback([panel,clicked,this](string tag){
		this->friendDetail->setVisible(false);
		this->mainLayout->setVisible(true);
		this->layoutDirty = true;
		panel->show(clicked,[](){});
		//this->show(this->activeNode);
	});

	friendDetail->setVisible(true);
	friendDetail->show(clicked);
	layoutDirty = true;
}

void FacebookFriendListView::layout(Vector position, cv::Size2f size)
{
	lastPosition = position;
	lastSize = size;
	
	if (friendDetail != NULL && friendDetail->isEnabled() && friendDetail->isVisible())
		friendDetail->layout(position,size);
	else
	{
		radialMenu->layout(position,size);
		mainLayout->layout(position,size);
	}
	layoutDirty = false;
}

void FacebookFriendListView::update()
{
	ViewGroup::update();
		
	double pos = itemScroll->getFlyWheel()->getPosition();
	if (abs(lastUpdatePos - (-pos)) > 100)
		updateLoading();
}

void FacebookFriendListView::onFrame(const Controller & controller)
{	
	if (friendDetail != NULL && friendDetail->isEnabled() && friendDetail->isVisible())
	{	
		friendDetail->onFrame(controller);
	}
	else
	{
		HandModel * hm = HandProcessor::LastModel();	
		Pointable testPointable = controller.frame().pointable(hm->IntentFinger);

		if (testPointable.isValid())
		{
			Leap::Vector screenPoint = LeapHelper::FindScreenPoint(controller,testPointable);

			if (friendGroup->getHitRect().contains(cv::Point_<float>(screenPoint.x,screenPoint.y)))
			{
				itemScroll->OnPointableEnter(testPointable);
			}	
		}
	}
}

void FacebookFriendListView::setFinishedCallback(const boost::function<void(std::string)> & callback)
{
	finishedCallback = callback;
}

void FacebookFriendListView::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	if (gestureType.compare("shake") == 0)
	{
		float targetPriority = 10;
		for (auto it = friendGroup->getChildren()->begin(); it != friendGroup->getChildren()->end();it++)
		{		
			FriendPanel * imagePanel = (FriendPanel*)*it;	
			imagePanel->setDataPriority(targetPriority);	
		}
		PointableElementManager::getInstance()->releaseGlobalGestureFocus(this);
		finishedCallback("done");
	} 
	else if (gestureType.compare("pointing") == 0)
	{
		itemScroll->getFlyWheel()->impartVelocity(0);
	}
}

bool FacebookFriendListView::onLeapGesture(const Controller & controller, const Gesture & gesture)
{
	if (radialMenu->checkMenuOpenGesture(gesture))
	{
		itemScroll->getFlyWheel()->impartVelocity(0);
		radialMenu->show();
		return true;
	}
	return itemScroll->onLeapGesture(controller, gesture);
}

void FacebookFriendListView::getTutorialDescriptor(vector<string> & tutorial)
{	
	tutorial.push_back("point_stop");
	tutorial.push_back("swipe");
	tutorial.push_back("shake");
}

