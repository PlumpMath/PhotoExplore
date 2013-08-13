#include "InputEventHandler.hpp"


InputEventHandler::InputEventHandler()
{

}

void InputEventHandler::init(GLFWwindow * eventWindow)
{
	glfwSetCharCallback(eventWindow,InputEventHandler::glfwCharCallback);
	glfwSetWindowFocusCallback(eventWindow,InputEventHandler::glfwFocusCallback);
	glfwSetKeyCallback(eventWindow,InputEventHandler::glfwKeyCallback);
	glfwSetWindowSizeCallback(eventWindow,InputEventHandler::glfwWindowSizeCallback);
	glfwSetWindowPosCallback(eventWindow,InputEventHandler::glfwWindowPositionCallback);
	glfwSetFramebufferSizeCallback(eventWindow,InputEventHandler::glfwFrameBufferSizeCallback);
}

void InputEventHandler::addUnicodeCharacterListener(boost::function<bool(GLFWwindow*,unsigned int)> unicodeCallback)
{
	unicodeCallbacks.push_back(unicodeCallback);
}

void InputEventHandler::addKeyCallback(boost::function<bool(GLFWwindow*,int,int,int,int)> keyCallback)
{
	keyCallbacks.push_back(keyCallback);
}

void InputEventHandler::addWindowPositionCallback(boost::function<bool(GLFWwindow*,int,int)> windowPositionCallback)
{
	windowPositionCallbacks.push_back(windowPositionCallback);
}

void InputEventHandler::addWindowSizeCallback(boost::function<bool(GLFWwindow*,int,int)> windowSizeCallback)
{
	windowSizeCallbacks.push_back(windowSizeCallback);
}

void InputEventHandler::addFrameBufferSizeCallback(boost::function<bool(GLFWwindow*,int,int)> frameBufferSizeCallback)
{
	frameBufferSizeCallbacks.push_back(frameBufferSizeCallback);
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

void InputEventHandler::glfwCharCallback(GLFWwindow * window, unsigned int key)
{
	InputEventHandler::getInstance().onCharEvent(window,key);
}


void InputEventHandler::addFocusChangedCallback(boost::function<bool(GLFWwindow*,int)> focusChanged)
{
	focusChangedCallbacks.push_back(focusChanged);
}

void InputEventHandler::onFocusEvent(GLFWwindow * window, int focused)
{
	for (auto it = focusChangedCallbacks.begin(); it != focusChangedCallbacks.end();it++)
	{
		bool handled = (*it)(window,focused);
		if (handled)
			break;
	}
}

void InputEventHandler::glfwFocusCallback(GLFWwindow * window, int focused)
{
	InputEventHandler::getInstance().onFocusEvent(window,focused);
}

void InputEventHandler::glfwKeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	for (auto it = InputEventHandler::getInstance().keyCallbacks.begin(); it != InputEventHandler::getInstance().keyCallbacks.end(); it++)
	{
		bool handled = (*it)(window,key,scancode,action,mods);
		if (handled)
			break;
	}
}

void InputEventHandler::glfwWindowPositionCallback(GLFWwindow * window, int xPos, int yPos)
{
	for (auto it = InputEventHandler::getInstance().windowPositionCallbacks.begin(); it != InputEventHandler::getInstance().windowPositionCallbacks.end(); it++)
	{
		bool handled = (*it)(window,xPos,yPos);
		if (handled)
			break;
	}
}

void InputEventHandler::glfwWindowSizeCallback(GLFWwindow * window, int width, int height)
{
	for (auto it = InputEventHandler::getInstance().windowSizeCallbacks.begin(); it != InputEventHandler::getInstance().windowSizeCallbacks.end(); it++)
	{
		bool handled = (*it)(window,width,height);
		if (handled)
			break;
	}
}

void InputEventHandler::glfwFrameBufferSizeCallback(GLFWwindow * window, int width, int height)
{
	for (auto it = InputEventHandler::getInstance().frameBufferSizeCallbacks.begin(); it != InputEventHandler::getInstance().frameBufferSizeCallbacks.end(); it++)
	{
		bool handled = (*it)(window,width,height);
		if (handled)
			break;
	}
}