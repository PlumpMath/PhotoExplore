#include "Button.hpp"
#include "LeapHelper.h"

Button::Button(string _text) : TextPanel(_text)
{	
	pressedLevel = 0.0f;
	pressedLevelTimer.start();
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
	static float buttonPressFilterRC = GlobalConfig::tree()->get<float>("Button.PressedLevelFilterRC");

	if (isClickable())
	{
		pressedLevel = min<float>(1.0f,.5f-pressed);
		pressedLevelTimer.start();
	}
	else
		this->pressedLevel = 0.0f;
}

void Button::drawContent(Vector drawPosition, float drawWidth, float drawHeight)
{		
	if (pressedLevel > 0.0f)
	{
		Color newBG = Colors::HoloBlueBright;
		Color tmp = getBackgroundColor();
		newBG.setAlpha(pressedLevel*.6f);
		setBackgroundColor(newBG);
		PanelBase::drawBackground(drawPosition, drawWidth, drawHeight);
		setBackgroundColor(tmp);
	}
	
	TextPanel::drawContent(drawPosition,drawWidth,drawHeight);
}


