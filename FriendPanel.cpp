#include "FriendPanel.hpp"
#include "CustomGrid.hpp"
#include "LinearLayout.hpp"
#include <boost/property_tree/ptree.hpp>


FriendPanel::FriendPanel(cv::Size2f targetSize) : 
	ContentPanel()
{
	vector<RowDefinition> gridDefinition;


	gridDefinition.push_back(RowDefinition(.5f));
	gridDefinition.push_back(RowDefinition(.5f));
	gridDefinition[0].ColumnWidths.push_back(.5f);
	gridDefinition[0].ColumnWidths.push_back(.5f);
	gridDefinition[1].ColumnWidths.push_back(1);

	layoutGroup = new CustomGrid(gridDefinition, false);

	friendViewGroup = new UniformGrid(cv::Size2i(2,1));

	this->setContentView(layoutGroup);

	this->setEventHandlingMode(true);

	this->NudgeAnimationEnabled = true;
	setBackgroundColor(Colors::White);


}

void FriendPanel::setDataPriority(float _dataPriority)
{
	this->dataPriority = _dataPriority;
	for (auto it = friendViewGroup->getChildren()->begin(); it != friendViewGroup->getChildren()->end();it++)
	{
		Panel * imagePanel = dynamic_cast<Panel*>(*it);
		imagePanel->setDataPriority(dataPriority);
	}
	if (friendPhotoPanel != NULL)
		friendPhotoPanel->setDataPriority(dataPriority);
}



void FriendPanel::show(FBNode * _node, boost::function<void()> loadCompleteCallback)
{	
	this->activeNode = _node;
	
	View * item = ViewOrchestrator::getInstance()->requestView(activeNode->getId() + "/name", this);
	 
	if (item != NULL)
	{
		nameText = (TextPanel*)item;
		//PanelFactory::getInstance().setStyle(nameText,TextStyles::Heading2);
	}
	else
	{
		nameText = new TextPanel(activeNode->getAttribute("name")); // PanelFactory::getInstance().buildTextPanel(TextStyles::Heading2);
		ViewOrchestrator::getInstance()->registerView(activeNode->getId() + "/name",nameText,this);
	}

	boost::property_tree::ptree labelConfig = GlobalConfig::tree()->get_child("FriendPanel.NameLabel");
	nameText->setTextSize(labelConfig.get<float>("FontSize"));
	nameText->setTextFitPadding(labelConfig.get<float>("TextPadding"));
	nameText->setTextColor(Color(labelConfig.get_child("TextColor")));
	nameText->setBackgroundColor(Color(labelConfig.get_child("BackgroundColor")));

	if (labelConfig.get<bool>("RandomizeBackground"))
	{
		float ra = ((float)(std::rand()%20))/40.0f;
		nameText->getBackgroundColor().setAlpha(.5f + ra);
	}


	friendPhotoPanel = (Panel*)ViewOrchestrator::getInstance()->requestView(activeNode->getId() + "/friendphoto", this);
	if (friendPhotoPanel == NULL)
	{
		friendPhotoPanel = new Panel(0,0);
		friendPhotoPanel->setNode(activeNode);
		ViewOrchestrator::getInstance()->registerView(activeNode->getId() + "/friendphoto",friendPhotoPanel, this);
	}
	
	friendPhotoPanel->setVisible(true);		
	friendPhotoPanel->setDataPriority(dataPriority);		


	layoutGroup->clearChildren();	

	layoutGroup->addChild(nameText);
	layoutGroup->addChild(friendPhotoPanel);

	layoutGroup->addChild(friendViewGroup);

	updateLoading();

	//if (items.size() < 2 && activeNode->loadState["photos"].requestedCount < 2)
	//{
	//	FriendPanel * v = this;
	//	stringstream loadstr;
	//	loadstr << activeNode->getId();
	//	loadstr << "?fields=photos.limit(2).fields(id,name,images)";
	//	activeNode->loadState["photos"].requestedCount  = 2;
	//	//}
	//	//else 
	//	//	loadstr << "?fields=albums.limit(2).fields(id,name,photos.limit(1).fields(id,name,images))";

	//	FBDataSource::instance->loadField(activeNode,loadstr.str(),"",[v](FBNode * _node){

	//		FriendPanel * v2= v;
	//		v->postTask([v2,_node](){

	//			if (v2->activeNode == _node)
	//				v2->updateLoading();
	//		});		
	//	});
	//}

	//NodeQuerySpec friendConfig(2,3);	
	//friendConfig.layers[0].insert(make_pair("photos",SelectionConfig(3)));
	//friendConfig.layers[0].insert(make_pair("albums",SelectionConfig(3,0,false)));
	//friendConfig.layers[1].insert(make_pair("photos",SelectionConfig(1)));


	//bool isLoading;
	//vector<FBNode*> dataView = DataViewGenerator::getInstance()->getDataView(activeNode,friendConfig,[this,loadCompleteCallback](vector<FBNode*> & data){
	//	this->viewChanged("",data);
	//	if (data.size() > 0)
	//		loadCompleteCallback();
	//},isLoading);

	//if (dataView.size() > 0)
	//{
	//	loadCompleteCallback();
	//	viewChanged("",dataView);
	//}

}

void FriendPanel::updateLoading()
{	
	auto photoNodes = activeNode->Edges.get<EdgeTypeIndex>().equal_range("photos");
	int itemCount = 2;

	bool photosLoaded = false, albumsLoaded = true;// false;

	bool loaded = false;
	for (int i=0;i < itemCount;i++)
	{
		if (photoNodes.first != photoNodes.second)
		{
			addNode(photoNodes.first->Node);
			photoNodes.first++;
			photosLoaded = true;
		}
	}

	//if (!photosLoaded)// && !albumsLoaded)
	//{
	//	FriendPanel * v = this;
	//	stringstream loadstr;
	//	loadstr << activeNode->getId();
	//	if (!photosLoaded && activeNode->loadState["photos"].requestedCount < 2)
	//	{
	//		loadstr << "?fields=photos.limit(2).fields(id,name,images)";
	//		activeNode->loadState["photos"].requestedCount  = 2;
	//	}
	//	else 
	//		loadstr << "?fields=albums.limit(2).fields(id,name,photos.limit(1).fields(id,name,images))";

	//	FBDataSource::instance->loadField(activeNode,loadstr.str(),"",[v](FBNode * _node){

	//		FriendPanel * v2= v;
	//		v->postTask([v2,_node](){

	//			if (v2->activeNode == _node)
	//				v2->updateLoading();
	//		});		
	//	});

	//}

}

void FriendPanel::addNode(FBNode * childNode)
{
	if (childNode->getNodeType().compare("photos") == 0)
	{
		items.insert(make_pair(childNode->getId(),childNode));
		View * item = ViewOrchestrator::getInstance()->requestView(childNode->getId(), this);
		if (item == NULL)
		{
			Panel * p = new Panel(0,0);
			p->setNode(childNode);
			p->setVisible(true);
			item = p;			
			ViewOrchestrator::getInstance()->registerView(childNode->getId(),item, this);
		}		
		item->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(0,0,0,0)));
		((Panel*)item)->setDataPriority(dataPriority);
		friendViewGroup->addChild(item);		
	}
	layoutDirty = true;
}



void FriendPanel::viewOwnershipChanged(View * view, ViewOwner * newOwner)
{	
	if (!friendViewGroup->remove(view))	
	{
		layoutGroup->remove(view);
	}
}

