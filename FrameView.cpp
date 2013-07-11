#include "FrameView.hpp"
#include "GLImport.h"

FrameView::FrameView()
{
	this->contentView = NULL;
}

FrameView::FrameView(View * _contentView)
{
	this->contentView = _contentView;
}

FrameView::FrameView(View * _contentView, Color _backgroundColor):
	contentView(_contentView),
	backgroundColor(_backgroundColor)
{

}

void FrameView::drawContent(Vector drawPosition, float drawWidth, float drawHeight)
{
	if (contentView != NULL && contentView->isVisible())
		contentView->draw();
}

void FrameView::setContentView(View * _contentView)
{
	this->contentView = _contentView;
}

View * FrameView::getContentView()
{
	return this->contentView;
}

void FrameView::measure(cv::Size2f & measuredSize)
{
	if (contentView != NULL)
		contentView->measure(measuredSize);
}

void FrameView::layout(Vector _layoutPosition, cv::Size2f _layoutSize)
{
	this->lastPosition = _layoutPosition;
	this->lastSize = _layoutSize;

	if (this->contentView != NULL && contentView->isVisible())
		contentView->layout(_layoutPosition,_layoutSize);
}

LeapElement * FrameView::elementAtPoint(int x, int y, int & elementStateFlags)
{
	if (contentView != NULL && contentView->isVisible() && contentView->isEnabled())
		return contentView->elementAtPoint(x,y,elementStateFlags);
	else
		return NULL;
}

void FrameView::update()
{
	if (contentView != NULL && contentView->isEnabled())
		contentView->update();
}

void FrameView::draw()
{
	glColor4fv(backgroundColor.getFloat());

	glBindTexture( GL_TEXTURE_2D, NULL);

	float x1 = lastPosition.x, x2 = x1 + lastSize.width;
	float y1 = lastPosition.y, y2 = y1 + lastSize.height;
	float z1 = lastPosition.z -1;
	glBegin( GL_QUADS );
		glVertex3f(x1,y1,z1);
		glVertex3f(x2,y1,z1);
		glVertex3f(x2,y2,z1);
		glVertex3f(x1,y2,z1);
	glEnd();

	if (contentView != NULL && contentView->isVisible())
		contentView->draw();
}

void FrameView::setBackgroundColor(Color & _backgroundColor)
{
	backgroundColor = _backgroundColor;
}

Color FrameView::getBackgroundColor()
{
	return backgroundColor;
}