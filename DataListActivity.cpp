#include "DataListActivity.hpp"
#include "GlobalConfig.hpp"
#include "FixedAspectGrid.hpp"
#include "PanelBase.h"
#include "TextPanel.h"

DataListActivity::DataListActivity(int _rowCount) :
	rowCount(_rowCount)
{
	cursor = NULL;

	itemGroup = new FixedAspectGrid(cv::Size2i(0,rowCount),1.0f);	
	scrollBar = new ScrollBar();
	itemScroll = new ScrollingView(itemGroup);
	scrollBar->setScrollView(itemScroll);

	loadIndicator = new TextPanel();

	addChild(itemScroll);
	addChild(scrollBar);
	addChild(loadIndicator);
}

DataListActivity::~DataListActivity()
{

}


void DataListActivity::show(FBDataCursor * _cursor)
{
	PointableElementManager::getInstance()->requestGlobalGestureFocus(this);

	if (this->cursor != NULL)
		this->cursor->cursorChangedCallback = [](){};
		
	currentRightBoundary = 0;
	lastUpdatePos = 100000;
	itemScroll->getFlyWheel()->overrideValue(0);

	items.clear();
	itemGroup->clearChildren();
		
	this->cursor = _cursor;
	cursor->cursorChangedCallback = [this](){
		this->updateLoading();
	};
	cursor->getNext();

	layoutDirty = true;
}

void DataListActivity::suspend()
{
	PointableElementManager::getInstance()->releaseGlobalGestureFocus(this);

	float targetPriority = 100;
	for (auto it = itemGroup->getChildren()->begin(); it != itemGroup->getChildren()->end();it++)
	{		
		FBDataView * dataView= (FBDataView*)*it;	
		dataView->setDataPriority(targetPriority);	
	}
}

void DataListActivity::layout(Vector position, cv::Size2f size)
{
	lastPosition = position;
	lastSize = size;

	float tutorialHeight = GlobalConfig::tree()->get<float>("Tutorial.Height");
	float scrollBarHeight = GlobalConfig::tree()->get<float>("ScrollView.ScrollBar.Height");

	itemScroll->layout(position,size);

	float scrollBarWidth = size.width * 0.4f;

	scrollBar->layout(position + Vector((size.width-scrollBarWidth)*.5f,size.height+(tutorialHeight-scrollBarHeight)*.5f,1),cv::Size2f(scrollBarWidth,scrollBarHeight));

	layoutDirty = false;
}

void DataListActivity::update()
{
	ViewGroup::update();
		
	double pos = itemScroll->getFlyWheel()->getPosition();
	if (abs(lastUpdatePos - (-pos)) > 100)
		updateLoading();
}

void DataListActivity::updatePriorities()
{
	float peakPriority = (itemScroll->getMeasuredSize().width*.5f) - itemScroll->getFlyWheel()->getPosition();
	peakPriority -= itemScroll->getFlyWheel()->getVelocity() * GlobalConfig::tree()->get<float>("FriendListView.ScrollAheadTime");

	for (auto it = itemGroup->getChildren()->begin(); it != itemGroup->getChildren()->end();it++)
	{
		PanelBase * item = (PanelBase*)(*it);
			
		float distance = abs((item->getPosition().x + item->getWidth()/2.0f) - peakPriority);
		float targetPriority = min<float>(10,distance/1000.0f);
		
		((FBDataView*)item)->setDataPriority(targetPriority);	
	}	
}

void DataListActivity::updateLoading()
{
	float scrollPosition = itemScroll->getFlyWheel()->getPosition();
	cv::Size2f visibleSize = itemScroll->getMeasuredSize();

	lastUpdatePos = -scrollPosition;

	if (cursor->isLoading)
	{		
		((TextPanel*)loadIndicator)->setText("Loading...");
		((TextPanel*)loadIndicator)->refresh();
	}

	if (!cursor->canLoad)
	{
		((TextPanel*)loadIndicator)->setText("Done!");
		((TextPanel*)loadIndicator)->refresh();
	}


	float leftBound = -scrollPosition;
	float rightBound = -scrollPosition + visibleSize.width;

	float itemWidth = (visibleSize.height/((float)rowCount));

	int loadMore = 0;
	float remainingPixels =  currentRightBoundary - rightBound;
	if (remainingPixels < (itemWidth * 1.0f))
	{
		loadMore = rowCount * 2;
		if (remainingPixels < 0)
			loadMore += ceilf(-remainingPixels/itemWidth)*rowCount;

		int itemCount = itemGroup->getChildren()->size() + loadMore;	
		
		while (items.size() < itemCount)
		{
			FBNode * next = cursor->getNext();
			
			if (next == NULL || !cursor->canLoad)
				break;

			addNode(next);			
		}			
	}

	updatePriorities();
}

void DataListActivity::addNode(FBNode * node)
{
	if (items.count(node) == 0)
	{
		FBDataView * dataView = getDataView(node);
		
		if (dataView == NULL)
			return;

		View * newView = dynamic_cast<View*>(dataView);
		
		if (newView == NULL)
			return;
		
		items.insert(make_pair(node,dataView));
		itemGroup->addChild(newView);
	}

	float itemWidth = (lastSize.height/((float)rowCount));
	currentRightBoundary =  (itemWidth * ceilf((float)(itemGroup->getChildren()->size())/(float)rowCount));	
	this->layoutDirty = true;
}

void DataListActivity::draw()
{
	ViewGroup::draw();
}