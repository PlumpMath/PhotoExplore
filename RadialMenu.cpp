#include "RadialMenu.hpp"
#include "GraphicContext.hpp"
#include "GLImport.h"
#include "Button.hpp"
#include "FixedAspectGrid.hpp"
#include "UniformGrid.hpp"
#include "CustomGrid.hpp"
#include "LeapDebug.h"
#include "ImagePanel.hpp"

RadialMenu::RadialMenu(vector<RadialMenuItem> & items)
{		
	
	vector<RowDefinition> gridDefinition;
	
	gridDefinition.push_back(RowDefinition(.65f));
	gridDefinition.push_back(RowDefinition(.05f));
	gridDefinition.push_back(RowDefinition(.05f));
	gridDefinition.push_back(RowDefinition(.05f));
	gridDefinition.push_back(RowDefinition(.05f));
	gridDefinition.push_back(RowDefinition(.05f));

	gridDefinition[0].ColumnWidths.push_back(1);	
	gridDefinition[1].ColumnWidths.push_back(1);
	gridDefinition[2].ColumnWidths.push_back(1);
	gridDefinition[3].ColumnWidths.push_back(1);
	gridDefinition[4].ColumnWidths.push_back(1);
	gridDefinition[5].ColumnWidths.push_back(1);
	
	rootView = new CustomGrid(gridDefinition);	
		
	TextPanel * helpPanel_1 = new TextPanel("1. Make a quick circle gesture to show the menu");
	helpPanel_1->setTextSize(8);
	helpPanel_1->setTextColor(Colors::White);
	helpPanel_1->setBackgroundColor(Colors::WhiteSmoke.withAlpha(0));

	TextPanel * helpPanel_2 = new TextPanel("2. Swipe left and right with a spread hand to scroll. The cursor will be outlined in orange when a spread hand is detected.");
	helpPanel_2->setTextSize(8);
	helpPanel_2->setTextColor(Colors::White);
	helpPanel_2->setBackgroundColor(Colors::WhiteSmoke.withAlpha(0));
	
	TextPanel * helpPanel_3 = new TextPanel("3. Use a pointed finger to select a photo or a button, and to stop scrolling. The cursor will be outlined in blue when a pointed finger is detected.");
	helpPanel_3->setTextSize(8);
	helpPanel_3->setTextColor(Colors::White);
	helpPanel_3->setBackgroundColor(Colors::WhiteSmoke.withAlpha(0));
		
	TextPanel * helpPanel_4 = new TextPanel("4. Shake your hand or finger to go back");
	helpPanel_4->setTextSize(8);
	helpPanel_4->setTextColor(Colors::White);
	helpPanel_4->setBackgroundColor(Colors::WhiteSmoke.withAlpha(0));

	TextPanel * helpPanel_6 = new TextPanel("5. Point with two hands to stretch selected photos");
	helpPanel_6->setTextSize(8);
	helpPanel_6->setTextColor(Colors::White);
	helpPanel_6->setBackgroundColor(Colors::WhiteSmoke.withAlpha(0));

	
	rootView->addChild(new TextPanel(""));
	rootView->addChild(helpPanel_1);
	rootView->addChild(helpPanel_2);
	rootView->addChild(helpPanel_3);
	rootView->addChild(helpPanel_4);
	rootView->addChild(helpPanel_6);

	menuLaunchButton = new ImagePanel(GlobalConfig::tree()->get<string>("Menu.OpenMenuImage"));
	//	("Menu");
	//menuLaunchButton->setBackgroundColor(Colors::SteelBlue);	
	//menuLaunchButton->setTextColor(Colors::White);
	//menuLaunchButton->setTextSize(10);

	
	addChild(rootView);
	addChild(menuLaunchButton);

	menuLaunchButton->elementClickedCallback = [this](LeapElement * clicked){

		if (this->state == MenuState_ButtonOnly)
		{
			this->show();
		}
	};

	setItems(items);

	state = MenuState_ButtonOnly;
}

void RadialMenu::setItems(vector<RadialMenuItem> & items)
{
	clearChildren();		
	for (auto it = items.begin(); it != items.end(); it++)
	{
		Button * item = new Button(it->label);
		item->setTextColor(Colors::White);
		Color c = Colors::DarkRed;

		if (it->buttonColor.a != 0)
			c = it->buttonColor;

		c.setAlpha(.8f);
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


LeapElement * RadialMenu::elementAtPoint(int x, int y, int & state)
{
	return ViewGroup::elementAtPoint(x,y,state);
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
		rootView->layout(pos,size);
	}
	else if (state == MenuState_ButtonOnly) 
	{
		cv::Size2f menuButtonSize = cv::Size2f(size.width*.15f,size.height*.1f);
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