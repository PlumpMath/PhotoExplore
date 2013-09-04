#ifndef LEAPIMAGE_UTIL_DRAWING_UTIL_HPP_
#define LEAPIMAGE_UTIL_DRAWING_UTIL_HPP_

#include "GLImport.h"
#include <Leap.h>
#include "Types.h"

class DrawingUtil {
	
public:
	static void drawCircleLine(Leap::Vector drawPoint, Color lineColor, float lineWidth, float outerRadius, float startAngle, float endAngle);
	
	static void drawCircleFill(Leap::Vector drawPoint,Color fillColor, float innerRadius, float outerRadius, float startAngle, float endAngle);
	
	
	static void drawCircleFill(Leap::Vector drawPoint,Color innerColor, Color outerColor, float innerRadius, float outerRadius, float startAngle, float endAngle);
	
};

#endif
