#ifndef LEAPIMAGE_PANEL_COLOR_PANEL_HPP_
#define LEAPIMAGE_PANEL_COLOR_PANEL_HPP_

#include "PanelBase.h"

class FloatingPanel : public PanelBase {

public:
	FloatingPanel(float _width, float _height, Vector _position) : PanelBase()
	{				
		setPanelSize(_width,_height);
		setPosition(_position);
	}
	void update(double deltaTime);
	void draw();
	void drawContent(Vector drawPosition, float drawWidth, float drawHeight);
};

#endif