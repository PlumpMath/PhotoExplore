#include "AlbumPanel.hpp"
#include "CustomGrid.hpp"

AlbumPanel::AlbumPanel() : ContentPanel()
{
	albumGroup = new UniformGrid(cv::Size2i(2,2));
		
	this->setContentView(albumGroup);

	this->NudgeAnimationEnabled = true;

	setBorderThickness(0);
	setBackgroundColor(Colors::White);

	this->setEventHandlingMode(true);
}

FBNode * AlbumPanel::getNode()
{
	return albumNode;
}

void AlbumPanel::show(FBNode * node)
{	
	albumNode = node;
	NodeQuerySpec friendConfig(1);
	friendConfig.layers[0].insert(make_pair("photos",SelectionConfig(4)));
		
	bool isLoading;
	vector<FBNode*> initData = DataViewGenerator::getInstance()->getDataView(node,friendConfig,[this](vector<FBNode*> & data){
		this->viewChanged(std::string(""),data);
	},isLoading);
	viewChanged(std::string(""),initData);
}

static cv::Size2i calculatePanelSize(int count)
{	
	int numRows, numColumns;
	
	switch (count)
	{
	case 0:
		numRows = numColumns = 1;
		break;
	case 1:
		numRows = numColumns = 1;
		break;
	case 2:
	case 3:		
		numRows = 2;
		numColumns = 1;
		break;		
	case 4:
	case 5:
		numRows = numColumns = 2;
		break;
	case 6:
	case 7:
	case 8:
		numRows = 2;
		numColumns = 3;
		break;
	case 9:
	default:
		numRows = numColumns = 3;
		break;					
	}

	return cv::Size2i(numColumns,numRows);
}

void AlbumPanel::viewChanged(string viewIdentifier, vector<FBNode*> & viewData)
{
	albumGroup->clearChildren();
	for (auto it = viewData.begin(); it != viewData.end(); it++)
	{
		FBNode * childNode = (*it);
		View * item = ViewOrchestrator::getInstance()->requestView(childNode->getId(), this);

		if (item == NULL)
		{
			PicturePanel * p = new PicturePanel();
			p->show(childNode);
			p->setVisible(true);
			item = p;
			ViewOrchestrator::getInstance()->registerView(childNode->getId(),item, this);
		}
		item->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(2,2,2,2)));
		((PicturePanel*)item)->setDataPriority(currentDataPriority);
		albumGroup->addChild(item);					
	}

	((UniformGrid*)albumGroup)->resize(calculatePanelSize(albumGroup->getChildren()->size()));

	layoutDirty = true;
}


void AlbumPanel::setDataPriority(float dataPriority)
{
	if (currentDataPriority != dataPriority)
	{
		currentDataPriority = dataPriority;

		for (auto it = albumGroup->getChildren()->begin(); it != albumGroup->getChildren()->end();it++)
		{
			PicturePanel * imagePanel = dynamic_cast<PicturePanel*>(*it);
			if (imagePanel != NULL)
				imagePanel->setDataPriority(dataPriority);
		}
	}
}

void AlbumPanel::viewOwnershipChanged(View * view, ViewOwner * newOwner)
{
	if (this->albumGroup != NULL)
		this->albumGroup->remove(view);
}