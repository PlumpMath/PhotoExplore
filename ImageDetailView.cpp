#include "ImageDetailView.hpp"
#include "GraphicContext.hpp"
#include "FacebookLoader.h"


ImageDetailView::ImageDetailView() 
{
	imagePanel = NULL;
	initButtonBar();

}

void ImageDetailView::notifyOffsetChanged(Leap::Vector _offset)
{
	this->hostOffset = _offset;
}

void ImageDetailView::initButtonBar()
{

	Color c = Colors::HoloBlueBright;
	c.setAlpha(.6f);
	likeButton = new Button("Like this photo?");
	likeButton->setBackgroundColor(Colors::DarkBlue);
	likeButton->setTextColor(Colors::White);
	likeButton->setTextSize(8);
	likeButton->setVisible(false);
	likeButton->setAnimateOnLayout(false);
	likeButton->setTextFitPadding(6);

	
	alreadyLikedButton = new Button("You like this.");
	alreadyLikedButton->setBackgroundColor(Colors::HoloBlueDark);
	alreadyLikedButton->setTextColor(Colors::White);
	alreadyLikedButton->setTextSize(8);
	alreadyLikedButton->setVisible(false);
	alreadyLikedButton->setAnimateOnLayout(false);
	alreadyLikedButton->setTextFitPadding(6);
	alreadyLikedButton->setEnabled(false);
	alreadyLikedButton->setTextFitPadding(6);
	
	//likeButton->layout(likeButton->getPosition(),cv::Size2f(250,200));

	//likeButton->setBorderThickness(1);
	//likeButton->setBorderColor(Colors::HoloBlueBright);

	photoComment = new Button("");
	photoComment->setLayoutParams(LayoutParams(cv::Size2f(400,100),cv::Vec4f(20,20,20,80)));
	photoComment->setAnimateOnLayout(false);
	photoComment->setTextColor(Colors::HoloBlueBright);
	photoComment->setBackgroundColor(Colors::Transparent);
	photoComment->setTextSize(8);
	photoComment->setTextFitPadding(12);
	photoComment->setEnabled(false);
	

	addChild(photoComment);
	addChild(likeButton);
	addChild(alreadyLikedButton);
}

void ImageDetailView::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	if (gestureType.compare("shake") == 0)
	{
		this->setImagePanel(NULL);
		PointableElementManager::getInstance()->releaseGlobalGestureFocus(this);
		this->finishedCallback("");
	}
}


bool ImageDetailView::onLeapGesture(const Controller & controller, const Gesture & gesture)
{
	if (GlobalConfig::tree()->get<bool>("FullscreenImageView.AllowSwipeDownExit"))
	{
		if (gesture.type() == Gesture::Type::TYPE_SWIPE && (gesture.state() == Gesture::State::STATE_UPDATE|| gesture.state() == Gesture::State::STATE_UPDATE))
		{
			SwipeGesture swipe(gesture);

			if (swipe.direction().angleTo(Vector::down()) < PI/4.0f)
			{
				this->setImagePanel(NULL);
				PointableElementManager::getInstance()->releaseGlobalGestureFocus(this);
				this->finishedCallback("");
				return true;
			}
		}	
	}
	return false;
}

void ImageDetailView::getTutorialDescriptor(vector<string> & tutorial)
{
	tutorial.push_back("shake");
	tutorial.push_back("stretch");	
}

void ImageDetailView::setImageMetaData()
{
	likeButton->setVisible(false);
	alreadyLikedButton->setVisible(false);
	photoComment->setVisible(false);

	if (this->imagePanel != NULL)
	{		
		imageNode = dynamic_cast<Facebook::FBNode*>(imagePanel->getNode());
		if (imageNode != NULL)
		{
			//SELECT like_info FROM photo where album_object_id=10151865693300599 and owner = me()
			imageNode->setLoadCompleteDelegate([this](){

				if (imageNode->getAttribute("user_likes").compare("1") == 0)
				{
					alreadyLikedButton->setVisible(true);
					likeButton->setVisible(false);
				}
				else 
				{				
					alreadyLikedButton->setVisible(false);
					likeButton->setVisible(true);
					ImageDetailView * me = this;
					FBNode * nodeToLike = imageNode;
					likeButton->elementClickedCallback = [nodeToLike ,me](LeapElement * clicked){
						
						me->alreadyLikedButton->setVisible(true);				
						me->likeButton->setVisible(false);
						//nodeToLike->Edges.insert(Facebook::Edge("user_likes","1"));
						auto likeIt = nodeToLike->Edges.get<EdgeTypeIndex>().find("user_likes");
						if (likeIt != nodeToLike->Edges.get<EdgeTypeIndex>().end())
							nodeToLike->Edges.replace(likeIt,Facebook::Edge("user_likes","1"));
						else
							nodeToLike->Edges.insert(Facebook::Edge("user_likes","1"));
						((FacebookLoader*)Facebook::FBDataSource::instance)->postRequest(nodeToLike->getId() + "/likes");
					};	
				}
				imageNode->clearLoadCompleteDelegate();
			});
			Facebook::FBDataSource::instance->loadQuery(imageNode,"fql?q=SELECT%20like_info%20FROM%20photo%20where%20object_id%3D" + imageNode->getId() + "%20and%20owner%20%3D%20me()","");
			
			if (imageNode->getAttribute("name").size() != 0)
			{
				photoComment->setVisible(true);
				string comment = imageNode->getAttribute("name");
				int maxLength = GlobalConfig::tree()->get<int>("FullscreenImageView.MaxCommentLength");
				if (comment.length() > maxLength)
				{
					comment = comment.substr(0,maxLength) + "...";

				}

				photoComment->setText(comment);
				photoComment->reloadText();
				this->layoutDirty = true;
				//photoComment->layout(photoComment->getPosition(),cv::Size2f(400,100));
			}
			else
				photoComment->setVisible(false);
		}
		else
		{
			likeButton->setVisible(false);
		}
	}
}

void ImageDetailView::setImagePanel(Panel * _imagePanel)
{
	if (imagePanel != NULL)
	{
		this->imagePanel->NudgeAnimationEnabled = true;
		this->imagePanel->setVisible(true);
		this->imagePanel->setClickable(true);
		this->imagePanel->setDataPriority(0);
		this->imagePanel->setTextureWindow(Vector(),Vector(1,1,1));
		remove(imagePanel);

		activePanelInteraction.interactingPointables.clear();
		activePanelInteraction.pointableCentroid = Vector(0,0,0);
		activePanelInteraction.pointableRange = 0;
		activePanelInteraction.panel = NULL;
	}

	this->imagePanel = _imagePanel;
	if (this->imagePanel != NULL)
	{
		if (std::find(children.begin(),children.end(),imagePanel) == children.end())
		{
			children.push_back(imagePanel);
		}

		this->imagePanel->setDataPriority(0);
		this->imagePanel->NudgeAnimationEnabled = false;
		this->imagePanel->setVisible(false);
		this->imagePanel->setClickable(false);
		this->imagePanel->setFullscreenMode(true);
		setImageMetaData();
	}
}

Panel * ImageDetailView::getImagePanel()
{
	return this->imagePanel;
}

void ImageDetailView::setFinishedCallback(const boost::function<void(std::string)> & callback)
{
	finishedCallback = callback;
}


void ImageDetailView::update()
{
	if (imageNode != NULL)
		imageNode->update();
}

void ImageDetailView::layout(Vector position, cv::Size2f size)
{
	if (imagePanel != NULL && size.width > 0 && size.height > 0)
	{
		size.height -= 150;

		lastSize = size;
		lastPosition = position;

		likeButton->layout(-hostOffset + position + Vector(size.width*.1f,size.height*.82f,1),cv::Size2f(size.width*.15f,size.height*.16f));
		alreadyLikedButton->layout(-hostOffset + position + Vector(size.width*.1f,size.height*.82f,1),cv::Size2f(size.width*.15f,size.height*.16f));
		photoComment->layout(-hostOffset + position + Vector(size.width*.3f,size.height*.8f,1),cv::Size2f(size.width*.4f,size.height*.2f));

		imagePanel->fitPanelToBoundary(-hostOffset + position + Vector(size.width*.5f,size.height*.4f,10),size.width,size.height*.8f, false);
	
		layoutDirty = false;
	}
}

void ImageDetailView::setVisible(bool _visible)
{
	GraphicsContext::getInstance().setBlurEnabled(_visible);

	View::setVisible(_visible);

	if (imagePanel != NULL)
		imagePanel->setFullscreenMode(isVisible());	
}


LeapElement * ImageDetailView::elementAtPoint(int x, int y, int & elementStateFlags)
{
	LeapElement * hit = NULL;
	if (this->imagePanel != NULL)
		hit = imagePanel->elementAtPoint(x-hostOffset.x,y-hostOffset.y,elementStateFlags);
	if (hit == NULL)
		return ViewGroup::elementAtPoint(x-hostOffset.x,y-hostOffset.y,elementStateFlags);
}

void ImageDetailView::onFrame(const Controller & controller)
{
	handleImageManipulation(controller);	
}

void ImageDetailView::draw()
{
	if (!GraphicsContext::getInstance().IsBlurCurrentPass)
	{
		glTranslatef(hostOffset.x,0,0);
		ViewGroup::draw();
		if (this->imagePanel != NULL)
			this->imagePanel->draw();
		
		glTranslatef(-hostOffset.x,0,0);
	}
	else
	{
		GraphicsContext::getInstance().requestClearDraw([this](){this->draw();});
	}
}

static bool isValidInteractionPointable(const Controller & controller, Pointable p)
{
	bool result = false;
	if (p.isValid())
	{
		float dist = LeapHelper::ClosestScreen(controller, p.stabilizedTipPosition()).project(p.stabilizedTipPosition(),false).distanceTo(p.stabilizedTipPosition());

		if (dist < GlobalConfig::MinimumInteractionScreenDistance)
		{
			result = true;
		}
	}
	return result;
}

static bool getNewPanelInteraction(const Controller & controller, Frame frame, PanelBase * panel, PanelInteraction & activePanelInteraction, Vector hostOffset)
{
	activePanelInteraction.interactingPointables.clear();
	activePanelInteraction.pointableCentroid = Vector(0,0,0);
	activePanelInteraction.pointableRange = 0;

	vector<Pointable> interactionPointables;

	bool processed = false;

	
	if (!activePanelInteraction.lockoutTimer.elapsed())
	{		
		return false;
	}

	//TODO: Also account for hands with invalid pointables
	if (GlobalConfig::AllowSingleHandInteraction && frame.hands().count() == 1)
	{
		Hand hand = frame.hands()[0];
		HandModel * model = HandProcessor::LastModel(hand.id());

		vector<int> desiredFingers;
		desiredFingers.push_back(1);
		desiredFingers.push_back(2);
		desiredFingers.push_back(3);

		vector<int> selectedFingers = model->SelectFingers(desiredFingers);

		if (selectedFingers.size() == 3)
		{
			for (auto it = selectedFingers.begin(); it != selectedFingers.end();it++)
			{
				interactionPointables.push_back(hand.pointable(*it));
			}
		}
	}
	else if (frame.hands().count() >= 2)
	{
		Hand lh = frame.hands().leftmost();
		Hand rh = frame.hands().rightmost();

		int p1 = HandProcessor::LastModel(lh.id())->IntentFinger;
		int p2 = HandProcessor::LastModel(rh.id())->IntentFinger;

		interactionPointables.push_back(lh.pointable(p1));
		interactionPointables.push_back(rh.pointable(p2));
	}

	if (interactionPointables.size() >= 2)
	{			
		bool canStartInteraction = true;
		for (int i=0;i<interactionPointables.size();i++)
		{
			bool iterationSuccess = false;
			if (isValidInteractionPointable(controller, interactionPointables[i]) && interactionPointables.at(i).tipVelocity().magnitude() < GlobalConfig::SteadyVelocity)
			{						
				Vector imgPoint = LeapHelper::FindScreenPoint(controller,interactionPointables[i]);			

				int flags;
				if (panel->elementAtPoint((int)imgPoint.x-hostOffset.x,(int)imgPoint.y-hostOffset.y,flags) != NULL)
				{
					activePanelInteraction.interactingPointables.push_back(make_pair(interactionPointables[i].id(),imgPoint));		
					iterationSuccess = true;				
				}	
				else
				{
					activePanelInteraction.panel = NULL;
				}
			}
			canStartInteraction = canStartInteraction && iterationSuccess;
		}

		if (!canStartInteraction)
		{
			activePanelInteraction.interactingPointables.clear();
		}
		else if (activePanelInteraction.panel != panel)
		{
			activePanelInteraction.translation = Vector(0,0,0);
			activePanelInteraction.scale = Vector(1,1,1);
			activePanelInteraction.panel = (Panel*)panel;
		}				
		processed = canStartInteraction;
	}
	return processed;
}

bool ImageDetailView::handleImageManipulation(const Controller & controller)
{
	static Timer timer;
	PanelBase * panel = imagePanel;
	bool processed = false;

	if (panel != NULL)
	{
		Frame frame = controller.frame();
		bool error = true;

		if (activePanelInteraction.panel == panel && activePanelInteraction.interactingPointables.size() >= 2)
		{
			error = false;
			int size = activePanelInteraction.interactingPointables.size();
			
			vector<Vector> newImagePoints,newPoints;
			
			for (int i=0;i < size; i++)
			{
				pair<int,Vector> item = activePanelInteraction.interactingPointables.at(i);

				Pointable p = frame.pointable(item.first);
								
				if (isValidInteractionPointable(controller,p))
				{
					Vector imgPoint = LeapHelper::FindScreenPoint(controller,p);

					newImagePoints.push_back(Vector(imgPoint.x,imgPoint.y,0));
					newPoints.push_back(p.stabilizedTipPosition());

					activePanelInteraction.interactingPointables[i] = make_pair(item.first,imgPoint);

					LeapDebug::instance->addDebugVisual(new LeapDebugVisual(Point2f(imgPoint.x,imgPoint.y),1,LeapDebugVisual::LiveByTime,30,Colors::MediumVioletRed));
				}
				else
				{
					error = true;
					break;
				}
			}

			if (!error)
			{
				int size = newPoints.size();
				
				if (activePanelInteraction.interactingPointables.size() > 2)
				{			
					Vector newImageCentroid, newCentroid;

					for (int i=0;i<size;i++)
					{
						newImageCentroid += newImagePoints.at(i);
						newCentroid += newPoints.at(i);
					}

					newCentroid /= (float)size;
					newImageCentroid /= (float)size;
											
					Vector oldCentroid = activePanelInteraction.pointableCentroid;
					if (oldCentroid.x != 0)
					{
						LeapDebug::instance->addDebugVisual(new LeapDebugVisual(Point2f(newImageCentroid.x,newImageCentroid.y),1,LeapDebugVisual::LiveByTime,35,Colors::LightBlue));

						newImageCentroid.x = LeapHelper::lowpass(oldCentroid.x,newImageCentroid.x,40,timer.millis());
						newImageCentroid.y = LeapHelper::lowpass(oldCentroid.y,newImageCentroid.y,40,timer.millis());
					
						LeapDebug::instance->addDebugVisual(new LeapDebugVisual(Point2f(newImageCentroid.x,newImageCentroid.y),1,LeapDebugVisual::LiveByTime,40,Colors::Blue));

						activePanelInteraction.translation += (newImageCentroid - oldCentroid);
					}
					activePanelInteraction.pointableCentroid = newImageCentroid;
				}


				if (activePanelInteraction.interactingPointables.size() == 2)
				{
					float newDist = 0;
					for (int i=0;i<size;i++)
					{
						for (int j=0;j<size;j++)
						{
							if (i != j)
							{
								newDist = max<float>(newDist,newPoints.at(i).distanceTo(newPoints.at(j)));
							}
						}
					}									

					float oldDist = activePanelInteraction.pointableRange;
					if (oldDist > 0)
					{
						newDist = LeapHelper::lowpass(oldDist,newDist,40,timer.millis());

						float newScale = activePanelInteraction.scale.x *  (newDist/oldDist); 						

						newScale = max<float>(newScale,.5);
						newScale = min<float>(newScale,10);

						activePanelInteraction.scale.x = newScale;
					}

					activePanelInteraction.pointableRange = newDist;
				}

				timer.start();
				processed = true;
			}
		}

		//Something bad happened, so try and find new pointable to interact with the image
		if (error)
		{
			processed = getNewPanelInteraction(controller,frame,panel,activePanelInteraction,hostOffset);
		}
		else
		{
			activePanelInteraction.panel->setTextureWindow(Vector(),Vector());

			Vector pos;
			float w1,h1;
			activePanelInteraction.panel->getBoundingArea(pos,w1,h1);

			float sc1 = activePanelInteraction.scale.x;

			Vector position = pos;
			cv::Size2f size = lastSize;// cv::Size2f(w1,h1);//lastSize;
			//size.width = size.width * activePanelInteraction.scale.x;
			//size.height = size.height * activePanelInteraction.scale.x;
			
			//imagePanel->fitPanelToBoundary(-hostOffset + position + Vector(size.width*.5f,size.height*.5f,10),size.width,size.height, true);

			activePanelInteraction.panel->setTextureWindow((Vector(size.width*.5f,size.height*.4f,10)-pos)-Vector(w1*sc1*.5f,h1*sc1*.5f,0)-hostOffset,activePanelInteraction.scale);
		}
	}
	return processed;
}

