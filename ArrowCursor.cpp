#include "LeapCursor.hpp"


ArrowCursor::ArrowCursor(float size, Color fillColor) : PointableCursor((double)size,fillColor)
{
	cursorDirection = 0;
	squareFactor = 0;
	
	filledTouchDist = 0.0f;
	triangleTouchDist = 0.5f;
	squareTouchDist = 0.8f;
}

void ArrowCursor::setDirection(float _cursorDirection)
{
	this->cursorDirection = _cursorDirection;
}

float ArrowCursor::getDirection()
{
	return this->cursorDirection;
}

void ArrowCursor::onFrame(const Controller & controller)
{
	PointableCursor::onFrame(controller);

	Pointable p1 = controller.frame().pointable(trackPointableId);

	if (p1.isValid())
	{
		squareFactor = p1.touchDistance() - triangleTouchDist;
		squareFactor /= (squareTouchDist - triangleTouchDist);
		squareFactor = max<float>(0,squareFactor);
		squareFactor = min<float>(1,squareFactor);

		float alpha = p1.touchDistance() - filledTouchDist;
		alpha /= (squareTouchDist - filledTouchDist);
		alpha = max<float>(0,alpha);
		alpha = min<float>(1,alpha);

		setFillAlpha(1.0f - alpha);
	}
}

void ArrowCursor::draw()
{
	float drawWidth,drawHeight,x1,x2,y1,y2, z1;

	z1 = depth + 60;		
	drawHeight = drawWidth = size;	

	if (drawHeight == 0 || (drawPoint.x == 0 && drawPoint.y == 0))
		return;

	static float vertices = 4;
	static float cornerAngle = 0;// GeomConstants::DegToRad*GlobalConfig::tree()->get<float>("Overlay.CornerAngle");
	//static float angleOffset = GeomConstants::DegToRad*GlobalConfig::tree()->get<float>("Overlay.AngleOffset");

	float length = drawHeight*.5f;
	float anglePerVertex = (Leap::PI*2.0f)/(vertices-1);
	
	float longLength = length*(1.5f - squareFactor/2.0f);
	float lengthArray [] = {length,longLength,longLength,length};
	float a = 1.0f - (squareFactor/4.0f);
	//float angles [] = {anglePerVertex*a,anglePerVertex*a*(1.5f - squareFactor/2.0f),(anglePerVertex*a)*(squareFactor),anglePerVertex*a*(1.5f - squareFactor/2.0f)};

	float tri = 1.0f - squareFactor;

	float angles [] = {Leap::PI*0.5f,Leap::PI*(0.5f + tri*0.25f),Leap::PI*(0.5f - tri/2.0f),Leap::PI*(0.5f + tri*.25f)};

	glColor4fv(fillColor.getFloat());
	glBindTexture( GL_TEXTURE_2D, NULL);
	glLineWidth(0);
	glTranslatef(drawPoint.x,drawPoint.y,0);
	glBegin(GL_POLYGON);
	float angle = cursorDirection;
	for (float v=0;v<vertices;v++)
	{
		angle += angles[(int)v];
		glVertex3f(sinf(angle)*lengthArray[(int)v],cosf(angle)*lengthArray[(int)v],z1);
	}
	glEnd();

	float alphaScale = lineColor.colorArray[3];

	float lineWidths [] = {lineWidth*3.0f,lineWidth*2.0f,lineWidth};
	float * lineColors [] = {lineColor.withAlpha(.2f*alphaScale).getFloat(),lineColor.withAlpha(.4f*alphaScale).getFloat(),lineColor.withAlpha(1.0f*alphaScale).getFloat()};




	for (int i=0; i < 3; i++)
	{
		glColor4fv(lineColors[i]);
		glLineWidth(lineWidths[i]);

		glBegin(GL_LINE_LOOP);
		angle = cursorDirection;
		for (float v=0;v<vertices;v++)
		{
			angle += angles[(int)v];
			glVertex3f(sinf(angle-cornerAngle)*lengthArray[(int)v],cosf(angle-cornerAngle)*lengthArray[(int)v],z1+ ((float)i * .1f));	
			//glVertex3f(sinf(angle+cornerAngle)*lengthArray[(int)v],cosf(angle+cornerAngle)*lengthArray[(int)v],z1+ ((float)i * .1f));		
		}
		glEnd();	
	}
	glTranslatef(-drawPoint.x,-drawPoint.y,0);
}

