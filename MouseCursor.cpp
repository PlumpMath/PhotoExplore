#include "LeapCursor.hpp"
#include "GraphicContext.hpp"
#include "GLImport.h"

MouseCursor::MouseCursor(double size, Color fillColor) : PointableCursor(size,fillColor)
{

}


void MouseCursor::update()
{
	double x,y;
	glfwGetCursorPos(GraphicsContext::getInstance().MainWindow,&x,&y);

	drawPoint = Vector((float)x,(float)y,0);
}