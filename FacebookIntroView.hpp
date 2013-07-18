#ifndef LEAPIMAGE_DATA_BROWSER_HPP_
#define LEAPIMAGE_DATA_BROWSER_HPP_

#include "View.hpp"
#include "UniformGrid.hpp"
#include "DataViewGenerator.hpp"
#include "ViewOrchestrator.hpp"
#include "Panel.h"
#include "ScrollingView.hpp"
#include "Button.hpp"
#include "RadialMenu.hpp"

using namespace std;

class FacebookIntroView : public ViewGroup, public GlobalGestureListener {

private:
	ViewGroup * friendPhotoGrid;
	ViewGroup * myPhotoGrid;
	ViewGroup * buttonGrid, * mainLayout;
	ScrollingView *friendScroll, *photoScroll;
	
	Button * photoButton, *friendListButton;
	
	boost::function<void(std::string)> finishedCallback;

	FBNode * activeNode;

public:
	FacebookIntroView();
	
	void setFinishedCallback(const boost::function<void(std::string)> & callback);

	void show(FBNode * node);
	
	void viewChanged(string viewIdentifier, vector<FBNode*> viewData);

	void buttonClicked(LeapElement * clicked);

	void onFrame(const Controller & controller);
	void layout(Vector position, cv::Size2f size);

	void onGlobalGesture(const Controller & controller, std::string gestureType);
	bool onLeapGesture(const Controller & controller, const Gesture & gesture);
	void getTutorialDescriptor(vector<string> & tutorial);
};


#endif