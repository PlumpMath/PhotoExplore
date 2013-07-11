#include <Leap.h>
#include <LeapMath.h>
#include "HandModel.h"
#include "LeapDebug.h"
#include "SDLTimer.h"
#include <opencv2/opencv.hpp>
#include <set>
#include "FileNode.h"
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

#include "CefFileBrowser.h"
#include "Cefalopod.h"

#ifndef LEAP_START_SCREEN_H_
#define LEAP_START_SCREEN_H_


using namespace Leap;


class FloatingPanel : public PanelBase {

	Vector startPosition;
	Vector gravityWellPosition;
	bool gravityWellPresent;
	float startSize;

public:
	FloatingPanel(float _width, float _height, Vector _position) : PanelBase()
	{				
		startSize = _width;
		gravityWellPresent = false;
		startPosition = _position;

		setPanelSize(_width,_height);
		setPosition(_position);
	}


	void update(double deltaTime);

	void inputGravityWell(Vector wellPosition)
	{
		gravityWellPresent = true;
		wellPosition.z = this->startPosition.z;
		gravityWellPosition = wellPosition;		
	}

	void clearGravityWell()
	{
		gravityWellPresent = false;
	}
	
	void draw();
	void drawContent(Vector drawPosition, float drawWidth, float drawHeight);

};


class LeapStartScreen : public View, public GlobalGestureListener {

private:
	//Static fields
	const static int StartState = 0;
	const static int BrowserOpenState = 1;
	const static int FinishedState = 2;

	const static int PreCheckState = 3;


	TextPanel * facebookLoginButton;
	Button * logoutButton;
	CustomGrid * mainLayout;
	vector<PanelBase*> panelList;	
	View * rootView; 
	RadialMenu * radialMenu;
	TextPanel * statusPanel;
	
	LeapElement * lastHit;
	std::string startDir;	

	FBNode * loggedInNode;

	int state;
	
	CefRefPtr<CefFileBrowser> fileBrowserCallback;	
	CefRefPtr<Cefalopod> facebookClient,facebookPreCheckClient;

	//Members
	FBNode * createMockData();
	void deleteCookies();
	
	Timer updateTimer;

	boost::function<void()> finishedCallback;

public:
	LeapStartScreen(std::string startDir);
	void init();
	void update(double delta);
	void draw();
		

	void doPreCheck();

	void launchBrowser();

	void elementClicked(LeapElement * element);
	void onFrame(const Controller & controller);
	
	bool onLeapGesture(const Controller & controller, const Gesture & gesture);
	void onGlobalGesture(const Controller & controller, std::string gestureType);

	void setFinishedCallback(boost::function<void()> finishedCallback);

	void layout(Leap::Vector pos, cv::Size2f size);

	LeapElement * elementAtPoint(int x, int y, int & elementStateFlags);

	void shutdown();
};

#endif