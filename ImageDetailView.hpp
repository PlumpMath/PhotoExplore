#ifndef LEAPIMAGE_IMAGE_DETAIL_VIEW_HPP_
#define LEAPIMAGE_IMAGE_DETAIL_VIEW_HPP_

#include "View.hpp"
#include "DynamicImagePanel.hpp"
#include "LeapDebug.h"
#include "UniformGrid.hpp"
#include "Button.hpp"
#include "ContentPanel.hpp"
#include "CustomGrid.hpp"
#include "NotchedWheel.hpp"
#include <list>
#include "DataCursor.hpp"
#include "DataNode.hpp"


struct PanelInteraction {

	PanelInteraction() : panel(NULL)
	{

	}

	PanelInteraction(DynamicImagePanel * _panel) :
		panel(_panel)
	{
		pointableRange = 0;
	}

	DynamicImagePanel * panel;
	Vector translation, scale;
	Vector pointableCentroid;
	float pointableRange;

	Timer lockoutTimer;

	vector<pair<int,Vector> > interactingPointables;	
};

class ImageDetailView : public ActivityView {

private:
	boost::function<void(std::string)> finishedCallback;
	PanelInteraction activePanelInteraction;
	bool handleImageManipulation(const Controller & controller);	
	bool canClickToExit, skipAnimationNextLayout;

	DynamicImagePanel * currentNext, * currentPrev;
	NotchedWheel * scrollWheel;

	list<DynamicImagePanel*> panelList;
	int maxPanelCount, mainIndex;

	void setMainPanel(DynamicImagePanel * mainPanel);
	void scrollPanelList(int count);
		
	BidirectionalCursor * fwdCursor, * reverseCursor;

protected:	
	Leap::Vector hostOffset;	
	DynamicImagePanel * imagePanel;

	void setPanelState(DynamicImagePanel * _imagePanel);
	void restorePanelState(DynamicImagePanel * _imagePanel);

	virtual DynamicImagePanel * getDetailedDataView(DataNode * node) = 0;

public:
	ImageDetailView();

	void notifyOffsetChanged(Leap::Vector offset);

	void setFinishedCallback(const boost::function<void(std::string)> & callback);


	void onFrame(const Controller & controller);
	void layout(Vector position, cv::Size2f size);

	void onGlobalGesture(const Controller & controller, std::string gestureType);	
	void getTutorialDescriptor(vector<string> & tutorial);
	void onGlobalFocusChanged(bool isFocused);


	void setVisible(bool visible);
	void draw();

	float getZValue();
	
	cv::Rect_<int> getHitRect();

	void OnElementClicked(Pointable & pointable);

	LeapElement * elementAtPoint(int x, int y, int & elementStateFlags);
	bool isClickable();

	void setCursor(BidirectionalCursor * reverseCursor, BidirectionalCursor * fwdCursor);
};


#endif