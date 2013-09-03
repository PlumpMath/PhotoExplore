#include "KnobVisual.hpp"

void KnobVisual::draw()
{
	
	static float offset = -Leap::PI;
	static float lineWidth = 2;
	
	float angularWidth = (2.0f*Leap::PI) - (maxAngle-minAngle);
	float startAngle = offset + (Leap::PI-angularWidth)*0.5f;
	float endAngle = startAngle + angularWidth;
	
	float innerRad =config.get<float>("InnerDrawRadius");
	float outerRad =config.get<float>("OuterDrawRadius");
	
	float knobAngle = 0;
	
	for (auto it = orderedFingers.begin(); it != orderedFingers.end(); it++)
	{
		if (fingerErrorMap.count(it->first.id()))
		{
			float errorVal = fingerErrorMap[it->first.id()];
			drawData.push_back(make_pair(0.0f,errorVal));
		}
	}
	
	knobAngle = flyWheel->getPosition()/1000.0;
	
	knobAngle = max<float>(knobAngle,minAngle);
	knobAngle = min<float>(knobAngle,maxAngle);
	

	
	Color drawWheelColor = circleColor.blendRGB(Colors::Red,nonGraspedRatio);
	
	DrawingUtil::drawCircleFill(drawPoint,circleColor.withAlpha(fillAlpha),innerRad,outerRad,startAngle+knobAngle,endAngle+knobAngle);
	DrawingUtil::drawCircleFill(drawPoint+Vector(0,0,-1.0f),circleColor.withAlpha(0.1f),innerRad,outerRad,startAngle+minAngle,endAngle+maxAngle);
	DrawingUtil::drawCircleLine(drawPoint,circleColor,lineWidth,outerRad,0,Leap::PI*2.0f);
	DrawingUtil::drawCircleLine(drawPoint,drawWheelColor,lineWidth+(6.0f*nonGraspedRatio),innerRad,0,Leap::PI*2.0f);
	
	if (!grasped && !drawData.empty())
		drawFingerOffsets(drawPoint+Vector(0,0,0.5f),circleColor.withAlpha(0.6f),outerRad,drawData);
	
}
