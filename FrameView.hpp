#ifndef LEAPIMAGE_FRAME_VIEW_HPP_
#define LEAPIMAGE_FRAME_VIEW_HPP_

#include "View.hpp"
#include "Types.h"

class FrameView : public View {

protected:
	View * contentView;
	Color backgroundColor;

public:	
	FrameView();
	FrameView(View * contentView);
	FrameView(View * contentView, Color backgroundColor);
	void drawContent(Vector drawPosition, float drawWidth, float drawHeight);

	void setContentView(View * contentView);
	View * getContentView();

	void setBackgroundColor(Color & backgroundColor);
	Color getBackgroundColor();

	void measure(cv::Size2f & measuredSize);
	void update();
	void draw();
	void layout(Vector layoutPosition, cv::Size2f layoutSize);

	LeapElement * elementAtPoint(int x, int y, int & elementStateFlags);

};

#endif