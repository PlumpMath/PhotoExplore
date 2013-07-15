#ifndef LEAPIMAGE_IMAGE_DETAIL_VIEW_HPP_
#define LEAPIMAGE_IMAGE_DETAIL_VIEW_HPP_

#include "View.hpp"
#include "Panel.h"
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

	PanelInteraction(Panel * _panel) :
	panel(_panel)
	{
		pointableRange = 0;
	}

	Panel * panel;
	Vector translation, scale;
	Vector pointableCentroid;
	float pointableRange;

	Timer lockoutTimer;

	vector<pair<int,Vector> > interactingPointables;	
};

class ImageDetailView : public ViewGroup, public GlobalGestureListener {

private:	
	boost::function<void(std::string)> finishedCallback;
	Panel * imagePanel;

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

public:
	ImageDetailView();

	void notifyOffsetChanged(Leap::Vector offset);
	
	void setFinishedCallback(const boost::function<void(std::string)> & callback);
	
	void setImagePanel(Panel * imagePanel);
	Panel * getImagePanel();

	void onFrame(const Controller & controller);

	void layout(Vector position, cv::Size2f size);

	void onGlobalGesture(const Controller & controller, std::string gestureType);	
	bool onLeapGesture(const Controller & controller, const Gesture & gesture);
	void getTutorialDescriptor(vector<string> & tutorial);

	void setVisible(bool visible);
	void update();
	void draw();

	LeapElement * elementAtPoint(int x, int y, int & elementStateFlags);

};

#endif