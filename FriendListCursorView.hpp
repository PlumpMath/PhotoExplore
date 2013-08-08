#ifndef LEAPIMAGE_FRIEND_LIST_CURSOR_VIEW_HPP_
#define LEAPIMAGE_FRIEND_LIST_CURSOR_VIEW_HPP_

#include "DataListActivity.hpp"
#include "FacebookDataDisplay.hpp"
#include "SwipeGestureDetector.hpp"
#include "FriendPanel.hpp"
#include "TextEditPanel.hpp"
#include "FBDataCursor.hpp"
#include "SDLTimer.h"

#include <set>

class FriendListCursorView : public DataListActivity, public ViewOwner {

private:
	TextEditPanel * editText;
	TextPanel * labelText;
	View * lookupPanel;
	
	int lookupDialogState;

	FBFriendsFQLCursor * searchCursor;
	FBDataCursor * allFriendsCursor;

	Timer lookupDialogTimer;
	Timer lookupDialogMovementTimer;

	bool showPicturelessFriends;
	
	set<FBNode*> pendingItems;

public:
	FriendListCursorView();

	FBDataView * getDataView(FBNode * itemNode);

	void suspend();
	void resume();

	void setFinishedCallback(const boost::function<void(std::string)> & callback);

	void onGlobalGesture(const Controller & controller, std::string gestureType);
	bool onLeapGesture(const Controller & controller, const Gesture & gesture);
	void getTutorialDescriptor(vector<string> & tutorial);
	//void onGlobalFocusChanged(bool isFocused);

	void layout(Vector position, cv::Size2f size);
	void update();


	void setUserNode(FBNode * node);

};



#endif