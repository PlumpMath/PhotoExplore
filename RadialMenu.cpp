#include "RadialMenu.hpp"
#include "GraphicContext.hpp"
#include "GLImport.h"
#include "Button.hpp"
#include "FixedAspectGrid.hpp"
#include "UniformGrid.hpp"
#include "CustomGrid.hpp"
#include "LeapDebug.h"
#include "ImagePanel.hpp"

RadialMenu * RadialMenu::instance = NULL;

RadialMenu::RadialMenu(vector<RadialMenuItem> & items)
{		
	menuLaunchButton = new ImagePanel(GlobalConfig::tree()->get<string>("Menu.OpenMenuImage"));	
	addChild(menuLaunchButton);
	menuLaunchButton->elementClickedCallback = [this](LeapElement * clicked){

		if (this->state == MenuState_ButtonOnly)
		{
			this->show();
		}
	};
	//((ImagePanel*)menuLaunchButton)->setBackgroundColor(Colors::HoloBlueDark.withAlpha(.5f));

	setItems(items);

	state = MenuState_ButtonOnly;
	instance = this;
}

void RadialMenu::setItems(vector<RadialMenuItem> & items)
{
	clearChildren();		
	for (auto it = items.begin(); it != items.end(); it++)
	{
		Button * item = new Button(it->label);
		item->setTextColor(Colors::Black);
		item->setBackgroundColor(Colors::White);
		item->setBorderColor(it->buttonColor);
		item->setBorderThickness(2);
		Color c = Colors::DarkRed;

		if (it->buttonColor.a != 0)
			c = it->buttonColor;

		c.setAlpha(.7f);
		item->setBackgroundColor(c);		
		item->setTextSize(12);
		string itemId = it->id;
		item->setLayoutParams(LayoutParams(cv::Size2f(400,150),cv::Vec4f(20,20,20,20)));
		item->elementClickedCallback = [this,itemId](LeapElement * element){
			this->itemClicked(itemId);
		};
		addChild(item);
	}	
	addChild(menuLaunchButton);
	layoutDirty = true;
}


LeapElement * RadialMenu::elementAtPoint(int x, int y, int & _state)
{
	if (state == MenuState_DisplayFull)
	{
		return ViewGroup::elementAtPoint(x,y,_state);
	}
	else
	{
		return menuLaunchButton->elementAtPoint(x,y,_state);
	}
}


float RadialMenu::getZValue()
{
	return 100.0f;
}

void RadialMenu::itemClicked(string id)
{
	if (ItemClickedCallback(id))
	{
		dismiss();
	}	
}

void RadialMenu::layout(Vector pos, cv::Size2f size)
{
	this->lastPosition = pos;
	this->lastSize = size;

	if (state == MenuState_DisplayFull)
	{
		float layoutHeight;
		if (!visible)
			layoutHeight = 10;
		else
			layoutHeight = size.height*.12f;

		cv::Size2f childSize = cv::Size2f(size.width * .4f,layoutHeight);

		Vector center = pos+Vector(size.width*.5f,size.height*.5f,0);
		float offset = -1;
		float spacing = childSize.height*1.2f;

		for (auto it = children.begin(); it != children.end(); it++)
		{			
			(*it)->layout(center + Vector(-childSize.width/2.0f,offset*spacing,0),childSize);
			offset += 1.0f;
		}
	}
	else if (state == MenuState_ButtonOnly) 
	{
		cv::Size2f menuButtonSize = cv::Size2f(size.width*.15f,GlobalConfig::tree()->get<float>("Tutorial.Height"));
		menuLaunchButton->layout(Vector(size.width - menuButtonSize.width,size.height - menuButtonSize.height,10) + pos,menuButtonSize);
	}
	this->layoutDirty = false;
}

void RadialMenu::show()
{
	state = MenuState_DisplayFull;
	PointableElementManager::getInstance()->requestGlobalGestureFocus(this);
	GraphicsContext::getInstance().setBlurEnabled(true);
	menuLaunchButton->setVisible(false);
	layout(lastPosition,lastSize);
}

void RadialMenu::dismiss()
{
	state = MenuState_ButtonOnly;
	PointableElementManager::getInstance()->releaseGlobalGestureFocus(this);
	GraphicsContext::getInstance().setBlurEnabled(false);	
	menuLaunchButton->setVisible(true);
	layout(lastPosition,lastSize);
}

void RadialMenu::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	if (gestureType.compare("shake") == 0)
	{
		dismiss();
	}
}

bool RadialMenu::onLeapGesture(const Controller & controller, const Gesture & gesture)
{
	if (GlobalConfig::tree()->get<bool>("Menu.AllowSwipeDownExit") && 
		(gesture.type() == Gesture::Type::TYPE_SWIPE && (gesture.state() == Gesture::State::STATE_UPDATE|| gesture.state() == Gesture::State::STATE_UPDATE)))
	{
		SwipeGesture swipe(gesture);

		if (swipe.direction().angleTo(Vector::down()) < PI/4.0f)
		{
			this->setVisible(false);
			return true;
		}
	}
	return false;
}

void RadialMenu::getTutorialDescriptor(vector<string> & tutorial)
{
	tutorial.push_back("shake");
	tutorial.push_back("point");
}

void RadialMenu::draw()
{
	if (state == MenuState_DisplayFull)
	{
		if (!GraphicsContext::getInstance().IsBlurCurrentPass)
		{		
			ViewGroup::draw();				
		}
		else
		{
			GraphicsContext::getInstance().requestClearDraw([this](){this->draw();});
		}
	}
	else if (state == MenuState_ButtonOnly) 
	{
		menuLaunchButton->draw();
	}

}


bool RadialMenu::checkMenuOpenGesture(const Gesture & gesture)
{
	if (GlobalConfig::tree()->get<bool>("Menu.CircleGesture.Enabled") && 
		gesture.type() == Gesture::Type::TYPE_CIRCLE && gesture.state() == Gesture::STATE_UPDATE)
	{
		CircleGesture circle(gesture);
		if(	(  
				!GlobalConfig::tree()->get<bool>("Menu.CircleGesture.SinglePointableOnly") || 
				circle.hands().count() == 0 || circle.hands()[0].pointables().count() == 1
			) 
			&& 
			circle.progress() >  GlobalConfig::tree()->get<float>("Menu.CircleGesture.MinRotations") && 
			circle.durationSeconds() <  GlobalConfig::tree()->get<float>("Menu.CircleGesture.MaxDuration") &&
			(circle.normal().angleTo(Leap::Vector::backward()) < (.25f * PI) || 
			circle.normal().angleTo(Leap::Vector::forward()) < PI/4.0f) && 
			circle.radius() > GlobalConfig::tree()->get<float>("Menu.CircleGesture.MinRadius") &&
			circle.radius() < GlobalConfig::tree()->get<float>("Menu.CircleGesture.MaxRadius") 
		)
		{
			return true;
		}	
	}
	return false;
}