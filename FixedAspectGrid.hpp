#ifndef LEAPIMAGE_FIXED_ASPECT_RATIO_GRID_HPP_
#define LEAPIMAGE_FIXED_ASPECT_RATIO_GRID_HPP_

#include "View.hpp"

class FixedAspectGrid : public ViewGroup {

private:	
	cv::Size2i gridDimensions;
	bool columnFirst;
	cv::Size2f defaultSize;
	cv::Vec4f cellPadding;
	float cellAspectRatio;
	vector<cv::Size2f> childSizes;
	//cv::Size2f measuredSize;
	void measureChildren();

	bool interiorMarginsOnly;

public:
	FixedAspectGrid(cv::Size2i gridSize, float cellAspectRatio, bool columnFirst = true);
	
	void measure(cv::Size2f & measuredSize);

	void setGridSize(cv::Size2i gridSize);
	cv::Size2i getGridSize();

	void layout(Vector position, cv::Size2f size);

	void setInteriorMarginsOnly(bool interiorOnly);
	bool getInteriorMarginsOnly();

	
	cv::Rect_<int> getHitRect();

	void draw();
	LeapElement * elementAtPoint(int x, int y, int & state);


};


#endif