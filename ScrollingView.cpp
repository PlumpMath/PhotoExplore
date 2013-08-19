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

void ScrollingView::onFrame(const Controller & controller)
{

}
