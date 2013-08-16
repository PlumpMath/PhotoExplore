#include "ImageDetailView.hpp"
#include "GraphicContext.hpp"
#include "SwipeGestureDetector.hpp"

ImageDetailView::ImageDetailView() 
{
	canClickToExit = false;
	imagePanel = NULL;
	scrollWheel = new FlyWheel(0);
}
DynamicImagePanel * ImageDetailView::getSiblingByOffset(int offset)
{
	return NULL;
}


void ImageDetailView::notifyOffsetChanged(Leap::Vector _offset)
{
	this->hostOffset = _offset;
}

void ImageDetailView::onGlobalFocusChanged(bool isFocused)
{
	if (isFocused)
		SwipeGestureDetector::getInstance().setFlyWheel(scrollWheel);
}

void ImageDetailView::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	if (gestureType.compare("shake") == 0)
	{
		this->setImagePanel(NULL);
		LeapInput::getInstance()->releaseGlobalGestureFocus(this);
		this->finishedCallback("");
	}
	else if (gestureType.compare("point") == 0)
	{
		this->scrollWheel->setVelocity(0);
	}
}

float ImageDetailView::getZValue()
{
	return 1000;
}

void ImageDetailView::getTutorialDescriptor(vector<string> & tutorial)
{
	tutorial.push_back("stretch");	
	tutorial.push_back("shake_inv");
}

void ImageDetailView::OnElementClicked(Pointable & pointable)
{
	this->setImagePanel(NULL);
	LeapInput::getInstance()->releaseGlobalGestureFocus(this);
	this->finishedCallback("");
}

bool ImageDetailView::isClickable()
{
	return (this->imagePanel != NULL && canClickToExit);
}

void ImageDetailView::restorePanelState(DynamicImagePanel * _imagePanel)
{
	if (_imagePanel != NULL)
	{
		_imagePanel->NudgeAnimationEnabled = true;
		_imagePanel->setVisible(true);
		_imagePanel->setClickable(true);
		_imagePanel->setDataPriority(0);
		_imagePanel->setMaxResolutionMode(false);
		_imagePanel->setTextureWindow(Vector(),Vector(1,1,1));
		remove(_imagePanel);
	}
}

void ImageDetailView::setPanelState(DynamicImagePanel * _imagePanel)
{
	_imagePanel->setDataPriority(0);
	_imagePanel->NudgeAnimationEnabled = false;
	_imagePanel->setVisible(false);
	_imagePanel->setClickable(false);
	_imagePanel->setMaxResolutionMode(true);
}


void ImageDetailView::setImagePanel(DynamicImagePanel * _imagePanel)
{
	activePanelInteraction.interactingPointables.clear();
	activePanelInteraction.pointableCentroid = Vector(0,0,0);
	activePanelInteraction.pointableRange = 0;
	activePanelInteraction.panel = NULL;

	restorePanelState(this->imagePanel);

	this->imagePanel = _imagePanel;

	if (this->imagePanel != NULL)
	{
		addChild(imagePanel);
		setPanelState(imagePanel);
		for (int i=-1;i <= -1; i++)
		{
			if (i==0) continue;
			DynamicImagePanel * siblingPanel = getSiblingByOffset(i);
			if (siblingPanel != NULL)
				setPanelState(siblingPanel);
		}
	}

	layoutDirty = true;
}


void ImageDetailView::setFinishedCallback(const boost::function<void(std::string)> & callback)
{
	finishedCallback = callback;
}

void ImageDetailView::layout(Vector position, cv::Size2f size)
{
	Vector center = Vector(size.width*.5f,size.height*.5f,5) - hostOffset;

	if (imagePanel != NULL && size.width > 0 && size.height > 0)
	{
		lastSize = size;
		lastPosition = position;

		imagePanel->fitPanelToBoundary(center,size.width,size.height*.8f, false);	

		for (int i=-1;i <= -1; i++)
		{
			if (i==0) continue;
			DynamicImagePanel * siblingPanel = getSiblingByOffset(i);
			if (siblingPanel != NULL)
				siblingPanel->fitPanelToBoundary(center+Vector(((float)i)*size.width*.8f,0,0),size.width,size.height*.8f, false);	
		}

		layoutDirty = false;
	}
}

void ImageDetailView::setVisible(bool _visible)
{
	if (_visible)
	{
		GraphicsContext::getInstance().requestExclusiveClarity(this);
	}
	else
	{
		GraphicsContext::getInstance().releaseExclusiveClarity(this);
	}

	View::setVisible(_visible);

	if (imagePanel != NULL)
		imagePanel->setMaxResolutionMode(isVisible());	

	layoutDirty = true;

	LeapInput::getInstance()->enableNonDominantCursor(_visible);
}


LeapElement * ImageDetailView::elementAtPoint(int x, int y, int & elementStateFlags)
{
	LeapElement * hit = NULL;
	if (this->imagePanel != NULL)
		hit = imagePanel->elementAtPoint(x-hostOffset.x,y-hostOffset.y,elementStateFlags);
	if (hit == NULL)
		hit = ViewGroup::elementAtPoint(x-hostOffset.x,y-hostOffset.y,elementStateFlags);
	if (hit == NULL)
		return this;
}

cv::Rect_<int> ImageDetailView::getHitRect()
{
	return cv::Rect_<int>(lastPosition.x-hostOffset.x,lastPosition.y-hostOffset.y,lastSize.width,lastSize.height);
}

void ImageDetailView::onFrame(const Controller & controller)
{	
	handleImageManipulation(controller);	

	canClickToExit = controller.frame().hands().count() < 2;
}

void ImageDetailView::draw()
{
	if (!GraphicsContext::getInstance().IsBlurCurrentPass)
	{
		float drawOffset = hostOffset.x+scrollWheel->getPosition();
		glTranslatef(drawOffset,0,0);
		ViewGroup::draw();
		if (this->imagePanel != NULL)
			this->imagePanel->draw();

		for (int i=-1;i <= -1; i++)
		{
			if (i==0) continue;
			DynamicImagePanel * siblingPanel = getSiblingByOffset(i);
			if (siblingPanel != NULL)
				siblingPanel->draw();
		}
		
		glTranslatef(-drawOffset,0,0);
	}
}

static bool isValidInteractionPointable(const Controller & controller, Pointable p)
{
	return p.isValid() && (p.touchDistance() < GlobalConfig::tree()->get<float>("ImageDetailView.ResizeInteraction.MaxTouchDistance"));
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

	if (frame.hands().count() >= 2)
	{
		Hand lh = frame.hands().leftmost();
		Hand rh = frame.hands().rightmost();

		int p1 = HandProcessor::LastModel(lh.id())->IntentFinger;
		int p2 = HandProcessor::LastModel(rh.id())->IntentFinger;

		interactionPointables.push_back(lh.pointable(p1));
		interactionPointables.push_back(rh.pointable(p2));
	}
	else
	{
		Hand h = frame.hand(HandProcessor::LastModel()->HandId);
		
		if (h.fingers().count() == 2)
		{			
			interactionPointables.push_back(h.fingers()[0]);
			interactionPointables.push_back(h.fingers()[1]);
		}		
	}

	if (interactionPointables.size() >= 2)
	{			
		bool canStartInteraction = true;
		for (int i=0;i<interactionPointables.size();i++)
		{
			bool iterationSuccess = false;
			float steadyVelocity = GlobalConfig::tree()->get<float>("ImageDetailView.ResizeInteraction.SteadyVelocity");
			if (isValidInteractionPointable(controller, interactionPointables[i]) && interactionPointables.at(i).tipVelocity().magnitude() < steadyVelocity)
			{						
				Vector imgPoint = LeapHelper::FindScreenPoint(controller,interactionPointables[i]);		
				activePanelInteraction.interactingPointables.push_back(make_pair(interactionPointables[i].id(),imgPoint));		
				iterationSuccess = true;		
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
			activePanelInteraction.panel = (DynamicImagePanel*)panel;
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
	static float cursorDimension = (((float)GlobalConfig::ScreenWidth) / 2560.0f) * 41;

	static LeapDebugVisual * ldv1 = NULL, * ldv2 = NULL;
	
	if (ldv1 == NULL)
	{
		ldv1 = new LeapDebugVisual(0,Colors::MediumVioletRed.withAlpha(.7f));
		ldv2 = new LeapDebugVisual(0,Colors::MediumVioletRed.withAlpha(.7f));
	}

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
				
//				if (activePanelInteraction.interactingPointables.size() > 2)
//				{			
//					Vector newImageCentroid, newCentroid;
//
//					for (int i=0;i<size;i++)
//					{
//						newImageCentroid += newImagePoints.at(i);
//						newCentroid += newPoints.at(i);
//					}
//
//					newCentroid /= (float)size;
//					newImageCentroid /= (float)size;
//											
//					Vector oldCentroid = activePanelInteraction.pointableCentroid;
//					if (oldCentroid.x != 0)
//					{
//						//LeapDebug::getInstance().addDebugVisual(new LeapDebugVisual(newImageCentroid,1,LeapDebugVisual::LiveByTime,35,Colors::LightBlue));
//
//						newImageCentroid.x = LeapHelper::lowpass(oldCentroid.x,newImageCentroid.x,40,timer.millis());
//						newImageCentroid.y = LeapHelper::lowpass(oldCentroid.y,newImageCentroid.y,40,timer.millis());
//					
//						//LeapDebug::getInstance().addDebugVisual(new LeapDebugVisual(newImageCentroid,1,LeapDebugVisual::LiveByTime,40,Colors::Blue));
//
//						activePanelInteraction.translation += (newImageCentroid - oldCentroid);
//					}
//					activePanelInteraction.pointableCentroid = newImageCentroid;
//				}


				if (activePanelInteraction.interactingPointables.size() == 2)
				{
					Pointable p1 = controller.frame().pointable(activePanelInteraction.interactingPointables.at(0).first);
					Pointable p2 = controller.frame().pointable(activePanelInteraction.interactingPointables.at(1).first);

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
			ldv1->size = 0;
			ldv2->size = 0;
			processed = getNewPanelInteraction(controller,frame,panel,activePanelInteraction,hostOffset);
		}
		else
		{
			cv::Size2f size = lastSize;
			activePanelInteraction.panel->setTextureWindow(Vector(),Vector());

			Vector pos;
			float w1,h1;
			activePanelInteraction.panel->getBoundingArea(pos,w1,h1);

			float sc1 = activePanelInteraction.scale.x;
			
			float likeButtonWidth = (size.height*.2f) + 50;

			if (GlobalConfig::tree()->get<bool>("ImageDetailView.ResizeInteraction.LimitToScreen"))
			{
				float maxHeight = min<float>(activePanelInteraction.panel->getHeight()*sc1,size.height);
				float maxWidth = min<float>(activePanelInteraction.panel->getWidth()*sc1,size.width-(2*likeButtonWidth));
				sc1 = min<float>(maxHeight/activePanelInteraction.panel->getHeight(),maxWidth/activePanelInteraction.panel->getWidth());
			}

			activePanelInteraction.scale.x= sc1;

			activePanelInteraction.panel->setTextureWindow((Vector(size.width*.5f,size.height*.5f,10)-pos)-Vector(w1*sc1*.5f,h1*sc1*.5f,0)-hostOffset,activePanelInteraction.scale);
			
			
			activePanelInteraction.panel->getBoundingArea(pos,w1,h1);
			//photoComment->setPosition(-hostOffset + Vector(size.width*.3f,pos.y + h1,10));
			float buttonPadding = 50;
			cv::Size2f buttonSize = cv::Size2f(size.height*.2f,size.height*.15f);
			Vector buttonPos = Vector(w1 + pos.x + (buttonPadding),pos.y + h1*.5f,10);

			//likeButton->setPosition(buttonPos);
			//alreadyLikedButton->setPosition(buttonPos);
		}
	}
	return processed;
}

