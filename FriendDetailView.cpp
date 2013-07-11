#include "FriendDetailView.hpp"
#include "FixedAspectGrid.hpp"
#include "GraphicContext.hpp"


FriendDetailView::FriendDetailView()
{
	vector<RowDefinition> gridDefinition;	
	gridDefinition.push_back(RowDefinition(.1f));
	gridDefinition.push_back(RowDefinition(.9f));
	gridDefinition[0].ColumnWidths.push_back(1);
	gridDefinition[1].ColumnWidths.push_back(1);

	photoLoadCount = albumLoadCount = 0;
	albumLoadTarget = photoLoadTarget = 0;

	mainLayout = new CustomGrid(gridDefinition);	
	imageGroup = new FixedAspectGrid(cv::Size2i(0,2),true);	
	itemScroll = new ScrollingView(imageGroup);

	mainLayout->addChild(itemScroll);

	itemScroll->visibleRectChangedListener = [this](Vector newPos,cv::Size2f visibleSize){
		
		this->updateLoading(newPos,visibleSize,false);
	};
	
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

	vector<RadialMenuItem> menuItems;
	menuItems.push_back(RadialMenuItem("Exit Photo Explorer","exit",Colors::DarkRed));	
	menuItems.push_back(RadialMenuItem("Cancel","cancel",Colors::OrangeRed));
	radialMenu = new RadialMenu(menuItems);
	radialMenu->setVisible(false);
	radialMenu->ItemClickedCallback = [this](string id) -> bool{

		if (id.compare("exit") == 0)
		{
			GraphicsContext::getInstance().invokeApplicationExitCallback();
		}
		return true;
	};
	addChild(radialMenu);
}


void FriendDetailView::updateLoading(Vector newPos,cv::Size2f visibleSize, bool updatePriorityOnly)
{
	float leftBound = -newPos.x;
	float rightBound = -newPos.x + visibleSize.width;
		
	cv::Size2f s = imageGroup->getMeasuredSize();

	if (!updatePriorityOnly)
	{

		float newDataPixels = rightBound - s.width;
		if (newDataPixels > 0)
		{
			bool load = false;
			NodeQuerySpec querySpec(2);		

			float itemWidth = 400;
		
			int loadMore = 2 * (newDataPixels/itemWidth);

			if (albumLoadCount + loadMore > albumLoadTarget)
			{
				albumLoadTarget = albumLoadCount+loadMore;
				//cout << "Loading albums to " << albumLoadTarget << endl;
				load = true;
				querySpec.layers[0].insert(make_pair("albums",SelectionConfig(albumLoadTarget,albumLoadCount,true)));
				querySpec.layers[1].insert(make_pair("photos",SelectionConfig(4,0,false)));
			}

			if (photoLoadCount + loadMore > photoLoadTarget)
			{
				photoLoadTarget = photoLoadCount+loadMore;
				//cout << "Loading photos to " << photoLoadTarget << endl;
				load = true;
				querySpec.layers[0].insert(make_pair("photos",SelectionConfig(photoLoadTarget,photoLoadCount,true)));
			}

			bool loadStarted;
			viewChanged(1,DataViewGenerator::getInstance()->getDataView(activeNode,querySpec,[this](vector<FBNode*> & viewData){ this->viewChanged(1,viewData);},loadStarted));	
			if (loadStarted)
			{

			}			
		}
	}
	for (auto it = imageGroup->getChildren()->begin(); it != imageGroup->getChildren()->end();it++)
	{
		PanelBase * imagePanel = (PanelBase*)(*it);
		float targetPriority;
		float leftDist = leftBound - (imagePanel->getPosition().x + imagePanel->getWidth());
		float rightDist = imagePanel->getPosition().x - rightBound;

		if (itemScroll->getFlyWheel()->getVelocity() < 0)
		{				
			if (leftDist > 2000)
			{
				targetPriority = leftDist/4000.0f;
			}
			else if (rightDist < 3000)
				targetPriority = 0;
			else
				targetPriority = rightDist/4000.0f;

		}
		else
		{
			if (rightDist > 2000)
			{
				targetPriority = rightDist/4000.0f;
			}
			else if (leftDist < 3000)
				targetPriority = 0;
			else
				targetPriority = leftDist/4000.0f;

		}

		if (dynamic_cast<Panel*>(imagePanel) != NULL)
			((Panel*)imagePanel)->setDataPriority(targetPriority);	
		else if (dynamic_cast<AlbumPanel*>(imagePanel) != NULL)
		{
			AlbumPanel * ap = (AlbumPanel*)imagePanel;
			ap->setChildDataPriority(targetPriority);
		}
	}
}

bool FriendDetailView::onLeapGesture(const Controller & controller, const Gesture & gesture)
{
	if (radialMenu->checkMenuOpenGesture(gesture))
	{
		itemScroll->getFlyWheel()->impartVelocity(0);
		radialMenu->show();
		return true;
	}
	return itemScroll->onLeapGesture(controller, gesture);
}

void FriendDetailView::show(FBNode * root)
{	
	topView = mainLayout;
	topView->setVisible(true);

	PointableElementManager::getInstance()->requestGlobalGestureFocus(this);
	
	itemScroll->getFlyWheel()->overrideValue(0);

	activeNode = root;
	if (friendNameHeading != NULL)
		mainLayout->remove(friendNameHeading);

	
	View * item = ViewOrchestrator::getInstance()->requestView(root->getId() + "/name", this);
		
	if (item != NULL)
	{
		friendNameHeading = (TextPanel*)item;
		PanelFactory::getInstance().setStyle(friendNameHeading,TextStyles::Title);			
	}
	else
	{
		friendNameHeading = PanelFactory::getInstance().buildTextPanel(root->getAttribute("name"),TextStyles::Title);
		ViewOrchestrator::getInstance()->registerView(root->getId() + "/name",friendNameHeading,this);
	}
	mainLayout->getChildren()->insert(mainLayout->getChildren()->begin(),friendNameHeading);
	
	imageGroup->clearChildren();
	albumLoadTarget = albumLoadCount = photoLoadCount = photoLoadTarget = 0;
	
	updateLoading(Vector(),cv::Size2f(3000,1000),false);
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

void FriendDetailView::viewChanged(int offset, vector<FBNode*> & viewData)
{
	for (auto it = viewData.begin(); it != viewData.end(); it++)
	{
		FBNode * node = (*it);
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

				int c1 = imageGroup->getChildren()->size();
				imageGroup->addChild(item);		

				if (c1 != imageGroup->getChildren()->size())
					photoLoadCount++;

			}
			else if (node->getNodeType().compare(NodeType::FacebookAlbum) == 0)
			{				
				((AlbumPanel*)item)->show(node);				
				((AlbumPanel*)item)->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(5,5,5,5)));
				item->elementClickedCallback = [this, node](LeapElement * element){
					this->albumPanelClicked(node);
				};

				//item->setLayoutParams(cv::Size2f(500,450));
				int c1 = imageGroup->getChildren()->size();
				imageGroup->addChild(item);		
				if (c1 != imageGroup->getChildren()->size())
					albumLoadCount++;
			}
		}
	}
	
	double pos = itemScroll->getFlyWheel()->getPosition();
	updateLoading(Vector((float)pos,0,0),itemScroll->getMeasuredSize(),true);

	layoutDirty = true;
}

void FriendDetailView::layout(Vector position, cv::Size2f size)
{
	lastSize = size;
	lastPosition = position;

	topView->layout(position,size);	
	radialMenu->layout(position,size);
	//updateLoading(Vector((float)pos,0,0),itemScroll->getMeasuredSize(),true);
	layoutDirty = false;
}

void FriendDetailView::update()
{
	ViewGroup::update();

	double pos = itemScroll->getFlyWheel()->getPosition();
	imageDetailView->notifyOffsetChanged(Vector((float)pos,0,0));
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