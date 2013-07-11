#ifndef LEAPIMAGE_DYNAMIC_GRID_HPP_
#define LEAPIMAGE_DYNAMIC_GRID_HPP_

#include "View.hpp"

class DynamicGrid : public ViewGroup {
	
private:	
	cv::Size2i gridDimensions;
	bool columnFirst;
	cv::Size2f defaultSize;
	cv::Vec4f cellPadding;
	

	vector<cv::Size2f> childSizes;
	cv::Size2f measuredSize;
	void measureChildren();

public:
	DynamicGrid(cv::Size2i gridSize, bool columnFirst = true);

	void setPadding(cv::Vec4f padding);
	cv::Vec4f getPadding();

	void setDefaultCellSize(cv::Size2f defaultSize);

	void measure(cv::Size2f & measuredSize);

	void layout(Vector position, cv::Size2f size);
	
	cv::Rect_<int> getHitRect();


};

#endif