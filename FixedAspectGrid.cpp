#include "FixedAspectGrid.hpp"
#include "GraphicContext.hpp"

FixedAspectGrid::FixedAspectGrid(cv::Size2i _gridDimensions, float _cellAspectRatio, bool _columnFirst) :
	gridDimensions(_gridDimensions),
	columnFirst(_columnFirst),
	cellAspectRatio(_cellAspectRatio),
	interiorMarginsOnly(false)
{}

void FixedAspectGrid::setInteriorMarginsOnly(bool interiorOnly)
{
	interiorMarginsOnly = interiorOnly;
}

bool FixedAspectGrid::getInteriorMarginsOnly()
{
	return interiorMarginsOnly;
}
	

void FixedAspectGrid::measure(cv::Size2f & _measuredSize)
{
	if (columnFirst)
	{	
		if (_measuredSize.height == 0)
			throw std::runtime_error("Initial height must be provided");

		float height = _measuredSize.height;
		float cellHeight = height/gridDimensions.height;
		float cellWidth = cellHeight*cellAspectRatio;

		//int columnCount = min<int>(children.size()/gridDimensions.height,gridDimensions.width);
		//columnCount = max<int>(1,columnCount);

		childSizes.clear();

		int row = 0, col = 0, index = 0;

		for (auto it = children.begin();it != children.end();it++)
		{
			cv::Size2f childSize = cv::Size2f(cellWidth,cellHeight);		
			
			childSizes.push_back(childSize);

			if (++row == gridDimensions.height)
			{
				col++;
				row = 0;
			}
		}

		if (row != 0)
			col++;

		_measuredSize.width = col * cellWidth;
	}
	else
	{
		throw std::runtime_error("Row-first not yet implemented");
	}
}


void FixedAspectGrid::layout(Vector position, cv::Size2f _size)
{
	this->lastPosition = position;
	this->lastSize = _size;

	int row = 0, col = 0, index = 0;

	float xPos = 0, yPos = 0;
	
	float columnWidth = 0;
	for (auto it = children.begin();it != children.end();it++)
	{
		if (index >= childSizes.size())
		{
#ifdef LEAPIMAGE_DEBUG
			throw std::runtime_error("FixedAspectGrid.layout() - ERROR: More children than measurements.");			
#endif
			cv::Size2f mSize(lastSize);
			measure(mSize);			
		}
		cv::Size2f childSize = childSizes.at(index++);
		cv::Vec4f childPadding = (*it)->getLayoutParams().padding;
			
		if (interiorMarginsOnly)
		{	
			if (row == 0)
				childPadding[1] = 0;
			if (row == gridDimensions.height - 1)
				childPadding[3] = 0;

			if (col == 0)
				childPadding[0] = 0;
			if (col == gridDimensions.width - 1)
				childPadding[2] = 0;
		
		}
		Leap::Vector childPosition;
		childPosition.x = xPos + childPadding[0];
		childPosition.y = yPos + childPadding[1];
		
		yPos += childSize.height;

		columnWidth = max<float>(columnWidth,childSize.width);

		childPosition += position;		
		
		(*it)->layout(childPosition,cv::Size2f(childSize.width - (childPadding[0]+childPadding[2]), childSize.height-(childPadding[1]+childPadding[3])));
		
		if (++row == gridDimensions.height)
		{
			xPos += columnWidth; 
			col++;
			row = 0;
			yPos = 0;
		}
	}
}

cv::Rect_<int> FixedAspectGrid::getHitRect()
{
	return cv::Rect_<int>((int)lastPosition.x,(int)lastPosition.y,(int)lastSize.width,(int)lastSize.height);
}

void FixedAspectGrid::draw()
{
	if (children.size() == 0)
		return;

	float height = lastSize.height;
	float cellHeight = height/gridDimensions.height;
	float cellWidth = cellHeight*cellAspectRatio;

	auto drawIt = children.begin();

	bool found;
	float visibleWidth = GraphicsContext::getInstance().getDrawHint("VisibleWidth", found);
	float offset = GraphicsContext::getInstance().getDrawHint("Offset", found);

	if (found)
	{
		int advance = ((int)(-offset/cellWidth)) * gridDimensions.height;		

		advance = min<int>(children.size()-1,advance);
		advance = max<int>(advance,0);
		std::advance(drawIt,advance);
	}

	for (;drawIt != children.end();drawIt++)
	{
		View * child = *drawIt;
		if (found)
		{
			float lastX = child->getLastPosition().x;
			float lastWidth = child->getMeasuredSize().width;

			if (lastWidth + lastX + offset < 0)
				continue;
			else if ((lastX + offset) > visibleWidth)
				break;
		}

		if (child->isVisible())
			child->draw();
	}
}


LeapElement * FixedAspectGrid::elementAtPoint(int x, int y, int & elementStateFlags)
{
	LeapElement * hit = NULL;

	if (children.size() > 0)
	{
		float height = lastSize.height;
		float cellHeight = height/gridDimensions.height;
		float cellWidth = cellHeight*cellAspectRatio;

		auto searchIt = children.begin();

		bool found;
		float visibleWidth = GraphicsContext::getInstance().getDrawHint("VisibleWidth", found);
		float offset = GraphicsContext::getInstance().getDrawHint("Offset", found);

		if (found)
		{
			int advance = ((int)(-offset/cellWidth)) * gridDimensions.height;		

			advance = min<int>(children.size()-1,advance);
			advance = max<int>(advance,0);
			std::advance(searchIt,advance);
		}

		multimap<float,LeapElement*> lazyDepthMap;
		for (;searchIt != children.end();searchIt++)
		{
			View * test = (*searchIt);
			if (test->isVisible() && test->isEnabled())
			{
				if (found)
				{
					float lastX = test->getLastPosition().x;
					float lastWidth = test->getMeasuredSize().width;

					if (lastWidth + lastX + offset < 0)
						continue;
					else if ((lastX + offset) > visibleWidth)
						break;
				}

				hit = test->elementAtPoint(x,y,elementStateFlags);

				if (hit != NULL)
				{
					float depth = hit->getZValue();
					lazyDepthMap.insert(make_pair(depth,hit));
				}
			}
		}

		if (lazyDepthMap.size() > 0)
			return (--lazyDepthMap.end())->second;
		else
			return View::elementAtPoint(x,y,elementStateFlags);

	}
	else
		return View::elementAtPoint(x,y,elementStateFlags);

}