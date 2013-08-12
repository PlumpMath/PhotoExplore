#ifndef LEAPIMAGE_IMAGE_DETAIL_VIEW_HPP_
#define LEAPIMAGE_IMAGE_DETAIL_VIEW_HPP_

#include "View.hpp"
#include "PicturePanel.hpp"
#include "LeapDebug.h"
#include "UniformGrid.hpp"
#include "Button.hpp"
#include "ContentPanel.hpp"
#include "CustomGrid.hpp"
#include "FBNode.h"

struct PanelInteraction {
	
	PanelInteraction() : panel(NULL)
	{

	}

	PanelInteraction(PicturePanel * _panel) :
	panel(_panel)
	{
		pointableRange = 0;
	}

	PicturePanel * panel;
	Vector translation, scale;
	Vector pointableCentroid;
	float pointableRange;

	Timer lockoutTimer;

	vector<pair<int,Vector> > interactingPointables;	
};

class ImageDetailView : public ActivityView {

private:	
	boost::function<void(std::string)> finishedCallback;
	PicturePanel * imagePanel;

	//ViewGroup * buttonBar;
	//ContentPanel * buttonBarPanel;

	Button * likeButton, * alreadyLikedButton;
	TextPanel * photoComment;

	PanelInteraction activePanelInteraction;

	bool handleImageManipulation(const Controller & controller);
	void initButtonBar();

	void setImageMetaData();

	Leap::Vector hostOffset;

	Facebook::FBNode * imageNode;

	bool canClickToExit;

	void initLikeButton(Facebook::FBNode * node);

public:
	ImageDetailView();

	void notifyOffsetChanged(Leap::Vector offset);
	
	void setFinishedCallback(const boost::function<void(std::string)> & callback);
	
	void setImagePanel(PicturePanel * imagePanel);
	PicturePanel * getImagePanel();

	void onFrame(const Controller & controller);

	void layout(Vector position, cv::Size2f size);

	void onGlobalGesture(const Controller & controller, std::string gestureType);	
	bool onLeapGesture(const Controller & controller, const Gesture & gesture);
	void getTutorialDescriptor(vector<string> & tutorial);

	void setVisible(bool visible);
	void draw();

	float getZValue();

	
	cv::Rect_<int> getHitRect();

	void OnElementClicked(Pointable & pointable);

	LeapElement * elementAtPoint(int x, int y, int & elementStateFlags);
	bool isClickable();

};

#endif