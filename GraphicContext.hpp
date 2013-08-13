#ifndef LEAPIMAGE_GRAPHICS_CONTEXT_HPP_
#define LEAPIMAGE_GRAPHICS_CONTEXT_HPP_

#include <queue>
#include "Animation.h"
#include <map>
#include <stack>
#include <set>
#include "GLImport.h"
#include "View.hpp"

class GraphicsContext {

public:
	static GraphicsContext& getInstance()
	{
		static GraphicsContext instance; 
		return instance;
	}

private:
	GraphicsContext()
	{
	}
	GraphicsContext(GraphicsContext const&);
	void operator=(GraphicsContext const&); 

	~GraphicsContext()
	{
	}

	std::queue<std::function<void()> > drawCallbacks;
	DoubleAnimation anim;



public:
	
	boost::function<void(string)> globalActionCallback;
	boost::function<void()> applicationExitCallback;
	bool BlurRenderEnabled;
	bool IsBlurCurrentPass;
	
	GLFWwindow * MainWindow;

	map<string,float> drawHintMap;

	stack<View*> clearViewStack;
	set<View*> constantlyClearViews;

	void requestConstantClarity(View * clearView)
	{
		constantlyClearViews.insert(clearView);
	}

	void requestExclusiveClarity(View * clearView)
	{
		if (clearView != NULL && (clearViewStack.empty() || clearViewStack.top() != clearView))
			clearViewStack.push(clearView);

		BlurRenderEnabled = !clearViewStack.empty();
	}

	void releaseExclusiveClarity(View * clearView)
	{
		if (!clearViewStack.empty() && clearViewStack.top() == clearView)
			clearViewStack.pop();

		BlurRenderEnabled = !clearViewStack.empty();
	}

	float getBlurScale()
	{
		if (anim.isRunning())
			return (float)anim.getValue();
		else
			return 4;
	}
	
	void setDrawHint(std::string key,float value)
	{
		drawHintMap[key] = value;
	}

	float getDrawHint(std::string key, bool & found)
	{		
		auto it = drawHintMap.find(key);
		if (it == drawHintMap.end())
		{
			found = false;
			return 0;
		}
		found = true;
		return it->second;
	}

	void clearDrawHint(std::string key)
	{
		drawHintMap.erase(key);
	}
	
	void doClearDraw()
	{
		if (!clearViewStack.empty())
			clearViewStack.top()->draw();

		for (auto it = constantlyClearViews.begin(); it != constantlyClearViews.end(); it++)
		{
			(*it)->draw();
		}
	}

	void invokeGlobalAction(std::string args)
	{
		globalActionCallback(args);
	}

	void invokeApplicationExitCallback()
	{
		applicationExitCallback();
	}

};

#endif