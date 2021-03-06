#include "AlbumDetailView.hpp"
#include "PanelFactory.hpp"
#include "FixedAspectGrid.hpp"
#include "GraphicContext.hpp"
#include "AbsoluteLayout.hpp"
#include "SwipeGestureDetector.hpp"

AlbumDetailView::AlbumDetailView()
{	
	mainLayout = new AbsoluteLayout();

	((AbsoluteLayout*)mainLayout)->layoutCallback = [this](Vector position, cv::Size2f size){

		float menuHeight = GlobalConfig::tree()->get<float>("Menu.Height");
		float tutorialHeight = GlobalConfig::tree()->get<float>("Tutorial.Height");
		float scrollBarHeight = GlobalConfig::tree()->get<float>("ScrollView.ScrollBar.Height");

		albumName->layout(position-Vector(0,menuHeight,0),cv::Size2f(size.width,menuHeight));
		itemScroll->layout(position,size);

		float scrollBarWidth = size.width * 0.4f;

		scrollBar->layout(position + Vector((size.width-scrollBarWidth)*.5f,size.height+(tutorialHeight-scrollBarHeight)*.5f,1),cv::Size2f(scrollBarWidth,scrollBarHeight));
	};
		
	rowCount = GlobalConfig::tree()->get<int>("AlbumDetailView.RowCount");
	imageGroup = new FixedAspectGrid(cv::Size2i(0,rowCount),1.0f,true);
	itemScroll = new ScrollingView(imageGroup);


	scrollBar = new ScrollBar();
	scrollBar->setScrollView(itemScroll);
	
	mainLayout->addChild(itemScroll);
	mainLayout->addChild(scrollBar);

	imageDetailView = new ImageDetailView();
	imageDetailView->setVisible(false);
	imageDetailView->setFinishedCallback([this](string a){
		PointableElementManager::getInstance()->releaseGlobalGestureFocus(this->imageDetailView);	
		this->imageDetailView->setVisible(false);
		this->layoutDirty = true;						
	});

	addChild(mainLayout);
	addChild(imageDetailView);

	lastUpdatePos = 1000;
	avgItemWidth = 400;
	albumName = NULL;
}


void AlbumDetailView::loadItems(int photos)
{
	int requestedPhotos = activeNode->loadState["photos"].requestedCount;
	
	stringstream loadstr;
	loadstr << activeNode->getId() << "?fields=";

	if (photos > requestedPhotos)
	{
		activeNode->loadState["photos"].requestedCount = photos;
		loadstr << "photos.offset(" << requestedPhotos << ").limit(" << photos << ").fields(id,name,images)";

		AlbumDetailView * v = this;
		FBDataSource::instance->loadField(activeNode,loadstr.str(),"",[v](FBNode * _node){
			AlbumDetailView * v2= v;
			v->postTask([v2,_node](){
				if (v2->activeNode == _node)
				{
					v2->itemScroll->setDrawLoadingIndicator(0,Colors::SteelBlue);
					v2->updateLoading();
				}
			});
		});
	}
}



void AlbumDetailView::onGlobalFocusChanged(bool isFocused)
{
	if (isFocused)
	{
		SwipeGestureDetector::getInstance().setFlyWheel(itemScroll->getFlyWheel());
	}
	else
	{
		SwipeGestureDetector::getInstance().setFlyWheel(NULL);
	}
}


void AlbumDetailView::updateLoading()
{
	float scrollPosition = itemScroll->getFlyWheel()->getPosition();
	cv::Size2f visibleSize = itemScroll->getMeasuredSize();

	lastUpdatePos = -scrollPosition;

	float leftBound = -scrollPosition;
	float rightBound = -scrollPosition + visibleSize.width;

	float itemWidth = 1.0f * (lastSize.height/((float)rowCount));
		
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

		auto photoNodes = activeNode->Edges.get<EdgeTypeIndex>().equal_range("photos");
		
		while (items.size() < itemCount)
		{
			if (photoNodes.first != photoNodes.second)
			{
				addNode(photoNodes.first->Node);
				photoNodes.first++;
			} else 
			{
				if (!activeNode->edgeLimitReached("photos"))
				{
					int loadPhotos =  GlobalConfig::tree()->get<int>("AlbumDetailView.PhotosPerRequest") + availablePhotos;	
					loadItems(loadPhotos);
					if (imageGroup->getMeasuredSize().width > itemScroll->getMeasuredSize().width)
						itemScroll->setDrawLoadingIndicator(2,Colors::DimGray);
				}
				else
				{
					if (imageGroup->getMeasuredSize().width > itemScroll->getMeasuredSize().width)
						itemScroll->setDrawLoadingIndicator(1,Colors::DarkRed);
				}

				break;
			}
		}						
	}
	
	loadingTimer.start();
	
	float peakPriority = (itemScroll->getMeasuredSize().width*.5f) - itemScroll->getFlyWheel()->getPosition();
	peakPriority -= itemScroll->getFlyWheel()->getVelocity() * GlobalConfig::tree()->get<float>("AlbumDetailView.ScrollAheadTime");
	
	for (auto it = imageGroup->getChildren()->begin(); it != imageGroup->getChildren()->end();it++)
	{
		PanelBase * imagePanel = (PanelBase*)(*it);
		float targetPriority;

		float distance = abs((imagePanel->getPosition().x + imagePanel->getWidth()/2.0f) - peakPriority);

		targetPriority = min<float>(10,distance/1000.0f);

		if (dynamic_cast<Panel*>(imagePanel) != NULL)
			((Panel*)imagePanel)->setDataPriority(targetPriority);	
	}	
}

void AlbumDetailView::suspend()
{	
	PointableElementManager::getInstance()->releaseGlobalGestureFocus(this->imageDetailView);	
	this->imageDetailView->setVisible(false);	

	float targetPriority = 100;
	for (auto it = imageGroup->getChildren()->begin(); it != imageGroup->getChildren()->end();it++)
	{		
		Panel * imagePanel = (Panel*)*it;	
		imagePanel->setDataPriority(targetPriority);	
	}
	
	PointableElementManager::getInstance()->releaseGlobalGestureFocus(this);
	layoutDirty = true;
}

void AlbumDetailView::show(FBNode * node)
{			
	PointableElementManager::getInstance()->requestGlobalGestureFocus(this);
	SwipeGestureDetector::getInstance().setFlyWheel(itemScroll->getFlyWheel());

	activeNode = node;
	mainLayout->remove(albumName);
	currentRightBoundary = 0;
	this->itemScroll->getFlyWheel()->overrideValue(0);
	
	lastUpdatePos = 1000;
	
	View * item = ViewOrchestrator::getInstance()->requestView(node->getId() + "/name",this);
	if (item != NULL)
	{
		albumName = (TextPanel*)item;
	}
	else
	{
		albumName = new TextPanel(node->getAttribute("name"));
		ViewOrchestrator::getInstance()->registerView(node->getId() + "/name",albumName,this);
	}

	boost::property_tree::ptree labelConfig = GlobalConfig::tree()->get_child("AlbumDetailView.Title");
	albumName->setTextSize(labelConfig.get<float>("FontSize"),true);
	albumName->setTextFitPadding(labelConfig.get<float>("TextPadding"));
	albumName->setTextColor(Color(labelConfig.get_child("TextColor")));
	albumName->setBackgroundColor(Color(labelConfig.get_child("BackgroundColor")));

	items.clear();
	imageGroup->clearChildren();
	mainLayout->getChildren()->insert(mainLayout->getChildren()->begin(),albumName);
}

void AlbumDetailView::showChild(FBNode * node)
{
	if (items.count(node->getId()) == 0)
	{		
		items.insert(make_pair(node->getId(),node));
		View * v= ViewOrchestrator::getInstance()->requestView(node->getId(), this);

		Panel * item = NULL;
		if (v == NULL)
		{
			item = new Panel(0,0);
			item->show(node);
			ViewOrchestrator::getInstance()->registerView(node->getId(),item, this);
		}
		else
		{
			item = dynamic_cast<Panel*>(v);
		}
		item->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(5,5,5,5)));
		item->setClickable(true);

		item->elementClickedCallback = [this,item](LeapElement * clicked){
			this->itemScroll->getFlyWheel()->impartVelocity(0);			

			this->imageDetailView->notifyOffsetChanged(Vector((float)this->itemScroll->getFlyWheel()->getCurrentPosition(),0,0));
			
			this->imageDetailView->setImagePanel(item);										
			this->imageDetailView->setVisible(true);
			PointableElementManager::getInstance()->requestGlobalGestureFocus(this->imageDetailView);
			this->layoutDirty = true;			
		};

		imageGroup->addChild(item);		
		float itemWidth = 1.0f * (lastSize.height/((float)rowCount));
		currentRightBoundary =  (itemWidth * ceilf((float)(imageGroup->getChildren()->size())/(float)rowCount));	
		
		item->elementClicked();
	}
	else
	{
		Panel * childPanel= dynamic_cast<Panel*>(ViewOrchestrator::getInstance()->requestView(node->getId(), this));
		if (childPanel != NULL)
		{
			childPanel->elementClicked();
		}
	}
}

FBNode * AlbumDetailView::getNode()
{
	return activeNode;
}

void AlbumDetailView::addNode(FBNode * node)
{
	if (items.count(node->getId()) == 0)
	{
		items.insert(make_pair(node->getId(),node));
		View * v= ViewOrchestrator::getInstance()->requestView(node->getId(), this);

		Panel * item = NULL;
		if (v == NULL)
		{
			item = new Panel(0,0);
			item->show(node);
			ViewOrchestrator::getInstance()->registerView(node->getId(),item, this);
		}
		else
		{
			item = dynamic_cast<Panel*>(v);
		}

		item->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(5,5,5,5)));
		item->layout(Vector(lastSize.width-itemScroll->getFlyWheel()->getPosition(),lastSize.height*.5f,-10),cv::Size2f(lastSize.height*(1.0f/((float)rowCount)),10));
		item->setClickable(true);
		item->setVisible(true);

		item->elementClickedCallback = [this,item](LeapElement * clicked){
			this->itemScroll->getFlyWheel()->impartVelocity(0);			

			this->imageDetailView->notifyOffsetChanged(Vector((float)this->itemScroll->getFlyWheel()->getCurrentPosition(),0,0));

			this->imageDetailView->setImagePanel(item);										
			this->imageDetailView->setVisible(true);
			PointableElementManager::getInstance()->requestGlobalGestureFocus(this->imageDetailView);
			this->layoutDirty = true;			
		};

		imageGroup->addChild(item);
		
		float viewHeight = lastSize.height;
		if (viewHeight == 0) viewHeight = GlobalConfig::ScreenHeight;
		float itemWidth = lastSize.height / ((float)rowCount);

		currentRightBoundary =  (itemWidth * ceilf((float)(imageGroup->getChildren()->size())/(float)rowCount));	
		layoutDirty = true;
	}
}

void AlbumDetailView::layout(Vector position, cv::Size2f size)
{
	lastSize = size;
	lastPosition = position;


	//else
	//{		
	mainLayout->layout(position,size);
	//}

	if (imageDetailView->isVisible())
	{
		imageDetailView->layout(position,size);
	}
	layoutDirty = false;


}

void AlbumDetailView::update()
{
	ViewGroup::update();

	if (imageDetailView->isVisible())
	{
		imageDetailView->update();
	}
	//else
	//{
	double pos = itemScroll->getFlyWheel()->getPosition();
	if (!imageDetailView->isVisible())
		imageDetailView->notifyOffsetChanged(Vector((float)pos,0,0));

	if (abs(pos - lastUpdatePos) > 100)
		updateLoading();
	//}

}

void AlbumDetailView::onFrame(const Controller & controller)
{	
	if (imageDetailView->isVisible())
	{
		imageDetailView->onFrame(controller);
	}
}


void AlbumDetailView::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	if (gestureType.compare("shake") == 0)
	{
		if (!finishedCallback.empty())
			finishedCallback("album_detail");
	} 
	else if (gestureType.compare("pointing") == 0)
	{
		itemScroll->getFlyWheel()->impartVelocity(0);
	}
}

bool AlbumDetailView::onLeapGesture(const Controller & controller, const Gesture & gesture)
{
	return itemScroll->onLeapGesture(controller, gesture);
}

void AlbumDetailView::getTutorialDescriptor(vector<string> & tutorial)
{
	tutorial.push_back("swipe");
	tutorial.push_back("point_stop");
	tutorial.push_back("shake");
}

void AlbumDetailView::setFinishedCallback(const boost::function<void(std::string)> & callback)
{
	finishedCallback = callback;
}

void AlbumDetailView::viewOwnershipChanged(View * view, ViewOwner * newOwner)
{	
	auto r1 = std::find(imageGroup->getChildren()->begin(),imageGroup->getChildren()->end(),view);
	if (r1 != imageGroup->getChildren()->end())
		imageGroup->getChildren()->erase(r1);
	//else
	//{
	//	r1 = std::find(children.begin(),children.end(),view);
	//	if (r1 != children.end())
	//		children.erase(r1);	
	//}
}