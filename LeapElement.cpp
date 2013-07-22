#include "LeapElement.hpp"



void LeapElement::setClickable(bool _clickable)
{
	this->clickable = _clickable;
}

bool LeapElement::isClickable()
{
	return (clickable && !elementClickedCallback.empty());
}

LeapElement::LeapElement()
{
	parent = NULL;
	clickable = true;
	enabled = true;
}

void LeapElement::setParent(LeapElement * _parent)
{
	this->parent = _parent;
}

LeapElement * LeapElement::getParent()
{
	return this->parent;
}

cv::Rect_<int> LeapElement::getHitRect()
{
	return cv::Rect_<int>();
}

float LeapElement::getZValue()
{
	return -1000.0f;
}	

bool LeapElement::onGesture(const Gesture & gesture)
{
	return false;
}

void LeapElement::gestureFocusLost(LeapElement * lostTo)
{

}

void LeapElement::OnPointableEnter(Pointable & pointable)
{
	;
}

void LeapElement::setEnabled(bool _enabled)
{
	this->enabled = _enabled;
}

bool LeapElement::isEnabled()
{
	return this->enabled;
}

void LeapElement::pointableEnter(Pointable & pointable)
{
	if (!pointableEnterCallback.empty())
		pointableEnterCallback(this, pointable);

	OnPointableEnter(pointable);
}

void LeapElement::OnPointableExit(Pointable & pointable)
{
	;
}

bool LeapElement::interceptPointableExit(LeapElement * origin, Pointable & pointable)
{
	return OnInterceptPointableExit(origin,pointable);
}

bool LeapElement::interceptPointableEnter(LeapElement * origin, Pointable & pointable)
{
	return OnInterceptPointableEnter(origin, pointable);
}

bool LeapElement::OnInterceptPointableExit(LeapElement * origin, Pointable & pointable)
{
	return false;
}

bool LeapElement::OnInterceptPointableEnter(LeapElement * origin, Pointable & pointable)
{
	return false;
}

void LeapElement::pointableExit(Pointable & pointable)
{
	if (!pointableExitCallback.empty())
		pointableExitCallback(this, pointable);

	OnPointableExit(pointable);
}

void LeapElement::elementClicked()
{
	if (!elementClickedCallback.empty())
		elementClickedCallback(this);
}

void LeapElement::onFrame(const Controller & controller)
{
	;
}

LeapElement * LeapElement::elementAtPoint(int x, int y, int & elementStateFlags)
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