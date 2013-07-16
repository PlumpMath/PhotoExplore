#include "Button.hpp"


Button::Button(string _text) : TextPanel(_text)
{	
	pressedLevel = 1.0f;
}

void Button::OnPointableEnter(Pointable & pointable)
{
	TextPanel::OnPointableEnter(pointable);
	this->activePointable = Pointable(pointable);
	if (activePointable.isValid())
	{
		setPressedScale(activePointable.touchDistance());
	}
}

void Button::OnPointableExit(Pointable & pointable)
{
	TextPanel::OnPointableExit(pointable);
	this->activePointable = Pointable();
	setPressedScale(1.0f);	
}

void Button::onFrame(const Controller & controller)
{
	activePointable = controller.frame().pointable(activePointable.id());
	if (activePointable.isValid())
	{
		setPressedScale(activePointable.touchDistance());
	}
}

void Button::setPressedScale(float pressed)
{
	this->pressedLevel = pressed;	
}

void Button::drawContent(Vector drawPosition, float drawWidth, float drawHeight)
{		
	float scale = min<float>(1.0f,.5f-pressedLevel);
	if (scale > 0.0f)
	{
		Color newBG = Colors::HoloBlueBright;
		Color tmp = getBackgroundColor();
		newBG.setAlpha(scale*.6f);
		setBackgroundColor(newBG);
		PanelBase::drawBackground(drawPosition, drawWidth, drawHeight);
		setBackgroundColor(tmp);
	}
	
	TextPanel::drawContent(drawPosition,drawWidth,drawHeight);
}


