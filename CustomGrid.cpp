#include "CustomGrid.hpp"


CustomGrid::CustomGrid(vector<RowDefinition> _rowDefinitions, bool _tranpose) :
		rowDefinitions(_rowDefinitions),
		transpose(_tranpose)

{

}

void CustomGrid::measureChildren()
{
	childSizes.clear();
	childPositions.clear();

	int ix = 0, iy = 0;

	float dynamicDimension = 0;

	Leap::Vector childPosition(0,0,0);

	if (transpose)
	{
		float currentColumnWidth = 0;
		float currentY  = 0;
		for (auto it = children.begin();it != children.end();it++)
		{		
			cv::Size2f childSize;
			Leap::Vector childPosition;

			childSize.width = rowDefinitions[ix].Height * gridSize.width;		
			childSize.height = rowDefinitions[ix].ColumnWidths[iy] * gridSize.height;

			childPosition = Leap::Vector(currentColumnWidth, currentY,0);		

			currentY += childSize.height;		

			childPositions.push_back(childPosition);
			childSizes.push_back(childSize);

			if (++iy >= rowDefinitions[ix].ColumnWidths.size())
			{
				currentColumnWidth += rowDefinitions[ix].Height * gridSize.width;

				ix++;
				iy = 0;
				currentY = 0;
				if (ix >= rowDefinitions.size())
					break;			
			}
		}
	}
	else
	{
		float currentHeight = 0;
		float currentX  = 0;
		for (auto it = children.begin();it != children.end();it++)
		{		
			cv::Size2f childSize;
			Leap::Vector childPosition;
		
			childSize.height = rowDefinitions[iy].Height * gridSize.height;		
			
			if (rowDefinitions[iy].ColumnWidths[ix] != 0)
			{
				childSize.width = rowDefinitions[iy].ColumnWidths[ix] * gridSize.width;			
			}
			else
			{
				(*it)->measure(childSize);
			}

			childPosition = Leap::Vector(currentX, currentHeight,0);		

			currentX += childSize.width;		

			childPositions.push_back(childPosition);
			childSizes.push_back(childSize);
		
			if (++ix >= rowDefinitions[iy].ColumnWidths.size())
			{
				currentHeight += rowDefinitions[iy].Height * gridSize.height;

				iy++;
				ix = 0;
				
				dynamicDimension = max<float>(currentX,dynamicDimension);
				currentX = 0;
				if (iy >= rowDefinitions.size())
					break;			
			}
		}
		//if (dynamicDimension != 0)
			//desiredSize.width = dynamicDimension;
	}
}

void CustomGrid::measure(cv::Size2f & measuredSize)
{
	measureChildren();
	measuredSize = desiredSize;
}


void CustomGrid::layout(Vector position, cv::Size2f _size)
{

	this->lastPosition = position;
	this->lastSize = _size;
	gridSize = _size;
	
	measureChildren();

	int index = 0;
	for (auto it = children.begin();it != children.end();it++)
	{
		if (index >= childSizes.size() || index >= childPositions.size())
		{
			throw std::runtime_error("CustomGrid.layout() - ERROR: More children than measurements.");
		}
		cv::Size2f childSize = childSizes.at(index);
		cv::Vec4f childPadding = (*it)->getLayoutParams().padding;

		Leap::Vector childPosition = childPositions.at(index);				
		
		childPosition.z = .5;

		childPosition += position + Vector(childPadding[0],childPadding[1],0);

		childSize.width -= childPadding[0]+childPadding[2];
		childSize.height -= childPadding[1]+childPadding[3];

		(*it)->layout(childPosition,childSize);

		index++;
	}
}

cv::Rect_<int> CustomGrid::getHitRect()
{
	return cv::Rect_<int>((int)lastPosition.x,(int)lastPosition.y,(int)lastSize.width,(int)lastSize.height);
}