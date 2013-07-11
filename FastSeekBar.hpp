#ifndef LEAPIMAGE_FAST_SEEK_BAR_HPP_
#define LEAPIMAGE_FAST_SEEK_BAR_HPP_

#include "View.hpp"
#include "Button.hpp"
#include "ScrollingView.hpp"
#include "SDLTimer.h"

using namespace Leap;

class FastSeekBar : public ViewGroup {

private:
	ScrollingView * scrollView;
	int state;
	Timer moveTimer, selectTimer;
	Vector startPosition;
	double flywheelStartPosition;

	float lastScrollDelta;
	float lastHandX;
	
	float scrollBarHeight, scrollThumbWidth;

	Button * scrollThumb;

	const static int StateWaiting = 0;
	const static int StateSelecting = 1;
	const static int StateScrolling = 2;

	//Pointable capturedPointable;
	Hand capturedHand;

	void startScroll();
	void endScroll();

public:
	FastSeekBar(ScrollingView * scrollView);

	void draw();
	void update();
	void layout(Vector position, cv::Size2f size);

	void onFrame(const Controller & controller);

};


#endif