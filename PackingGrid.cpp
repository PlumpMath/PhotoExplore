#include "PackingGrid.hpp"


PackingGrid::PackingGrid(cv::Size2f _gridBoundaries, bool _columnFirst) :
	gridBoundaries(_gridBoundaries),
	columnFirst(_columnFirst)
{	
	maxCellSize = cv::Size2f(0,0);
	cellPadding = cv::Vec4f(0,0,0,0);
}


void PackingGrid::setPadding(cv::Vec4f padding)
{
	this->cellPadding = padding;
}

cv::Vec4f PackingGrid::getPadding()
{
	return this->cellPadding;
}

void PackingGrid::setMaxCellSize(cv::Size2f _maxCellSize)
{
	this->maxCellSize = _maxCellSize;
}

cv::Size2f PackingGrid::getMaxCellSize()
{
	return maxCellSize;
}

void PackingGrid::measureChildren()
{
	childSizes.clear();
	childPositions.clear();
	measuredSize = cv::Size2f(0,0);
	int index = 0;

	Leap::Vector childPosition(0,0,0);
	float currentRowColumn = 0;
	for (auto it = children.begin();it != children.end();it++)
	{
		cv::Size2f childSize;
		(*it)->measure(childSize);

		if (childSize.width > maxCellSize.width)
		{
			float scale = maxCellSize.width/childSize.width;

			childSize.width *= scale;
			childSize.height *= scale;
		}
		else if (childSize.height > maxCellSize.height)
		{
			float scale = maxCellSize.height/childSize.height;

			childSize.width *= scale;
			childSize.height *= scale;
		}
		
		childSizes.push_back(childSize);

		if (columnFirst)
		{
			if (currentRowColumn + childSize.height > gridBoundaries.height)
			{
				currentRowColumn = 0;
				childPosition.x += maxCellSize.width;
				childPosition.y = 0;
				measuredSize.width += childSize.width;
			}
			else
			{
				currentRowColumn += childSize.height;
				childPosition.y += childSize.height;
			}
			measuredSize.height = max<float>(measuredSize.height,currentRowColumn);
		}
		else
		{
			if (currentRowColumn + childSize.width > gridBoundaries.width)
			{
				currentRowColumn = 0;
				childPosition.y += maxCellSize.height;
				childPosition.x = 0;
				measuredSize.height += childSize.height;
			}
			else
			{
				currentRowColumn += childSize.width;
				childPosition.x += childSize.width;
			}
			measuredSize.width = max<float>(measuredSize.width,currentRowColumn);
		}

		childPositions.push_back(childPosition);

		index++;
	}
}

void PackingGrid::measure(cv::Size2f & measuredSize)
{
	measuredSize = gridBoundaries;
}

void PackingGrid::layout(Vector position, cv::Size2f _size)
{
	gridBoundaries = _size;

	measureChildren();

	this->lastPosition = position;
	this->lastSize = measuredSize;

	int index = 0;
	for (auto it = children.begin();it != children.end();it++)
	{
		if (index >= childSizes.size() || index >= childPositions.size())
		{
			cout << "Error! More children than measurements." << endl;
			break;
		}
		cv::Size2f childSize = childSizes.at(index);
		Leap::Vector childPosition = childPositions.at(index);				

		childPosition.x += childSize.width/2.0f;
		childPosition.y += childSize.height/2.0f;

		childPosition.z = .5;

		childPosition += position;

		childSize.width -= cellPadding[0];
		childSize.height -= cellPadding[1];

		(*it)->layout(childPosition,childSize);

		index++;
	}
}

cv::Rect_<int> PackingGrid::getHitRect()
{
	return cv::Rect_<int>((int)lastPosition.x,(int)lastPosition.y,(int)lastSize.width,(int)lastSize.height);
}