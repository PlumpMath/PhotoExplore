#include "TextEditPanel.hpp"
#include "GLImport.h"
#include "GraphicContext.hpp"


TextEditPanel::TextEditPanel()
{
	cursorAnimateTimer.start();
	keyRepeatTimer.start();
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
	GLFWwindow * checkWindow = GraphicsContext::getInstance().MainWindow;
	int pressedLetter = -1;
	double updateTime = keyRepeatTimer.millis();
	for (int key = 'A'; key <= 'Z'; key++)
	{
		if (checkKey(checkWindow, key,updateTime))
			pressedLetter = std::tolower(key);			
	}
	
	set<int> pressedKeys;
	
	if (checkKey(checkWindow, GLFW_KEY_BACKSPACE,updateTime))
		pressedKeys.insert(GLFW_KEY_BACKSPACE);

	if (checkKey(checkWindow, GLFW_KEY_SPACE,updateTime))
		pressedLetter = (int)' ';

	
	//int checkKeys [] = {GLFW_KEY_BACKSPACE,GLFW_KEY_SPACE};
	//int num = 2;

	//for (int i=0;i<num;i++)
	//{
	//	int key = checkKeys[i];
	//	int newState = glfwGetKey(key);
	//	if (newState == GLFW_PRESS && keyStateMap[key] == GLFW_RELEASE)
	//	{
	//		pressedKeys.insert(key);
	//	}			
	//	keyStateMap[key] = newState;
	//}

	int modifierKeys [] = {GLFW_KEY_LEFT_SHIFT,GLFW_KEY_RIGHT_SHIFT};
	int num = 2;

	bool shiftKey = false;
	 
	for (int i=0;i<num;i++)
	{
		int key = modifierKeys[i];
		int newState = glfwGetKey(checkWindow,key);
		if (newState == GLFW_PRESS)
		{
			shiftKey = true;
			break;
		}			
	}


	bool textChanged = false;
	if (pressedKeys.count(GLFW_KEY_BACKSPACE) > 0 && text.length() > 0)
	{
		text = text.substr(0,text.length()-1);

		setText(text);
		textChanged = true;
	}
	else if (text.length() < maxLength)
	{		
		if (pressedLetter > 0)
		{
			if (pressedLetter != ((int)' ') && shiftKey)
				pressedLetter = std::toupper(pressedLetter);

			stringstream ss;
			ss << text << (char)pressedLetter;
			text = ss.str();
			setText(text);
			textChanged = true;
		}
	}

	if (textChanged)
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
