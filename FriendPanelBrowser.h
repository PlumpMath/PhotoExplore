#include "PanelBrowser.h"
#include "PointableElementManager.h"
#include <boost/function.hpp>
#include "PanelProvider.h"
#include "FBHumanNode.h"

#ifndef FRIEND_PANEL_BROWSER_H_
#define FRIEND_PANEL_BROWSER_H_

class FriendPanelBrowser : public PanelBrowser {

public:
	const static int StartState = 0;
	const static int PhotoState = 1;
	const static int FriendState = 2;
	const static int FriendPreviewState = 3;
	const static int PhotoPreviewState = 4;

	int state;

	TextPanel * photoButton;
	TextPanel * friendButton;

	TextPanel * photoInfo;
	Panel * goingTo;

	FriendPanelBrowser(float height) : PanelBrowser(cv::Size2f(height, height))
	{
		state = StartState;
		goingTo = NULL;

		photoInfo = new TextPanel("INFO");
		photoInfo->setTextFitMode(true);
		photoInfo->setTextFitPadding(20);
		photoInfo->panelId = "photo_info";
		photoInfo->setTextSize(6);
		photoInfo->setTextColor(Colors::HoloBlueBright);
		photoInfo->setBackgroundColor(Color(30,30,30,200));
		photoInfo->setPanelSize(400,150);
		//photoInfo->setScaleMode(ScaleMode::Fit);
		

		photoButton = new TextPanel("My Photos and Albums");
		photoButton->panelId = "my_photo_button";
		photoButton->setTextSize(10);
		photoButton->setTextColor(Colors::White);
		photoButton->setBackgroundColor(Colors::DarkBlue);
		photoButton->BorderSelectionThickness = 3;
		photoButton->setBorderColor(Colors::HoloBlueBright);
		//photoButton->elementClickedCallback = boost::bind(&FriendPanelBrowser::buttonClicked,this,_1);

		friendButton= new TextPanel("Photos and Albums of my Friends");
		friendButton->panelId = "friend_button";
		friendButton->setTextSize(10);
		friendButton->setTextColor(Colors::White);
		friendButton->setBackgroundColor(Colors::DarkBlue);
		friendButton->setBorderColor(Colors::HoloBlueBright);
		friendButton->BorderSelectionThickness = 3;
		//friendButton->elementClickedCallback = boost::bind(&FriendPanelBrowser::buttonClicked,this,_1);

		PointableElementManager::getInstance()->registerElement(photoButton);
		PointableElementManager::getInstance()->registerElement(friendButton);
	}

	void panelEntered(LeapElement * element, Pointable & pointable)
	{
		Panel * panel = dynamic_cast<Panel*>(element);

		if (panel != NULL && panel != goingTo)
		{
			FBNode * fbNode = dynamic_cast<FBNode*>(panel->getNode());
			stringstream info;
			
			Vector ps =panel->getPosition();

			if (fbNode->getNodeType() == NodeType::FacebookFriend)
			{
				info << "Pictures of " << ((FBFriendNode*)fbNode)->getName();
				photoInfo->setText(info.str());
				ps.x = (size.height * .6f) + photoInfo->getWidth()/2.0f;
				goingTo = panel;
			}
			else if (fbNode->getNodeType() == NodeType::FacebookAlbum)
			{
				info << ((FBAlbumNode*)fbNode)->name;
				photoInfo->setText(info.str());
				ps.x = (size.width - (size.height * .6f)) - photoInfo->getWidth()/2.0f;
				goingTo = panel;
			}
			else
			{
				photoInfo->setText("??");
				photoInfo->animatePanelSize(0,photoInfo->getHeight(),150);
			}

			ps.z += 1;
			photoInfo->animateToPosition(ps,150,150);
		}
		
	}
		
	void panelExit(LeapElement * element, Pointable & pointable)
	{
		//Vector ps = photoInfo->getPosition();
		//photoInfo->animateToPosition(ps,1000,1000);
		//photoInfo->animatePanelSize(1,1,1000);
	}
	

protected:

	Vector PositionOfIndex(int index, int numRows)
	{	
		Vector position;
		
		if (index < 0)
		{
			position.x = floor((double)index / (double)numRows);
			position.y = (numRows + index) % numRows;
		}
		else
		{
			position.x = index / numRows;
			position.y = index % numRows;
		}

		return position;
	}


	void buttonClicked(LeapElement * element)
	{
		if (state == StartState)
		{
			if (element == photoButton)
			{
				cout << "State = PhotoState\n";
				state = PhotoState;

				FBHumanNode * me = (FBHumanNode*)activeNode;
				
				vector<PanelBase*> filteredPanels;
				for (auto it = me->children.begin(); it != me->children.end(); it++)
				{
					if (it->second->getNodeType() == NodeType::FacebookAlbum || it->second->getNodeType() == NodeType::FacebookImage)
						filteredPanels.push_back(PanelProvider::getPanel(it->second,cellSize));
				}
				
				initPanels(filteredPanels, true);
			}
			else if (element == friendButton)
			{
				cout << "State = FriendState\n";
				state = FriendState;

				FBHumanNode * me = (FBHumanNode*)activeNode;
				vector<PanelBase*> filteredPanels;
				for (auto it = me->children.begin(); it != me->children.end(); it++)
				{
					if (it->second->getNodeType() == NodeType::FacebookFriend)
						filteredPanels.push_back(PanelProvider::getPanel(it->second,cellSize));
				}
				
				initPanels(filteredPanels, true);
			}
			else
			{
				Panel * panel = dynamic_cast<Panel*>(element);

				if (panel != NULL && panel->getNode()->getNodeType() == NodeType::FacebookFriend)
				{
					state = FriendPreviewState;
				}
				else if (panel != NULL && panel->getNode()->getNodeType() == NodeType::FacebookAlbum)
				{
					state = PhotoPreviewState;
				}
			}
		}

	}
	

	vector<PanelBase*> getPanelsForNode(NodeBase * rootNode)
	{
		vector<PanelBase*> loadedPanels;

		if (rootNode->getNodeType() == NodeType::FacebookOwner)
		{
			if (state == StartState)
			{
				loadedPanels = PanelBrowser::getPanelsForNode(rootNode);

				loadedPanels.push_back(photoButton);
				photoButton->setVisible(true);

				loadedPanels.push_back(friendButton);
				friendButton->setVisible(true);


			}
			else if (state == PhotoState)
			{
				FBHumanNode * me = (FBHumanNode*)rootNode;
				for (auto it = me->children.begin(); it != me->children.end(); it++)
				{
					if (it->second->getNodeType() == NodeType::FacebookAlbum || it->second->getNodeType() == NodeType::FacebookImage)
						loadedPanels.push_back(PanelProvider::getPanel(it->second,cellSize));
				}

			}
			else if (state == FriendState)
			{

				FBHumanNode * me = (FBHumanNode*)rootNode;
				for (auto it = me->children.begin(); it != me->children.end(); it++)
				{
					if (it->second->getNodeType() == NodeType::FacebookFriend)
						loadedPanels.push_back(PanelProvider::getPanel(it->second,cellSize));
				}

			}
		}
		else
		{
			loadedPanels = PanelBrowser::getPanelsForNode(rootNode);
		}

		for (auto it = loadedPanels.begin(); it != loadedPanels.end();it++)
		{
			//(*it)->ScaleOnEnter = 1.1f;

			if (state == StartState)
			{
				(*it)->pointableEnterCallback = boost::bind(&FriendPanelBrowser::panelEntered,this,_1,_2);
				(*it)->pointableExitCallback = boost::bind(&FriendPanelBrowser::panelExit,this,_1,_2);
				(*it)->elementClickedCallback = boost::bind(&FriendPanelBrowser::buttonClicked,this,_1);
			}
			else
			{
				
				(*it)->pointableEnterCallback.clear();// = boost::bind(&FriendPanelBrowser::panelEntered,this,_1,_2);
				(*it)->pointableExitCallback.clear();// = boost::bind(&FriendPanelBrowser::panelExit,this,_1,_2);
				(*it)->elementClickedCallback.clear();// = boost::bind(&FriendPanelBrowser::panelExit,this,_1,_2);
			}
		}

		return loadedPanels;
	}
	

	//void layoutInRing(int index, Vector & childPosition, cv::Size2f panelSize, int ringDim)
	//{		
	//	childPosition = Vector(panelSize.width * ((ringDim-1)/-2.0f), panelSize.height * ((ringDim-1)/-2.0f),0);

	//	int i = 0;
	//	while (i++ < index)
	//	{
	//		if (i < ringDim)
	//			childPosition.x += panelSize.width;
	//		else if (i < (ringDim * 2) - 1)
	//			childPosition.y += panelSize.height;
	//		else if (i < (ringDim * 3) - 2)
	//			childPosition.x -= panelSize.width;
	//		else if (i < (ringDim * 4) - 3)
	//			childPosition.y -= panelSize.height;
	//	}

	//}


	void layoutChild(int index, Vector & childPosition, cv::Size2f & childSize)
	{
		float screenWidth = size.width;
		float screenHeight = size.height;
		int numRows = 2;

		if (state == StartState && activeNode->getNodeType() == NodeType::FacebookOwner)
		{
			flyWheel->setMinValue(0);
			PanelBase * panel = panels.at(index);
			if (panel->panelId.compare("my_photo_button") == 0)
			{
				childSize = cv::Size2f(screenHeight * .6f,screenHeight/5.0f);
				childPosition = Vector(screenWidth - (screenHeight * .3f),screenHeight/2.0f,1);
				cout << "Drawing photo button at " << childPosition.toString() << " with size [" << childSize.height << "," << childSize.width << "] \n";
			}
			else if (panel->panelId.compare("friend_button") == 0)
			{
				childSize = cv::Size2f(screenHeight * .6f,screenHeight/5.0f);
				childPosition = Vector(screenHeight * .3f,screenHeight/2.0f,1);
				cout << "Drawing friend button at " << childPosition.toString() << " with size [" << childSize.height << "," << childSize.width << "] \n";
			}

			if (dynamic_cast<Panel*>(panel) != NULL)
			{
				NodeBase * panelNode = ((Panel*)panel)->getNode();
				
				if (panelNode->getNodeType() == NodeType::FacebookFriend)
				{
					int trueIndex = std::count_if(panels.begin(),panels.begin() + index,[](PanelBase * testPanel) 
					{
						return 
							dynamic_cast<Panel*>(testPanel) != NULL && 
							((Panel*)testPanel)->getNode()->getNodeType() == NodeType::FacebookFriend;
					});
					
					childPosition = PositionOfIndex(trueIndex, 4);
					if (childPosition.y >= 2)
						childPosition.y++;

					if (childPosition.x > 2)
					{
						childPosition.x = -1;
						childSize = cv::Size2f(0,0);
					}
					else
					{

						childSize = cv::Size2f(screenHeight/5.0f,screenHeight/5.0f);

						childPosition.y *= (childSize.height);
						childPosition.x *= childSize.width;

						childPosition.x += childSize.width/2.0f;
						childPosition.y += childSize.height/2.0f;	
					
						childPosition.z = max<float>(-index,-300);
					}
					
					//cout << "Layout of friend " << panelNode->getURI() << " at " << childPosition.toString() << "\n";
				}
				else if (panelNode->getNodeType() == NodeType::FacebookAlbum || panelNode->getNodeType() == NodeType::FacebookImage)
				{					
					int trueIndex = std::count_if(panels.begin(),panels.begin() + index,[](PanelBase * testPanel) 
					{
						return 
							dynamic_cast<Panel*>(testPanel) != NULL && (
							((Panel*)testPanel)->getNode()->getNodeType() == NodeType::FacebookAlbum || 
							((Panel*)testPanel)->getNode()->getNodeType() == NodeType::FacebookImage);
					});
					
					
					
					childPosition = PositionOfIndex(trueIndex, 4);
					if (childPosition.y >= 2)
						childPosition.y++;


					if (childPosition.x > 2)
					{
						childPosition.x = -1;
						childSize = cv::Size2f(0,0);
					}
					else
					{
						childSize = cv::Size2f(screenHeight/5.0f,screenHeight/5.0f);
						childPosition.y *= (childSize.height);
						childPosition.x = screenWidth - (childPosition.x * childSize.width);

						childPosition.x -= childSize.width/2.0f;
						childPosition.y += childSize.height/2.0f;

						childPosition.z = max<float>(-index,-300);
					}

					//cout << "Layout of " << panelNode->getURI() << " at " << childPosition.toString() << "\n";
				}
			}
		}
		else
		{
			
			if (panels.at(index)->panelId.compare("name") == 0)
			{
				childSize = cv::Size2f(cellSize.width * 2,cellSize.height *.5f); 	
				childPosition = Vector(childSize.width/2.0f,childSize.height * .5f,5);
			}
			else
			{
				int layoutIndex = std::count_if(panels.begin(),panels.begin() + index,[](PanelBase * testPanel) 
				{
					return dynamic_cast<Panel*>(testPanel) != NULL;
				});

				childPosition = PositionOfIndex(layoutIndex, numRows);
				childSize = cv::Size2f(defaultPanelSize.width,defaultPanelSize.height);


				childPosition.x *= childSize.width + 5;
				childPosition.y *= childSize.height + 5;

				childPosition.x += childSize.width * .5f;
				childPosition.y += childSize.height * .5f;

				childPosition.z = max<float>(-index,-300);

				childPosition.y += (screenHeight/2.0f)- (numRows*cellSize.height*.5f);
			}
		}

	}

	bool handleBackGesture()
	{
		if (!stateTransitionTimer.elapsed())
			return true;
		
		if (state == StartState)
		{
			return false;
		}
		else if (state == FriendPreviewState || state == PhotoPreviewState)
		{
			state = StartState;
			//initPanels(panels,false);
			return false;
		}
		else if (state == PhotoState || state == FriendState)
		{			
			stateTransitionTimer.countdown(500);

			if (activeNode->getNodeType() == NodeType::FacebookOwner)
			{
				cout << "State = StartState\n";
				state = StartState;

				vector<PanelBase*> p = getPanelsForNode(activeNode);
				initPanels(p, true);

				return true;
			}
		}
		return false;
	}

	void renderChildren()
	{
		PanelBrowser::renderChildren();

		if (state == StartState)
			photoInfo->draw();
	}

};

#endif