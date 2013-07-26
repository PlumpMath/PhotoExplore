#include "FacebookFriendListView.hpp"
#include "FixedAspectGrid.hpp"
#include "GraphicContext.hpp"
#include "SwipeGestureDetector.hpp"
#include "FacebookDataDisplay.hpp"
#include "AbsoluteLayout.hpp"
#include "ScrollBar.hpp"


FacebookFriendListView::FacebookFriendListView()
{

	ScrollBar * scrollBar = new ScrollBar();

	mainLayout = new AbsoluteLayout();

	((AbsoluteLayout*)mainLayout)->layoutCallback = [this,scrollBar](Vector position, cv::Size2f size){

		float tutorialHeight = GlobalConfig::tree()->get<float>("Tutorial.Height");
		float scrollBarHeight = GlobalConfig::tree()->get<float>("ScrollView.ScrollBar.Height");

		itemScroll->layout(position,size);

		float scrollBarWidth = size.width * 0.4f;

		scrollBar->layout(position + Vector((size.width-scrollBarWidth)*.5f,size.height+(tutorialHeight-scrollBarHeight)*.5f,1),cv::Size2f(scrollBarWidth,scrollBarHeight));
	};


	rowCount = GlobalConfig::tree()->get<int>("FriendListView.RowCount");

	friendGroup = new FixedAspectGrid(cv::Size2i(0,rowCount),1.0f, true);
	((FixedAspectGrid*)friendGroup)->setInteriorMarginsOnly(true);
		
	itemScroll = new ScrollingView(friendGroup);
	scrollBar->setScrollView(itemScroll);

	mainLayout->addChild(itemScroll);
	mainLayout->addChild(scrollBar);

	addChild(mainLayout);
}

void FacebookFriendListView::suspend()
{
	PointableElementManager::getInstance()->releaseGlobalGestureFocus(this);

	float targetPriority = 100;
	for (auto it = friendGroup->getChildren()->begin(); it != friendGroup->getChildren()->end();it++)
	{		
		FriendPanel * imagePanel = (FriendPanel*)*it;	
		imagePanel->setDataPriority(targetPriority);	
	}
}

void FacebookFriendListView::show(FBNode * root)
{	
	activeNode = root;

	PointableElementManager::getInstance()->requestGlobalGestureFocus(this);
	
	currentRightBoundary = 0;
	lastUpdatePos = 100000;
	itemScroll->getFlyWheel()->overrideValue(0);

	items.clear();
	friendGroup->clearChildren();
}

void FacebookFriendListView::addNode(FBNode * node)
{
	if (items.count(node->getId()) == 0)
	{
		if (node->Edges.get<EdgeTypeIndex>().count("photos") + node->Edges.get<EdgeTypeIndex>().count("albums") >= 2)
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

			((FriendPanel*)item)->show(node);
			float itemWidth = (lastSize.height/((float)rowCount));
			friendGroup->addChild(item);
			currentRightBoundary =  (itemWidth * ceilf((float)(friendGroup->getChildren()->size())/(float)rowCount));	
			this->layoutDirty = true;
		}
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
		
		FacebookFriendListView * v = this;
		FBDataSource::instance->loadField(activeNode,loadstr.str(),"",[v](FBNode * _node){
			
			auto fRange = v->activeNode->Edges.get<EdgeTypeIndex>().equal_range("friends");
			for (; fRange.first != fRange.second; fRange.first++)
			{
				if (fRange.first->Node->Edges.get<EdgeTypeIndex>().count("photos") < 2)
				{					
					stringstream load2;
					load2 << fRange.first->Node->getId() << "?fields=photos.fields(id,name,images).limit(2)";
					
					FacebookFriendListView * v2= v;
					FBDataSource::instance->loadField(fRange.first->Node,load2.str(),"",[v2](FBNode * nn)
					{
						FacebookFriendListView * v3= v2;
						v2->postTask([v3](){
							
							v3->itemScroll->setDrawLoadingIndicator(0,Colors::SteelBlue);
							v3->updateLoading();
						});	
					}); 
				}
			}				
		});
	}
}



void FacebookFriendListView::updateLoading()
{
	float scrollPosition = itemScroll->getFlyWheel()->getPosition();
	cv::Size2f visibleSize = itemScroll->getMeasuredSize();

	lastUpdatePos = -scrollPosition;

	float leftBound = -scrollPosition;
	float rightBound = -scrollPosition + visibleSize.width;

	float itemWidth = (visibleSize.height/((float)rowCount));

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
		
		Logger::stream("FriendListView","INFO") << "Setting friend count to " << itemCount << endl;

		while (items.size() < itemCount)
		{
			if (friendNodes.first != friendNodes.second)
			{
				addNode(friendNodes.first->Node);
				friendNodes.first++;
			} else 
			{
				if (!activeNode->edgeLimitReached("friends"))
				{
					int loadFriends =  GlobalConfig::tree()->get<int>("FriendListView.FriendsPerRequest") + availableFriends;	
					loadItems(loadFriends);
					itemScroll->setDrawLoadingIndicator(2,Colors::DimGray);
				}
				else
					itemScroll->setDrawLoadingIndicator(0,Colors::SteelBlue);

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


void FacebookFriendListView::friendPanelClicked(FriendPanel * panel, FBNode * clicked)
{	
	FacebookDataDisplay::getInstance()->displayNode(activeNode,panel->getNode(),"");
}

void FacebookFriendListView::layout(Vector position, cv::Size2f size)
{
	lastPosition = position;
	lastSize = size;
	
	mainLayout->layout(position,size);
	
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
	;
}

void FacebookFriendListView::setFinishedCallback(const boost::function<void(std::string)> & callback)
{
	finishedCallback = callback;
}

void FacebookFriendListView::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	if (gestureType.compare("shake") == 0)
	{
		finishedCallback("done");
	} 
	else if (gestureType.compare("pointing") == 0)
	{
		itemScroll->getFlyWheel()->impartVelocity(0);
	}
}

bool FacebookFriendListView::onLeapGesture(const Controller & controller, const Gesture & gesture)
{
	return itemScroll->onLeapGesture(controller, gesture);
}

void FacebookFriendListView::getTutorialDescriptor(vector<string> & tutorial)
{	
	tutorial.push_back("swipe");
	tutorial.push_back("point_stop");
	tutorial.push_back("shake");
}

void FacebookFriendListView::onGlobalFocusChanged(bool isFocused)
{
	if (isFocused)
		SwipeGestureDetector::getInstance().setFlyWheel(itemScroll->getFlyWheel());
	else
		SwipeGestureDetector::getInstance().setFlyWheel(NULL);
}
