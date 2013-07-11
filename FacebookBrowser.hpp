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

class FacebookBrowser : public View {

private:
	FBNode * rootNode;
	
	View * topView;
	Timer updateTimer;
	int state;
	
	FriendDetailView * myView;
	FacebookFriendListView * friendList;
	FacebookIntroView * introView;
	FriendPhotoLinkedView * fplv;


public:
	FacebookBrowser(FBNode * rootNode);
	
	void childViewFinished(string action);

	void layout(Vector position, cv::Size2f size);
	
	void onFrame(const Controller & controller);

	void update();

	void draw();
};


#endif