#include "LeapCursor.hpp"


OpposingArrowCursor::OpposingArrowCursor(float size, Color color)
{
	ArrowCursor * c1 = new ArrowCursor(size,color);
	ArrowCursor * c2 = new ArrowCursor(size,color);

	c1->setBorderAlpha(0.9f);
	c2->setBorderAlpha(0.9f);

	c2->setFillAlpha(0);
	c1->setFillAlpha(0);

	c1->lineWidth = c2->lineWidth = GlobalConfig::tree()->get<float>("Cursors.InnerWidth");
		

	ArrowCursor * c3 = new ArrowCursor(size*0.6f,color);
	ArrowCursor * c4 = new ArrowCursor(size*0.6f,color);

	c3->setBorderAlpha(0.9f);
	c4->setBorderAlpha(0.9f);

	c3->setFillAlpha(0);
	c4->setFillAlpha(0);

	c3->lineWidth = c4->lineWidth = GlobalConfig::tree()->get<float>("Cursors.InnerWidth");
		
	
	arrows.push_back(c1);
	arrows.push_back(c2);

	arrows.push_back(c3);
	arrows.push_back(c4);
}

void OpposingArrowCursor::setPointables(Pointable p1, Pointable p2)
{
	arrows.at(0)->trackPointableId = p1.id();
	//arrows.at(2)->trackPointableId = p1.id();

	arrows.at(1)->trackPointableId = p2.id();
	//arrows.at(3)->trackPointableId = p2.id();
}

void OpposingArrowCursor::setSize(float size)
{
	int count = 0;
	for (auto it = arrows.begin(); it != arrows.end(); it++)
	{
		if (count++ < 1)
			(*it)->size = size;
		else
			(*it)->size = size*0.6f;
	}
}


void OpposingArrowCursor::onFrame(const Controller & controller)
{
	for (auto it = arrows.begin(); it != arrows.end(); it++)
	{
		(*it)->onFrame(controller);
	}

	Vector delta = arrows.at(0)->getDrawPoint() - arrows.at(1)->getDrawPoint();
	float angle = atan2(delta.x,delta.y) + Leap::PI*.5f;
	
	int count = 0;
	for (auto it = arrows.begin(); it != arrows.end(); it++)
	{
		float td1 = controller.frame().pointable((*it)->trackPointableId).touchDistance() - 0.8f;
		td1 /= 0.2f;
		td1 = 1.0f - td1;
	
		td1 = min<float>(1.0f,td1);
		td1 = max<float>(0.0f,td1);

		if (count++ % 3 == 0)
			(*it)->setDirection(angle*td1 + Leap::PI*.25f);
		else
			(*it)->setDirection(angle*td1 - Leap::PI*.75f);
	}
}

void OpposingArrowCursor::draw()
{
	for (auto it = arrows.begin(); it != arrows.end(); it++)
	{
		(*it)->draw();
	}
}