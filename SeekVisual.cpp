#include "SeekVisual.hpp"


void SeekVisual::setHandRotationDetector(HandRotationGestureDetector * handRotationDetector)
{
	this->handRotationDetector = handRotationDetector;
}


void SeekVisual::drawRotationBar(Vector drawPosition, float drawWidth, float drawHeight)
{	
	float x1 = drawPosition.x - drawWidth/2.0f;
	float x2 = x1 + drawWidth;
	float y1 = drawPosition.y - drawHeight/2.0f;
	float y2 = y1 + drawHeight;
	float z1 = drawPosition.z;
	
	glBindTexture( GL_TEXTURE_2D, NULL);

	glPushMatrix();
	glLoadIdentity();

	
	Color bgEnd = Colors::Lime;
	Color bg = Colors::HoloBlueBright;

	if (handRotationDetector->getState() > 0)
	{
		glTranslatef(drawPosition.x,drawPosition.y,0);
		glRotatef(GeomConstants::RadToDeg*handRotationDetector->getRotation()*-1,0,0,1);
		glTranslatef(-drawPosition.x,-drawPosition.y,0);

		float mixRatio = abs(handRotationDetector->getRotation()/(Leap::PI/2));
		mixRatio = max<float>(0,mixRatio);
		mixRatio = min<float>(1,mixRatio);
	
		bg.scaleRGB(1.0f - mixRatio);
		bgEnd.scaleRGB(mixRatio);
		bg.addRGB(bgEnd);
	
		bg.setAlpha(.8);
	}
	else
	{
		bg = Colors::DarkRed;
		bg.setAlpha(.5);
	}
	
	glColor4fv(bg.getFloat());

	glBegin(GL_QUADS);
		glVertex3f(x1,y1,z1);
		glVertex3f(x2,y1,z1);
		glVertex3f(x2,y2,z1);
		glVertex3f(x1,y2,z1);
	glEnd();

	
	glPopMatrix();
}


void SeekVisual::drawTriangles(Vector drawPosition, float xOffset)
{

	float movementMagnitude = abs(xOffset);

	if (movementMagnitude > 0)
	{
		glPushMatrix();
		glLoadIdentity();
				
		Color bg = Colors::HoloBlueBright;
		
		float triangleSpacing = 50, triangleOffset = 100, movementDivisor = 50;
		float triangleCount = movementMagnitude / movementDivisor;
		
		for (int i =0;i < triangleCount;i++)
		{
			bg.setAlpha(min<float>(1,triangleCount-(float)i));
			glColor4fv(bg.getFloat());

			if (handRotationDetector->getState() == HandRotationGestureDetector::MovingState)
			{
				float x1,y1,x2,y2,y3;
				float z1 = drawPosition.z;

				y1 = drawPosition.y;
				y2 = y1 + 50;	
				y3 = y1 - 50;

				float offset = triangleOffset + (i * triangleSpacing);

				if (handRotationDetector->getOffset().x > 0)
				{
					x1 = drawPosition.x + offset;
					x2 = x1 - 20;
				}
				else
				{
					x1 = drawPosition.x - offset;
					x2 = x1 + 20;
				}

				glBegin(GL_QUADS);
				glVertex3f(x1,y1,z1);
				glVertex3f(x2,y2,z1);
				glVertex3f(x2,y1,z1);
				glVertex3f(x2,y3,z1);
				glEnd();
			}
		}
		glPopMatrix();
	}
}


void SeekVisual::draw()
{
	if (handRotationDetector != NULL)
	{
		drawRotationBar(position,size.width,size.height);
		drawTriangles(position,handRotationDetector->getOffset().x);
	}
}