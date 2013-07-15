#ifndef LEAPIMAGE_FRIEND_DETAIL_VIEW_HPP_
#define LEAPIMAGE_FRIEND_DETAIL_VIEW_HPP_

#include "View.hpp"
#include "DataViewGenerator.hpp"
#include "ViewOrchestrator.hpp"
#include "Panel.h"
#include "ScrollingView.hpp"
#include "DynamicGrid.hpp"
#include "TextPanel.h"
#include "AlbumPanel.hpp"
#include "AlbumDetailView.hpp"
#include "ImageDetailView.hpp"
#include "PanelFactory.hpp"
#include "CustomGrid.hpp"
#include "RadialMenu.hpp"

class FriendDetailView : public ViewGroup, public ViewOwner, public GlobalGestureListener {

private:
	ViewGroup * imageGroup;
	TextPanel * friendNameHeading;

	View * topView;
	AlbumDetailView * albumDetail;
	ImageDetailView * imageDetailView;
	FBNode * activeNode;

	RadialMenu * radialMenu;

	ViewGroup * mainLayout;

	int rowCount;
	ScrollingView * itemScroll;
	boost::function<void(std::string)> finishedCallback;

	void updateLoading();
	void addNode(FBNode * node);

	float projectedRightBoundary,currentRightBoundary;
	float lastUpdatePos;

	map<string,FBNode*> items;
	void loadItems(int albums, int photos);
	

	

public:
	FriendDetailView();		
	void setFinishedCallback(const boost::function<void(std::string)> & callback);

	void albumPanelClicked(FBNode * clicked);
		
	void show(FBNode * root);
	
	void viewChanged(int offset, vector<FBNode*> & viewData);

	void onFrame(const Controller & controller);

	void update();

	void layout(Vector position, cv::Size2f size);

	void onGlobalGesture(const Controller & controller, std::string gestureType);
	bool onLeapGesture(const Controller & controller, const Gesture & gesture);
	void getTutorialDescriptor(vector<string> & tutorial);

	void viewOwnershipChanged(View * view, ViewOwner * newOwner);

};


#endif