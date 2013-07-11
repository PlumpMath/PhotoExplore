#include "LinearLayout.hpp"


LinearLayout::LinearLayout(bool _horizontal) :
	horizontal(_horizontal)
{
	;
}

void LinearLayout::measure(cv::Size2f & _measuredSize)
{
	if (horizontal)
	{	
		if (_measuredSize.height == 0)
			throw std::exception("Initial height must be provided");

		float height = _measuredSize.height;

		childSizes.clear();

		float width = 0;
		for (auto it = children.begin();it != children.end();it++)
		{
			cv::Vec4f childPadding = (*it)->getLayoutParams().padding;
			cv::Size2f childSize = cv::Size2f(-1,height-(childPadding[1]+childPadding[3]));		
			
			(*it)->measure(childSize);
			childSize.width += childPadding[0]+childPadding[2];
			width += childSize.width;
			childSizes.push_back(childSize);			
		}

		_measuredSize.width = width;
	}
	else
	{
		throw std::exception("Vertical layout not yet implemented");
	}
}


void LinearLayout::layout(Vector position, cv::Size2f _size)
{
	this->lastPosition = position;
	this->lastSize = _size;

	int index = 0;

	float xPos = 0;
	
	float columnWidth = 0;
	for (auto it = children.begin();it != children.end();it++)
	{
		if (index >= childSizes.size())
		{
			cout << "Error! More children than measurements." << endl;
			break;
		}
		cv::Size2f childSize = childSizes.at(index++);
		cv::Vec4f childPadding = (*it)->getLayoutParams().padding;
				
		Leap::Vector childPosition;
		childPosition.x = xPos + childPadding[0];
		xPos += childSize.width + childPadding[2]; 
		
		childPosition += position;		

		(*it)->layout(childPosition,cv::Size2f(childSize.width - (childPadding[0]+childPadding[2]), childSize.height-(childPadding[1]+childPadding[3])));	
	}
}

cv::Rect_<int> LinearLayout::getHitRect()
{
	return cv::Rect_<int>((int)lastPosition.x,(int)lastPosition.y,(int)lastSize.width,(int)lastSize.height);
}