#include "TextEditPanel.hpp"
#include "GLImport.h"
#include "GraphicContext.hpp"
#include "InputEventHandler.hpp"

TextEditPanel::TextEditPanel()
{
	cursorAnimateTimer.start();
	keyRepeatTimer.start();

	InputEventHandler::getInstance().addUnicodeCharacterListener([this](GLFWwindow * window, unsigned int key) -> bool {
		if (this->isVisible())
		{
			stringstream newString;
			newString << this->getText();
			newString << (char)key;
			this->setText(newString.str());
			return true;
		}
		return false;
	});

	InputEventHandler::getInstance().addKeyCallback([&](GLFWwindow * window, int key, int scancode, int action, int mods) -> bool {

		if (this->isVisible() && key == GLFW_KEY_BACKSPACE && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			if (text.length() > 0)
			{
				string newText = text.substr(0,text.length()-1);
				this->setText(newText);
			}
			else if (!textDirty)
			{
				if (!textChangedCallback.empty())
					textChangedCallback(this->getText());
			}
			return true;
		}
		return false;
	});
}

void TextEditPanel::setTextChangedCallback(boost::function<void(std::string newText)> callback)
{
	this->textChangedCallback = callback;
}

void TextEditPanel::setMaxLength(int _maxLength)
{
	this->maxLength = _maxLength;
}

bool TextEditPanel::checkKey(GLFWwindow * checkWindow, int key, double updateTime)
{
	bool trigger = false;
	static double initialRepeatDelay = GlobalConfig::tree()->get<double>("TextEditPanel.InitialRepeatDelay");
	static double keyRepeatPeriod = GlobalConfig::tree()->get<double>("TextEditPanel.KeyRepeatPeriod");
	int newState = glfwGetKey(checkWindow,key);

	if (keyStateMap.find(key) != keyStateMap.end())
	{
		if (newState == GLFW_PRESS && keyStateMap[key].first == GLFW_RELEASE)
		{
			trigger = true;
			keyStateMap[key] = make_pair(newState,updateTime+initialRepeatDelay);
		}	
		else if (newState == GLFW_PRESS && keyStateMap[key].first == GLFW_PRESS && keyStateMap[key].second < updateTime)
		{
			trigger = true;
			keyStateMap[key] = make_pair(newState,updateTime+keyRepeatPeriod);
		}			
		else if (newState == GLFW_RELEASE)
		{
			keyStateMap[key] = make_pair(newState,0);
		}
	}
	else
	{
		keyStateMap.insert(make_pair(key,make_pair(newState,updateTime+initialRepeatDelay)));
	}

	return trigger;
}

void TextEditPanel::update()
{
	//GLFWwindow * checkWindow = GraphicsContext::getInstance().MainWindow;
	//int pressedLetter = -1;
	//double updateTime = keyRepeatTimer.millis();
	//set<int> pressedKeys;
	//if (checkKey(checkWindow, GLFW_KEY_BACKSPACE,updateTime))
	//	pressedKeys.insert(GLFW_KEY_BACKSPACE);
	//
	//if (pressedKeys.count(GLFW_KEY_BACKSPACE) > 0)
	//{
	//	if (text.length() > 0)
	//	{
	//		text = text.substr(0,text.length()-1);
	//		setText(text);
	//	}
	//	else if (!textDirty)
	//	{
	//		if (!textChangedCallback.empty())
	//			textChangedCallback(text);
	//	}
	//}

 	if (textDirty)
	{
		reloadText();
		if (!textChangedCallback.empty())
			textChangedCallback(text);
	}
}

void TextEditPanel::drawContent(Vector drawPosition, float drawWidth, float drawHeight)
{
	static float cursorWidth = GlobalConfig::tree()->get<float>("TextEditPanel.CursorWidth");
	static double cursorPeriod = GlobalConfig::tree()->get<double>("TextEditPanel.CursorBlinkPeriod");
	
	TextPanel::drawContent(drawPosition,drawWidth,drawHeight);


	//if (cursorAnimateTimer.millis() > cursorPeriod)
	//{
	//	cursorOn = !cursorOn;
	//	cursorAnimateTimer.start();
	//}

	//if (cursorOn)
	//{
	//	glBindTexture(GL_TEXTURE_2D,NULL);
	//	glColor4fv(this->getTextColor().getFloat());

	//	float x1 = drawPosition.x + .5f * (drawWidth + currentTextRect.width);
	//	float x2 = x1 + cursorWidth;
	//	float y1 = drawPosition.y;
	//	float y2 = drawPosition.y + textureHeight;
	//	float z1 = drawPosition.z + 1.1f;

	//	glBegin( GL_QUADS );
	//		glVertex3f(x1,y1,z1); 
	//		glVertex3f(x2,y1,z1);
	//		glVertex3f(x2,y2,z1);
	//		glVertex3f(x1,y2,z1);
	//	glEnd();
	//}

}
