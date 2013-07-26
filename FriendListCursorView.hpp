#ifndef LEAPIMAGE_FRIEND_LIST_CURSOR_VIEW_HPP_
#define LEAPIMAGE_FRIEND_LIST_CURSOR_VIEW_HPP_

#include "DataListActivity.hpp"
#include "FacebookDataDisplay.hpp"
#include "SwipeGestureDetector.hpp"
#include "FriendPanel.hpp"

class FriendListCursorView : public DataListActivity, public ViewOwner {

public:
	FriendListCursorView() : DataListActivity(2)
	{

	}

	FBDataView * getDataView(FBNode * itemNode)
	{
		FriendPanel * item = NULL;
		if (itemNode->Edges.get<EdgeTypeIndex>().count("photos") + itemNode->Edges.get<EdgeTypeIndex>().count("albums") >= 2)
		{
			item = (FriendPanel*)ViewOrchestrator::getInstance()->requestView(itemNode->getId(), this);

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
		}
		return item;
	}

	void setFinishedCallback(const boost::function<void(std::string)> & callback)
	{
		viewFinishedCallback = callback;
	}

	void onGlobalGesture(const Controller & controller, std::string gestureType)
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

	bool onLeapGesture(const Controller & controller, const Gesture & gesture)
	{
		return itemScroll->onLeapGesture(controller, gesture);
	}

	void getTutorialDescriptor(vector<string> & tutorial)
	{	
		tutorial.push_back("swipe");
		tutorial.push_back("point_stop");
		tutorial.push_back("shake");
	}

	void onGlobalFocusChanged(bool isFocused)
	{
		if (isFocused)
			SwipeGestureDetector::getInstance().setFlyWheel(itemScroll->getFlyWheel());
		else
			SwipeGestureDetector::getInstance().setFlyWheel(NULL);
	}



};



#endif