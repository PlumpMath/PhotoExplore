#include "ScrollBar.hpp"
#include "GLImport.h"
#include "LeapHelper.h"
#include "GlobalConfig.hpp"

ScrollBar::ScrollBar()
{
	background = Colors::LightGray;
	foreground = Colors::DimGray;
	scrollingView = NULL;
	flyWheel = NULL;
}

void ScrollBar::setScrollView(ScrollingView * _scrollView)
{
	this->scrollingView = _scrollView;
	if (_scrollView != NULL)
	{
		this->flyWheel = _scrollView->getFlyWheel();
		scrollWidthFilterTimer.start();
		lastScrollRange = -1;
	}
}

void ScrollBar::draw()
{
	if (scrollingView == NULL || flyWheel == NULL)
		return;

	
	float filterRC = GlobalConfig::tree()->get<float>("ScrollView.ScrollBar.WidthFilterRC");

	float x1 = lastPosition.x; 
	float x2 = x1 + lastSize.width;
	float y1 = lastPosition.y ;
	float y2 = y1 + lastSize.height;
	float z1 = lastPosition.z;
			
	glColor4fv(background.getFloat());
	
	glLineWidth(0);		
	glBindTexture( GL_TEXTURE_2D, NULL);

	glBegin( GL_QUADS );
		glVertex3f(x1,y1,z1);
		glVertex3f(x2,y1,z1);
		glVertex3f(x2,y2,z1);
		glVertex3f(x1,y2,z1);
	glEnd();

	float scrollRange = flyWheel->getMaxValue() - flyWheel->getMinValue();
	scrollRange += scrollingView->getMeasuredSize().width;	

	if (lastScrollRange < 0)
		lastScrollRange = scrollRange;
	else
		lastScrollRange = LeapHelper::lowpass(lastScrollRange,scrollRange,filterRC,scrollWidthFilterTimer.seconds());
		
	float scrollOffset = -flyWheel->getCurrentPosition() / lastScrollRange;
	float scrollWidth = scrollingView->getMeasuredSize().width / lastScrollRange;

	scrollOffset *= lastSize.width;
	scrollWidth *= lastSize.width;

	scrollWidthFilterTimer.start();

	x1 = lastPosition.x + scrollOffset; 
	x2 = x1 + scrollWidth;

	z1 = z1 + 1.0f;
			
	glColor4fv(foreground.getFloat());
	
	glBegin( GL_QUADS );
		glVertex3f(x1,y1,z1);
		glVertex3f(x2,y1,z1);
		glVertex3f(x2,y2,z1);
		glVertex3f(x1,y2,z1);
	glEnd();

}

void ScrollBar::layout(Vector position, cv::Size2f size)
{
	this->lastSize = size;
	this->lastPosition = position;
}