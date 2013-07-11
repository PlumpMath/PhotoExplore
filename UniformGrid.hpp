#ifndef LEAPIMAGE_UNIFORM_GRID_HPP_
#define LEAPIMAGE_UNIFORM_GRID_HPP_

#include "View.hpp"

class UniformGrid : public ViewGroup {
	
private:
	cv::Size2i gridDimensions;

public:
	UniformGrid(cv::Size2i & _gridDimensions) :
	  gridDimensions(_gridDimensions)
	{

	}

	void resize(cv::Size2i & gridDimensions);

	cv::Rect_<int> getHitRect();
	void layout(Leap::Vector position, cv::Size2f size);
	void measure(cv::Size2f & measuredSize);
};

#endif