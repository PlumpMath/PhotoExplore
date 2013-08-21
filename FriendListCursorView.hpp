#ifndef LEAPIMAGE_FRIEND_LIST_CURSOR_VIEW_HPP_
#define LEAPIMAGE_FRIEND_LIST_CURSOR_VIEW_HPP_

#include "DataListActivity.hpp"
#include "FacebookDataDisplay.hpp"
#include "SwipeGestureDetector.hpp"
#include "FriendPanel.hpp"
#include "TextEditPanel.hpp"
#include "FBDataCursor.hpp"
#include "SDLTimer.h"
#include "FBDataCursor.hpp"

#include <set>

class FriendListCursorView : public DataListActivity, public ViewOwner {

private:
	TextEditPanel * editText;
	TextPanel * labelText;
	View * lookupPanel;
	
	int lookupDialogState;

	FBFriendsFQLCursor * searchCursor;
	DataCursor * allFriendsCursor;

	Timer lookupDialogTimer;
	Timer lookupDialogMovementTimer;
		
	set<FBNode*> pendingItems;

public:
	FriendListCursorView();

	View * getDataView(DataNode * itemNode);
	void setItemPriority(float priority, View * itemView);
	virtual void refreshDataView(DataNode * node, View * view);

	void suspend();
	void resume();

	void setFinishedCallback(const boost::function<void(std::string)> & callback);

	void onGlobalGesture(const Controller & controller, std::string gestureType);
	void getTutorialDescriptor(vector<string> & tutorial);

	void layout(Vector position, cv::Size2f size);
	void update();


	void setUserNode(FBNode * node);

};



#endif