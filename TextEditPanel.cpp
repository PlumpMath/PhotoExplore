#include "TextEditPanel.hpp"
#include "GLImport.h"


TextEditPanel::TextEditPanel()
{

}

void TextEditPanel::setTextChangedCallback(boost::function<void(std::string newText)> callback)
{
	this->textChangedCallback = callback;
}

void TextEditPanel::update()
{
	int pressedLetter = -1;
	for (int key = 'A'; key <= 'Z'; key++)
	{
		int newState = glfwGetKey(key);

		if (newState == GLFW_PRESS && keyStateMap[key] == GLFW_RELEASE)
		{
			pressedLetter = std::tolower(key);						
		}			
		keyStateMap[key] = newState;
	}
	
	int checkKeys [] = {GLFW_KEY_BACKSPACE,GLFW_KEY_SPACE};
	int num = 2;

	set<int> pressedKeys;
	for (int i=0;i<num;i++)
	{
		int key = checkKeys[i];
		int newState = glfwGetKey(key);
		if (newState == GLFW_PRESS && keyStateMap[key] == GLFW_RELEASE)
		{
			pressedKeys.insert(key);
		}			
		keyStateMap[key] = newState;
	}

	int modifierKeys [] = {GLFW_KEY_LSHIFT,GLFW_KEY_RSHIFT};
	num = 2;

	bool shiftKey = false;
	 
	for (int i=0;i<num;i++)
	{
		int key = modifierKeys[i];
		int newState = glfwGetKey(key);
		if (newState == GLFW_PRESS)
		{
			shiftKey = true;
			break;
		}			
	}


	bool textChanged = false;
	if (pressedKeys.count(GLFW_KEY_BACKSPACE) > 0)
	{
		text = text.substr(0,text.length()-1);

		setText(text);
		textChanged = true;
	}
	else
	{		
		if (pressedKeys.count(GLFW_KEY_SPACE) > 0)
			pressedLetter = (int)' ';

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

