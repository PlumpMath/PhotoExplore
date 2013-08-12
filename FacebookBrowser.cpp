#include "FacebookBrowser.hpp"
#include "LinearLayout.hpp"
#include "GraphicContext.hpp"

FacebookBrowser::FacebookBrowser()
{		
	introView  = new FacebookIntroView();
	albumCursorView = new AlbumCursorView();
	friendDetailView = new FriendCursorView();
	friendCursorView = new FriendListCursorView();

	friendCursorView->setFinishedCallback([this](string action){				
		this->displayNode(this->userNode,"");
	});		
	
	state = 0;
		
	topView = NULL;
	userNode = NULL;

	pathView = new LinearLayout();

	float cornerPadding = GlobalConfig::tree()->get<float>("Menu.CornerPadding");
	TextPanel * homeButtonText = new Button("Home");
	homeButtonText->setTextSize(10,false);
	homeButtonText->setTextColor(Colors::SteelBlue);
	homeButtonText->setTextFitPadding(cornerPadding);
	homeButtonText->setTextFitMode(true);
	
	
	float menuHeight = ActivityView::getMenuHeight();

	homeButton = homeButtonText;
	//((Button*)homeButton)->setTextAlignment(TextPanel::Left);

	
	homeButton->setLayoutParams(LayoutParams(cv::Size2f(menuHeight*1.5f,menuHeight),cv::Vec4f(0,0,0,0)));
	homeButton->elementClickedCallback = [this](LeapElement * clicked){		
		this->displayNode(this->userNode,"");
	};
	
	pathView->addChild(homeButton);
}

void FacebookBrowser::viewFinished(View * finishedView)
{

}

void FacebookBrowser::displayNode(FBNode * node, string action)
{
	if (node == NULL)
		return;
		
	homeButton->setClickable(true);
	if (node->getNodeType().compare(NodeType::FacebookImage) == 0)
	{
		auto parentIt = node->ReverseEdges.get<EdgeTypeIndex>().find("albums");
		if (parentIt != node->ReverseEdges.get<EdgeTypeIndex>().end())
		{			
			displayNode(parentIt->Node,"");
			albumCursorView->showPhoto(node);
		}
		else
		{
			parentIt = node->ReverseEdges.get<EdgeTypeIndex>().find("friends");			
			if (parentIt != node->ReverseEdges.get<EdgeTypeIndex>().end())
			{
				displayNode(parentIt->Node,"");
				friendDetailView->showPhoto(node);
			}
			else
			{			
				parentIt = node->ReverseEdges.get<EdgeTypeIndex>().find("me");
				if (parentIt != node->ReverseEdges.get<EdgeTypeIndex>().end())
				{
					displayNode(parentIt->Node,"my_photos");
					friendDetailView->showPhoto(node);
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
			FBNode * show = parentIt->Node;
			albumCursorView->setFinishedCallback([this,show](string action){				
				this->displayNode(show,"");
			});			
		}
		else
		{			
			parentIt = node->ReverseEdges.get<EdgeTypeIndex>().find("me");

			if (parentIt != node->ReverseEdges.get<EdgeTypeIndex>().end())
			{
				FBNode * show = parentIt->Node;
				albumCursorView->setFinishedCallback([this,show](string action){				
					this->displayNode(show,"my_photos");
				});
			}
			else
			{
				throw new std::runtime_error("Orphan node!");
			}
		}		

		setTopView(albumCursorView);
		albumCursorView->setAlbumOwner(node);
	}
	else if (node->getNodeType().compare(NodeType::FacebookFriend) == 0)
	{	
		friendDetailView->setFinishedCallback([this,node](string action){
			this->setTopView(this->friendCursorView);
		});		
		
		setTopView(friendDetailView);
		friendDetailView->setFriendNode(node);	
	}
	else if (node->getNodeType().compare("me") == 0)
	{		
		userNode = node;
		if (action.compare("my_photos") == 0)
		{
			node->Edges.insert(Facebook::Edge("name","My Photos"));

			friendDetailView->setFinishedCallback([this,node](string action){				
				this->displayNode(node,"");
			});		

			setTopView(friendDetailView);
			friendDetailView->setFriendNode(node);
		}
		else if (action.compare("friend_list_view") == 0)
		{	
			//friendCursorView->setUserNode(node);				
			setTopView(friendCursorView);
		}
		else
		{
			introView->setFinishedCallback([this,node](string _action){
				this->displayNode(node,_action);
			});		
			setTopView(introView);
			introView->show(node);	
			homeButton->setClickable(false);
		}
	}
	else
	{
		throw new std::runtime_error("Unknown node type: ");// + node->getNodeType().c_str);
	}
	//this->layout(Vector(),cv::Size2f(0,0));
}

void FacebookBrowser::setTopView(View * _topView)
{
	if (this->topView != NULL && this->topView != _topView)
	{
		if (topView == friendCursorView)
		{
			friendCursorView->suspend();
			friendCursorView->setViewState(ActivityView::Suspended);
		}
		else if (topView == albumCursorView)
			albumCursorView->suspend();
		else if (topView == friendDetailView)
			friendDetailView->suspend();
	}

	this->topView = _topView;

	
	if (this->topView != NULL)
	{
		if (topView == friendCursorView)
		{
			if (friendCursorView->getViewState() == ActivityView::Suspended)
				friendCursorView->resume();
			else
				friendCursorView->setUserNode(userNode);

			friendCursorView->setViewState(ActivityView::Active);
		}
		else if (topView == albumCursorView)
		{
			//albumDetailView->show();
		}
		else if (topView == friendDetailView)
		{
			//friendDetailView->show();
		}
	}
	this->layout(Vector(),cv::Size2f(0,0));
}

void FacebookBrowser::onFrame(const Controller & controller)
{
	topView->onFrame(controller);
}

void FacebookBrowser::layout(Vector position, cv::Size2f size)
{ 	
	float menuHeight = ActivityView::getMenuHeight();
	float tutorialHeight = ActivityView::getTutorialHeight();

	Vector menuBarPosition = Vector();
	cv::Size2f menuBarSize = cv::Size2f(GlobalConfig::ScreenWidth-menuHeight, menuHeight);
	
	pathView->measure(menuBarSize);
	pathView->layout(menuBarPosition,menuBarSize);

	Vector contentPosition = Vector(0,menuHeight,0);
	cv::Size2f contentSize = cv::Size2f(GlobalConfig::ScreenWidth, GlobalConfig::ScreenHeight-(tutorialHeight+menuHeight));

	topView->layout(contentPosition,contentSize);
}

LeapElement * FacebookBrowser::elementAtPoint(int x, int y, int & state)
{
	LeapElement * hit = pathView->elementAtPoint(x,y,state);

	if (hit != NULL)
		return hit;

	if (topView != NULL)
		return topView->elementAtPoint(x,y,state);
	
	return NULL;
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



