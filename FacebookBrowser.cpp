#include "FacebookBrowser.hpp"
#include "LinearLayout.hpp"
#include "GraphicContext.hpp"

FacebookBrowser::FacebookBrowser()
{		
	friendList = new FacebookFriendListView();
	introView  = new FacebookIntroView();
	albumDetailView = new AlbumDetailView();
	friendDetailView = new FriendDetailView();
	friendCursorView = new FriendListCursorView();
	
	state = 0;
		
	topView = NULL;
	userNode = NULL;

	pathView = new LinearLayout();

	TextPanel * homeButtonText = new Button("Home");
	homeButtonText->setTextSize(10,false);
	homeButtonText->setTextColor(Colors::SteelBlue);

	
	float menuHeight = GlobalConfig::tree()->get<float>("Menu.Height");

	homeButton = homeButtonText;
	homeButton->setLayoutParams(LayoutParams(cv::Size2f(menuHeight*1.5f,menuHeight),cv::Vec4f(5,5,5,5)));
	homeButton->elementClickedCallback = [this](LeapElement * clicked){		
		this->displayNode(NULL,this->userNode,"");
	};
	
	pathView->addChild(homeButton);
}

void FacebookBrowser::displayNode(FBNode * previousNode, FBNode * node, string action)
{
	if (node == NULL)
		return;

	previousNode = NULL;
	
	homeButton->setClickable(true);
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
			if (GlobalConfig::tree()->get<bool>("FriendLookupView.Enable"))
			{
				friendCursorView->setFinishedCallback([this,node](string action){				
					this->displayNode(NULL,node,"");
				});		

				friendCursorView->setUserNode(node);
				setTopView(friendCursorView);
			}
			else
			{
				friendList->setFinishedCallback([this,node](string action){				
					this->displayNode(NULL,node,"");
				});		

				friendList->show(node);
				setTopView(friendList);
			}
		}
		else
		{
			introView->setFinishedCallback([this,node](string _action){
				this->displayNode(NULL,node,_action);
			});		
			setTopView(introView);
			introView->show(node);	
			homeButton->setClickable(false);
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
	float menuHeight = GlobalConfig::tree()->get<float>("Menu.Height");
	Vector menuBarPosition = Vector();
	cv::Size2f menuBarSize = cv::Size2f(GlobalConfig::ScreenWidth-menuHeight, menuHeight);
	
	pathView->measure(menuBarSize);
	pathView->layout(menuBarPosition,menuBarSize);

	Vector contentPosition = Vector(0,menuHeight,0);
	cv::Size2f contentSize = cv::Size2f(GlobalConfig::ScreenWidth, GlobalConfig::ScreenHeight-(GlobalConfig::tree()->get<float>("Tutorial.Height")+menuHeight));

	topView->layout(contentPosition,contentSize);
}

LeapElement * FacebookBrowser::elementAtPoint(int x, int y, int & state)
{
	LeapElement * hit = pathView->elementAtPoint(x,y,state);

	if (hit != NULL)
		return hit;
	return View::elementAtPoint(x,y,state);
}

void FacebookBrowser::draw()
{
	topView->draw();

	if (!GraphicsContext::getInstance().IsBlurCurrentPass)
	{	
		pathView->draw();		
	}
	else
	{
		GraphicsContext::getInstance().requestClearDraw([this](){
			pathView->draw();
		});
	}
}

void FacebookBrowser::update()
{
	topView->update();
}



