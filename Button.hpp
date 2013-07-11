#ifndef LEAPIMAGE_BUTTON_HPP_
#define LEAPIMAGE_BUTTON_HPP_

#include "TextPanel.h"

class Button : public TextPanel {

private:
	Pointable activePointable;
	float pressedLevel;

	ViewGroup * rootView;

	
public:
	void setPressedScale(float pressed);

	Button(string text);

	void OnPointableEnter(Pointable & pointable);
	void OnPointableExit(Pointable & pointable);

	void onFrame(const Controller & controller);

	void drawContent(Vector drawPosition, float drawWidth, float drawHeight);

};

#endif