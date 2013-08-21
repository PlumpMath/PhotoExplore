#include "LeapStatusOverlay.hpp"
#include "TextPanel.h"
#include "GlobalConfig.hpp"
#include "GraphicContext.hpp"
#include "LeapInput.hpp"

LeapStatusOverlay::LeapStatusOverlay()
{	
	isConnected = true;
	isFocused = true;

	notConnectedView = new TextPanel(GlobalConfig::tree()->get_child("StatusOverlay.NotConnected"));
	notConnectedView->setEnabled(false);

	notFocusedView = new TextPanel(GlobalConfig::tree()->get_child("StatusOverlay.NotFocused"));
	notFocusedView->setEnabled(false);

	addChild(notConnectedView);
	addChild(notFocusedView);
}

void LeapStatusOverlay::layout(Vector position, cv::Size2f size)
{	
	this->lastSize = size;
	this->lastPosition = position;

	cv::Size2f statusSize;
	Vector statusPosition,hiddenPosition;

	if (false)
	{
		statusSize.height = size.height * GlobalConfig::tree()->get<float>("StatusOverlay.RelativeHeight");
		statusSize.width = size.width * GlobalConfig::tree()->get<float>("StatusOverlay.RelativeWidth");

		statusPosition = position + Vector(size.width - statusSize.width,size.height - statusSize.height,20);
		statusPosition.x *= .5f;
		statusPosition.y *= .5f;

		hiddenPosition = statusPosition;
		hiddenPosition.y = size.height + statusSize.height;

		if (!isConnected || !isFocused)
		{
			GraphicsContext::getInstance().requestExclusiveClarity(this);
			LeapInput::getInstance()->requestGlobalGestureFocus(this);
		}
		else
		{
			GraphicsContext::getInstance().releaseExclusiveClarity(this);
			LeapInput::getInstance()->releaseGlobalGestureFocus(this);
		}
	}
	else
	{		
		statusSize.height = getTutorialHeight();
		statusSize.width = getTutorialHeight() * 3;

		statusPosition = Vector(0,GlobalConfig::ScreenHeight-statusSize.height,20);
		
		hiddenPosition = statusPosition;
		hiddenPosition.y = size.height + statusSize.height*0.2f;

	}

	if (!isConnected)
	{
		notConnectedView->layout(statusPosition,statusSize);
	}
	else
	{
		notConnectedView->layout(hiddenPosition,statusSize);
	}

	if (isConnected && !isFocused)
	{
		notFocusedView->layout(statusPosition,statusSize);
	}
	else
	{
		notFocusedView->layout(hiddenPosition,statusSize);
	}

	

}


void LeapStatusOverlay::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	;
}

void LeapStatusOverlay::getTutorialDescriptor(vector<string> & tutorial)
{
	;
}


void LeapStatusOverlay::onFramePoll(const Controller & controller)
{
	if (isConnected != controller.isConnected() || isFocused != controller.hasFocus())
	{
		isConnected = controller.isConnected();
		isFocused = controller.hasFocus();
		this->layoutDirty = true;
	}
}


void LeapStatusOverlay::draw()
{
	if (!GraphicsContext::getInstance().IsBlurCurrentPass)
	{
		ViewGroup::draw();
	}
}
