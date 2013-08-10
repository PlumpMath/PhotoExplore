#ifndef LEAPIMAGE_FACEBOOK_BROWSER_HPP_
#define LEAPIMAGE_FACEBOOK_BROWSER_HPP_

#include <Leap.h>
#include <boost/function.hpp>

#include "LeapInput.hpp"
#include "FacebookIntroView.hpp"
#include "SDLTimer.h"
#include "FacebookDataDisplay.hpp"
#include "ActivityView.hpp"
#include "FriendListCursorView.hpp"
#include "AlbumCursorView.hpp"
#include "FriendCursorView.hpp"

#include <stack>

using namespace std;
using namespace Leap;

class FacebookBrowser : public View, public FacebookDataDisplay {

private:	
	View * topView;
	Timer updateTimer;
	int state;
	
	FacebookIntroView * introView;
	FriendListCursorView * friendCursorView;
	AlbumCursorView * albumCursorView;
	FriendCursorView * friendDetailView;
	
	void setTopView(View * topView);

	FBNode * userNode;

	ViewGroup * pathView;
	View * homeButton;

public:
	FacebookBrowser();
		
	void viewFinished(View * finishedView);
	void displayNode(FBNode * newNode, string action);

	void layout(Vector position, cv::Size2f size);
	
	void onFrame(const Controller & controller);

	void update();

	void draw();

	LeapElement * elementAtPoint(int x, int y, int & state);


};


#endif