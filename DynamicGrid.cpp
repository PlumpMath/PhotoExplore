#include "DynamicGrid.hpp"

DynamicGrid::DynamicGrid(cv::Size2i _gridDimensions, bool _columnFirst) :
	gridDimensions(_gridDimensions),
	columnFirst(_columnFirst)
{
	defaultSize = cv::Size2f(0,0);
	cellPadding = cv::Vec4f(0,0,0,0);
}

void DynamicGrid::setDefaultCellSize(cv::Size2f _defaultSize)
{
	this->defaultSize = _defaultSize;
}

void DynamicGrid::setPadding(cv::Vec4f padding)
{
	this->cellPadding = padding;
}

cv::Vec4f DynamicGrid::getPadding()
{
	return this->cellPadding;
}

void DynamicGrid::measureChildren()
{
	//childSizes.clear();
	//measuredSize = cv::Size2f(0,0);
	//int index = 0;
	//
	//for (auto it = children.begin();it != children.end();it++)
	//{
	//	int iX = (index / gridDimensions.height);
	//	int iY = index % gridDimensions.height;

	//	cv::Size2f childSize(defaultSize);
	//	(*it)->measure(childSize);

	//	cv::Vec4f childPadding = (*it)->getLayoutParams().padding;

	//	childSizes.push_back(childSize);
	//	
	//	if (iY == 0)
	//		measuredSize.width += childSize.width + childPadding[0] + childPadding[2];
	//	if (iX == 0)
	//		measuredSize.height += childSize.height + childPadding[1] + childPadding[3];

	//	index++;
	//}
}

void DynamicGrid::measure(cv::Size2f & _measuredSize)
{
	childSizes.clear();
	measuredSize = cv::Size2f(0,0);
	int index = 0;

	if (columnFirst && _measuredSize.height == 0)
		throw std::runtime_error("Initial height must be provided");
	else if (!columnFirst && _measuredSize.width == 0)
		throw std::runtime_error("Initial height must be provided");


	for (auto it = children.begin();it != children.end();it++)
	{
		int iX = (index / gridDimensions.height);
		int iY = index % gridDimensions.height;

		cv::Size2f childSize;
		if (columnFirst)
		{
			childSize.height = _measuredSize.height / gridDimensions.height;
		}
		else
		{
			childSize.width = _measuredSize.width / gridDimensions.width;
		}		

		(*it)->measure(childSize);

		cv::Vec4f childPadding = (*it)->getLayoutParams().padding;

		childSizes.push_back(childSize);

		if (iY == 0)
			measuredSize.width += childSize.width + childPadding[0] + childPadding[2];
		if (iX == 0)
			measuredSize.height += childSize.height + childPadding[1] + childPadding[3];

		index++;
	}

	_measuredSize = measuredSize;
}


void DynamicGrid::layout(Vector position, cv::Size2f _size)
{
	this->lastPosition = position;
	this->lastSize = measuredSize;


	int row = 0, col = 0, index = 0;

	float xPos = 0, yPos = 0;
	
	float columnWidth = 0;
	for (auto it = children.begin();it != children.end();it++)
	{
		if (index >= childSizes.size())
		{
			throw std::runtime_error("DynamicGrid.layout() - ERROR: More children than measurements.");
			break;
		}
		cv::Size2f childSize = childSizes.at(index++);
		cv::Vec4f childPadding = (*it)->getLayoutParams().padding;
			
				
		Leap::Vector childPosition;
		childPosition.x = xPos + childPadding[0];
		childPosition.y = yPos + childPadding[1];
		
		yPos += childSize.height;

		columnWidth = max<float>(columnWidth,childSize.width);

		childPosition += position;		
		(*it)->layout(childPosition,childSize);
		
		if (++row == gridDimensions.height)
		{
			xPos += columnWidth; 
			col++;
			row = 0;
			yPos = 0;
		}
	}


	//int index = 0;
	//for (auto it = children.begin();it != children.end();it++)
	//{
	//	int iX = (index / gridDimensions.height); // % gridDimensions.width;
	//	int iY = index % gridDimensions.height;
	//	
	//	if (index >= childSizes.size())
	//	{
	//		cout << "Error! More children than measurements." << endl;
	//		break;
	//	}
	//	cv::Size2f childSize = childSizes.at(index);
	//	Leap::Vector childPosition;
	//	
	//	cv::Vec4f childPadding = (*it)->getLayoutParams().padding;

	//	childPosition.x = ((iX) * childSize.width) + childPadding[0];
	//	childPosition.y = ((iY) * childSize.height) + childPadding[1];

	//	childPosition.z = .5;

	//	childPosition += position;

	//	//childSize.width -= cellPadding[0];
	//	//childSize.height -= cellPadding[1];

	//	(*it)->layout(childPosition + Vector(childPadding[0],childPadding[1],0),cv::Size2f(childSize.width - (childPadding[0]+childPadding[2]), childSize.height-(childPadding[1]+childPadding[3])));

	//	index++;
	//}
}

cv::Rect_<int> DynamicGrid::getHitRect()
{
	return cv::Rect_<int>((int)lastPosition.x,(int)lastPosition.y,(int)lastSize.width,(int)lastSize.height);
}