#include "InputEventHandler.hpp"


InputEventHandler::InputEventHandler()
{

}

void InputEventHandler::glfwCharCallback(GLFWwindow * window, unsigned int key)
{
	InputEventHandler::getInstance().onCharEvent(window,key);
}

void InputEventHandler::init(GLFWwindow * eventWindow)
{
	glfwSetCharCallback(eventWindow,InputEventHandler::glfwCharCallback);
}

void InputEventHandler::addUnicodeCharacterListener(boost::function<bool(GLFWwindow*,unsigned int)> unicodeCallback)
{
	unicodeCallbacks.push_back(unicodeCallback);
}


void InputEventHandler::onCharEvent(GLFWwindow * window, unsigned int key)
{
	for (auto it = unicodeCallbacks.begin(); it != unicodeCallbacks.end();it++)
	{
		bool handled = (*it)(window,key);
		if (handled)
			break;
	}
}