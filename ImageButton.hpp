#ifndef LEAPIMAGE_NEWPANEL_IMAGE_BUTTON_HPP_
#define LEAPIMAGE_NEWPANEL_IMAGE_BUTTON_HPP_

#include "ImagePanel.hpp"

class ImageButton : public ImagePanel
{
private:	
	Pointable activePointable;
	float pressedLevel;

	void setPressedScale(float pressedScale);

	ImagePanel * overlayPanel;

public:
	ImageButton(string imagePath, string overlayPath);
		
	void OnPointableEnter(Pointable & pointable);
	void OnPointableExit(Pointable & pointable);
	
	void drawContent(Vector drawPosition, float drawWidth, float drawHeight);

	void onFrame(const Controller & controller);


};


#endif