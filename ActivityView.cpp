#include"ActivityView.hpp"
#include "GlobalConfig.hpp"

float ActivityView::getMenuHeight()
{
	float relativeHeight = GlobalConfig::tree()->get<float>("Menu.RelativeHeight") * GlobalConfig::ScreenHeight;
	float minHeight = GlobalConfig::tree()->get<float>("Menu.MinimumHeight");
	
	return max<float>(minHeight,relativeHeight);
}


float ActivityView::getTutorialHeight()
{	
	float relativeHeight = GlobalConfig::tree()->get<float>("Tutorial.RelativeHeight") * GlobalConfig::ScreenHeight;
	float minHeight = GlobalConfig::tree()->get<float>("Tutorial.MinimumHeight");
	
	return max<float>(minHeight,relativeHeight);
}


void ActivityView::setViewFinishedCallback(boost::function<void(std::string)> _viewFinishedCallback)
{
	this->viewFinishedCallback = _viewFinishedCallback;
}