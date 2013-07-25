#include "FacebookBrowser.hpp"

FacebookBrowser::FacebookBrowser()
{	
	
	friendList = new FacebookFriendListView();

	introView  = new FacebookIntroView();
	albumDetailView = new AlbumDetailView();
	friendDetailView = new FriendDetailView();
	
	state = 0;
	updateTimer.start();
	topView = NULL;

	userNode = NULL;
}

void FacebookBrowser::displayNode(FBNode * previousNode, FBNode * node, string action)
{
	if (node == NULL)
		return;

	previousNode = NULL;

	if (node->getNodeType().compare(NodeType::FacebookImage) == 0)
	{
		auto parentIt = node->ReverseEdges.get<EdgeTypeIndex>().find("albums");
		if (parentIt != node->ReverseEdges.get<EdgeTypeIndex>().end())
		{			
			displayNode(NULL,parentIt->Node,"");
			albumDetailView->showChild(node);
		}
		else
		{
			parentIt = node->ReverseEdges.get<EdgeTypeIndex>().find("friends");			
			if (parentIt != node->ReverseEdges.get<EdgeTypeIndex>().end())
			{
				displayNode(NULL,parentIt->Node,"");
				friendDetailView->showChild(node);
			}
			else
			{			
				parentIt = node->ReverseEdges.get<EdgeTypeIndex>().find("me");
				if (parentIt != node->ReverseEdges.get<EdgeTypeIndex>().end())
				{
					displayNode(NULL,parentIt->Node,"my_photos");
					friendDetailView->showChild(node);
				}
				else
				{
					throw new std::runtime_error("Orphan node!");
				}
			}
		}
	}
	else if (node->getNodeType().compare(NodeType::FacebookAlbum) == 0)
	{
		auto parentIt = node->ReverseEdges.get<EdgeTypeIndex>().find("friends");
		if (parentIt != node->ReverseEdges.get<EdgeTypeIndex>().end())
		{
			previousNode = parentIt->Node;
			albumDetailView->setFinishedCallback([this,previousNode,node](string action){				
				this->displayNode(node,previousNode,"");
			});			
		}
		else
		{			
			parentIt = node->ReverseEdges.get<EdgeTypeIndex>().find("me");

			if (parentIt != node->ReverseEdges.get<EdgeTypeIndex>().end())
			{
				previousNode = parentIt->Node;
				albumDetailView->setFinishedCallback([this,previousNode,node](string action){				
					this->displayNode(node,previousNode,"my_photos");
				});			
			}
			else
			{
				throw new std::runtime_error("Orphan node!");
			}
		}		

		setTopView(albumDetailView);
		albumDetailView->show(node);	
	}
	else if (node->getNodeType().compare(NodeType::FacebookFriend) == 0)
	{	
		friendDetailView->setFinishedCallback([this,node](string action){				
			this->displayNode(node,userNode,"friend_list_view");
		});		
		
		setTopView(friendDetailView);
		friendDetailView->show(node);	
	}
	else if (node->getNodeType().compare("me") == 0)
	{		
		userNode = node;
		if (action.compare("my_photos") == 0)
		{
			node->Edges.insert(Facebook::Edge("name","My Photos and Albums"));

			friendDetailView->setFinishedCallback([this,node](string action){				
				this->displayNode(NULL,node,"");
			});		

			setTopView(friendDetailView);
			friendDetailView->show(node);
		}
		else if (action.compare("friend_list_view") == 0)
		{
			friendList->setFinishedCallback([this,node](string action){				
				this->displayNode(NULL,node,"");
			});		

			friendList->show(node);
			setTopView(friendList);
		}
		else
		{
			introView->setFinishedCallback([this,node](string _action){
				this->displayNode(NULL,node,_action);
			});		
			setTopView(introView);
			introView->show(node);			
		}
	}
	else
	{
		throw new std::runtime_error("Unknown node type: " + node->getNodeType());
	}
	this->layout(Vector(),cv::Size2f(0,0));
}

void FacebookBrowser::setTopView(View * _topView)
{
	if (this->topView != NULL)
	{
		if (topView == friendList)
			friendList->suspend();
		else if (topView == albumDetailView)
			albumDetailView->suspend();
		else if (topView == friendDetailView)
			friendDetailView->suspend();
	}

	this->topView = _topView;
}

void FacebookBrowser::onFrame(const Controller & controller)
{
	topView->onFrame(controller);
}

void FacebookBrowser::layout(Vector position, cv::Size2f size)
{ 	
	position = Vector(0,GlobalConfig::tree()->get<float>("Menu.Height"),0);
	size = cv::Size2f(GlobalConfig::ScreenWidth, GlobalConfig::ScreenHeight-(GlobalConfig::tree()->get<float>("Tutorial.Height")+GlobalConfig::tree()->get<float>("Menu.Height")));

	topView->layout(position,size);
}

void FacebookBrowser::draw()
{
	topView->draw();
}

void FacebookBrowser::update()
{
	topView->update();
}



