#include "UniformGrid.hpp"


void UniformGrid::layout(Leap::Vector position, cv::Size2f size)
{
	this->lastPosition = position;
	this->lastSize = size;

	cv::Size2f childSize;
	childSize.width = size.width/(float)gridDimensions.width;
	childSize.height = size.height/(float)gridDimensions.height;


	int index = 0;
	for (auto it = children.begin();it != children.end();it++)
	{
		int iX = (index / gridDimensions.height) % gridDimensions.width;
		int iY = index % gridDimensions.height;

		
		cv::Vec4f childPadding = (*it)->getLayoutParams().padding;

		Leap::Vector childPosition;

		childPosition.x = ((float)iX) * childSize.width;
		childPosition.y = ((float)iY) * childSize.height;

		childPosition.z = .5f;

		childPosition += position;

		(*it)->layout(childPosition + Vector(childPadding[0],childPadding[1],0),cv::Size2f(childSize.width - (childPadding[0]+childPadding[2]), childSize.height-(childPadding[1]+childPadding[3])));

		index++;
	}
}

void UniformGrid::measure(cv::Size2f & measuredSize)
{
	measuredSize = desiredSize;
}

cv::Rect_<int> UniformGrid::getHitRect()
{
	return cv::Rect_<int>((int)lastPosition.x,(int)lastPosition.y,(int)lastSize.width,(int)lastSize.height);
}

void UniformGrid::resize(cv::Size2i & _gridDimensions)
{
	this->gridDimensions = _gridDimensions;
	layout(lastPosition,lastSize);
}