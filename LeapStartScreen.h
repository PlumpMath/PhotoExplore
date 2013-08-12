#ifndef LEAP_START_SCREEN_H_
#define LEAP_START_SCREEN_H_

#include <Leap.h>
#include <LeapMath.h>
#include "HandModel.h"
#include "LeapDebug.h"
#include "SDLTimer.h"
#include <opencv2/opencv.hpp>
#include <set>
#include "FBNode.h"
#include "TextPanel.h"
#include "Types.h"
#include <math.h>
#include "FacebookLoader.h"
#include "RadialMenu.hpp"
#include "CustomGrid.hpp"
#include "Button.hpp"

#include <random>

#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/cef_app.h>
#include <include/cef_urlrequest.h>

#include "Cefalopod.h"

#include "ScrollingView.hpp"
#include "ColorPanel.hpp"


using namespace Leap;


class LeapStartScreen : public ActivityView {

private:
	//Static fields
	const static int StartState = 0;
	const static int BrowserOpenState = 1;
	const static int FinishedState = 2;
	const static int TutorialState = 4;


	Button * facebookLoginButton, * tutorialButton;

	TextPanel * noticePanel;
	CustomGrid * mainLayout;
	View * rootView; 
	RadialMenu * radialMenu;
			
	int state;
	
	CefRefPtr<Cefalopod> facebookClient;

	//Members
	void deleteCookies();
	
	boost::function<void()> finishedCallback;

	ScrollingView * floatingPanelsView;
	void generateBackgroundPanels();

	void launchTutorial();
	void launchBrowser();


public:
	LeapStartScreen();
	~LeapStartScreen();

	void init();
	void update(double delta);
	void draw();
		
	void startApplication(std::string token);
	void onFrame(const Controller & controller);
	
	bool onLeapGesture(const Controller & controller, const Gesture & gesture);
	void onGlobalGesture(const Controller & controller, std::string gestureType);
	void getTutorialDescriptor(vector<string> & tutorial);
	void onGlobalFocusChanged(bool focused);

	void setFinishedCallback(boost::function<void()> finishedCallback);

	void layout(Leap::Vector pos, cv::Size2f size);

	LeapElement * elementAtPoint(int x, int y, int & elementStateFlags);
};

#endif