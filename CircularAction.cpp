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
		cursors.push_back(new PointableCursor(20,Colors::PrettyPurple));
	}

	flyWheel = new FlyWheel();
	flyWheel->setMaxValue(Leap::PI*1000.0);
	flyWheel->setMinValue(-Leap::PI*1000.0);
	flyWheel->setTargetActive(false);
	flyWheel->setTargetPosition(0);
}


void CircularAction::draw()
{
	//if (mutey.try_lock())
	//{

	mutey.lock();
	if (handId >= 0)
	{
		for (auto it = cursors.begin(); it != cursors.end(); it++)
		{
			(*it)->draw();
		}

		float fillAlpha = 0.6f;
		if (state == Steadying)
			fillAlpha = 0.1f;

		float innerRad =config.get<float>("InnerDrawRadius");
		float outerRad =config.get<float>("OuterDrawRadius");

		vector<pair<float,float> > drawData;
		
		for (auto it = drawFingerAngles.begin(); it != drawFingerAngles.end(); it++)
		{
			float errorVal = fingerErrorMap[it->first];
			drawData.push_back(make_pair(it->second,errorVal));
		}

		float drawAngle = flyWheel->getPosition()/1000.0;

		float startAngle = -Leap::PI + drawAngle;
		float endAngle = drawAngle;
		
		Color lineColor = (grasped) ? Colors::Lime : circleColor.withAlpha(1.0f);
		drawPolygon(drawPoint,lineColor,circleColor.withAlpha(fillAlpha),innerRad,outerRad,startAngle,endAngle);

		drawFingerOffsets(drawPoint+Vector(0,0,0.5f),circleColor.withAlpha(1.0f),outerRad,drawData);
	}
	mutey.unlock();
	//}
}

void CircularAction::setNewHand(Hand newHand)
{
	handId = newHand.id();
	state = Steadying;
	sphereRadiusTimer.start();
	drawRadius = 0;
	drawPitch = 0;
	grasped = false;

}

float CircularAction::getValue()
{
	return flyWheel->getCurrentPosition()/1000.0f;
}

bool CircularAction::isGrasped()
{
	return grasped;
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
	
	auto oldest = std::max_element(fingerAngleMap.begin(),fingerAngleMap.end(),[hand](const pair<int,float> & p0, const pair<int,float> & p1) -> bool {
		
		Finger f0 = hand.finger(p0.first);
		Finger f1 = hand.finger(p1.first);
		
		return f0.stabilizedTipPosition().y < f1.stabilizedTipPosition().y;
//		timeVisible() < f1.timeVisible();
	});

	int oldId = -1;
	if (oldest != fingerAngleMap.end())
		oldId = oldest->first;
	
	if (hand.isValid())
	{
		for (int f = 0; f < hand.fingers().count() && f < 5; f++)
		{
			int fingerId = hand.fingers()[f].id();
			if (fingerId == oldId)
				cursors.at(f)->setCursorColor(Colors::Red);
			else
				cursors.at(f)->setCursorColor(Colors::PrettyPurple);
				
			cursors.at(f)->trackPointableId = fingerId;
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
	
	Vector centroid = hand.stabilizedPalmPosition();// LeapHelper::FindNormalizedScreenPoint(controller,hand.stabilizedPalmPosition()); //hand.sphereCenter();
	centroid.z = 0;
	averageRadius = config.get<float>("TargetSphereRadius"); 

	float upperBound = (grasped) ? config.get<float>("ExitRadiusTolerance") : config.get<float>("RadiusTolerance");
	float lowerBound = (grasped) ? config.get<float>("LowerExitRadiusTolerance") : config.get<float>("LowerRadiusTolerance");

	int minFingerCount = (grasped) ? config.get<int>("Grasped.MinFingerCount") : config.get<int>("MinFingerCount");
	int maxNonGraspedFingerCount = (grasped) ? config.get<int>("Grasped.MaxNonGraspedCount") : config.get<int>("MaxNonGraspedCount");

	//grasped = hand.fingers().count() >= minFingerCount;

	int graspCount = 0, nonGraspedCount = 0;
	
	for (int f = 0; f < hand.fingers().count(); f++)
	{
		Finger finger = hand.fingers()[f];
		Vector p = finger.stabilizedTipPosition(); //LeapHelper::FindNormalizedScreenPoint(controller,finger.stabilizedTipPosition());
		p.z = 0;
		
		float errorVal = p.distanceTo(centroid) - averageRadius;
		
		fingerErrorMap.insert(make_pair(finger.id(),errorVal));
		
		if (errorVal < upperBound && errorVal > lowerBound)
		{
			graspCount++;
		}
		else
		{
			nonGraspedCount++;
		}
	}

	grasped = graspCount >= minFingerCount && nonGraspedCount < maxNonGraspedFingerCount;
		
	if (grasped)
	{
		flyWheel->setTargetActive(false);
		//drawPitch = hand.rotationAngle(rotationStartFrame,Vector::backward()) + startPitch;
		flyWheel->overrideValue(drawPitch*1000.0);
	}
	else
	{
		flyWheel->setTargetActive(true);
		drawPitch = flyWheel->getCurrentPosition() / 1000.0f;
		flyWheel->setTargetPosition(0);
		rotationStartFrame = frame;
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
			
			if (it == fingerAngleMap.end())
				break;
		}
	}

	
	auto oldest = std::max_element(fingerAngleMap.begin(),fingerAngleMap.end(),[hand](const pair<int,float> & p0, const pair<int,float> & p1) -> bool {
		
		Finger f0 = hand.finger(p0.first);
		Finger f1 = hand.finger(p1.first);
		
		
		return f0.stabilizedTipPosition().y < f1.stabilizedTipPosition().y;
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
				if (it == fingerAngleMap.end())
					break;
			}
		}
	}
		
	drawFingerAngles.clear();
	for (int f = 0; f < hand.fingers().count(); f++)
	{
		Finger finger = hand.fingers()[f];
				
		if (fingerAngleMap.count(finger.id()) == 0)
		{
			auto adjustedVal = make_pair(finger.id(),getHandFingerPitch(controller,finger) - drawPitch);
			fingerAngleMap.insert(adjustedVal);
		}
		auto val = make_pair(finger.id(),getHandFingerPitch(controller,finger));
		drawFingerAngles.insert(val);
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
			else
				handId = -1;
		}
		
		
		if (hand.isValid())
		{	
			updateErrorMap(controller,hand);
			updateRotateMap(controller,hand);
			
			float newDrawRadius = hand.sphereRadius() - config.get<float>("SphereRadiusThreshold");
			drawPoint = LeapHelper::FindScreenPoint(controller,hand.stabilizedPalmPosition()) + Vector(0,0,50);

			drawRadius = LeapHelper::lowpass(drawRadius,newDrawRadius,config.get<float>("SphereRadiusRC"),sphereRadiusTimer.seconds());

			drawRadius = max<float>(-config.get<float>("SphereRadiusMaxOffset"),drawRadius);
			drawRadius = min<float>(config.get<float>("SphereRadiusMaxOffset"),drawRadius);
			
			updateCursors(controller);

			switch (state)
			{
			case Steadying:
				if (grasped) //hand.palmVelocity().magnitude() < config.get<float>("SteadyVelocity"))
				{
					state = Rotating;
					fingerAngleMap.clear();
					rotationStartFrame = frame;
				}
				else
				{					
					break;
				}
			case Rotating:
				if (!grasped)
				{
					//drawPitch = 0;
					fingerAngleMap.clear();
				}
				break;
			default:
				break;
			}
		}
		else
			state = Steadying;

		mutey.unlock();
	}
}


void CircularAction::drawFingerOffsets(Vector center, Color lineColor,float radius, vector<pair<float, float> > & offsets)
{
	//float z1 = center.z;
	float angleOffset = -Leap::PI/2.0f;

	
	float upperBound = (grasped) ? config.get<float>("ExitRadiusTolerance") : config.get<float>("RadiusTolerance");
	float lowerBound = (grasped) ? config.get<float>("LowerExitRadiusTolerance") : config.get<float>("LowerRadiusTolerance");

	float margin = 10;
	for (auto it = offsets.begin(); it != offsets.end(); it++)
	{
		float angle = it->first;
		float length = radius + it->second;
		float start = radius;
		
		angle += angleOffset;

		Color color = lineColor;
		if (it->second < upperBound && it->second > lowerBound)
		{
			length = max<float>(length,radius + margin);
			start = min<float>(start,radius - margin / 2.0f);
			color = Colors::Lime.withAlpha(0.8f);
		}
		drawPolygon(center,Colors::Transparent,color,start,length,angle-Leap::PI*0.1f,angle+Leap::PI*0.1f);
	}
}

void CircularAction::drawPolygon(Vector drawPoint, Color lineColor, Color fillColor, float innerRadius, float outerRadius, float startAngle, float endAngle)
{
	float lineWidth = 1.0f, z1 = drawPoint.z;
	if (outerRadius == 0)
		return;

	float angleRange = endAngle - startAngle;

	int vertices = ceilf(40 * angleRange/(Leap::PI*2.0f));

	float anglePerVertex = angleRange/(float)vertices;
	
	glColor4fv(fillColor.getFloat());
	glBindTexture( GL_TEXTURE_2D, NULL);
	glLineWidth(0);

	glTranslatef(drawPoint.x,drawPoint.y,0);
	glBegin(GL_QUAD_STRIP);
	for (float v=0;v<vertices;v++)
	{
		float angle = v*anglePerVertex;
		angle += startAngle;
		glVertex3f(cosf(angle)*innerRadius,sinf(angle)*innerRadius,z1);
		glVertex3f(cosf(angle)*outerRadius,sinf(angle)*outerRadius,z1);
	}
	glEnd();


	float alphaScale = lineColor.colorArray[3];

	if (alphaScale > 0)
	{		
		vertices = 40;
		anglePerVertex = (Leap::PI*2.0f)/(float)vertices;

		//float lineWidths [] = {lineWidth*3.0f,lineWidth*2.0f,lineWidth};

		float radii [] = {outerRadius - 2.5f,outerRadius, outerRadius + 2.5f};
	
		float * lineColors [] = {
			Colors::Red.withAlpha(.2f*alphaScale).getFloat(),
			lineColor.withAlpha(alphaScale).getFloat(),
			Colors::Red.withAlpha(.2f*alphaScale).getFloat(),
		};

		for (int lap = 0; lap < 2; lap++)
		{
			glBegin(GL_TRIANGLE_STRIP);		
			for (float v=0;v<vertices;v++)
			{
				float angle = v*anglePerVertex;
				angle += startAngle;

				for (int i=0; i < 2; i++)
				{
					float rad = radii[i+lap];
					glColor4fv(lineColors[i+lap]);
					glVertex3f(cosf(angle)*rad,sinf(angle)*rad,z1+ ((float)i * .1f));	
				}
			}
			glEnd();	
		}
	}
	glTranslatef(-drawPoint.x,-drawPoint.y,0);
}