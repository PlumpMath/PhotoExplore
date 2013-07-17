#include "ImageButton.hpp"




ImageButton::ImageButton(string path, string overlayPath) : ImagePanel(path)
{	
	pressedLevel = 1.0f;
	overlayPanel = new ImagePanel(overlayPath);	
}

void ImageButton::OnPointableEnter(Pointable & pointable)
{
	ImagePanel::OnPointableEnter(pointable);
	this->activePointable = Pointable(pointable);
	if (activePointable.isValid())
	{
		setPressedScale(activePointable.touchDistance());
	}
}

void ImageButton::OnPointableExit(Pointable & pointable)
{
	ImagePanel::OnPointableExit(pointable);
	this->activePointable = Pointable();
	setPressedScale(1.0f);	
}

void ImageButton::onFrame(const Controller & controller)
{
	activePointable = controller.frame().pointable(activePointable.id());
	if (activePointable.isValid())
	{
		setPressedScale(activePointable.touchDistance());
	}
}

void ImageButton::setPressedScale(float pressed)
{
	this->pressedLevel = pressed;	
}

void ImageButton::drawContent(Vector drawPosition, float drawWidth, float drawHeight)
{		
	ImagePanel::drawContent(drawPosition,drawWidth,drawHeight);

	float scale = min<float>(1.0f,1.0f-pressedLevel);
	if (scale > 0.0f)
	{
		overlayPanel->setTextureTint(Colors::White.withAlpha(scale));
		overlayPanel->drawPanel(drawPosition,drawWidth,drawHeight);
	}	
}

