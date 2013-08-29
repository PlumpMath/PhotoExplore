#include "CircularAction.hpp"
#include "GlobalConfig.hpp"
#include "LeapHelper.h"
#include "HandModel.h"


CircularAction::CircularAction()
{
	state = Steadying;
	config = GlobalConfig::tree()->get_child("Leap.CircularAction");
	handId = -1;

	circleColor = config.get_child("Color");

	for (int i=0; i < 5; i++)
	{
		cursors.push_back(new PointableCursor(40,Colors::PrettyPurple));
	}

}


void CircularAction::draw()
{
	//if (mutey.try_lock())
	//{

	mutey.lock();
	for (auto it = cursors.begin(); it != cursors.end(); it++)
	{
		(*it)->draw();
	}

	float fillAlpha = 0.6f;
	if (state == Steadying)
		fillAlpha = 0.1f;

	float rad =config.get<float>("DrawRadius") + drawRadius;
	drawPolygon(drawPoint,circleColor.withAlpha(1.0f),circleColor.withAlpha(fillAlpha), rad,drawPitch);

	vector<pair<float,float> > drawData;

	for (auto it = fingerAngleMap.begin(); it != fingerAngleMap.end(); it++)
	{
		drawData.push_back(make_pair(it->second,fingerErrorMap[it->first]));
	}

	drawFingerOffsets(drawPoint,circleColor.withAlpha(1.0f),rad,drawData);

	mutey.unlock();
	//}
}

void CircularAction::setNewHand(Hand newHand)
{
	handId = newHand.id();
	state = Steadying;
	sphereRadiusTimer.start();
	drawRadius = 0;

}

float CircularAction::getHandFingerPitch(const Controller & controller, Finger f)
{
	Vector delta = getHandFingerDelta(controller,f);
	return atan2(delta.x,delta.y);
}

Vector CircularAction::getHandFingerDelta(const Controller & controller, Finger f)
{
	Vector p1 = LeapHelper::FindNormalizedScreenPoint(controller,f.stabilizedTipPosition());
	Vector p2 = LeapHelper::FindNormalizedScreenPoint(controller,f.hand().stabilizedPalmPosition());
	Vector delta = p1 - p2;
	return delta;
}

void CircularAction::updateCursors(const Controller & controller)
{	
	Frame frame = controller.frame();

	Hand hand = frame.hand(handId);

	if (hand.isValid())
	{
		for (int f = 0; f < hand.fingers().count() && f < 5; f++)
		{
			cursors.at(f)->trackPointableId = hand.fingers()[f].id();
		}
	}

	for (auto it = cursors.begin(); it != cursors.end(); it++)
	{
		(*it)->onFrame(controller);
	}
}


void CircularAction::updateErrorMap(const Controller & controller, Hand hand)
{	
	Frame frame = controller.frame();
	
	fingerErrorMap.clear();
	
	float count = 0;
	Vector centroid = hand.sphereCenter();
	//for (int f = 0; f < hand.fingers().count(); f++)
	//{
	//	Finger finger = hand.fingers()[f];		
	//	centroid += finger.stabilizedTipPosition(); //LeapHelper::FindNormalizedScreenPoint(controller,finger.stabilizedTipPosition());
	//	count++;
	//}
	//centroid /= count;
	
	averageRadius = hand.sphereRadius();
	//count = 0;
	//for (int f = 0; f < hand.fingers().count(); f++)
	//{
	//	Finger finger = hand.fingers()[f];
	//	Vector p =  LeapHelper::FindNormalizedScreenPoint(controller,finger.stabilizedTipPosition());
	//	averageRadius += centroid.distanceTo(p);
	//	count++;
	//}
	//averageRadius /= count;

	for (int f = 0; f < hand.fingers().count(); f++)
	{
		Finger finger = hand.fingers()[f];
		Vector p = finger.stabilizedTipPosition();//LeapHelper::FindNormalizedScreenPoint(controller,finger.stabilizedTipPosition());
		fingerErrorMap.insert(make_pair(finger.id(),p.distanceTo(centroid) - averageRadius));		
	}
}

void CircularAction::updateRotateMap(const Controller & controller, Hand hand)
{	
	Frame frame = controller.frame();

	for (auto it = fingerAngleMap.begin(); it != fingerAngleMap.end(); it++)
	{
		Finger f = hand.finger(it->first);
		if (!f.isValid())
		{
			it = fingerAngleMap.erase(it);
		}
	}

	
	auto oldest = std::max_element(fingerAngleMap.begin(),fingerAngleMap.end(),[hand](const pair<int,float> & p0, const pair<int,float> & p1) -> bool {
		
		Finger f0 = hand.finger(p0.first);
		Finger f1 = hand.finger(p1.first);

		return f0.timeVisible() < f1.timeVisible();
	});

	if (oldest != fingerAngleMap.end())
	{
		int oldestId = oldest->first;
		Finger oldFinger = hand.finger(oldest->first);
		drawPitch = getHandFingerPitch(controller, oldFinger) - oldest->second;

		for (auto it = fingerAngleMap.begin(); it != fingerAngleMap.end(); it++)
		{
			Finger f = hand.finger(it->first);
			if (f.id() != oldestId)
			{
				it = fingerAngleMap.erase(it);
			}
		}
	}
		
	for (int f = 0; f < hand.fingers().count(); f++)
	{
		Finger finger = hand.fingers()[f];
		
		if (fingerAngleMap.count(finger.id()) == 0)
		{
			fingerAngleMap.insert(make_pair(finger.id(),getHandFingerPitch(controller,finger)));
		}
	}

}


void CircularAction::onFrame(const Controller & controller)
{
	if (mutey.try_lock())
	{

		Frame frame = controller.frame();

		Hand hand = frame.hand(handId);
	
		if (!hand.isValid())
		{
			hand = frame.hands().rightmost();
			if (hand.isValid())
			{
				setNewHand(hand);	
			}
		}
		
		float newDrawRadius = hand.sphereRadius() - config.get<float>("SphereRadiusThreshold");

		drawRadius = LeapHelper::lowpass(drawRadius,newDrawRadius,config.get<float>("SphereRadiusRC"),sphereRadiusTimer.seconds());

		drawRadius = max<float>(-config.get<float>("SphereRadiusMaxOffset"),drawRadius);
		drawRadius = min<float>(config.get<float>("SphereRadiusMaxOffset"),drawRadius);

		updateCursors(controller);

		switch (state)
		{
		case Steadying:
			if (hand.isValid() && hand.palmVelocity().magnitude() < config.get<float>("SteadyVelocity"))
			{
				state = Rotating;
				fingerAngleMap.clear();
				updateRotateMap(controller,hand);
			}
			else
				break;
		case Rotating:
			if (hand.isValid())
			{
				drawPoint = LeapHelper::FindScreenPoint(controller,hand.stabilizedPalmPosition());
				updateRotateMap(controller,hand);
			}
			else
				state = Steadying;
			break;
		default:
			break;
		}
	
		updateErrorMap(controller,hand);

		mutey.unlock();
	}
}


void CircularAction::drawFingerOffsets(Vector center, Color lineColor,float radius, vector<pair<float, float> > & offsets)
{
	float z1 = center.z;
	float angleOffset = 0;

	glBindTexture( GL_TEXTURE_2D, NULL);
	glColor4fv(lineColor.getFloat());
	glLineWidth(8);
	glTranslatef(drawPoint.x,drawPoint.y,0);
	glBegin(GL_LINES);
	for (auto it = offsets.begin(); it != offsets.end(); it++)
	{
		float angle = it->first;
		float length = radius + it->second;
		angle += angleOffset;

		glVertex3f(sinf(angle)*radius,cosf(angle)*radius,z1);
		glVertex3f(sinf(angle)*length,cosf(angle)*length,z1);
	}
	glEnd();
	glTranslatef(-drawPoint.x,-drawPoint.y,0);

}

void CircularAction::drawPolygon(Vector drawPoint, Color lineColor, Color fillColor, float radius, float totalAngle)
{
	float drawWidth,drawHeight,z1;

	float lineWidth = 2.0f;

	z1 = drawPoint.z;

	if (drawHeight == 0 || (drawPoint.x == 0 && drawPoint.y == 0))
		return;

	float vertices = 20;

	float anglePerVertex;

	
	anglePerVertex = (totalAngle)/vertices;


	float angleOffset = 0; 

	glColor4fv(fillColor.getFloat());
	glBindTexture( GL_TEXTURE_2D, NULL);
	glLineWidth(0);
	glTranslatef(drawPoint.x,drawPoint.y,0);
	glBegin(GL_POLYGON);
	glVertex3f(0,0,z1);	
	for (float v=0;v<vertices;v++)
	{
		float angle = v*anglePerVertex;
		angle += angleOffset;
		glVertex3f(cosf(angle)*radius,sinf(angle)*radius,z1);
	}
	glEnd();


	anglePerVertex = (Leap::PI*2.0f)/vertices;

	float alphaScale = lineColor.colorArray[3];

	float lineWidths [] = {lineWidth*3.0f,lineWidth*2.0f,lineWidth};
	
	float * lineColors [] = {
		lineColor.withAlpha(.2f*alphaScale).getFloat(),
		lineColor.withAlpha(.4f*alphaScale).getFloat(),
		lineColor.withAlpha(1.0f*alphaScale).getFloat()
	};

	for (int i=0; i < 3; i++)
	{
		glColor4fv(lineColors[i]);
		glLineWidth(lineWidths[i]);

		glBegin(GL_LINE_LOOP);
		
		for (float v=0;v<vertices;v++)
		{
			float angle = v*anglePerVertex;
			angle += angleOffset;
			glVertex3f(cosf(angle)*radius,sinf(angle)*radius,z1+ ((float)i * .1f));	
		}
		glEnd();	
	}
	glTranslatef(-drawPoint.x,-drawPoint.y,0);
}