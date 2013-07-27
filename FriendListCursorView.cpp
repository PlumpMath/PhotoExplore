#include "FriendListCursorView.hpp"
#include "CustomGrid.hpp"
#include "Button.hpp"

FriendListCursorView::FriendListCursorView() : DataListActivity(2)
{
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
	gridDefinition.push_back(RowDefinition(.2f));
	gridDefinition.push_back(RowDefinition(.5f));
	gridDefinition.push_back(RowDefinition(.3f));
	gridDefinition[0].ColumnWidths.push_back(1);
	gridDefinition[1].ColumnWidths.push_back(0.7f);
	gridDefinition[1].ColumnWidths.push_back(0.3f);
	gridDefinition[2].ColumnWidths.push_back(.5f);
	gridDefinition[2].ColumnWidths.push_back(.5f);
	
	editTextConfig = GlobalConfig::tree()->get_child("FriendLookupView.LookupDialog.FriendLookupLabel");

	TextPanel * labelText = new TextPanel(editTextConfig.get<string>("Text"));
	labelText->setTextSize(editTextConfig.get<float>("TextSize"),false);
	labelText->setTextColor(Color(editTextConfig.get_child("TextColor")));
	labelText->setBackgroundColor(Color(editTextConfig.get_child("BackgroundColor")));
	labelText->setBorderColor(Color(editTextConfig.get_child("BorderColor")));
	labelText->setBorderThickness(editTextConfig.get<float>("BorderThickness"));
	labelText->setTextFitPadding(editTextConfig.get<float>("TextPadding"));
	labelText->setTextAlignment(editTextConfig.get<int>("TextAlignment"));

	
	
	editTextConfig = GlobalConfig::tree()->get_child("FriendLookupView.LookupDialog.ClearButton");

	TextPanel * clearButton = new Button(editTextConfig.get<string>("Text"));
	clearButton->setTextSize(editTextConfig.get<float>("TextSize"),false);
	clearButton->setTextColor(Color(editTextConfig.get_child("TextColor")));
	clearButton->setBackgroundColor(Color(editTextConfig.get_child("BackgroundColor")));
	clearButton->setBorderColor(Color(editTextConfig.get_child("BorderColor")));
	clearButton->setBorderThickness(editTextConfig.get<float>("BorderThickness"));
	clearButton->setTextFitPadding(editTextConfig.get<float>("TextPadding"));
	clearButton->setTextAlignment(editTextConfig.get<int>("TextAlignment"));

	clearButton->elementClickedCallback = [this](LeapElement * element){
		((FBFriendsCursor*)allFriendsCursor)->reset();
		this->show(allFriendsCursor);
	};


	
	editTextConfig = GlobalConfig::tree()->get_child("FriendLookupView.LookupDialog.SearchButton");

	TextPanel * searchButton = new Button(editTextConfig.get<string>("Text"));
	searchButton->setTextSize(editTextConfig.get<float>("TextSize"),false);
	searchButton->setTextColor(Color(editTextConfig.get_child("TextColor")));
	searchButton->setBackgroundColor(Color(editTextConfig.get_child("BackgroundColor")));
	searchButton->setBorderColor(Color(editTextConfig.get_child("BorderColor")));
	searchButton->setBorderThickness(editTextConfig.get<float>("BorderThickness"));
	searchButton->setTextFitPadding(editTextConfig.get<float>("TextPadding"));
	searchButton->setTextAlignment(editTextConfig.get<int>("TextAlignment"));

	searchButton->elementClickedCallback = [this](LeapElement * element){
		
		if (this->editText->getText().length() > 0)
		{
			this->show(searchCursor);
			searchCursor->lookupName(this->editText->getText());
		}
		else 
		{
			((FBFriendsCursor*)allFriendsCursor)->reset();
			this->show(allFriendsCursor);
		}
	};

	editTextConfig = GlobalConfig::tree()->get_child("FriendLookupView.LookupDialog.CloseButton");

	TextPanel * backButton = new Button(editTextConfig.get<string>("Text"));
	backButton->setTextSize(editTextConfig.get<float>("TextSize"),false);
	backButton->setTextColor(Color(editTextConfig.get_child("TextColor")));
	backButton->setBackgroundColor(Color(editTextConfig.get_child("BackgroundColor")));
	backButton->setBorderColor(Color(editTextConfig.get_child("BorderColor")));
	backButton->setBorderThickness(editTextConfig.get<float>("BorderThickness"));
	backButton->setTextFitPadding(editTextConfig.get<float>("TextPadding"));
	backButton->setTextAlignment(editTextConfig.get<int>("TextAlignment"));

	backButton->elementClickedCallback = [this](LeapElement * element){
		
		this->lookupActive = false;
		this->layoutDirty = true;
	};


	CustomGrid * lookupDialogGrid = new CustomGrid(gridDefinition);
	lookupDialogGrid->addChild(labelText);	
	lookupDialogGrid->addChild(editText);
	lookupDialogGrid->addChild(searchButton);
	lookupDialogGrid->addChild(backButton);
	lookupDialogGrid->addChild(clearButton);
	
	lookupPanel = lookupDialogGrid;
	
	addChild(lookupDialogGrid);

	lookupActive = false;
	editText->setTextChangedCallback([this](string newText){
		this->lookupActive = true;
		this->layoutDirty = true;
	});

}

void FriendListCursorView::suspend()
{
	DataListActivity::suspend();

	editText->setText("");
	this->lookupActive = false;
}

void FriendListCursorView::setUserNode(FBNode * node)
{
	searchCursor = new FBFriendsFQLCursor("",node);
	allFriendsCursor = new FBFriendsCursor(node);

	this->show(allFriendsCursor);
}

FBDataView * FriendListCursorView::getDataView(FBNode * itemNode)
{
	FBDataView * itemV = NULL;
	if (itemNode->Edges.get<EdgeTypeIndex>().count("photos") + itemNode->Edges.get<EdgeTypeIndex>().count("albums") >= 2)
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
		stringstream load2;
		load2 << itemNode->getId() << "?fields=photos.fields(id,name,images).limit(2)";					
		FBDataSource::instance->loadField(itemNode,load2.str(),"",[itemNode,this](FBNode * nn)
		{
			itemNode->Edges.insert(Edge("tried_load","yup"));
			this->addNode(itemNode);
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
	DataListActivity::layout(position,size);
	
	cv::Size2f dialogSize = cv::Size2f(GlobalConfig::tree()->get<float>("FriendLookupView.LookupDialog.Width"),GlobalConfig::tree()->get<float>("FriendLookupView.LookupDialog.Height"));
	Vector dialogPosition = position + Vector((size.width-dialogSize.width)*.5f,(size.height-dialogSize.height)*.5f,10);

	if (lookupActive)
	{
		lookupPanel->layout(dialogPosition,dialogSize);
	}
	else
	{		
		lookupPanel->layout(dialogPosition+Vector(0,size.height,-20),dialogSize);
	}
}


