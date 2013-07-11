#ifndef LEAPIMAGE_RELEVANT_IMAGE_VIEW_HPP_
#define LEAPIMAGE_RELEVANT_IMAGE_VIEW_HPP_

#include "View.hpp"
#include "UniformGrid.hpp"
#include "DataViewGenerator.hpp"
#include "ViewOrchestrator.hpp"
#include "Panel.h"
#include "ScrollingView.hpp"
#include "DynamicGrid.hpp"
#include "TextPanel.h"
#include "Button.hpp"
#include "CustomGrid.hpp"
#include "ContentPanel.hpp"
#include "FrameView.hpp"

class RelevantImageView : public ViewGroup, public ViewOwner, public GlobalGestureListener {

private:
	UniformGrid * buttonBar;
	ViewGroup * imageGroup;
	View * topView;
	CustomGrid * mainLayout;

	ContentPanel * photoInfoPanel;
	TextPanel * photoFriendInfo;
	TextPanel * photoAlbumInfo;
	TextPanel * photoCaption;

	FBNode * activeNode;

	boost::function<void(std::string)> finishedCallback;

	string viewMode;
	void viewModeChanged(string viewMode);
	void initButtonBar();
	void initPhotoInfoPanel();
	

public:
	RelevantImageView();		
	void setFinishedCallback(const boost::function<void(std::string)> & callback);

	void albumPanelClicked(FBNode * clicked);
		
	void show(FBNode * root);
	
	void viewChanged(string viewIdentifier, vector<FBNode*> & viewData);

	void onFrame(const Controller & controller);

	void layout(Vector position, cv::Size2f size);

	void onGlobalGesture(const Controller & controller, std::string gestureType);

	void viewOwnershipChanged(View * view, ViewOwner * newOwner);

	void draw();

};

#endif