#include "FacebookIntroView.hpp"
#include "CustomGrid.hpp"
#include "GraphicContext.hpp"
#include "LeapDebug.h"
#include "EmptyView.hpp"
#include "FacebookDataDisplay.hpp"

using namespace cv;

FacebookIntroView::FacebookIntroView()
{
	friendPhotoGrid = new UniformGrid(Size2i(4,4));	
	friendPhotoGrid->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(0,0,10,0)));

	myPhotoGrid = new UniformGrid(Size2i(4,4));
	myPhotoGrid->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(10,0,0,0)));
		
	vector<RowDefinition> gridDefinition;	
	gridDefinition.push_back(RowDefinition(1));	
	gridDefinition[0].ColumnWidths.push_back(.5f);
	gridDefinition[0].ColumnWidths.push_back(.5f);

	mainLayout = new CustomGrid(gridDefinition);
		
	layoutDirty = true;
	lastPosition = Leap::Vector(0,0,0);
	lastSize = cv::Size2f(0,0);

	Color buttonColor = Colors::SteelBlue;

	photoButton = new Button("My Photos");
	photoButton->panelId = "my_photo_button";
	photoButton->setTextSize(12);
	photoButton->setTextColor(Colors::White);
	photoButton->setBackgroundColor(buttonColor);
	photoButton->setBorderThickness(0);
	photoButton->elementClickedCallback = boost::bind(&FacebookIntroView::buttonClicked,this,_1);
	photoButton->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(50,50,50,50)));
		
	friendListButton = new Button("Photos of my Friends");
	friendListButton->setTextSize(12);
	friendListButton->setTextColor(Colors::White);
	friendListButton->setBackgroundColor(buttonColor);	
	friendListButton->setBorderThickness(0);
	friendListButton->elementClickedCallback = boost::bind(&FacebookIntroView::buttonClicked,this,_1);
	friendListButton->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(50,50,50,50)));
		
	
	mainLayout->addChild(friendPhotoGrid);
	mainLayout->addChild(myPhotoGrid);

	addChild(mainLayout);
	addChild(friendListButton);
	addChild(photoButton);
}

void FacebookIntroView::setFinishedCallback(const boost::function<void(std::string)> & callback)
{
	finishedCallback = callback;
}

void FacebookIntroView::buttonClicked(LeapElement * element)
{
	for (auto it =  friendPhotoGrid->getChildren()->begin(); it != friendPhotoGrid->getChildren()->end();it++)
	{
		Panel * p = dynamic_cast<Panel*>(*it);

		if (p != NULL)
			p->setDataPriority(100);
	}
	friendPhotoGrid->clearChildren();

	for (auto it =  myPhotoGrid->getChildren()->begin(); it != myPhotoGrid->getChildren()->end();it++)
	{
		Panel * p = dynamic_cast<Panel*>(*it);

		if (p != NULL)
			p->setDataPriority(100);
	}
	myPhotoGrid->clearChildren();


	if (element == photoButton)
	{
		finishedCallback("my_photos");
	}
	else if (element == friendListButton)
	{
		finishedCallback("friend_list_view");
	}
}



void FacebookIntroView::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	;
}

bool FacebookIntroView::onLeapGesture(const Controller & controller, const Gesture & gesture)
{
	return false;
}


void FacebookIntroView::getTutorialDescriptor(vector<string> & tutorial)
{
	tutorial.push_back("point");
}

void FacebookIntroView::show(FBNode * node)
{	
	PointableElementManager::getInstance()->requestGlobalGestureFocus(this);	
	friendPhotoGrid->clearChildren();
	myPhotoGrid->clearChildren();

	activeNode = node;
	
	vector<FBNode*> localMyFriendList;
	NodeQuerySpec friendConfig_local(2);	
	friendConfig_local.layers[0].insert(make_pair("friends",SelectionConfig(40)));
	friendConfig_local.layers[1].insert(make_pair("photos",SelectionConfig(1)));
	DataViewGenerator::getInstance()->SelectNodes(node,friendConfig_local,localMyFriendList);

	if (localMyFriendList.size() < 12)
	{
		stringstream friendQuery;
		friendQuery << node->getId() << "?fields=friends.limit(20).fields(id,name)"; //,photos.limit(3).fields(id,name,images))";
		
		FacebookIntroView * v = this;
		FBDataSource::instance->loadField(node,friendQuery.str(),"",[v](FBNode * loaded){

			auto fRange = v->activeNode->Edges.get<EdgeTypeIndex>().equal_range("friends");
			for (; fRange.first != fRange.second; fRange.first++)
			{
				if (fRange.first->Node->Edges.get<EdgeTypeIndex>().count("photos") < 3)
				{					
					stringstream load2;
					load2 << fRange.first->Node->getId() << "?fields=photos.fields(id,name,images).limit(3)";

					FacebookIntroView * v2= v;
					FBDataSource::instance->loadField(fRange.first->Node,load2.str(),"",[v2](FBNode * nn)
					{
						if (v2->friendPhotoGrid->getChildren()->size() < 16)
						{
							NodeQuerySpec friendConfig(2);	
							friendConfig.layers[0].insert(make_pair("friends",SelectionConfig(20)));
							friendConfig.layers[1].insert(make_pair("photos",SelectionConfig(2)));

							vector<FBNode*> photoList;
							DataViewGenerator::getInstance()->SelectNodes(v2->activeNode,friendConfig,photoList);

							if (photoList.size() > 12)
							{
								FacebookIntroView * v3= v2;
								v2->postTask([v3,photoList](){
									v3->viewChanged("friend_photos",photoList);
								});
							}
						}
					});
				}
			}

			//NodeQuerySpec friendConfig(2);	
			//friendConfig.layers[0].insert(make_pair("friends",SelectionConfig(40)));
			//friendConfig.layers[1].insert(make_pair("photos",SelectionConfig(2)));

			//vector<FBNode*> photoList;
			//DataViewGenerator::getInstance()->SelectNodes(loaded,friendConfig,photoList);

			//if (photoList.size() > 0)
			//{
			//	FacebookIntroView * v = this;
			//	postTask([v,photoList](){
			//		v->viewChanged("friend_photos",photoList);
			//	});
			//}
		});
	}
	else
	{
		viewChanged("friend_photos",localMyFriendList);
	}


	vector<FBNode*> localMyPhotoList;
	NodeQuerySpec myConfig_local(2);
	myConfig_local.layers[0].insert(make_pair("albums",SelectionConfig(10,0,false)));
	myConfig_local.layers[0].insert(make_pair("photos",SelectionConfig(5)));
	myConfig_local.layers[1].insert(make_pair("photos",SelectionConfig(5)));

	DataViewGenerator::getInstance()->SelectNodes(node,myConfig_local,localMyPhotoList);

	if (localMyPhotoList.size() < 12)
	{
		stringstream myPhotoQuery;
		myPhotoQuery << node->getId() << "?fields=photos.limit(5),albums.limit(10).fields(photos.limit(5).fields(id,name,images),id,name)";
		FBDataSource::instance->loadField(node,myPhotoQuery.str(),"",[this](FBNode * loaded){

			NodeQuerySpec myConfig(2);
			myConfig.layers[0].insert(make_pair("albums",SelectionConfig(10,0,false)));
			myConfig.layers[0].insert(make_pair("photos",SelectionConfig(5)));
			myConfig.layers[1].insert(make_pair("photos",SelectionConfig(5)));

			vector<FBNode*> photoList;
			DataViewGenerator::getInstance()->SelectNodes(loaded,myConfig,photoList);

			if (photoList.size() > 0)
			{
				FacebookIntroView * v = this;
				this->postTask([v,photoList](){
					v->viewChanged("my_photos",photoList);
				});
			}
		});
	}
	else
	{
		viewChanged("my_photos",localMyPhotoList);
	}

	//viewChanged("my_photos",DataViewGenerator::getInstance()->getDataView(node,myConfig,[this](vector<FBNode*> & data1){this->viewChanged("my_photos",data1);},isLoading));
}

void FacebookIntroView::viewChanged(string viewIdentifier, vector<FBNode*> viewData)
{
	bool friendPhotos = viewIdentifier.compare("friend_photos") == 0;

	if (friendPhotos)
		friendPhotoGrid->clearChildren();
	else
		myPhotoGrid->clearChildren();


	std::random_shuffle(viewData.begin(),viewData.end());

	
	for (auto it = viewData.begin(); it != viewData.end(); it++)
	{
		if (friendPhotos)
		{
			int size = friendPhotoGrid->getChildren()->size() ;
			if (size >= 16)
				break;
			else if (size == 5 || size == 6 || size == 9 || size == 10)
			{
				friendPhotoGrid->addChild(new EmptyView());
				continue;
			}
		}
		else
		{
			int size = myPhotoGrid->getChildren()->size();
			if (size >= 16)
				break;
			else if (size == 5 || size == 6 || size == 9 || size == 10)
			{
				myPhotoGrid->addChild(new EmptyView());
				continue;
			}
		}

		
		FBNode * node = (*it);
		if (node->getNodeType().compare(NodeType::FacebookImage) == 0)
		{
			View * item = ViewOrchestrator::getInstance()->requestView((*it)->getId(), NULL);

			if (item == NULL)
			{
				Panel * p = new Panel(0,0);
				p->show(node);
				p->setVisible(true);
				item = p;				
				ViewOrchestrator::getInstance()->registerView(node->getId(),item, NULL);
			}



			((Panel*)item)->setClickable(true);
			((Panel*)item)->setDataPriority(0);
			item->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(5,5,5,5)));
			item->elementClickedCallback = [node](LeapElement * element){
				FacebookDataDisplay::getInstance()->displayNode(node,"");
			};


			if (friendPhotos)
			{
 				friendPhotoGrid->addChild(item);
			}
			else
			{
				myPhotoGrid->addChild(item);
			}

		}
		//else if (node->getNodeType().compare(NodeType::FacebookFriend) == 0)
		//{
		//	NodeQuerySpec friendPhotos(1);
		//	friendPhotos.layers[0].insert(make_pair("photos",SelectionConfig(5)));
		//	bool isLoading;
		//	DataViewGenerator::getInstance()->getDataView(node,friendPhotos,[this](vector<FBNode*> & data){this->viewChanged("friend_photos",data);},isLoading);
		//}

	}

	layoutDirty = true;
}

void FacebookIntroView::layout(Leap::Vector position, Size2f size)
{
	lastPosition = position;
	lastSize = size;

	mainLayout->layout(position,size);

	cv::Size2f gridCellSize = cv::Size2f(size.width * .5f, size.height);
	gridCellSize.width -= 10;
	//gridCellSize.height -= 10;

	gridCellSize.height /= 4.0f;
	gridCellSize.width /= 4.0f;

	cv::Size2f buttonSize = cv::Size2f((gridCellSize.width *2.0f)-10 ,(gridCellSize.height * 2.0f) - 10);
	friendListButton->layout(position + Leap::Vector(gridCellSize.width + 5, gridCellSize.height+5 ,1), buttonSize);
	photoButton->layout(position + Leap::Vector(gridCellSize.width + (size.width * .5f) + 15 ,gridCellSize.height+5,1), buttonSize);
		
	layoutDirty = false;
}

//Note: This is a hack.
void FacebookIntroView::onFrame(const Controller & controller)
{	
	//HandModel * hm = HandProcessor::LastModel();	
	//Pointable testPointable = controller.frame().pointable(hm->IntentFinger);
	//
	//if (testPointable.isValid())
	//{
	//	Leap::Vector screenPoint = LeapHelper::FindScreenPoint(controller,testPointable);

	//	if (friendPhotoGrid->getHitRect().contains(cv::Point_<float>(screenPoint.x,screenPoint.y)))
	//	{
	//		friendScroll->OnPointableEnter(testPointable);
	//	}
	//	else if (myPhotoGrid->getHitRect().contains(cv::Point_<float>(screenPoint.x,screenPoint.y)))
	//	{
	//		photoScroll->OnPointableEnter(testPointable);
	//	}
	//}
}
