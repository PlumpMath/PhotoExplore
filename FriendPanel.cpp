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

	friendViewGroup = new UniformGrid(cv::Size2i(3,1));

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



void FriendPanel::show(FBNode * _node)
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
	viewChanged("",DataViewGenerator::getInstance()->getDataView(node,friendConfig,[this](vector<FBNode*> & data){
		this->viewChanged("",data);
	},isLoading));
}

void FriendPanel::OnPointableEnter(Pointable & pointable)
{
	PanelBase::OnPointableEnter(pointable);
	//if (pointable.isValid())
	//{		
	//	activePointable = pointable;
	//	originalSize = cv::Size2f(600,400);
	//	float sizeScale = 1.5f;
	//	if (lastSizeScale != sizeScale)
	//	{
	//		lastSizeScale = sizeScale;
	//		cv::Size2f newSize = cv::Size2f(originalSize.width*sizeScale,originalSize.height*sizeScale);
	//		layout(position-Vector(newSize.width/2.0f,newSize.height/2.0f,0),newSize);
	//	}
	//}
}


void FriendPanel::OnPointableExit(Pointable & pointable)
{
	PanelBase::OnPointableExit(pointable);
	//float sizeScale = 1.0f;
	//if (lastSizeScale != sizeScale)
	//{
	//	lastSizeScale = sizeScale;
	//	cv::Size2f newSize = cv::Size2f(originalSize.width*sizeScale,originalSize.height*sizeScale);
	//	layout(position-Vector(newSize.width/2.0f,newSize.height/2.0f,0),newSize);
	//}
	//activePointable = Pointable();	
	//setBackgroundColor(Colors::White);
}

void FriendPanel::onFrame(const Controller & controller)
{
	//activePointable = controller.frame().pointable(activePointable.id());
	//
	//if (activePointable.isValid())
	//{
	//	float sizeScale = 1;
	//	if (activePointable.touchDistance() < .1f)
	//		sizeScale = 1.8f;
	//	else if (activePointable.touchDistance() < .5f)
	//		sizeScale = 1.4f;
	//	else if (activePointable.touchDistance() < .9f)
	//		sizeScale = 1.0f;
	//	
	//	originalSize = cv::Size2f(600,400);
	//	if (lastSizeScale != sizeScale)
	//	{			
	//		lastSizeScale = sizeScale;
	//		cv::Size2f newSize = cv::Size2f(originalSize.width*sizeScale,originalSize.height*sizeScale);
	//		//animatePanelSize(newSize, 400);
	//		layout(position-Vector(newSize.width/2.0f,newSize.height/2.0f,0),newSize);
	//	}




		//float scale = min<float>(1.0f,.5f-activePointable.touchDistance());
		//if (scale >= 0.0f)
		//{
		//	Color newBG = Colors::HoloBlueBright;
		//	//newBG.r += (1.0f - newBG.r)*(1.0f-scale);
		//	//newBG.g += (1.0f - newBG.g)*(1.0f-scale);
		//	//newBG.b += (1.0f - newBG.b)*(1.0f-scale);
		//	newBG.setAlpha(scale);
		//	setBackgroundColor(newBG);
	//}
	//}
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

