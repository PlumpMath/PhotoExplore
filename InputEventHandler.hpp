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
	std::list<boost::function<bool(GLFWwindow*,int,int,int,int)> > keyCallbacks;
	std::list<boost::function<bool(GLFWwindow*,int,int)> > windowSizeCallbacks;
	std::list<boost::function<bool(GLFWwindow*,int,int)> > windowPositionCallbacks;
	std::list<boost::function<bool(GLFWwindow*,int,int)> > frameBufferSizeCallbacks;
	std::list<boost::function<bool(GLFWwindow*)> > windowRefreshCallbacks;
	std::list<boost::function<bool(GLFWwindow*,double,double)> > scrollCallbacks;
	std::list<boost::function<bool(GLFWwindow*,int,int,int)> > mouseButtonCallbacks;


public:
	static InputEventHandler& getInstance()
	{
		static InputEventHandler instance; 
		return instance;
	}

	static void glfwCharCallback(GLFWwindow * window, unsigned int key);
	static void glfwFocusCallback(GLFWwindow * window, int focused);
	static void glfwKeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods);
	static void glfwWindowSizeCallback(GLFWwindow * window, int, int);
	static void glfwWindowPositionCallback(GLFWwindow * window, int, int);
	static void glfwFrameBufferSizeCallback(GLFWwindow * window, int, int);
	static void glfwWindowRefreshCallback(GLFWwindow * window);
	static void glfwScrollCallback(GLFWwindow * window, double, double);
	static void glfwMouseButtonCallback(GLFWwindow*,int,int,int);
	
	
	void init(GLFWwindow * eventWindow);

	void addUnicodeCharacterListener(boost::function<bool(GLFWwindow*,unsigned int)> unicodeCallback);
	void addFocusChangedCallback(boost::function<bool(GLFWwindow*,int)> focusChangedCallback);
	void addKeyCallback(boost::function<bool(GLFWwindow*window,int,int,int,int)>);
	
	void addWindowSizeCallback(boost::function<bool(GLFWwindow*,int,int)>);
	void addFrameBufferSizeCallback(boost::function<bool(GLFWwindow*,int,int)>);
	void addWindowPositionCallback(boost::function<bool(GLFWwindow*,int,int)>);
	void addWindowRefreshCallback(boost::function<bool(GLFWwindow*)>);
	void addScrollCallback(boost::function<bool(GLFWwindow*,double,double)>);
	void addMouseButtonCallback(boost::function<bool(GLFWwindow*,int,int,int)>);


	void onCharEvent(GLFWwindow * window, unsigned int key);
	void onFocusEvent(GLFWwindow * window, int focused);
	

};

#endif