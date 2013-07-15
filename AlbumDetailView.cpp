#include "AlbumDetailView.hpp"
#include "PanelFactory.hpp"
#include "FixedAspectGrid.hpp"
#include "GraphicContext.hpp"

AlbumDetailView::AlbumDetailView()
{	
	vector<RowDefinition> gridDefinition;	
	gridDefinition.push_back(RowDefinition(.1f));
	gridDefinition.push_back(RowDefinition(.9f));
	gridDefinition[0].ColumnWidths.push_back(1);
	gridDefinition[1].ColumnWidths.push_back(1);
	mainLayout = new CustomGrid(gridDefinition);
		
	rowCount = GlobalConfig::tree()->get<int>("AlbumDetailView.RowCount");
	imageGroup = new FixedAspectGrid(cv::Size2i(0,rowCount),1.0f,true);
	itemScroll = new ScrollingView(imageGroup);

	mainLayout->addChild(itemScroll);

	imageDetailView = new ImageDetailView();
	imageDetailView->setVisible(false);
	imageDetailView->setFinishedCallback([this](string a){
		PointableElementManager::getInstance()->releaseGlobalGestureFocus(this->imageDetailView);	
		this->imageDetailView->setVisible(false);
		this->layoutDirty = true;						
	});

	vector<RadialMenuItem> menuItems;
	//menuItems.push_back(RadialMenuItem("Fullscreen Mode","full"));
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
		else if (id.compare("full") == 0)
			GraphicsContext::getInstance().invokeGlobalAction("full");
		return true;
	};

	addChild(mainLayout);
	addChild(imageDetailView);
	addChild(radialMenu);

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
	}

	AlbumDetailView * v = this;
	FBDataSource::instance->loadField(activeNode,loadstr.str(),"",[v](FBNode * _node){
			
		AlbumDetailView * v2= v;
		v->postTask([v2,_node](){
			
			if (v2->activeNode == _node)
				v2->updateLoading();
		});
	});
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
						itemScroll->setDrawLoadingIndicator(2,Colors::HoloBlueBright);
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

//
//void AlbumDetailView::updateLoading(Vector newPos,cv::Size2f visibleSize, bool updatePriorityOnly)
//{
//	float leftBound = -newPos.x;
//	float rightBound = -newPos.x + visibleSize.width;
//		
//	cv::Size2f s = imageGroup->getMeasuredSize();
//
//	lastPosUpdate = newPos.x;
//	
//	if (!updatePriorityOnly)
//	{
//		float newDataPixels = rightBound - s.width;
//		if (newDataPixels > 0)
//		{
//			bool load = false;
//			NodeQuerySpec querySpec(2);				
//		
//			float itemWidth = 300;//s.width / (imageGroup->getChildren()->size() / 3.0f);
//			int loadMore = 2*(newDataPixels/itemWidth);
//			if (photoLoadCount + loadMore > photoLoadTarget)
//			{
//				photoLoadTarget = photoLoadCount+loadMore;
//				//cout << "Loading photos from " << "photoLoadCount" << " to " << photoLoadTarget << endl;
//				load = true;
//				querySpec.layers[0].insert(make_pair("photos",SelectionConfig(photoLoadTarget,photoLoadCount,true)));
//			}
//
//			bool isLoading = false;
//			if (load)
//				viewChanged(DataViewGenerator::getInstance()->getDataView(activeNode,querySpec,[this](vector<FBNode*> & viewData){ this->viewChanged(viewData);},isLoading));	
//			//if (isLoading)
//				//itemScroll->setDrawLoadingIndicator();
//		}
//	}
//
//	for (auto it = imageGroup->getChildren()->begin(); it != imageGroup->getChildren()->end();it++)
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
//		if (dynamic_cast<Panel*>(imagePanel) != NULL)
//			((Panel*)imagePanel)->setDataPriority(targetPriority);			
//	}
//}
//

//void AlbumDetailView::updateLoading(Vector newPos,cv::Size2f visibleSize, bool updatePriorityOnly)
//{
//	float leftBound = -min<float>(0,newPos.x);
//	float rightBound = -min<float>(0,newPos.x) + visibleSize.width;//*1.5f;
//
//	lastPosUpdate = newPos.x;
//		
//	cv::Size2f s = imageGroup->getMeasuredSize();
//	
//	if (!updatePriorityOnly)
//	{
//		float newDataPixels = rightBound - s.width;
//
//		
//		if (itemScroll->getFlyWheel()->getVelocity() < 0)
//		{
//			newDataPixels += 2000;
//		}
//
//		if (newDataPixels > 0)
//		{
//			bool load = false;
//			NodeQuerySpec querySpec(2);				
//
//			float itemWidth = avgItemWidth;//s.width / (imageGroup->getChildren()->size() / 3.0f);
//			int loadMore = 3*(newDataPixels/itemWidth);
//			if (photoLoadCount + loadMore > photoLoadTarget)
//			{
//				photoLoadTarget = photoLoadCount+loadMore;
//				cout << "Loading photos from " << photoLoadCount << " to " << photoLoadTarget << endl;
//				load = true;
//				querySpec.layers[0].insert(make_pair("photos",SelectionConfig(photoLoadTarget,photoLoadCount,true)));
//			}
//
//			bool isLoading = false;
//			if (load)
//				viewChanged(DataViewGenerator::getInstance()->getDataView(activeNode,querySpec,[this](vector<FBNode*> & viewData){ this->viewChanged(viewData);},isLoading));	
//			//if (isLoading)
//			itemScroll->setDrawLoadingIndicator(true,true);
//		}
//		
//	}
//
//	for (auto it = imageGroup->getChildren()->begin(); it != imageGroup->getChildren()->end();it++)
//	{
//		PanelBase * imagePanel = (PanelBase*)(*it);
//		float targetPriority;
//		float leftDist = (imagePanel->getPosition().x + imagePanel->getWidth()) - leftBound;
//		float rightDist = imagePanel->getPosition().x - rightBound;
//
//		if (rightDist > 6000 || leftDist < -6000)
//			continue;
//
//		if (itemScroll->getFlyWheel()->getVelocity() < 0)
//		{				
//			if (leftDist < 200)
//			{
//				targetPriority = leftDist/1000.0f;
//			}
//			else if (rightDist < -600)
//				targetPriority = 0;
//			else
//				targetPriority = (600+rightDist)/1000.0f;
//
//		}
//		else
//		{
//			if (rightDist > 0)
//			{
//				targetPriority = rightDist/1000.0f;
//			}
//			else if (leftDist > 0)
//				targetPriority = 0;
//			else
//				targetPriority = leftDist/1000.0f;
//
//		}
//
//		if (dynamic_cast<Panel*>(imagePanel) != NULL)
//			((Panel*)imagePanel)->setDataPriority(targetPriority);			
//	}
//}


void AlbumDetailView::show(FBNode * node)
{			
	PointableElementManager::getInstance()->requestGlobalGestureFocus(this);
	activeNode = node;
	mainLayout->remove(albumName);
	currentRightBoundary = 0;
	
	lastUpdatePos = 1000;
	
	View * item = ViewOrchestrator::getInstance()->requestView(node->getId() + "/name",this);
	if (item != NULL)
	{
		albumName = (TextPanel*)item;
		PanelFactory::getInstance().setStyle(albumName,TextStyles::Title);
	}
	else
	{
		albumName = PanelFactory::getInstance().buildTextPanel(node->getAttribute("name"), TextStyles::Title);
		ViewOrchestrator::getInstance()->registerView(node->getId() + "/name",albumName,this);
	}

	items.clear();
	imageGroup->clearChildren();
	mainLayout->getChildren()->insert(mainLayout->getChildren()->begin(),albumName);
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
			item->setNode(node);
			ViewOrchestrator::getInstance()->registerView(node->getId(),item, this);
		}
		else
		{
			item = dynamic_cast<Panel*>(v);
		}

		item->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(5,5,5,5)));
		item->setClickable(true);
		item->setVisible(true);

		item->elementClickedCallback = [this,item](LeapElement * clicked){
			this->imageDetailView->setImagePanel(item);										
			imageDetailView->setVisible(true);
			this->itemScroll->getFlyWheel()->impartVelocity(0);
			PointableElementManager::getInstance()->requestGlobalGestureFocus(this->imageDetailView);
			this->layoutDirty = true;			
		};

		imageGroup->addChild(item);
		
		float itemWidth = 1.0f * (lastSize.height/((float)rowCount));
		currentRightBoundary =  (itemWidth * ceilf((float)(imageGroup->getChildren()->size())/(float)rowCount));	
		layoutDirty = true;
	}
}

void AlbumDetailView::layout(Vector position, cv::Size2f size)
{
	lastSize = size;
	lastPosition = position;

	if (imageDetailView->isVisible())
	{
		imageDetailView->layout(position,size);
	}
	else
	{
		radialMenu->layout(position,size);
		mainLayout->layout(position,size);
	}
	layoutDirty = false;


}

void AlbumDetailView::update()
{
	ViewGroup::update();

	double pos = itemScroll->getFlyWheel()->getPosition();
	imageDetailView->notifyOffsetChanged(Vector((float)pos,0,0));
	
	if (abs(pos - lastUpdatePos) > 100)
		updateLoading();

}

void AlbumDetailView::onFrame(const Controller & controller)
{	
	if (imageDetailView->isVisible())
	{
		imageDetailView->onFrame(controller);
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
				children[0]->OnPointableEnter(testPointable);
			}	
		}
	}
}



void AlbumDetailView::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	if (gestureType.compare("shake") == 0)
	{
		float targetPriority = 10;
		for (auto it = imageGroup->getChildren()->begin(); it != imageGroup->getChildren()->end();it++)
		{		
			Panel * imagePanel = (Panel*)*it;	
			imagePanel->setDataPriority(targetPriority);	
		}

		imageDetailView->setVisible(false);

		PointableElementManager::getInstance()->releaseGlobalGestureFocus(this);

		layoutDirty = true;
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
	if (radialMenu->checkMenuOpenGesture(gesture))
	{
		itemScroll->getFlyWheel()->impartVelocity(0);
		radialMenu->show();
		return true;
	}
	return itemScroll->onLeapGesture(controller, gesture);
}

void AlbumDetailView::getTutorialDescriptor(vector<string> & tutorial)
{
	tutorial.push_back("shake");
	tutorial.push_back("point_stop");
	tutorial.push_back("swipe");
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
	else
	{
		r1 = std::find(children.begin(),children.end(),view);
		if (r1 != children.end())
			children.erase(r1);	
	}
}