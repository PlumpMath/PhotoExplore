#ifndef LEAPIMAGE_EVENTHANDLING_LEAP_ELEMENT_HPP_
#define LEAPIMAGE_EVENTHANDLING_LEAP_ELEMENT_HPP_

#include <Leap.h>
#include <boost/function.hpp>
#include <opencv2/opencv.hpp>

using namespace Leap;

class LeapElement {

private:
	LeapElement * parent;

protected:
	bool clickable, enabled;
	
public:
	const static int Flags_ElementNonClickable = 0x0001;
	const static int Flags_None = 0;


	boost::function<void(LeapElement * node)> elementClickedCallback;
	boost::function<void(LeapElement * node, Pointable & pointable)> pointableEnterCallback;
	boost::function<void(LeapElement * node, Pointable & pointable)> pointableExitCallback;

	LeapElement()
	{
		parent = NULL;
		clickable = true;
		enabled = true;
	}
	
	void setParent(LeapElement * _parent)
	{
		this->parent = _parent;
	}

	LeapElement * getParent()
	{
		return this->parent;
	}

	virtual cv::Rect_<int> getHitRect()
	{
		return cv::Rect_<int>();
	}

	virtual float getZValue()
	{
		return -1000.0f;
	}	
	
	virtual bool onGesture(const Gesture & gesture)
	{
		return false;
	}

	virtual void gestureFocusLost(LeapElement * lostTo)
	{

	}

	virtual void OnPointableEnter(Pointable & pointable)
	{
		;
	}



	virtual void setClickable(bool _clickable);

	virtual bool isClickable();

	virtual void setEnabled(bool _enabled)
	{
		this->enabled = _enabled;
	}

	virtual bool isEnabled()
	{
		return this->enabled;
	}

	//void getRoutingStack(LeapElement * element, stack<LeapElement*> & result)
	//{
	//	while (element->getParent() != NULL)
	//	{
	//		result.push(element->getParent());
	//		element = element->getParent();
	//	}
	//}

	void pointableEnter(Pointable & pointable)
	{
		if (!pointableEnterCallback.empty())
			pointableEnterCallback(this, pointable);

		OnPointableEnter(pointable);
	}

	virtual void OnPointableExit(Pointable & pointable)
	{
		;
	}

	bool interceptPointableExit(LeapElement * origin, Pointable & pointable)
	{
		return OnInterceptPointableExit(origin,pointable);
	}

	bool interceptPointableEnter(LeapElement * origin, Pointable & pointable)
	{
		return OnInterceptPointableEnter(origin, pointable);
	}

	virtual bool OnInterceptPointableExit(LeapElement * origin, Pointable & pointable)
	{
		return false;
	}

	virtual bool OnInterceptPointableEnter(LeapElement * origin, Pointable & pointable)
	{
		return false;
	}

	void pointableExit(Pointable & pointable)
	{
		if (!pointableExitCallback.empty())
			pointableExitCallback(this, pointable);

		OnPointableExit(pointable);
	}
	
	virtual void elementClicked()
	{
		if (!elementClickedCallback.empty())
			elementClickedCallback(this);
	}

	virtual void onFrame(const Controller & controller)
	{
		;
	}


	virtual LeapElement * elementAtPoint(int x, int y, int & elementStateFlags)
	{
		cv::Rect_<int> hitRect = getHitRect();
		if (hitRect.width != 0 && hitRect.height != 0)
		{
			int left = hitRect.x;
			int bottom = hitRect.y;
			int top = bottom + hitRect.height;
			int right = left + hitRect.width;

			if (x >= left && x < right && y >= bottom && y < top)
			{
				return this;
			}
		}
		return NULL;
	}
};

#endif