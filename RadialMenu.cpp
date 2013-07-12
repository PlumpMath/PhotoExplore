#include "RadialMenu.hpp"
#include "GraphicContext.hpp"
#include "GLImport.h"
#include "Button.hpp"
#include "FixedAspectGrid.hpp"
#include "UniformGrid.hpp"
#include "CustomGrid.hpp"

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

	addChild(rootView);

	setItems(items);
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
	layoutDirty = true;
}

void RadialMenu::itemClicked(string id)
{
	if (ItemClickedCallback(id))
	{
		this->setVisible(false);
	}	
}

void RadialMenu::layout(Vector pos, cv::Size2f size)
{
	this->lastPosition = pos;
	this->lastSize = size;
	
	//float layoutRadius = size.height *.35f;
	float layoutHeight;
	if (!visible)
		layoutHeight = 10;
	else
		layoutHeight = size.height*.12f;

	//float angle = (Leap::PI/2.0)*.6f;
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

void RadialMenu::show()
{
	this->setVisible(true);
}

void RadialMenu::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	if (gestureType.compare("shake") == 0)
	{
		this->setVisible(false);
	}
}

bool RadialMenu::onLeapGesture(const Controller & controller, const Gesture & gesture)
{
	if (gesture.type() == Gesture::Type::TYPE_SWIPE && (gesture.state() == Gesture::State::STATE_UPDATE|| gesture.state() == Gesture::State::STATE_UPDATE))
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

void RadialMenu::setVisible(bool _visible)
{
	if (_visible)
		PointableElementManager::getInstance()->requestGlobalGestureFocus(this);
	else		
		PointableElementManager::getInstance()->releaseGlobalGestureFocus(this);
	GraphicsContext::getInstance().setBlurEnabled(_visible);
	View::setVisible(_visible);
	layout(lastPosition,lastSize);
}

void RadialMenu::draw()
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


bool RadialMenu::checkMenuOpenGesture(const Gesture & gesture)
{
	if (gesture.type() == Gesture::Type::TYPE_CIRCLE && gesture.state() == Gesture::STATE_UPDATE)
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