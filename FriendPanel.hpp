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
	FBNode * node;
	ViewGroup * friendViewGroup;
	ViewGroup * layoutGroup;
	TextPanel * nameText;
	Panel * friendPhotoPanel;

	Pointable activePointable;

	float lastSizeScale, dataPriority;
	cv::Size2f originalSize;

public:
	FriendPanel(cv::Size2f targetSize);
	void show(FBNode * node,boost::function<void()> loadCompleteCallback);
		
	void OnPointableEnter(Pointable & pointable);
	void OnPointableExit(Pointable & pointable);
	
	void onFrame(const Controller & controller);

	void viewChanged(string viewIdentifier, vector<FBNode*> & viewData);
	void viewOwnershipChanged(View * view, ViewOwner * newOwner);

	void setDataPriority(float dataPriority);

	boost::function<void(FriendPanel*)> photosLoadedCallback;
};


#endif