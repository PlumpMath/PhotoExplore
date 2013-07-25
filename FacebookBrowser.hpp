#ifndef LEAPIMAGE_FACEBOOK_BROWSER_HPP_
#define LEAPIMAGE_FACEBOOK_BROWSER_HPP_

#include <Leap.h>
#include <boost/function.hpp>

#include "PointableElementManager.h"
#include "FacebookIntroView.hpp"
#include "FacebookFriendListView.hpp"
#include "RelevantImageView.hpp"
#include "SDLTimer.h"
#include "FriendPhotoLinkedView.hpp"
#include "FacebookDataDisplay.hpp"
#include "AlbumDetailView.hpp"
#include "ActivityView.hpp"

#include <stack>

using namespace std;
using namespace Leap;

class FacebookBrowser : public View, public FacebookDataDisplay {

private:	
	View * topView;
	Timer updateTimer;
	int state;
	
	FacebookIntroView * introView;

	FacebookFriendListView * friendList;
	FriendDetailView * friendDetailView;
	AlbumDetailView * albumDetailView;
	
	void setTopView(View * topView);

	FBNode * userNode;

public:
	FacebookBrowser();
		
	void displayNode(FBNode * previousNode, FBNode * newNode, string action);

	void layout(Vector position, cv::Size2f size);
	
	void onFrame(const Controller & controller);

	void update();

	void draw();


};


#endif