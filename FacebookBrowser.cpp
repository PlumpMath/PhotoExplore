#include "FacebookBrowser.hpp"

FacebookBrowser::FacebookBrowser(FBNode * _rootNode)
{	
	this->rootNode = _rootNode;
	
	myView = new FriendDetailView();
	friendList = new FacebookFriendListView();
	introView  = new FacebookIntroView();
	//fplv = new FriendPhotoLinkedView();

	state = 0;
	updateTimer.start();
	topView = NULL;

	childViewFinished("");	

}

void FacebookBrowser::childViewFinished(string action)
{
	
	if (action.compare("my_photos") == 0)
	{
		rootNode->Edges.insert(Facebook::Edge("name","My Photos and Albums"));

		myView->setFinishedCallback(boost::bind(&FacebookBrowser::childViewFinished,this,_1));		
		myView->show(rootNode);
		topView = myView;
	}
	//else if (action.compare("friends_view") == 0)
	//{				
	//	fplv->setFinishedCallback(boost::bind(&FacebookBrowser::childViewFinished,this,_1));		
	//	fplv->show(rootNode);
	//	topView = fplv;
	//}
	else if (action.compare("friend_list_view") == 0)
	{
		friendList->setFinishedCallback(boost::bind(&FacebookBrowser::childViewFinished,this,_1));		
		friendList->show(rootNode);
		topView = friendList;
	}
	else
	{
		introView->setFinishedCallback(boost::bind(&FacebookBrowser::childViewFinished,this,_1));
		introView->show(rootNode);					
		topView = introView;
	}
	topView->layout(Vector(0,GlobalConfig::tree()->get<float>("Menu.Height"),0),cv::Size2f(GlobalConfig::ScreenWidth, GlobalConfig::ScreenHeight-(GlobalConfig::tree()->get<float>("Tutorial.Height")+GlobalConfig::tree()->get<float>("Menu.Height"))));
}

void FacebookBrowser::onFrame(const Controller & controller)
{

	topView->onFrame(controller);
}

void FacebookBrowser::layout(Vector position, cv::Size2f size)
{ 
	;
}

void FacebookBrowser::draw()
{
	topView->draw();
}

void FacebookBrowser::update()
{
	if (updateTimer.seconds() > 1)
	{
		rootNode->update();
		updateTimer.start();
	}

	topView->update();
}



