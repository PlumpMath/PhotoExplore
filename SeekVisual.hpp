#ifndef LEAPIMAGE_SEEK_VISUAL_HPP_
#define LEAPIMAGE_SEEK_VSUAL_HPP_

#include "GLImport.h"

#include "HandRotationGestureDetector.h"
#include "Types.h"
#include "View.hpp"

class SeekVisual : public View {
	
private:
	HandRotationGestureDetector * handRotationDetector;
	cv::Size2f size;
	Leap::Vector position;

	void drawTriangles(Vector drawPosition, float xOffset);
	void drawRotationBar(Leap::Vector drawPosition, float drawWidth, float drawHeight);

public:
	SeekVisual();
	
	void setHandRotationDetector(HandRotationGestureDetector * handRotationDetector);

	void layout(Leap::Vector position, cv::Size2f size);	
	void draw();
};

#endif