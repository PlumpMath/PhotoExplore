#ifndef LEAPIMAGE_GRAPHICS_CONTEXT_HPP_
#define LEAPIMAGE_GRAPHICS_CONTEXT_HPP_

#include <queue>
#include "Animation.h"
#include <map>
#include "GLImport.h"

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

	void setBlurEnabled(bool enabled)
	{
		if (enabled != BlurRenderEnabled)
		{
			BlurRenderEnabled = enabled;

			//if (enabled)
			//{
			//	anim = DoubleAnimation(30,5,200,new LinearInterpolator(),false,true);
			//	anim.start();
			//}
			//else
			//{
			//	anim.stop();
			//}
		}
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


	void requestClearDraw(std::function<void()> drawFunction)
	{
		drawCallbacks.push(drawFunction);
	}

	void doClearDraw()
	{
		while (!drawCallbacks.empty())
		{
			drawCallbacks.front()(); //(-)_(-)
			drawCallbacks.pop();
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