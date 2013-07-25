#ifndef LEAPIMAGE_FRIEND_PANEL_HPP_
#define LEAPIMAGE_FRIEND_PANEL_HPP_

#include "DataViewGenerator.hpp"
#include "ContentPanel.hpp"
#include "UniformGrid.hpp"
#include "FBNode.h"
#include "ViewOrchestrator.hpp"
#include "Panel.h"
#include "TextPanel.h"
#include "PanelFactory.hpp"

class FriendPanel : public ContentPanel, public ViewOwner {

private:
	FBNode * activeNode;
	map<string,FBNode*> items;

	ViewGroup * friendViewGroup;
	ViewGroup * layoutGroup;
	TextPanel * nameText;
	Panel * friendPhotoPanel;

	Pointable activePointable;

	float lastSizeScale, dataPriority;
	cv::Size2f originalSize;

	void updateLoading();
	void addNode(FBNode * node);

public:
	FriendPanel(cv::Size2f targetSize);
	void show(FBNode * node,boost::function<void()> loadCompleteCallback);
		
	void viewOwnershipChanged(View * view, ViewOwner * newOwner);

	void setDataPriority(float dataPriority);

	boost::function<void(FriendPanel*)> photosLoadedCallback;

	FBNode * getActiveNode();
};


#endif