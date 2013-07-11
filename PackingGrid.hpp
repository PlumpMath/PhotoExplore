#ifndef LEAPIMAGE_PACKING_GRID_HPP_
#define LEAPIMAGE_PACKING_GRID_HPP_

#include "View.hpp"

class PackingGrid : public ViewGroup {

private:	
	bool columnFirst;
	cv::Vec4f cellPadding;

	cv::Size2f gridBoundaries;
	cv::Size2f maxCellSize;


	vector<cv::Size2f> childSizes;
	vector<Leap::Vector> childPositions;

	cv::Size2f measuredSize;

	void measureChildren();

public:
	PackingGrid(cv::Size2f gridBoundaries, bool columnFirst = true);

	void setPadding(cv::Vec4f padding);
	cv::Vec4f getPadding();

	void setMaxCellSize(cv::Size2f maxCellSize);
	cv::Size2f getMaxCellSize();
	
	void measure(cv::Size2f & measuredSize);
	void layout(Vector position, cv::Size2f size);

	cv::Rect_<int> getHitRect();
};

#endif