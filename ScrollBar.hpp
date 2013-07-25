#ifndef LEAPIMAGE_NEWPANEL_SCROLLBAR_HPP_
#define LEAPIMAGE_NEWPANEL_SCROLLBAR_HPP_

#include "View.hpp"
#include "ScrollingView.hpp"

class ScrollBar : public View {

private:
	ScrollingView * scrollingView;
	FlyWheel * flyWheel;

	Color background,foreground;
	
	float lastScrollRange;
	Timer scrollWidthFilterTimer;

public:
	ScrollBar();

	void setScrollView(ScrollingView * scrollingView);

	void draw();
	void layout(Vector position, cv::Size2f size);

};

#endif