#include "FriendPanel.hpp"
#include "CustomGrid.hpp"
#include "LinearLayout.hpp"


FriendPanel::FriendPanel(cv::Size2f targetSize) : 
	ContentPanel()
{
	vector<RowDefinition> gridDefinition;


	gridDefinition.push_back(RowDefinition(.55f));
	gridDefinition.push_back(RowDefinition(.45f));
	gridDefinition[0].ColumnWidths.push_back(.45f);
	gridDefinition[0].ColumnWidths.push_back(.55f);
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
		//if (imagePanel != NULL)
			imagePanel->setDataPriority(dataPriority);
	}
	if (friendPhotoPanel != NULL)
		friendPhotoPanel->setDataPriority(dataPriority);
}



void FriendPanel::show(FBNode * _node, boost::function<void()> loadCompleteCallback)
{	
	this->node = _node;
	
	View * item = ViewOrchestrator::getInstance()->requestView(node->getId() + "/name", this);
	 
	if (item != NULL)
	{
		nameText = (TextPanel*)item;
		PanelFactory::getInstance().setStyle(nameText,TextStyles::Heading2);
	}
	else
	{
		nameText = PanelFactory::getInstance().buildTextPanel(node->getAttribute("name"),TextStyles::Heading2);
		ViewOrchestrator::getInstance()->registerView(node->getId() + "/name",nameText,this);
	}


	friendPhotoPanel = (Panel*)ViewOrchestrator::getInstance()->requestView(node->getId() + "/friendphoto", this);
	if (friendPhotoPanel == NULL)
	{
		friendPhotoPanel = new Panel(0,0);
		friendPhotoPanel->setNode(node);
		friendPhotoPanel->setVisible(true);
		friendPhotoPanel->setDataPriority(dataPriority);			
		ViewOrchestrator::getInstance()->registerView(node->getId() + "/friendphoto",friendPhotoPanel, this);
	}


	layoutGroup->clearChildren();	

	layoutGroup->addChild(nameText);
	layoutGroup->addChild(friendPhotoPanel);

	layoutGroup->addChild(friendViewGroup);
		
	NodeQuerySpec friendConfig(2,3);	
	friendConfig.layers[0].insert(make_pair("photos",SelectionConfig(3)));
	friendConfig.layers[0].insert(make_pair("albums",SelectionConfig(3,0,false)));
	friendConfig.layers[1].insert(make_pair("photos",SelectionConfig(1)));


	bool isLoading;
	vector<FBNode*> dataView = DataViewGenerator::getInstance()->getDataView(node,friendConfig,[this,loadCompleteCallback](vector<FBNode*> & data){
		this->viewChanged("",data);
		if (data.size() > 0)
			loadCompleteCallback();
	},isLoading);

	if (dataView.size() > 0)
	{
		loadCompleteCallback();
		viewChanged("",dataView);
	}

}

void FriendPanel::viewChanged(string viewIdentifier, vector<FBNode*> & viewData)
{
	for (auto it = viewData.begin(); it != viewData.end(); it++)
	{
		FBNode * childNode = (*it);

		if (childNode->getNodeType().compare("photos") == 0)
		{
			View * item = ViewOrchestrator::getInstance()->requestView(childNode->getId(), this);
			if (item == NULL)
			{
				Panel * p = new Panel(0,0);
				p->setNode(childNode);
				p->setVisible(true);
				p->setDataPriority(dataPriority);
				item = p;
			}

			if (item != NULL)
				ViewOrchestrator::getInstance()->registerView(childNode->getId(),item, this);

			friendViewGroup->addChild(item);		
		}
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

