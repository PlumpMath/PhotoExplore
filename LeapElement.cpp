#include "LeapElement.hpp"



void LeapElement::setClickable(bool _clickable)
{
	this->clickable = _clickable;
}

bool LeapElement::isClickable()
{
	return (clickable && !elementClickedCallback.empty());
}