#include "FriendDetailView.hpp"
#include "FixedAspectGrid.hpp"
#include "GraphicContext.hpp"
#include "AbsoluteLayout.hpp"
#include "SwipeGestureDetector.hpp"


FriendDetailView::FriendDetailView()
{

	rowCount = GlobalConfig::tree()->get<int>("FriendDetailView.RowCount");

	mainLayout = new AbsoluteLayout(); //(gridDefinition);	
	imageGroup = new FixedAspectGrid(cv::Size2i(0,rowCount),true);	
	itemScroll = new ScrollingView(imageGroup);
	friendNameHeading = NULL;

	mainLayout->addChild(itemScroll);


	imageDetailView = new ImageDetailView();
	imageDetailView->setVisible(false);
	imageDetailView->setFinishedCallback([this](string a){
		this->imageDetailView->setVisible(false);
		this->topView = mainLayout;
		this->layoutDirty = true;						
	});
	addChild(imageDetailView);

	albumDetail = new AlbumDetailView();	
	albumDetail->setVisible(false);
	addChild(albumDetail);	
	
	addChild(mainLayout);
	
	topView = mainLayout;
	projectedRightBoundary= 0;
}


void FriendDetailView::loadItems(int albums, int photos)
{
	int requestedAlbums = activeNode->loadState["albums"].requestedCount;
	int requestedPhotos = activeNode->loadState["photos"].requestedCount;
	
	bool load = false;
	stringstream loadstr;
	loadstr << activeNode->getId() << "?fields=";
	if (albums > requestedAlbums)
	{
		load = true;
		activeNode->loadState["albums"].requestedCount = albums;
		loadstr << "albums.offset(" << requestedAlbums << ").limit(" << albums << ").fields(id,name,photos.fields(id,name,images).limit(4))";
	}
	
	if (photos > requestedPhotos)
	{
		activeNode->loadState["photos"].requestedCount = photos;
		if (load)
			loadstr << ",";

		loadstr << "photos.offset(" << requestedPhotos << ").limit(" << photos << ").fields(id,name,images)";
		load = true;
	}

	if (load) 
	{
		FriendDetailView * v = this;
		FBDataSource::instance->loadField(activeNode,loadstr.str(),"",[v](FBNode * _node){

			FriendDetailView * v2 = v;
			v->postTask([v2,_node](){
						
				if (v2->activeNode == _node)
				{
					v2->itemScroll->setDrawLoadingIndicator(0,Colors::Transparent);
					v2->updateLoading();
				}
			});
		});
	}
}



void FriendDetailView::updateLoading()
{
	float scrollPosition = itemScroll->getFlyWheel()->getPosition();
	cv::Size2f visibleSize = itemScroll->getMeasuredSize();

	lastUpdatePos = -scrollPosition;

	float leftBound = -scrollPosition;
	float rightBound = -scrollPosition + visibleSize.width;

	float itemWidth = 400;
		
	cv::Size2f s = imageGroup->getMeasuredSize();
	Timer loadingTimer;
	loadingTimer.start();

	int loadMore = 0;
	float remainingPixels =  currentRightBoundary - rightBound;
	if (remainingPixels < (itemWidth * 1.0f))
	{
		loadMore = rowCount * 2;
		if (remainingPixels < 0)
			loadMore += ceilf(-remainingPixels/itemWidth)*rowCount;

		int itemCount = imageGroup->getChildren()->size() + loadMore;

		int availablePhotos = activeNode->Edges.get<EdgeTypeIndex>().count("photos");
		int availableAlbums = activeNode->Edges.get<EdgeTypeIndex>().count("albums");

		//if (itemCount > (availableAlbums + availablePhotos))
		auto photoNodes = activeNode->Edges.get<EdgeTypeIndex>().equal_range("photos");
		auto albumNodes = activeNode->Edges.get<EdgeTypeIndex>().equal_range("albums");
		
		while (items.size() < itemCount)
		{
			bool loaded = false;
			if (photoNodes.first != photoNodes.second)
			{
				addNode(photoNodes.first->Node);
				photoNodes.first++;
				loaded = true;
			}

			if (albumNodes.first != albumNodes.second)
			{
				addNode(albumNodes.first->Node);
				albumNodes.first++;
				loaded = true;
			}

			if (!loaded)
			{
				int loadAlbums = 0, loadPhotos = 0;
				if (!activeNode->edgeLimitReached("albums"))
				{
					loadAlbums = 25 + availableAlbums;
				}

				if (!activeNode->edgeLimitReached("photos"))
				{
					loadPhotos = 25 + availablePhotos;
				}

				if (loadAlbums > 0 || loadPhotos > 0)
				{
					loadItems(loadAlbums,loadPhotos);
					if (imageGroup->getMeasuredSize().width > itemScroll->getMeasuredSize().width)
						itemScroll->setDrawLoadingIndicator(1,Colors::DimGray);
				}
				else  if (imageGroup->getMeasuredSize().width > itemScroll->getMeasuredSize().width)				
					itemScroll->setDrawLoadingIndicator(0,Colors::DarkRed);

				break;
			}
		}						
		//Logger::stream("FriendDetailView","INFO") << "RightBound=" << rightBound << " RightProj = " << projectedRightBoundary << " LoadMore= " << loadMore << " remainingPixels=" << remainingPixels << endl;
	}

	if (loadingTimer.millis() > 0)
		Logger::stream("FriendDetailView","TIME") << "Count = " << imageGroup->getChildren()->size() << ". Item loading: " << loadingTimer.millis() << " ms" << endl;

	loadingTimer.start();
	
	float peakPriority = (itemScroll->getMeasuredSize().width*.5f) - itemScroll->getFlyWheel()->getPosition();
	peakPriority -= itemScroll->getFlyWheel()->getVelocity() * GlobalConfig::tree()->get<float>("FriendDetailView.ScrollAheadTime");
	
	for (auto it = imageGroup->getChildren()->begin(); it != imageGroup->getChildren()->end();it++)
	{
		PanelBase * imagePanel = (PanelBase*)(*it);
		float targetPriority;

		float distance = abs((imagePanel->getPosition().x + imagePanel->getWidth()/2.0f) - peakPriority);

		targetPriority = min<float>(10,distance/1000.0f);

		if (dynamic_cast<Panel*>(imagePanel) != NULL)
			((Panel*)imagePanel)->setDataPriority(targetPriority);	
		else if (dynamic_cast<AlbumPanel*>(imagePanel) != NULL)
		{
			AlbumPanel * ap = (AlbumPanel*)imagePanel;
			ap->setChildDataPriority(targetPriority);
		}
	}
	
	if (loadingTimer.millis() > 0)
		Logger::stream("FriendDetailView","TIME")  << "Count = " << imageGroup->getChildren()->size() << ". Priority assignment: " << loadingTimer.millis() << " ms" << endl;
}

bool FriendDetailView::onLeapGesture(const Controller & controller, const Gesture & gesture)
{
	return itemScroll->onLeapGesture(controller, gesture);
}

void FriendDetailView::getTutorialDescriptor(vector<string> & tutorial)
{
	tutorial.push_back("swipe");
	tutorial.push_back("point_stop");
	tutorial.push_back("shake");
}

void FriendDetailView::show(FBNode * root)
{	
	topView = mainLayout;
	topView->setVisible(true);
	currentRightBoundary= 0;
	items.clear();	
	lastUpdatePos = 1000;

		
	PointableElementManager::getInstance()->requestGlobalGestureFocus(this);
	SwipeGestureDetector::getInstance().setFlyWheel(itemScroll->getFlyWheel());

	//float menuHeight = GlobalConfig::tree()->get<float>("Menu.Height");
	//
	//((AbsoluteLayout*)mainLayout)->layoutCallback = [&menuHeight,this](Vector pos, cv::Size2f size){

	//	if (mainLayout->isVisible())
	//	{
	//		if (friendNameHeading != NULL && itemScroll != NULL && imageGroup != NULL)
	//		{			
	//			if (menuHeight == 0)
	//				menuHeight = 100;

	//			friendNameHeading->layout(pos-Vector(0,menuHeight,0),cv::Size2f(size.width,menuHeight));
	//			itemScroll->measure(size);
	//			itemScroll->layout(pos,size);
	//		}
	//	}
	//};
	//
	itemScroll->getFlyWheel()->overrideValue(0);

	activeNode = root;
	if (friendNameHeading != NULL)
		mainLayout->remove(friendNameHeading);

	
	View * item = ViewOrchestrator::getInstance()->requestView(root->getId() + "/name", this);
		
	if (item != NULL)
	{
		friendNameHeading = (TextPanel*)item;	
	}
	else
	{
		friendNameHeading = new TextPanel(root->getAttribute("name"));
		ViewOrchestrator::getInstance()->registerView(root->getId() + "/name",friendNameHeading,this);
	}
	
	boost::property_tree::ptree labelConfig = GlobalConfig::tree()->get_child("FriendDetailView.Title");
	friendNameHeading->setTextSize(labelConfig.get<float>("FontSize"),true);
	friendNameHeading->setTextFitPadding(labelConfig.get<float>("TextPadding"));
	friendNameHeading->setTextColor(Color(labelConfig.get_child("TextColor")));
	friendNameHeading->setBackgroundColor(Color(labelConfig.get_child("BackgroundColor")));


	mainLayout->getChildren()->insert(mainLayout->getChildren()->begin(),friendNameHeading);	
	imageGroup->clearChildren();
}

void FriendDetailView::albumPanelClicked(FBNode * clicked)
{
	topView = albumDetail;
	float targetPriority = 10;
	for (auto it = imageGroup->getChildren()->begin(); it != imageGroup->getChildren()->end();it++)
	{		
		PanelBase * imagePanel = (Panel*)*it;	
		if (dynamic_cast<Panel*>(imagePanel) != NULL)
			((Panel*)imagePanel)->setDataPriority(targetPriority);	
		else if (dynamic_cast<AlbumPanel*>(imagePanel) != NULL)
		{
			AlbumPanel * ap = (AlbumPanel*)imagePanel;
			ap->setChildDataPriority(targetPriority);
		}			
	}
	
	albumDetail->setFinishedCallback([this](string tag){
		this->albumDetail->setVisible(false);
		this->mainLayout->setVisible(true);
		this->layoutDirty = true;		
		this->show(this->activeNode);
	});
	
	mainLayout->setVisible(false);

	albumDetail->setVisible(true);
	albumDetail->show(clicked);

	layoutDirty = true;
}

void FriendDetailView::onGlobalGesture(const Controller & controller, std::string gestureType)
{	
	if (gestureType.compare("shake") == 0)
	{
		float targetPriority = 10;
		for (auto it = imageGroup->getChildren()->begin(); it != imageGroup->getChildren()->end();it++)
		{		
			PanelBase * imagePanel = (Panel*)*it;	
			if (dynamic_cast<Panel*>(imagePanel) != NULL)
				((Panel*)imagePanel)->setDataPriority(targetPriority);	
			else if (dynamic_cast<AlbumPanel*>(imagePanel) != NULL)
			{
				AlbumPanel * ap = (AlbumPanel*)imagePanel;
				ap->setChildDataPriority(targetPriority);
			}			
		}

		PointableElementManager::getInstance()->releaseGlobalGestureFocus(this);
		finishedCallback("friend_detail");
	} 
	else if (gestureType.compare("pointing") == 0)
	{
		itemScroll->getFlyWheel()->impartVelocity(0);
	}
}

void FriendDetailView::addNode(FBNode * node)
{
	if (items.count(node->getId()) == 0)
	{
		items.insert(make_pair(node->getId(),node));
		View * item = ViewOrchestrator::getInstance()->requestView(node->getId(),this);

		if (item == NULL)
		{
			if (node->getNodeType().compare(NodeType::FacebookImage) == 0)
			{
				Panel * p = new Panel(0,0);
				p->setNode(node);
				p->setVisible(true);
				p->setClickable(true);
				p->setDataPriority(0);
				p->layout(Vector(600-itemScroll->getFlyWheel()->getPosition(),600,10),cv::Size2f(100,100));
				item = p;
			} else if (node->getNodeType().compare(NodeType::FacebookAlbum) == 0)
			{
				item = new AlbumPanel();
			}

			if (item != NULL)
				ViewOrchestrator::getInstance()->registerView(node->getId(),item, this);
		}

		if (item != NULL)
		{
			if (node->getNodeType().compare(NodeType::FacebookImage) == 0)
			{				
				//item->setLayoutParams(cv::Size2f(500,450));
				Panel * p = (Panel*)item;
				p->setClickable(true);
				p->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(5,5,5,5)));
				item->elementClickedCallback = [this,p](LeapElement * clicked){

					this->itemScroll->getFlyWheel()->impartVelocity(0);
					this->imageDetailView->setImagePanel(p);
					this->imageDetailView->setVisible(true);						
					this->topView = this->imageDetailView;
					this->layoutDirty = true;
					PointableElementManager::getInstance()->requestGlobalGestureFocus(this->imageDetailView);							
				};
				imageGroup->addChild(item);		

			}
			else if (node->getNodeType().compare(NodeType::FacebookAlbum) == 0)
			{				
				((AlbumPanel*)item)->show(node);				
				((AlbumPanel*)item)->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(5,5,5,5)));
				item->elementClickedCallback = [this, node](LeapElement * element){
					this->albumPanelClicked(node);
				};

				imageGroup->addChild(item);		
			}
		}
		layoutDirty = true;		
		float itemWidth = 400;
		if (!imageGroup->getChildren()->empty())
		{
			itemWidth = imageGroup->getChildren()->at(0)->getMeasuredSize().width;
			if (itemWidth == 0)
				itemWidth = 400;
		}
		currentRightBoundary =  (itemWidth * ceilf((float)(imageGroup->getChildren()->size())/(float)rowCount));		
	}
}

void FriendDetailView::layout(Vector position, cv::Size2f size)
{
	lastSize = size;
	lastPosition = position;

	if (topView == mainLayout)
	{
		float menuHeight = GlobalConfig::tree()->get<float>("Menu.Height");
		friendNameHeading->layout(position-Vector(0,menuHeight,0),cv::Size2f(size.width,menuHeight));
		//itemScroll->measure(size);
		itemScroll->layout(position,size);
	}
	topView->layout(position,size);	
	layoutDirty = false;
}

void FriendDetailView::update()
{	
	ViewGroup::update();

	float pos = (float)itemScroll->getFlyWheel()->getPosition();
	imageDetailView->notifyOffsetChanged(Vector(pos,0,0));
	
	if (abs(lastUpdatePos - (-pos)) > 50)
		updateLoading();
}

void FriendDetailView::onFrame(const Controller & controller)
{	
	if (topView != NULL && topView->isVisible())
	{
		topView->onFrame(controller);
	}
	else
	{
		HandModel * hm = HandProcessor::LastModel();	
		Pointable testPointable = controller.frame().pointable(hm->IntentFinger);

		if (testPointable.isValid())
		{
			Leap::Vector screenPoint = LeapHelper::FindScreenPoint(controller,testPointable);

			if (imageGroup->getHitRect().contains(cv::Point_<float>(screenPoint.x,screenPoint.y)))
			{
				itemScroll->OnPointableEnter(testPointable);
			}	
		}
	}
}

void FriendDetailView::setFinishedCallback(const boost::function<void(std::string)> & callback)
{
	finishedCallback = callback;
}


void FriendDetailView::viewOwnershipChanged(View * view, ViewOwner * newOwner)
{	
	auto r1 = std::find(imageGroup->getChildren()->begin(),imageGroup->getChildren()->end(),view);
	if (r1 != imageGroup->getChildren()->end())
		imageGroup->getChildren()->erase(r1);
	else
	{
		r1 = std::find(children.begin(),children.end(),view);
		if (r1 != children.end())
			children.erase(r1);	
	}
}