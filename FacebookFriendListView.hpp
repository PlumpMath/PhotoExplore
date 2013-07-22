#ifndef LEAPIMAGE_FACEBOOK_DATA_VIEW_HPP_
#define LEAPIMAGE_FACEBOOK_DATA_VIEW_HPP_

#include "View.hpp"
#include "ScrollingView.hpp"
#include "FriendDetailView.hpp"
#include "FriendPanel.hpp"
#include "FBNode.h"
#include "RadialMenu.hpp"


class FacebookFriendListView :  public ViewGroup, public ViewOwner, public GlobalGestureListener {

private:
	ViewGroup * friendGroup, * mainLayout;

	FriendDetailView * friendDetail;
	boost::function<void(std::string)> finishedCallback;

	ScrollingView * itemScroll;

	int rowCount;
	float currentRightBoundary,lastUpdatePos;

	FBNode * activeNode;
	
	void loadItems(int friends);
	void updateLoading();
	void addNode(FBNode * node);

	map<string,FBNode*> items;

public:
	FacebookFriendListView();
	
	void friendPanelClicked(FriendPanel * panel, FBNode * node);
	void setFinishedCallback(const boost::function<void(std::string)> & callback);
	
	void show(FBNode * root);	
	void viewChanged(vector<FBNode*> & viewData);

	void onFrame(const Controller & controller);
	void layout(Vector position, cv::Size2f size);		
	void update();

	void onGlobalGesture(const Controller & controller, std::string gestureType);
	bool onLeapGesture(const Controller & controller, const Gesture & gesture);
	void getTutorialDescriptor(vector<string> & tutorial);
	void onGlobalFocusChanged(bool isFocused);

	void friendViewLoaded(FBNode * friendNode, vector<FBNode*> & viewData);




};

#endif