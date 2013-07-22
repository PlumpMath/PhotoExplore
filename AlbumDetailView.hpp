#ifndef LEAPIMAGE_ALBUM_DETAIL_VIEW_HPP_
#define LEAPIMAGE_ALBUM_DETAIL_VIEW_HPP_

#include "DataViewGenerator.hpp"
#include "ViewOrchestrator.hpp"
#include "Panel.h"
#include "ScrollingView.hpp"
#include "DynamicGrid.hpp"
#include "TextPanel.h"
#include "ImageDetailView.hpp"
#include "CustomGrid.hpp"
#include "RadialMenu.hpp"

class AlbumDetailView : public ViewGroup, public ViewOwner, public GlobalGestureListener {

private:
	ViewGroup * imageGroup;
	TextPanel * albumName;
	boost::function<void(std::string)> finishedCallback;

	ImageDetailView * imageDetailView;
	FBNode * activeNode;
	ViewGroup * mainLayout;
	ScrollingView * itemScroll;

	void updateLoading();
	void loadItems(int photos);

	int rowCount;
	float lastUpdatePos, avgItemWidth;
	float currentRightBoundary;
	
	void addNode(FBNode * node);

	map<string,FBNode*> items;

public:
	AlbumDetailView();		
	void setFinishedCallback(const boost::function<void(std::string)> & callback);
		
	void show(FBNode * root);

	FBNode * getNode();
	
	void viewChanged(vector<FBNode*> & viewData);

	void onFrame(const Controller & controller);

	void layout(Vector position, cv::Size2f size);
	void update();
	void onGlobalGesture(const Controller & controller, std::string gestureType);	
	bool onLeapGesture(const Controller & controller, const Gesture & gesture);

	void viewOwnershipChanged(View * view, ViewOwner * newOwner);

	void getTutorialDescriptor(vector<string> & tutorial);

	void onGlobalFocusChanged(bool isFocused);
};



#endif