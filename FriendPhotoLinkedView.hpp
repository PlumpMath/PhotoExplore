#ifndef LEAPIMAGE_FRIEND_PHOTO_LINKED_VIEW_HPP_
#define LEAPIMAGE_FRIEND_PHOTO_LINKED_VIEW_HPP_

#include "View.hpp"
#include "UniformGrid.hpp"
#include "DataViewGenerator.hpp"
#include "ViewOrchestrator.hpp"
#include "PicturePanel.hpp"
#include "ScrollingView.hpp"
#include "DynamicGrid.hpp"
#include "TextPanel.h"
#include "Button.hpp"
#include "CustomGrid.hpp"
#include "ContentPanel.hpp"
#include "FrameView.hpp"
#include "FriendDetailView.hpp"
#include "FBNode.h"
#include "ImageDetailView.hpp"
#include "AlbumDetailView.hpp"
#include "GLImport.h"
#include "FastSeekBar.hpp"

class FriendPhotoLinkedView : public ViewOwner, public ActivityView {

private:
	ViewGroup * imageGroup;
	ViewGroup * friendBar;
	View * topView;
	CustomGrid * mainLayout;

	FastSeekBar * seekBar;


	ImageDetailView * imageDetail;
	FriendDetailView * friendDetail;
	AlbumDetailView * albumDetail;

	ScrollingView * friendScroll, * photoScroll;

	FBNode * activeNode;

	boost::function<void(std::string)> finishedCallback;

	string viewMode;
	void viewModeChanged(string viewMode);
	void initButtonBar();
	void initPhotoInfoPanel();

	void setTopView(View * topView, bool hideCurrent = true);

	
	int friendLoadCount, friendLoadTarget;

	void photoPanelClicked(LeapElement * element);
	void albumPanelClicked(LeapElement * element);
		
	float friendToPhoto;

	void friendLoaded(int offset, vector<FBNode*> & viewData);
	void photoLoaded(int offset, FBNode * friendNode, vector<FBNode*> & viewData);

	map<FBNode*, View*> loadedFriends;

	vector<boost::function<void()> > drawCallbacks;

	void updateLoading(Vector newPos,cv::Size2f visibleSize);

public:
	FriendPhotoLinkedView();		
	void setFinishedCallback(const boost::function<void(std::string)> & callback);

	void show(FBNode * root);
	
	void viewChanged(int offset, vector<FBNode*> & viewData);

	void onFrame(const Controller & controller);

	void layout(Vector position, cv::Size2f size);

	void onGlobalGesture(const Controller & controller, std::string gestureType);
	bool onLeapGesture(const Controller & controller, const Gesture & gesture);
	void getTutorialDescriptor(vector<string> & tutorial);

	void viewOwnershipChanged(View * view, ViewOwner * newOwner);

	void update();
	void draw();

};

#endif