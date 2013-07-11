#include "RelevantImageView.hpp"

RelevantImageView::RelevantImageView()
{		
	vector<RowDefinition> gridDefinition;
	
	gridDefinition.push_back(RowDefinition(.2f));
	gridDefinition.push_back(RowDefinition(.8f));
	gridDefinition[0].ColumnWidths.push_back(1);
	gridDefinition[1].ColumnWidths.push_back(1.0f);

	mainLayout = new CustomGrid(gridDefinition);

	imageGroup = new DynamicGrid(cv::Size2i(0,3));
	((DynamicGrid*)imageGroup)->setDefaultCellSize(cv::Size2f(500,450));
	((DynamicGrid*)imageGroup)->setPadding(cv::Vec4f(5,5,5,5));		

	photoInfoPanel = NULL;
	buttonBar = NULL;
	topView = NULL;

	initButtonBar();
	mainLayout->addChild(buttonBar);
	mainLayout->addChild(new FrameView(new ScrollingView(imageGroup), Colors::DarkBlue));

	mainLayout->addChild(buttonBar);
	addChild(mainLayout);
	//addChild(photoInfoPanel);
}

void RelevantImageView::initButtonBar()
{
	buttonBar = new UniformGrid(cv::Size2i(3,1));
	buttonBar->addChild(new Button("Recent Activity"));
	buttonBar->addChild(new Button("Liked Photos"));
	buttonBar->addChild(new Button("Random"));

	TextPanel * selectionIndicator = new TextPanel(" ");
	Color bgColor =Colors::DarkBlue;
	bgColor.setAlpha(1);
	selectionIndicator->setBackgroundColor(bgColor);
	selectionIndicator->setEnabled(false);

	for (auto it = buttonBar->getChildren()->begin();it != buttonBar->getChildren()->end();it++)
	{
		Button * button = (Button*)(*it);
		button->setLayoutParams(LayoutParams(cv::Size2f(0,0),cv::Vec4f(0,30,0,30)));

		Color c = Colors::HoloBlueLight;
		c.setAlpha(.8f);
		button->setBackgroundColor(c);
		button->setTextColor(Colors::White);
		button->setTextSize(10.0f);
		button->setBorderThickness(1);
		button->setBorderColor(Colors::HoloBlueBright);

		button->elementClickedCallback = [this,selectionIndicator](LeapElement * clicked){

			Button * clickedButton = (Button*)clicked;
			cv::Size2f targetSize = cv::Size2f(clickedButton->getWidth(), clickedButton->getHeight());
			targetSize.height += 60;
			selectionIndicator->layout(clickedButton->getPosition() - Vector(0,30,1),targetSize);
			this->viewModeChanged(clickedButton->getText());

		};
	}
	addChild(selectionIndicator);
}

void RelevantImageView::initPhotoInfoPanel()
{	
	photoInfoPanel = new ContentPanel();

	Color c = Colors::Black;
	c.setAlpha(.6f);
	photoInfoPanel->setBackgroundColor(c);
	
	UniformGrid * g = new UniformGrid(cv::Size2i(1,3));

	photoAlbumInfo = new TextPanel();		 
	photoAlbumInfo->setTextColor(Colors::HoloBlueBright);
	//photoAlbumInfo->setAnimateOnLayout(false);

	photoFriendInfo = new TextPanel();
	photoFriendInfo->setTextColor(Colors::HoloBlueBright);
	//photoFriendInfo->setAnimateOnLayout(false);

	photoCaption = new TextPanel();
	photoCaption->setTextColor(Colors::HoloBlueBright);
	//photoCaption->setAnimateOnLayout(false);

	g->addChild(photoAlbumInfo);
	g->addChild(photoCaption);
	g->addChild(photoFriendInfo);

	photoInfoPanel->setContentView(g);

	photoInfoPanel->layout(Vector(),cv::Size2f(400,180));
}

void RelevantImageView::viewModeChanged(string _viewMode)
{
	if (viewMode.compare(_viewMode) != 0 || _viewMode.compare("Random") == 0)
	{
		viewMode = _viewMode;
		show(activeNode);
	}
}

void RelevantImageView::show(FBNode * root)
{	
	activeNode = root;
	NodeQuerySpec friendConfig(3);

	friendConfig.layers[0].insert(make_pair("friends",SelectionConfig(10)));
	friendConfig.layers[1].insert(make_pair("photos",SelectionConfig(10)));
	friendConfig.layers[1].insert(make_pair("albums",SelectionConfig(10)));
	friendConfig.layers[2].insert(make_pair("photos",SelectionConfig(10)));
	
	bool isLoading;
	viewChanged("",DataViewGenerator::getInstance()->getDataView(root,friendConfig,[this](vector<FBNode*> & data){
		this->viewChanged("",data);
	},isLoading));
}

void RelevantImageView::viewChanged(string viewIdentifier, vector<FBNode*> & viewData)
{
	if (isVisible())
	{
		if (this->viewMode.compare("Random") == 0)
			std::random_shuffle(viewData.begin(),viewData.end());
		else
		{
			std::sort(viewData.begin(),viewData.end(),[this](FBNode * a, FBNode * b) -> bool {
			
				if (this->viewMode.compare("Recent Activity") == 0)
					return a->getId().compare(b->getId()) > 0;
				else 
					return a->getId().length() > b->getId().length();

			});
		}
		imageGroup->clearChildren();
		for (auto it = viewData.begin(); it != viewData.end(); it++)
		{
			FBNode * node = (*it);
			View * item = ViewOrchestrator::getInstance()->requestView((*it)->getId(), this);

			if (item == NULL)
			{
				if (node->getNodeType() == NodeType::FacebookImage)
				{
					Panel * p = new Panel(0,0);
					p->setNode(node);
					p->setVisible(true);
					p->setDataPriority(0);
					item = p;
					p->setPanelSize(5,450);				
				}

				if (item != NULL)
					ViewOrchestrator::getInstance()->registerView(node->getId(),item, this);
			}

			if (item != NULL)
			{
				if (node->getNodeType() == NodeType::FacebookImage)
				{	
					item->pointableEnterCallback = [this,node](LeapElement*element, Pointable & pointable){

						if (photoInfoPanel != NULL)
						{
							this->photoAlbumInfo->setText("Album: " + node->getId());
							this->photoCaption->setText("Penis: " + node->getId());
							this->photoFriendInfo->setText("Friend: " + node->getId());

							Panel * photoPanel = (Panel*)element;
							//photoInfoPanel->setPosition(photoPanel->getPosition() - Vector(0,photoInfoPanel->getHeight(),-1));
							this->photoInfoPanel->layout(photoPanel->getPosition() - Vector(0,photoInfoPanel->getHeight(),-1), cv::Size2f(photoPanel->getWidth(), photoInfoPanel->getHeight()));
						}

					};
					item->elementClickedCallback = [this,node](LeapElement*element){
						//this->friendPanelClicked((FriendPanel*)element, node);
					};
					item->setLayoutParams(cv::Size2f(400,400));
					imageGroup->addChild(item);			
				}
			}
		}
		layoutDirty = true;
	}
}


void RelevantImageView::layout(Vector position, cv::Size2f size)
{
	lastPosition = position;
	lastSize = size;
	
	if (topView != NULL && topView->isEnabled() && topView->isVisible())
		topView->layout(position,size);
	else
	{
		mainLayout->layout(position,size);
	}

	layoutDirty = false;
}

void RelevantImageView::onFrame(const Controller & controller)
{	
	if (topView != NULL && topView->isEnabled() && topView->isVisible())
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
				mainLayout->getChildren()->at(1)->OnPointableEnter(testPointable);
			}	
		}
	}
}

void RelevantImageView::setFinishedCallback(const boost::function<void(std::string)> & callback)
{
	finishedCallback = callback;
}



void RelevantImageView::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	;
}

void RelevantImageView::viewOwnershipChanged(View * view, ViewOwner * newOwner)
{
	;
}


void RelevantImageView::draw()
{
	ViewGroup::draw();
}