#include "LeapDebug.h"

void LeapDebugVisual::draw()
{
	drawPointer(this);
}

void LeapDebugVisual::drawPointer(LeapDebugVisual * debugVisual)
{
	float drawWidth,drawHeight,x1,x2,y1,y2, z1;

	z1 = debugVisual->depth + 60;
		
	drawHeight = drawWidth = debugVisual->size;	

	if (drawHeight == 0 || (debugVisual->screenPoint.x == 0 && debugVisual->screenPoint.y == 0))
		return;

	static float vertices = GlobalConfig::tree()->get<float>("Overlay.VertexCount");
	static float cornerAngle = GeomConstants::DegToRad*GlobalConfig::tree()->get<float>("Overlay.CornerAngle");
	static float angleOffset = GeomConstants::DegToRad*GlobalConfig::tree()->get<float>("Overlay.AngleOffset");

	float length = drawHeight*.5f;
	float anglePerVertex = (Leap::PI*2.0f)/vertices;


	glColor4fv(debugVisual->fillColor.getFloat());
	glBindTexture( GL_TEXTURE_2D, NULL);
	glLineWidth(0);
	glTranslatef(debugVisual->screenPoint.x,debugVisual->screenPoint.y,0);
	glBegin(GL_POLYGON);
	for (float v=0;v<vertices;v++)
	{
		float angle = v*anglePerVertex;
		angle += angleOffset;
		glVertex3f(sinf(angle)*length,cosf(angle)*length,z1);
	}
	glEnd();

	float alphaScale = debugVisual->lineColor.colorArray[3];

	float lineWidth [] = {debugVisual->lineWidth*3.0f,debugVisual->lineWidth*2.0f,debugVisual->lineWidth};
	float * lineColor [] = {debugVisual->lineColor.withAlpha(.2f*alphaScale).getFloat(),debugVisual->lineColor.withAlpha(.4f*alphaScale).getFloat(),debugVisual->lineColor.withAlpha(1.0f*alphaScale).getFloat()};

	for (int i=0; i < 3; i++)
	{
		glColor4fv(lineColor[i]);
		glLineWidth(lineWidth[i]);

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
	glTranslatef(-debugVisual->screenPoint.x,-debugVisual->screenPoint.y,0);

}