#ifndef LEAPIMAGE_INPUT_EVENT_HANDLER_HPP_
#define LEAPIMAGE_INPUT_EVENT_HANDLER_HPP_

#include "GLImport.h"
#include <boost/function.hpp>
#include <list>

class InputEventHandler {
	

private:	
	InputEventHandler();
	InputEventHandler(InputEventHandler const&);
	void operator=(InputEventHandler const&); 

	std::list<boost::function<bool(GLFWwindow*,unsigned int)> > unicodeCallbacks;
	std::list<boost::function<bool(GLFWwindow*,int)> > focusChangedCallbacks;
	//std::list<boost::function<bool(GLFWwindow*,unsigned int)> > keyCallbacks;


public:
	static InputEventHandler& getInstance()
	{
		static InputEventHandler instance; 
		return instance;
	}

	static void glfwCharCallback(GLFWwindow * window, unsigned int key);
	static void glfwFocusCallback(GLFWwindow * window, int focused);

	void init(GLFWwindow * eventWindow);

	void addUnicodeCharacterListener(boost::function<bool(GLFWwindow*,unsigned int)> unicodeCallback);
	void addFocusChangedCallback(boost::function<bool(GLFWwindow*,int)> focusChangedCallback);

	void onCharEvent(GLFWwindow * window, unsigned int key);
	void onFocusEvent(GLFWwindow * window, int focused);

};

#endif