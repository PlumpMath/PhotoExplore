#include "LeapCursor.hpp"


PointableTouchCursor::PointableTouchCursor(float _minSize, float _maxSize, Color _cursorColor) :
	minSize(_minSize),
	maxSize(_maxSize)
{
	c1 = new PointableCursor(minSize,_cursorColor);
	c2 = new PointableCursor(minSize,_cursorColor);

	c1->setFillAlpha(0);
	c1->setBorderAlpha(1);
	c1->lineWidth = GlobalConfig::tree()->get<float>("Cursors.InnerWidth");
	c1->depth = 10;

	c2->setFillAlpha(0);
	c2->setBorderAlpha(1);
	c2->lineWidth = GlobalConfig::tree()->get<float>("Cursors.OuterWidth");
	c2->depth = 10;
}

void PointableTouchCursor::setPointable(Pointable & p)
{
	c1->trackPointableId = p.id();
	c2->trackPointableId = p.id();
}

void PointableTouchCursor::setColor(Color _color)
{
	c1->setCursorColor(_color);
	c2->setCursorColor(_color);
}

void PointableTouchCursor::onFrame(const Controller & controller)
{
	c1->onFrame(controller);
	c2->onFrame(controller);

	Pointable p1 = controller.frame().pointable(c1->trackPointableId);
	if (p1.isValid())
	{
		c1->setFillAlpha(min<float>(1.0f,-2.0f * p1.touchDistance()));
		
		c1->size = minSize;
		c2->size = minSize + (maxSize - minSize)*(max<float>(touchDistanceThreshold,p1.touchDistance()));
	}
	else
	{
		c1->size = 0;
		c2->size = 0;
	}
}

void PointableTouchCursor::draw()
{
	c1->draw();
	c2->draw();
}