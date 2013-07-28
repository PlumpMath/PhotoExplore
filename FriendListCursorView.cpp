#include "FriendListCursorView.hpp"
#include "CustomGrid.hpp"
#include "Button.hpp"

FriendListCursorView::FriendListCursorView() : DataListActivity(2)
{
	showPicturelessFriends = false;

	auto editTextConfig = GlobalConfig::tree()->get_child("FriendLookupView.LookupDialog.FriendNameTextBox");

	editText = new TextEditPanel();
	editText->setTextSize(editTextConfig.get<float>("TextSize"),false);
	editText->setTextColor(Color(editTextConfig.get_child("TextColor")));
	editText->setBackgroundColor(Color(editTextConfig.get_child("BackgroundColor")));
	editText->setBorderColor(Color(editTextConfig.get_child("BorderColor")));
	editText->setBorderThickness(editTextConfig.get<float>("BorderThickness"));
	editText->setTextFitPadding(editTextConfig.get<float>("TextPadding"));
	editText->setTextAlignment(editTextConfig.get<int>("TextAlignment"));

	

	vector<RowDefinition> gridDefinition;	
	gridDefinition.push_back(RowDefinition(.35f));
	gridDefinition.push_back(RowDefinition(.65f));
	//gridDefinition.push_back(RowDefinition(.5f));
	//gridDefinition.push_back(RowDefinition(.3f));
	gridDefinition[0].ColumnWidths.push_back(1);
	gridDefinition[1].ColumnWidths.push_back(1);
	//gridDefinition[1].ColumnWidths.push_back(0.7f);
	//gridDefinition[1].ColumnWidths.push_back(0.3f);
	//gridDefinition[2].ColumnWidths.push_back(.5f);
	//gridDefinition[2].ColumnWidths.push_back(.5f);
	
	editTextConfig = GlobalConfig::tree()->get_child("FriendLookupView.LookupDialog.FriendLookupLabel");

	labelText = new TextPanel(editTextConfig.get<string>("Text"));
	labelText->setTextSize(editTextConfig.get<float>("TextSize"),false);
	labelText->setTextColor(Color(editTextConfig.get_child("TextColor")));
	labelText->setBackgroundColor(Color(editTextConfig.get_child("BackgroundColor")));
	labelText->setBorderColor(Color(editTextConfig.get_child("BorderColor")));
	labelText->setBorderThickness(editTextConfig.get<float>("BorderThickness"));
	labelText->setTextFitPadding(editTextConfig.get<float>("TextPadding"));
	labelText->setTextAlignment(editTextConfig.get<int>("TextAlignment"));

	
	
	//editTextConfig = GlobalConfig::tree()->get_child("FriendLookupView.LookupDialog.ClearButton");

	//TextPanel * clearButton = new Button(editTextConfig.get<string>("Text"));
	//clearButton->setTextSize(editTextConfig.get<float>("TextSize"),false);
	//clearButton->setTextColor(Color(editTextConfig.get_child("TextColor")));
	//clearButton->setBackgroundColor(Color(editTextConfig.get_child("BackgroundColor")));
	//clearButton->setBorderColor(Color(editTextConfig.get_child("BorderColor")));
	//clearButton->setBorderThickness(editTextConfig.get<float>("BorderThickness"));
	//clearButton->setTextFitPadding(editTextConfig.get<float>("TextPadding"));
	//clearButton->setTextAlignment(editTextConfig.get<int>("TextAlignment"));

	//clearButton->elementClickedCallback = [this](LeapElement * element){
	//	((FBFriendsCursor*)allFriendsCursor)->reset();
	//	this->show(allFriendsCursor);
	//};


	//
	//editTextConfig = GlobalConfig::tree()->get_child("FriendLookupView.LookupDialog.SearchButton");

	//TextPanel * searchButton = new Button(editTextConfig.get<string>("Text"));
	//searchButton->setTextSize(editTextConfig.get<float>("TextSize"),false);
	//searchButton->setTextColor(Color(editTextConfig.get_child("TextColor")));
	//searchButton->setBackgroundColor(Color(editTextConfig.get_child("BackgroundColor")));
	//searchButton->setBorderColor(Color(editTextConfig.get_child("BorderColor")));
	//searchButton->setBorderThickness(editTextConfig.get<float>("BorderThickness"));
	//searchButton->setTextFitPadding(editTextConfig.get<float>("TextPadding"));
	//searchButton->setTextAlignment(editTextConfig.get<int>("TextAlignment"));

	//searchButton->elementClickedCallback = [this](LeapElement * element){
	//	
	//	if (this->editText->getText().length() > 0)
	//	{
	//		this->show(searchCursor);
	//		searchCursor->lookupName(this->editText->getText());
	//	}
	//	else 
	//	{
	//		((FBFriendsCursor*)allFriendsCursor)->reset();
	//		this->show(allFriendsCursor);
	//	}
	//};

	//editTextConfig = GlobalConfig::tree()->get_child("FriendLookupView.LookupDialog.CloseButton");

	//TextPanel * backButton = new Button(editTextConfig.get<string>("Text"));
	//backButton->setTextSize(editTextConfig.get<float>("TextSize"),false);
	//backButton->setTextColor(Color(editTextConfig.get_child("TextColor")));
	//backButton->setBackgroundColor(Color(editTextConfig.get_child("BackgroundColor")));
	//backButton->setBorderColor(Color(editTextConfig.get_child("BorderColor")));
	//backButton->setBorderThickness(editTextConfig.get<float>("BorderThickness"));
	//backButton->setTextFitPadding(editTextConfig.get<float>("TextPadding"));
	//backButton->setTextAlignment(editTextConfig.get<int>("TextAlignment"));

	//backButton->elementClickedCallback = [this](LeapElement * element){
	//	
	//	this->lookupActive = false;
	//	this->layoutDirty = true;
	//};


	CustomGrid * lookupDialogGrid = new CustomGrid(gridDefinition);
	lookupDialogGrid->addChild(labelText);	
	lookupDialogGrid->addChild(editText);
	//lookupDialogGrid->addChild(searchButton);
	//lookupDialogGrid->addChild(backButton);
	//lookupDialogGrid->addChild(clearButton);
	
	lookupPanel = lookupDialogGrid;
	
	addChild(lookupDialogGrid);

	lookupDialogState = 0;

	int dialogHideDelay = GlobalConfig::tree()->get<int>("FriendLookupView.LookupDialog.HideAfter");

	editText->setTextChangedCallback([this,dialogHideDelay](string newText){
		
		lookupDialogTimer.countdown(dialogHideDelay);
		if (lookupDialogState != 2 && newText.length() > 0)
		{
			lookupDialogState = 2;
			layoutDirty = true;			
		}
		
		
		if (lookupDialogState != 0 && newText.length() == 0)
		{			
			lookupDialogState = 0;
			layoutDirty = true;			
			
			showPicturelessFriends = false;
			((FBFriendsCursor*)allFriendsCursor)->reset();			
			this->show(allFriendsCursor);
			allFriendsCursor->getNext();
		}
		else		
		{
			if (this->cursor != searchCursor)
			{
				searchCursor->lookupName(newText);				
				this->show(searchCursor);			
				searchCursor->getNext();
				showPicturelessFriends = true;
			}
			else
			{
				currentRightBoundary = 0;
				lastUpdatePos = 100000;
				itemScroll->getFlyWheel()->overrideValue(0);

				items.clear();
				itemGroup->clearChildren();

				searchCursor->lookupName(newText);		
				searchCursor->getNext();
			}

			updateLoading();
		}
	});

}

void FriendListCursorView::suspend()
{
	DataListActivity::suspend();

	editText->setText("");
	this->lookupDialogState = 0;
}

void FriendListCursorView::setUserNode(FBNode * node)
{
	searchCursor = new FBFriendsFQLCursor("",node);
	allFriendsCursor = new FBFriendsCursor(node);

	allFriendsCursor->getNext();
	this->show(allFriendsCursor);
}

FBDataView * FriendListCursorView::getDataView(FBNode * itemNode)
{
	FBDataView * itemV = NULL;
	if (showPicturelessFriends || (itemNode->Edges.get<EdgeTypeIndex>().count("photos") + itemNode->Edges.get<EdgeTypeIndex>().count("albums") >= 2))
	{
		FriendPanel * item = (FriendPanel*)ViewOrchestrator::getInstance()->requestView(itemNode->getId(), this);

		if (item == NULL)
		{
			item = new FriendPanel(cv::Size2f(600,400));					
			ViewOrchestrator::getInstance()->registerView(itemNode->getId(),item, this);
		}

		item->elementClickedCallback = [item](LeapElement*element){
			FacebookDataDisplay::getInstance()->displayNode(NULL,item->getNode(),"");
		};
		item->setLayoutParams(LayoutParams(cv::Size2f(600,400),cv::Vec4f(5,5,5,5)));
		item->setVisible(true);
		item->show(itemNode);
		itemV = item;
	}
	else if (itemNode->getAttribute("tried_load").length() == 0)
	{			
		itemNode->Edges.insert(Edge("tried_load","1"));
		stringstream load2;
		load2 << itemNode->getId() << "?fields=photos.fields(id,name,images).limit(2)";					
		FBDataSource::instance->loadField(itemNode,load2.str(),"",[itemNode,this](FBNode * nn)
		{
			FriendListCursorView * me = this;
			this->postTask([me,nn](){				
				me->addNode(nn);
			});
		}); 
	}
	return itemV;
}

void FriendListCursorView::setFinishedCallback(const boost::function<void(std::string)> & callback)
{
	viewFinishedCallback = callback;
}

void FriendListCursorView::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	if (gestureType.compare("shake") == 0)
	{
		viewFinishedCallback("done");
	} 
	else if (gestureType.compare("pointing") == 0)
	{
		itemScroll->getFlyWheel()->impartVelocity(0);
	}
}

bool FriendListCursorView::onLeapGesture(const Controller & controller, const Gesture & gesture)
{
	return itemScroll->onLeapGesture(controller, gesture);
}

void FriendListCursorView::getTutorialDescriptor(vector<string> & tutorial)
{	
	tutorial.push_back("swipe");
	tutorial.push_back("point_stop");
	tutorial.push_back("shake");
	tutorial.push_back("KeyboardNameSearch");
}

void FriendListCursorView::onGlobalFocusChanged(bool isFocused)
{
	if (isFocused)
		SwipeGestureDetector::getInstance().setFlyWheel(itemScroll->getFlyWheel());
	else
		SwipeGestureDetector::getInstance().setFlyWheel(NULL);
}

void FriendListCursorView::layout(Vector position, cv::Size2f size)
{
	Logger::stream("FriendListCursorView","INFO") << "Layout. Position = " << position.toString() << endl;
	DataListActivity::layout(position,size);
	
	cv::Size2f dialogSize = cv::Size2f(GlobalConfig::tree()->get<float>("FriendLookupView.LookupDialog.Width"),GlobalConfig::tree()->get<float>("FriendLookupView.LookupDialog.Height"));
	Vector dialogPosition = position + Vector((size.width-dialogSize.width)*.5f,(size.height-dialogSize.height)*.5f,10);

	if (lookupDialogState == 2)
	{
		//editText->setLayoutDuration(200);
		//labelText->setLayoutDuration(200);
		labelText->setAnimateOnLayout(false);
		editText->setAnimateOnLayout(false);
		
		auto labelTextConfig = GlobalConfig::tree()->get_child("FriendLookupView.LookupDialog.FriendLookupLabel");
		labelText->setText(labelTextConfig.get<string>("Text"));

		lookupPanel->layout(dialogPosition,dialogSize);
	}
	else if (lookupDialogState == 1)
	{		
		
		editText->setLayoutDuration(600);
		editText->setAnimateOnLayout(true);
		labelText->setLayoutDuration(600);
		labelText->setAnimateOnLayout(true);
		float tutorialHeight = GlobalConfig::tree()->get<float>("Tutorial.Height");
		float scrollBarHeight = GlobalConfig::tree()->get<float>("ScrollView.ScrollBar.Height");

		dialogSize.height = (tutorialHeight - scrollBarHeight)*.5f;
		dialogPosition = position + Vector((size.width - dialogSize.width)*.5f,size.height,10);
		//lookupPanel->layout(dialogPosition,dialogSize);	

		editText->layout(dialogPosition,dialogSize);
		labelText->setText("Friends with name:");		
		labelText->layout(dialogPosition-Vector(dialogSize.width*.5f,0,0),cv::Size2f(dialogSize.width*.5f,dialogSize.height));
	}
	else
	{				

		editText->setLayoutDuration(600);
		editText->setAnimateOnLayout(true);
		labelText->setLayoutDuration(600);
		labelText->setAnimateOnLayout(true);
		lookupPanel->layout(dialogPosition+Vector(0,size.height,-20),dialogSize);
	}
}


void FriendListCursorView::update()
{
	if (lookupDialogState == 2 && lookupDialogTimer.elapsed())
	{
		lookupDialogState = 1;
		layoutDirty = true;			
	}

	DataListActivity::update();
}