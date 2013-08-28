#include "LeapCursor.hpp"
#include "LeapHelper.h"
#include "GraphicContext.hpp"


PointableCursor::PointableCursor(float _size, Color _baseColor)
{
	trackPointableId = -1;
	this->size = _size;
	this->depth= 10;
	this->lineWidth = 1;		
	setCursorColor(_baseColor);
	setFillAlpha(1);
	setBorderAlpha(1);
}


void PointableCursor::setBorderAlpha(float _borderAlpha)
{
	this->borderAlpha = _borderAlpha;
	lineColor = cursorBaseColor.withAlpha(cursorBaseColor.getFloat()[3] * borderAlpha);
}

void PointableCursor::setFillAlpha(float _fillAlpha)
{
	this->fillAlpha = _fillAlpha;
	fillColor = cursorBaseColor.withAlpha(cursorBaseColor.getFloat()[3] * fillAlpha);
}

void PointableCursor::setCursorColor(Color cursorColor)
{
	cursorBaseColor = cursorColor;
	lineColor = cursorBaseColor.withAlpha(cursorBaseColor.getFloat()[3] * borderAlpha);
	fillColor = cursorBaseColor.withAlpha(cursorBaseColor.getFloat()[3] * fillAlpha);
}

void PointableCursor::onFrame(const Controller & controller)
{	
	Pointable drawPointable = controller.frame().pointable(trackPointableId);

	if (drawPointable.isValid())
	{
		drawPoint = LeapHelper::FindScreenPoint(controller,drawPointable);
	}
	else 
		drawPoint = Vector(0,0,0);
}

Vector PointableCursor::getDrawPoint()
{
	return drawPoint;
}

void PointableCursor::update()
{
}

void PointableCursor::draw()
{
	float drawWidth,drawHeight,x1,x2,y1,y2, z1;

	z1 = depth + 60;		
	drawHeight = drawWidth = size;	

	if (drawHeight == 0 || (drawPoint.x == 0 && drawPoint.y == 0))
		return;

	static float vertices = GlobalConfig::tree()->get<float>("Overlay.VertexCount");
	static float cornerAngle = GeomConstants::DegToRad*GlobalConfig::tree()->get<float>("Overlay.CornerAngle");
	static float angleOffset = GeomConstants::DegToRad*GlobalConfig::tree()->get<float>("Overlay.AngleOffset");

	float length = drawHeight*.5f;
	float anglePerVertex = (Leap::PI*2.0f)/vertices;
	
	glColor4fv(fillColor.getFloat());
	glBindTexture( GL_TEXTURE_2D, NULL);
	glLineWidth(0);
	glTranslatef(drawPoint.x,drawPoint.y,0);
	glBegin(GL_POLYGON);
	for (float v=0;v<vertices;v++)
	{
		float angle = v*anglePerVertex;
		angle += angleOffset;
		glVertex3f(sinf(angle)*length,cosf(angle)*length,z1);
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
		for (float v=0;v<vertices;v++)
		{
			float angle = v*anglePerVertex;
			angle += angleOffset;
			glVertex3f(sinf(angle-cornerAngle)*length,cosf(angle-cornerAngle)*length,z1+ ((float)i * .1f));	
			glVertex3f(sinf(angle+cornerAngle)*length,cosf(angle+cornerAngle)*length,z1+ ((float)i * .1f));		
		}
		glEnd();	

		//glBegin(GL_QUAD_STRIP);
		//for (float v=0;v<(vertices+1);v++)
		//{
		//	float angle = v*anglePerVertex;
		//	if (v==vertices)
		//		angle = 0;
		//	angle += angleOffset;
		//	float b = lineWidth[i]*.5f;
		//	glVertex3f(sinf(angle-cornerAngle)*(length-b),cosf(angle-cornerAngle)*(length-b),z1+ ((float)i * .1f));
		//	glVertex3f(sinf(angle-cornerAngle)*(length+b),cosf(angle-cornerAngle)*(length+b),z1+ ((float)i * .1f));

		//	glVertex3f(sinf(angle+cornerAngle)*(length-b),cosf(angle+cornerAngle)*(length-b),z1+ ((float)i * .1f));	
		//	glVertex3f(sinf(angle+cornerAngle)*(length+b),cosf(angle+cornerAngle)*(length+b),z1+ ((float)i * .1f));		
		//}
		//glEnd();
	}
	glTranslatef(-drawPoint.x,-drawPoint.y,0);

}