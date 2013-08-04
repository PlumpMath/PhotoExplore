#include "ScrollingView.hpp"
#include "GraphicContext.hpp"
#include "HandModel.h"



ScrollingView::ScrollingView(View * contentView, ScrollOrientation _scrollOrientation)
{
	this->content = contentView;
	this->scrollingFlywheel= new FlyWheel();
	this->scrollOrientation = _scrollOrientation;
	drawNegativeLoad = drawPositiveLoad = false;
	hasGestureFocus = false;
	loadIndicatorMode = 0;
}

FlyWheel * ScrollingView::getFlyWheel()
{
	return scrollingFlywheel;
}

void ScrollingView::setContent(View * contentView)
{
	this->content = contentView;
}

View * ScrollingView::getContent()
{
	return content;
}

void ScrollingView::setOrientation(ScrollOrientation _scrollOrientation)
{
	this->scrollOrientation = _scrollOrientation;
}

ScrollOrientation ScrollingView::getOrientation()
{
	return scrollOrientation;
}

void ScrollingView::measure(cv::Size2f & measuredSize)
{
	measuredSize = desiredSize;
}

void ScrollingView::layout(Vector position, cv::Size2f size)
{
	static float loadingBarThickness = GlobalConfig::tree()->get<float>("ScrollView.LoadingBarThickness");
	lastSize = size;
	lastPosition = position;

	scrollingFlywheel->setMaxValue(0);
	if (scrollOrientation == Horizontal)
	{		
		cv::Size2f contentSize(-1,size.height);
		content->measure(contentSize);

		double limitOffset = (loadIndicatorMode != 0) ? loadingBarThickness : 0;
		scrollingFlywheel->setMinValue(size.width-(contentSize.width+limitOffset));
		contentSize.height = size.height;
		
		lastContentSize = contentSize;

		content->layout(position,contentSize);
	}
	else
	{		
		cv::Size2f contentSize(size.width,-1);
		content->measure(contentSize);

		scrollingFlywheel->setMinValue(size.height-contentSize.height);
		contentSize.width = size.width;

		
		lastContentSize = contentSize;

		content->layout(position,contentSize);
	}
}

void ScrollingView::update()
{
	content->update();
}

void ScrollingView::draw()
{
	static float loadingBarThickness = GlobalConfig::tree()->get<float>("ScrollView.LoadingBarThickness");
	static bool noSubpixelScrolling = GlobalConfig::tree()->get<bool>("GraphicsSettings.NoSubPixelScroll");

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();	

	if (scrollOrientation == Horizontal)
	{
		float offset = (float)scrollingFlywheel->getPosition();
		if (noSubpixelScrolling)
		{
			offset = floor(offset);
		}
		glTranslatef(offset,0,0);
	}
	else
		glTranslatef(0,(float)scrollingFlywheel->getPosition(),0);
	
	glMatrixMode(GL_MODELVIEW);

	drawLoadIndicator(lastContentSize.width,loadingBarThickness);

	GraphicsContext::getInstance().setDrawHint("Offset",scrollingFlywheel->getPosition());
	GraphicsContext::getInstance().setDrawHint("VisibleWidth",lastSize.width);
	content->draw();		
	GraphicsContext::getInstance().clearDrawHint("Offset");
	GraphicsContext::getInstance().clearDrawHint("VisibleWidth");
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

LeapElement * ScrollingView::elementAtPoint(int x, int y, int & elementStateFlags)
{
	static float maxClickSpeed = GlobalConfig::tree()->get<float>("ScrollView.MaximumClickSpeed");

	GraphicsContext::getInstance().setDrawHint("Offset",scrollingFlywheel->getPosition());
	GraphicsContext::getInstance().setDrawHint("VisibleWidth",lastSize.width);
	LeapElement * hit = NULL;
	if (content != NULL)
	{		
		if (scrollOrientation == Horizontal)
		{			

			hit = content->elementAtPoint(x-scrollingFlywheel->getPosition(),y,elementStateFlags);
			if (abs(scrollingFlywheel->getVelocity()) > maxClickSpeed)
				elementStateFlags |= LeapElement::Flags_ElementNonClickable;
		}
		else
			hit = content->elementAtPoint(x, y-scrollingFlywheel->getPosition(),elementStateFlags);
	}
	
	GraphicsContext::getInstance().clearDrawHint("Offset");
	GraphicsContext::getInstance().clearDrawHint("VisibleWidth");
	return hit;
}

void ScrollingView::drawLoadIndicator(float offset, float thickness)
{

	//

	if (loadIndicatorMode >= 1)
	{
		glBindTexture( GL_TEXTURE_2D, NULL);		

		float beatsPerMinute = 30;
		float secondsPerBeat = 60.0f/beatsPerMinute;
		float maxAlpha = .5f;

		float x = fmodf(loadAnimTimer.seconds(),secondsPerBeat);
		x /= secondsPerBeat;

		float bgAlpha = 0;

		float peak = .6f;

		if (x < peak)
			bgAlpha = sqrtf(x)/(sqrtf(peak));
		else 
			bgAlpha = 1.0f - pow(x-peak,2)/(pow(1.0f-peak,2));
		

		float drawWidth = thickness;
		float drawHeight = lastSize.height;

		glBindTexture( GL_TEXTURE_2D, NULL);	
		glColor4fv(loadIndicatorColor.withAlpha(bgAlpha).getFloat());

		float z1 = lastPosition.z + 1;

		glBegin( GL_QUADS );
		glVertex3f(offset,lastPosition.y,z1);
		glVertex3f(offset+thickness,lastPosition.y,z1);
		glVertex3f(offset+thickness,drawHeight+lastPosition.y,z1);
		glVertex3f(offset,drawHeight+lastPosition.y,z1);
		glEnd();

		//if (loadIndicatorMode == 2)
		//{
		//	glColor4fv(loadIndicatorColor.withAlpha(.9f).getFloat());
		//	for (int i = 0;i<3;i++)
		//	{		
		//		float angle = loadAnimTimer.seconds()/4.0 + ((float)(i*i)/10);

		//		float x1,x2,x3,x4;
		//		float angle2 = fmod(angle,2);


		//		x2 = x1 = ((angle2*.5f) * drawWidth) + offset;

		//		float lineWidth =  1.0f; //10.0f * (.5f + sin(angle*GeomConstants::PI_F));

		//		x4 = x3 = min<float>(x1 + lineWidth,offset+drawWidth);

		//		float y1,y2,y3,y4;

		//		y4 = y1 = lastPosition.y;
		//		y3 = y2 = lastPosition.y + drawHeight;


		//		glBegin( GL_QUADS );

		//		glVertex3f(x1,y1,z1);
		//		glVertex3f(x2,y2,z1);
		//		glVertex3f(x3,y3,z1);
		//		glVertex3f(x4,y4,z1);

		//		glEnd();
		//	}
		//}
	}
}

void ScrollingView::setDrawLoadingIndicator(int _mode, Color _indicatorColor)
{
	loadIndicatorMode = _mode;
	loadIndicatorColor = _indicatorColor;
	loadAnimTimer.start();
}

void ScrollingView::notifyScrollChanged(float newScroll)
{
	if (!visibleRectChangedListener.empty())
	{
		Vector newPosition = lastPosition;
		if (scrollOrientation == Horizontal)
		{
			newPosition.x += newScroll;
		}
		else
		{
			newPosition.y += newScroll;
		}
		visibleRectChangedListener(newPosition,lastSize);
	}

}

void ScrollingView::onGlobalGesture(const Controller & controller, std::string gestureType)
{

}

bool ScrollingView::onLeapGesture(const Controller & controller, const Gesture & gesture)
{
	static Timer positiveCooldown, negativeCooldown;

	bool handled = false;
	if (gesture.type() == Gesture::Type::TYPE_SWIPE && ( gesture.state() == Gesture::State::STATE_START || gesture.state() == Gesture::State::STATE_UPDATE))
	{
		SwipeGesture swipe(gesture);

		HandModel * handModel = HandProcessor::LastModel();

		if (swipe.hands().count() > 0 && swipe.hands()[0].fingers().count() > 1)
		{
			int handSwipeCount=0;
			Hand swipingHand = swipe.hands()[0];
			for (int g = 0; g < controller.frame().gestures().count(); g++)
			{
				Gesture otherGesture = controller.frame().gestures()[g];
				if (otherGesture.type() == Gesture::Type::TYPE_SWIPE && ( otherGesture.state() == Gesture::State::STATE_START || otherGesture.state() == Gesture::State::STATE_UPDATE))
				{
					SwipeGesture otherSwipe(otherGesture);
					if (otherSwipe.hands().count() == 1 && otherSwipe.hands()[0].id() == swipingHand.id())
						handSwipeCount++;
				}
			}

			if (handSwipeCount > 0)
			{
				Vector positiveScroll, negativeScroll, cancelDir;
				if (scrollOrientation == Horizontal)
				{
					negativeScroll = Vector::left();
					positiveScroll = Vector::right();
					cancelDir = Vector::down();
				}
				else
				{
					negativeScroll = Vector::up();
					positiveScroll = Vector::down();
					cancelDir = Vector::right();
				}

				float scrollBy = GlobalConfig::tree()->get<float>("ScrollView.ScrollSpeedFactor") * (swipe.speed());
				scrollBy = max<float>(GlobalConfig::tree()->get<float>("ScrollView.MinScrollDistance"),scrollBy);
				scrollBy = min<float>(GlobalConfig::tree()->get<float>("ScrollView.MaxScrollDistance"),scrollBy);

				if (positiveCooldown.elapsed() && swipe.direction().angleTo(positiveScroll) < PI/4.0f)
				{
					scrollingFlywheel->setFriction(.5);
					notifyScrollChanged(scrollingFlywheel->getPosition() + scrollBy);
					scrollingFlywheel->spinTo(scrollingFlywheel->getPosition() + scrollBy);			
					//negativeCooldown.countdown(1000);
					handled = true;
				}
				else if (negativeCooldown.elapsed() && swipe.direction().angleTo(negativeScroll) < PI/4.0f)
				{
					//positiveCooldown.countdown(1000);
					scrollingFlywheel->setFriction(.5);
					notifyScrollChanged(scrollingFlywheel->getPosition() - scrollBy);
					scrollingFlywheel->spinTo(scrollingFlywheel->getPosition() - scrollBy);
					handled = true;
				}
				else if (GlobalConfig::tree()->get<bool>("ScrollView.AllowScrollCancelGesture") && swipe.direction().angleTo(cancelDir) < PI/4.0f)
				{
					//negativeCooldown.countdown(0);
					//positiveCooldown.countdown(0);
					scrollingFlywheel->impartVelocity(0);
					handled = true;
				}
			}
		}
	}
	return handled;
}

void ScrollingView::onFrame(const Controller & controller)
{

}
