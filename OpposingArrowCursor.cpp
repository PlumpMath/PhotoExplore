#include "LeapCursor.hpp"


OpposingArrowCursor::OpposingArrowCursor(float size, Color color)
{
	c1 = new ArrowCursor(size,color.withAlpha(0.6f));
	c1->lineColor = color;

	c2 = new ArrowCursor(size,color.withAlpha(0.6f));
	c2->lineColor = color;
}

void OpposingArrowCursor::setPointables(Pointable p1, Pointable p2)
{
	c1->trackPointableId = p1.id();
	c2->trackPointableId = p2.id();
}

void OpposingArrowCursor::setSize(float size)
{
	c1->size = size;
	c2->size = size;
}


void OpposingArrowCursor::onFrame(const Controller & controller)
{
	c1->onFrame(controller);
	c2->onFrame(controller);

	Vector delta = c1->getDrawPoint() - c2->getDrawPoint();
	float angle = atan2(delta.x,delta.y);
	angle -= Leap::PI*.25f;

	c1->setDirection(angle + Leap::PI);
	c2->setDirection(angle);
}

void OpposingArrowCursor::draw()
{
	c1->draw();
	c2->draw();
}