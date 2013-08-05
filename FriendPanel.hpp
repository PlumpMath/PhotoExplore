#ifndef LEAPIMAGE_FRIEND_PANEL_HPP_
#define LEAPIMAGE_FRIEND_PANEL_HPP_

#include "DataViewGenerator.hpp"
#include "ContentPanel.hpp"
#include "UniformGrid.hpp"
#include "FBNode.h"
#include "ViewOrchestrator.hpp"
#include "PicturePanel.hpp"
#include "TextPanel.h"
#include "PanelFactory.hpp"
#include "FBDataView.hpp"

class FriendPanel : public ContentPanel, public ViewOwner, public FBDataView {

private:
	FBNode * activeNode;
	map<string,FBNode*> items;

	ViewGroup * friendViewGroup;
	ViewGroup * layoutGroup;
	TextPanel * nameText;
	PicturePanel * friendPhotoPanel;

	Pointable activePointable;

	float lastSizeScale, dataPriority;
	cv::Size2f originalSize;

	void updateLoading();
	void addNode(FBNode * node);

public:
	FriendPanel(cv::Size2f targetSize);
		
	void viewOwnershipChanged(View * view, ViewOwner * newOwner);

	void setDataPriority(float dataPriority);
	FBNode * getNode();
	void show(FBNode * node);

	boost::function<void(FriendPanel*)> photosLoadedCallback;

};


#endif