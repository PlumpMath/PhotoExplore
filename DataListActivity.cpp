#include "DataListActivity.hpp"
#include "GlobalConfig.hpp"
#include "FixedAspectGrid.hpp"
#include "PanelBase.h"
#include "TextPanel.h"
#include "Logger.hpp"
#include "SwipeGestureDetector.hpp"

DataListActivity::DataListActivity(int _rowCount) :
	rowCount(_rowCount)
{
	titlePanel = NULL;
	cursor = NULL;

	itemGroup = new FixedAspectGrid(cv::Size2i(0,rowCount),1.0f);	
	scrollBar = new ScrollBar();
	itemScroll = new ScrollingView(itemGroup);
	scrollBar->setScrollView(itemScroll);
	
	float cornerPadding = GlobalConfig::tree()->get<float>("Menu.CornerPadding");
	loadIndicator = new TextPanel("Loading");
	loadIndicator->setTextAlignment(1);
	loadIndicator->setTextSize(GlobalConfig::tree()->get<float>("DataListActivity.LoadingIndicator.TextSize"));
	loadIndicator->setTextFitMode(true);
	loadIndicator->setTextFitPadding(cornerPadding);
	loadIndicator->reloadText();

	loadIndicator->setAnimateOnLayout(false);
	loadIndicator->layout(Vector(),cv::Size2f(300,300));
	
	addChild(itemScroll);
	addChild(scrollBar);
	addChild(loadIndicator);
	loadIndicatorState = 0;
}

DataListActivity::~DataListActivity()
{

}


void DataListActivity::show(FBDataCursor * _cursor)
{
	Logger::stream("DataListActivity","INFO") << "Showing new cursor" << endl;

	LeapInput::getInstance()->requestGlobalGestureFocus(this);

	if (this->cursor != NULL)
		this->cursor->cursorChangedCallback = [](){};
		
	currentRightBoundary = 0;
	lastUpdatePos = 100000;
	itemScroll->getFlyWheel()->overrideValue(0);

	items.clear();
	itemGroup->clearChildren();
		
	this->cursor = _cursor;
	cursor->cursorChangedCallback = [this](){
		DataListActivity * me = this;
		this->postTask([me](){
			me->updateLoading();
		});
	};	
	this->layoutDirty = true;
}


void DataListActivity::setTitlePanel(TextPanel * _titlePanel)
{
	this->remove(titlePanel);
	this->titlePanel = _titlePanel;
	this->addChild(titlePanel);
}


void DataListActivity::suspend()
{
	LeapInput::getInstance()->releaseGlobalGestureFocus(this);

	float targetPriority = 100;
	for (auto it = itemGroup->getChildren()->begin(); it != itemGroup->getChildren()->end();it++)
	{		
		FBDataView * dataView= dynamic_cast<FBDataView*>(*it);	
		dataView->setDataPriority(targetPriority);	
	}
}

void DataListActivity::resume()
{
	LeapInput::getInstance()->requestGlobalGestureFocus(this);

	for (auto it = items.begin(); it != items.end(); it++)
	{
		if (it->second != NULL)
			it->second->show(it->first);
	}

	updatePriorities();	
}

void DataListActivity::layout(Vector position, cv::Size2f size)
{
	lastPosition = position;
	lastSize = size;

	static float scrollBarHeight = GlobalConfig::tree()->get<float>("ScrollView.ScrollBar.Height");

	float menuHeight = getMenuHeight();
	float tutorialHeight = getTutorialHeight();
			
 	if (titlePanel != NULL && titlePanel->isVisible())
	{
		cv::Size2f titleSize(size.width*.4f,menuHeight);
		titlePanel->layout(position + Vector((size.width - titleSize.width)*.5f, -menuHeight,0),titleSize);
	}

	cv::Size2f scrollSize = cv::Size2f(size.width-10,size.height-scrollBarHeight);

	itemScroll->layout(position,scrollSize);

	scrollBar->layout(position + Vector(5,scrollSize.height,1),cv::Size2f(scrollSize.width,scrollBarHeight));
	
	float loadWidth = tutorialHeight*2.0f;
	
	loadIndicator->layout(position + Vector(size.width-loadIndicator->getMeasuredSize().width,size.height,1),cv::Size2f(loadWidth,tutorialHeight));
	loadIndicator->layout(position + Vector(size.width-loadIndicator->getMeasuredSize().width,size.height,1),cv::Size2f(loadWidth,tutorialHeight));

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
		View * view = *it;

		PanelBase * panelView = dynamic_cast<PanelBase*>(view);

		float distance = abs((panelView->getPosition().x + panelView->getWidth()/2.0f) - peakPriority);
		float targetPriority = min<float>(10,distance/1000.0f);
		
		FBDataView * dataView = dynamic_cast<FBDataView*>(view);
		dataView->setDataPriority(targetPriority);	
	}	
}

void DataListActivity::onGlobalFocusChanged(bool isFocused)
{
	if (isFocused)
	{
		SwipeGestureDetector::getInstance().setFlyWheel(itemScroll->getFlyWheel());
	}
	else
	{
		itemScroll->getFlyWheel()->setVelocity(0);
		SwipeGestureDetector::getInstance().setFlyWheel(NULL);
	}
}



void DataListActivity::updateLoading()
{
	static double showLoadIndicatorThreshold = GlobalConfig::tree()->get<double>("DataListActivity.LoadingIndicator.LoadTimeDisplayThreshold");

	Timer loadTimer;
	loadTimer.start();

	float scrollPosition = itemScroll->getFlyWheel()->getPosition();
	cv::Size2f visibleSize = itemScroll->getMeasuredSize();

	lastUpdatePos = -scrollPosition;


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
			
			if (next == NULL)
				break;

			addNode(next);			
		}			
	}

	if (loadTimer.millis() > 0)
		Logger::stream("DataList","INFO") << "updateLoading() time = " << loadTimer.millis() << " ms" << endl;

	loadTimer.start();
	updatePriorities();
		
	if (loadTimer.millis() > 0)
		Logger::stream("DataList","INFO") << "updatePriorities() time = " << loadTimer.millis() << " ms" << endl;
	

	if (cursor->getState() == FBDataCursor::Loading)
	{		
		if (loadIndicatorState == 0)
		{
			loadingTime.start();
			loadIndicatorState = 1;
		}
		else if (loadIndicatorState == 1 && loadingTime.millis() > showLoadIndicatorThreshold)
		{
			loadIndicatorState = 2;
		}
		else if (loadIndicatorState == 2)
		{
			loadIndicator->setVisible(true);
		}
	}
	else
	{
		loadIndicatorState = 0;
		loadIndicator->setVisible(false);
	}
}

void DataListActivity::addNode(FBNode * node)
{
	auto it = items.find(node);

	if (it == items.end() || (it->second) == NULL)
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