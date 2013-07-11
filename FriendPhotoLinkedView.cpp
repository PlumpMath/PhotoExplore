#include "FriendPhotoLinkedView.hpp"
#include "PanelFactory.hpp"
#include "FixedAspectGrid.hpp"
#include "LinearLayout.hpp"

#define FriendGridSize cv::Size2i(1,3)
#define FriendGridAspectRatio 1.6f

FriendPhotoLinkedView::FriendPhotoLinkedView()
{		
	topView = NULL;

	//Main layout
	vector<RowDefinition> gridDefinition;	
	gridDefinition.push_back(RowDefinition(.8f));	
	gridDefinition.push_back(RowDefinition(.2f));
	gridDefinition[0].ColumnWidths.push_back(1);
	gridDefinition[1].ColumnWidths.push_back(1);
	mainLayout = new CustomGrid(gridDefinition);

	imageGroup = new LinearLayout(true);
	friendBar = new LinearLayout(true);
	
	friendScroll = new ScrollingView(friendBar);
	photoScroll = new ScrollingView(imageGroup);

	photoScroll->visibleRectChangedListener = [this](Vector newPos,cv::Size2f visibleSize){
		this->updateLoading(newPos,visibleSize);
	};

	mainLayout->addChild(new FrameView(photoScroll, Colors::Transparent));
	mainLayout->addChild(friendScroll);
	addChild(mainLayout);
	
	//Friend Detail
	friendDetail = new FriendDetailView();		
	friendDetail->setFinishedCallback([this](string tag){				
		this->setTopView(this->mainLayout);		
		this->show(this->activeNode);
	});

	friendDetail->setVisible(false);
	addChild(friendDetail);	

	//Image Detail
	imageDetail = new ImageDetailView();
	addChild(imageDetail);
	imageDetail->setVisible(false);
	imageDetail->setFinishedCallback([this](string c){
			
		this->imageDetail->setImagePanel(NULL);	
		this->setTopView(this->mainLayout);

	});

	//Album Detail
	albumDetail = new AlbumDetailView();	
	albumDetail->setFinishedCallback([this](string tag){		 
		setTopView(this->mainLayout);	
		this->show(this->activeNode);
	});
	addChild(albumDetail);
	albumDetail->setVisible(false);


	seekBar = new FastSeekBar(photoScroll);
	addChild(seekBar);

	//Start with main
	setTopView(mainLayout);
}


void FriendPhotoLinkedView::show(FBNode * root)
{		
	PointableElementManager::getInstance()->requestGlobalGestureFocus(this);

	activeNode = root;
	friendLoadCount = 0;
	friendLoadTarget = 0;

	loadedFriends.clear();

	imageGroup->clearChildren();
	friendBar->clearChildren();

	mainLayout->setVisible(true);
	
	updateLoading(Vector(),cv::Size2f(3000,1000));
}

void FriendPhotoLinkedView::setTopView(View * _topView, bool hideCurrent)
{
	if (topView != _topView)
	{
		if (topView != NULL)
		{			
			if (hideCurrent)
				topView->setVisible(false);
		}

		topView = _topView;
		
		if (topView != NULL)
		{
			topView->setVisible(true);
		}
		layoutDirty = true;
	}		
}

void FriendPhotoLinkedView::albumPanelClicked(LeapElement * element)
{
	float targetPriority = 10.0f;
	for (auto it = imageGroup->getChildren()->begin(); it != imageGroup->getChildren()->end();it++)
	{
		ViewGroup * friendGroup = (ViewGroup*)(((FrameView*)*it)->getContentView());
		for (auto panel_it = friendGroup->getChildren()->begin(); panel_it != friendGroup->getChildren()->end();panel_it++)
		{
			Panel * p = (Panel*)*panel_it;					
			p->setDataPriority(targetPriority);		
		}
	}

	this->setTopView(albumDetail);	
	this->albumDetail->show(((AlbumPanel*)element)->getNode());		
}

void FriendPhotoLinkedView::photoPanelClicked(LeapElement * element)
{
	if (this->imageDetail->getImagePanel() != element)
	{
		this->photoScroll->getFlyWheel()->impartVelocity(0);
		this->imageDetail->setImagePanel((Panel*)element);								
		this->setTopView(this->imageDetail, false);
		PointableElementManager::getInstance()->requestGlobalGestureFocus(this->imageDetail);
	}
	else
	{		
		PointableElementManager::getInstance()->releaseGlobalGestureFocus(this->imageDetail);
		this->imageDetail->setImagePanel(NULL);	
		this->setTopView(this->mainLayout);
	}
	this->layoutDirty = true;
}


void FriendPhotoLinkedView::friendLoaded(int offset, vector<FBNode*> & viewData)
{		
	for (auto it = viewData.begin(); it != viewData.end(); it++)
	{		
		FBNode * node = (*it);
		NodeQuerySpec querySpec(2,6);
		querySpec.layers[0].insert(make_pair("photos",SelectionConfig(6)));
		querySpec.layers[0].insert(make_pair("albums",SelectionConfig(2,0,false)));
		querySpec.layers[1].insert(make_pair("photos",SelectionConfig(4)));
		bool isLoading;
		photoLoaded(0,node,DataViewGenerator::getInstance()->getDataView(node,querySpec,[this,node](vector<FBNode*> & photoData){ this->photoLoaded(0,node,photoData);},isLoading));	
	}
}

void FriendPhotoLinkedView::updateLoading(Vector newPos,cv::Size2f visibleSize)
{
	float leftBound = -newPos.x;
	float rightBound = -newPos.x + visibleSize.width;
			
	cv::Size2f s = imageGroup->getMeasuredSize();
	float newDataPixels = rightBound - s.width;
	if (newDataPixels > 0)
	{
		float itemWidth = imageGroup->getChildren()->size() / 3.0f;
		if (itemWidth == 0)	itemWidth = (lastSize.height/3.0f)*1.6f;
		if (itemWidth == 0)	itemWidth = 400; //(lastSize.height/3.0f)*1.6f;
		int loadMore =  3*(newDataPixels/itemWidth);

		if (friendLoadCount + loadMore > friendLoadTarget)
		{
			friendLoadTarget = friendLoadCount+loadMore;
			//cout << "Loading to " << friendLoadTarget << endl;

			NodeQuerySpec querySpec(1);							
			querySpec.layers[0].insert(make_pair("friends",SelectionConfig(friendLoadTarget,friendLoadCount,true)));

			bool isLoading;
			friendLoaded(1,DataViewGenerator::getInstance()->getDataView(activeNode,querySpec,[this](vector<FBNode*> & viewData){ this->friendLoaded(1,viewData);},isLoading));	
		}
	}

	if (photoScroll->getFlyWheel()->getVelocity() < 0)
	{	
		for (auto it = imageGroup->getChildren()->begin(); it != imageGroup->getChildren()->end();it++)
		{
			ViewGroup * friendGroup = (ViewGroup*)(((FrameView*)*it)->getContentView());

			float targetPriority;
			float dist = leftBound - friendGroup->getLastPosition().x;
			float rightDist = friendGroup->getLastPosition().x - rightBound;


			if (dist > 2000)
			{
				targetPriority = dist/4000.0f;
			}
			else if (rightDist < 2000)
				targetPriority = 0;
			else
				targetPriority = rightDist/4000.0f;

			for (auto panel_it = friendGroup->getChildren()->begin(); panel_it != friendGroup->getChildren()->end();panel_it++)
			{
				Panel * p = (Panel*)*panel_it;					
				p->setDataPriority(targetPriority);		
			}
		}
	}
	else
	{
		for (auto it = imageGroup->getChildren()->begin(); it != imageGroup->getChildren()->end();it++)
		{
			ViewGroup * friendGroup = (ViewGroup*)(((FrameView*)*it)->getContentView());

			float targetPriority;
			float dist = friendGroup->getLastPosition().x - rightBound;

			float leftDist = leftBound - friendGroup->getLastPosition().x;

			if (dist > 2000)
			{
				targetPriority = dist/4000.0f;
			}
			else if (leftDist < 2000)
				targetPriority = 0;
			else
				targetPriority = leftDist/4000.0f;

			for (auto panel_it = friendGroup->getChildren()->begin(); panel_it != friendGroup->getChildren()->end();panel_it++)
			{
				Panel * p = (Panel*)*panel_it;					
				p->setDataPriority(targetPriority);		
			}
		}
	}
}

void FriendPhotoLinkedView::photoLoaded(int offset, FBNode * friendNode, vector<FBNode*> & viewData)
{
	ViewGroup * friendPhotoGrid  = NULL;
	if (loadedFriends.count(friendNode) == 0)
	{
		if (viewData.size() > 1)
		{			
			TextPanel * friendName = (TextPanel*) ViewOrchestrator::getInstance()->requestView(friendNode->getId() + "/name_small", this);
			if (friendName == NULL)
			{
				friendName = new TextPanel(friendNode->getAttribute("name"));
				friendName = PanelFactory::getInstance().buildTextPanel(friendNode->getAttribute("name"), TextStyles::Heading2);
				ViewOrchestrator::getInstance()->registerView(friendNode->getId()+ "/name_small",friendName, this);
			}
			else
				PanelFactory::getInstance().setStyle(friendName,TextStyles::Heading2);

			View * friendLabel = ViewOrchestrator::getInstance()->requestView(friendNode->getId() + "/friendLabel", this);
			if (friendLabel == NULL)
			{							

				Panel * friendPhoto = (Panel*)ViewOrchestrator::getInstance()->requestView(friendNode->getId() + "/friendphoto", this);
				if (friendPhoto == NULL)
				{
					friendPhoto = new Panel(0,0);
					friendPhoto->setNode(friendNode);
					friendPhoto->setVisible(true);
					friendPhoto->setDataPriority(0);								
					ViewOrchestrator::getInstance()->registerView(friendNode->getId()+ "/friendphoto",friendPhoto, this);
				}
				
				vector<RowDefinition> gridDefinition;	
				gridDefinition.push_back(RowDefinition(1));
				gridDefinition[0].ColumnWidths.push_back(.4f);
				gridDefinition[0].ColumnWidths.push_back(.6f);

				ViewGroup * host  = new CustomGrid(gridDefinition);
				host->addChild(friendName);
				host->addChild(friendPhoto);

				friendLabel = new ContentPanel(host);

			}
			friendName->elementClickedCallback = [this,friendNode](LeapElement * element){

				float targetPriority = 10.0f;
				for (auto it = this->imageGroup->getChildren()->begin(); it != this->imageGroup->getChildren()->end();it++)
				{
					ViewGroup * friendGroup = (ViewGroup*)(((FrameView*)*it)->getContentView());
					for (auto panel_it = friendGroup->getChildren()->begin(); panel_it != friendGroup->getChildren()->end();panel_it++)
					{
						Panel * p = (Panel*)*panel_it;					
						p->setDataPriority(targetPriority);		
					}
				}

				this->setTopView(this->friendDetail);	
				this->friendDetail->show(friendNode);						
			};

			ViewOrchestrator::getInstance()->registerView(friendNode->getId()+ "/friendLabel",friendLabel, this);


			friendLabel->setLayoutParams(LayoutParams(cv::Size2f(600,300),cv::Vec4f(30,0,30,0)));		
			friendBar->addChild(friendLabel);	


			friendLoadCount++;
			friendPhotoGrid = new FixedAspectGrid(FriendGridSize,FriendGridAspectRatio, true);
			((FixedAspectGrid*)friendPhotoGrid)->setInteriorMarginsOnly(true);

			View * frame = new FrameView(friendPhotoGrid,Colors::White);
			frame->setLayoutParams(LayoutParams(cv::Size2f(), cv::Vec4f(30,0,30,30)));

			ViewOrchestrator::getInstance()->registerView(friendNode->getId()+ "/friendgrid",friendPhotoGrid, this);			

			drawCallbacks.push_back([this,friendLabel,friendPhotoGrid](){

				Color c = Colors::DimGray;
				c.setAlpha(.7f);
				glColor4fv(c.getFloat());
				glBindTexture( GL_TEXTURE_2D, NULL);

				float photoOffset =  this->photoScroll->getFlyWheel()->getPosition();
				float friendOffset = this->friendScroll->getFlyWheel()->getPosition();

				float	photo_x1 = friendPhotoGrid->getLastPosition().x + photoOffset,
					photo_x2 = photo_x1 + friendPhotoGrid->getMeasuredSize().width,
					friend_x1 = friendLabel->getLastPosition().x + friendOffset,
					friend_x2 = friend_x1 + friendLabel->getMeasuredSize().width,
					photo_y = friendPhotoGrid->getLastPosition().y + friendPhotoGrid->getMeasuredSize().height+30+10,
					friend_y = friendLabel->getLastPosition().y - 11;


				float 
					bar_x1 = max<float>(photo_x1,friend_x1),
					bar_x2 = min<float>(photo_x2,friend_x2),
					bar_y1 = photo_y, // + verticalOffset+3,
					bar_y2 = friend_y, //bar_y1 + 3,
					z1 = friendPhotoGrid->getLastPosition().z-1;


				glBegin( GL_QUADS );
				glVertex3f(bar_x1,bar_y1,z1);
				glVertex3f(bar_x2,bar_y1,z1);
				glVertex3f(bar_x2,bar_y2,z1);
				glVertex3f(bar_x1,bar_y2,z1);
				glEnd();

			});

			loadedFriends[friendNode] = friendPhotoGrid;
			imageGroup->addChild(frame);
		}
	}
	else 
		friendPhotoGrid = (ViewGroup*)loadedFriends[friendNode];

	if (friendPhotoGrid != NULL)
		for (auto it = viewData.begin(); it != viewData.end(); it++)
		{		
			FBNode * photoNode = (*it);
			Panel * photoItem = (Panel*)ViewOrchestrator::getInstance()->requestView(photoNode->getId(), this);

			if (friendPhotoGrid->getChildren()->size() < 6)
			{
				if (photoItem == NULL)
				{
					photoItem= new Panel(0,0);
					photoItem->setNode(photoNode);
					photoItem->setDataPriority(0);	

					ViewOrchestrator::getInstance()->registerView(photoNode->getId(),photoItem, this);	
				}
				photoItem->setVisible(true);					
				photoItem->setClickable(true);	
				photoItem->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(5,5,5,5)));
				photoItem->setBackgroundColor(Colors::Transparent);
				photoItem->setBorderThickness(0);
				photoItem->elementClickedCallback = boost::bind(&FriendPhotoLinkedView::photoPanelClicked,this,_1);
				//photoItem->setLayoutParams(cv::Size2f(400,300));	
				friendPhotoGrid->addChild(photoItem);	
			}
			else if (photoItem != NULL)
			{				
				photoItem->elementClickedCallback = boost::bind(&FriendPhotoLinkedView::photoPanelClicked,this,_1);
			}
		}
		
	layoutDirty = true;

}


void FriendPhotoLinkedView::layout(Vector position, cv::Size2f size)
{
	lastPosition = position;
	lastSize = size;

	//seekBar->layout(position + Vector(0,0,10),size);

	topView->layout(position,size);	
		
	float s1 = friendBar->getMeasuredSize().width;
	float s2 = imageGroup->getMeasuredSize().width;

	friendToPhoto = s1/s2;

	layoutDirty = false;
}

void FriendPhotoLinkedView::onFrame(const Controller & controller)
{	
	//seekBar->onFrame(controller);
	//if (topView != mainLayout && topView != NULL && topView->isEnabled() && topView->isVisible())
	//{	
	//	topView->onFrame(controller);
	//}
}

void FriendPhotoLinkedView::setFinishedCallback(const boost::function<void(std::string)> & callback)
{
	finishedCallback = callback;
}

void FriendPhotoLinkedView::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	PointableElementManager::getInstance()->releaseGlobalGestureFocus(this);
	finishedCallback("");
}

bool FriendPhotoLinkedView::onLeapGesture(const Controller & controller, const Gesture & gesture)
{
	//if (gesture.type() == Gesture::Type::TYPE_CIRCLE && gesture.state() == Gesture::STATE_UPDATE)
	//{
	//	CircleGesture circle(gesture);
	//	if (circle.progress() > .8f && circle.normal().angleTo(Vector::forward()) < PI/4.0f && circle.radius() > GlobalConfig::getInstance().getFloat("MenuCircleRadius"))
	//	{
	//		radialMenu->show();
	//		return true;
	//	}
	//}
	return photoScroll->onLeapGesture(controller, gesture);
}

void FriendPhotoLinkedView::viewOwnershipChanged(View * view, ViewOwner * newOwner)
{
	;
}


void FriendPhotoLinkedView::update()
{
	ViewGroup::update();
	
	double pos = photoScroll->getFlyWheel()->getPosition();
	imageDetail->notifyOffsetChanged(Vector((float)pos,0,0));
	if (friendToPhoto > 0)
		friendScroll->getFlyWheel()->overrideValue(pos*friendToPhoto);
}

void FriendPhotoLinkedView::draw()
{
	ViewGroup::draw();
	if (mainLayout->isVisible())
		for (auto it=drawCallbacks.begin(); it != drawCallbacks.end(); it++)
		{
			(*it)();
		}

}