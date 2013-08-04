#include "FriendPanel.hpp"
#include "CustomGrid.hpp"
#include "LinearLayout.hpp"
#include <boost/property_tree/ptree.hpp>


FriendPanel::FriendPanel(cv::Size2f targetSize) : 
	ContentPanel()
{
	//vector<RowDefinition> gridDefinition;


	//gridDefinition.push_back(RowDefinition(.5f));
	//gridDefinition.push_back(RowDefinition(.5f));
	//gridDefinition[0].ColumnWidths.push_back(.5f);
	//gridDefinition[0].ColumnWidths.push_back(.5f);
	//gridDefinition[1].ColumnWidths.push_back(1);

	layoutGroup = new UniformGrid(cv::Size2i(2,2)); //new CustomGrid(gridDefinition, false);

	//friendViewGroup = new UniformGrid(cv::Size2i(2,1));

	this->setContentView(layoutGroup);

	this->setEventHandlingMode(true);

	this->NudgeAnimationEnabled = true;
	setBackgroundColor(Colors::White);


}

void FriendPanel::setDataPriority(float _dataPriority)
{
	this->dataPriority = _dataPriority;
	for (auto it = layoutGroup->getChildren()->begin(); it != layoutGroup->getChildren()->end();it++)
	{
		Panel * imagePanel = dynamic_cast<Panel*>(*it);
		if (imagePanel != NULL)
			imagePanel->setDataPriority(dataPriority);
	}
	if (friendPhotoPanel != NULL)
		friendPhotoPanel->setDataPriority(dataPriority);
}

FBNode * FriendPanel::getNode()
{
	return activeNode;
}


void FriendPanel::show(FBNode * _node)
{	
	this->activeNode = _node;
	
	View * item = ViewOrchestrator::getInstance()->requestView(activeNode->getId() + "/name_small", this);
	 
	if (item != NULL)
	{
		nameText = (TextPanel*)item;
	}
	else
	{
		nameText = new TextPanel(activeNode->getAttribute("name")); 
		ViewOrchestrator::getInstance()->registerView(activeNode->getId() + "/name_small",nameText,this);
	}

	boost::property_tree::ptree labelConfig = GlobalConfig::tree()->get_child("FriendPanel.NameLabel");
	nameText->setTextSize(labelConfig.get<float>("TextSize"));
	nameText->setTextFitPadding(labelConfig.get<float>("TextPadding"));
	nameText->setTextColor(Color(labelConfig.get_child("TextColor")));
	nameText->setBackgroundColor(Color(labelConfig.get_child("BackgroundColor")));

	//if (labelConfig.get<bool>("RandomizeBackground"))
	//{
	//	float ra = ((float)(std::rand()%20))/40.0f;
	//	nameText->getBackgroundColor().setAlpha(.5f + ra);
	//}


	friendPhotoPanel = (Panel*)ViewOrchestrator::getInstance()->requestView(activeNode->getId() + "/friendphoto", this);
	if (friendPhotoPanel == NULL)
	{
		friendPhotoPanel = new Panel(0,0);
		friendPhotoPanel->show(activeNode);
		ViewOrchestrator::getInstance()->registerView(activeNode->getId() + "/friendphoto",friendPhotoPanel, this);
	}
	
	friendPhotoPanel->setVisible(true);		
	friendPhotoPanel->setDataPriority(dataPriority);		


	layoutGroup->clearChildren();	

	layoutGroup->addChild(nameText);
	layoutGroup->addChild(friendPhotoPanel);

	//layoutGroup->addChild(friendViewGroup);

	updateLoading();

	layoutDirty = true;
}

void FriendPanel::updateLoading()
{	
	auto photoNodes = activeNode->Edges.get<EdgeTypeIndex>().equal_range("photos");
	int itemCount = 2;

	bool photosLoaded = false, albumsLoaded = true;// false;

	bool loaded = false;
	int i=0;
	for (;i < itemCount;i++)
	{
		if (photoNodes.first != photoNodes.second)
		{
			addNode(photoNodes.first->Node);
			photoNodes.first++;
			photosLoaded = true;
		}
		else
			break;
	}
	
	auto albumNodes = activeNode->Edges.get<EdgeTypeIndex>().equal_range("albums");
	for (;i < itemCount;i++)
	{
		if (albumNodes.first != albumNodes.second)
		{
			photoNodes = albumNodes.first->Node->Edges.get<EdgeTypeIndex>().equal_range("photos");
			if (photoNodes.first != photoNodes.second)
			{
				addNode(photoNodes.first->Node);
			}
			albumNodes.first++;
		}
		else
			break;
	}	
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
			p->show(childNode);
			p->setVisible(true);
			item = p;			
			ViewOrchestrator::getInstance()->registerView(childNode->getId(),item, this);
		}		
		item->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(0,0,0,0)));
		((Panel*)item)->setDataPriority(dataPriority);
		layoutGroup->addChild(item);		
	}
	layoutDirty = true;
}



void FriendPanel::viewOwnershipChanged(View * view, ViewOwner * newOwner)
{	
	/*if (!friendViewGroup->remove(view))	
	{*/
	layoutGroup->remove(view);
	//}
}

