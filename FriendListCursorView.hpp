#ifndef LEAPIMAGE_FRIEND_LIST_CURSOR_VIEW_HPP_
#define LEAPIMAGE_FRIEND_LIST_CURSOR_VIEW_HPP_

#include "DataListActivity.hpp"
#include "FacebookDataDisplay.hpp"
#include "SwipeGestureDetector.hpp"
#include "FriendPanel.hpp"
#include "TextEditPanel.hpp"
#include "FBDataCursor.hpp"

class FriendListCursorView : public DataListActivity, public ViewOwner {

private:
	TextEditPanel * editText;
	View * lookupPanel;
	
	bool lookupActive;

	FBFriendsFQLCursor * searchCursor;
	FBDataCursor * allFriendsCursor;

public:
	FriendListCursorView();

	FBDataView * getDataView(FBNode * itemNode);

	void suspend();

	void setFinishedCallback(const boost::function<void(std::string)> & callback);

	void onGlobalGesture(const Controller & controller, std::string gestureType);
	bool onLeapGesture(const Controller & controller, const Gesture & gesture);
	void getTutorialDescriptor(vector<string> & tutorial);
	void onGlobalFocusChanged(bool isFocused);

	void layout(Vector position, cv::Size2f size);

	void setUserNode(FBNode * node);

};



#endif