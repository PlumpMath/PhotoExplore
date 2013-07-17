#ifndef LEAPIMAGE_ABSOLUTE_LAYOUT_HPP_
#define LEAPIMAGE_ABSOLUTE_LAYOUT_HPP_

#include "View.hpp"

class AbsoluteLayout : public ViewGroup {

public:

	boost::function<void(Vector pos, cv::Size2f size)> layoutCallback;

	void layout(Vector pos, cv::Size2f size)
	{
		this->lastPosition = pos;
		this->lastSize = size;

		if (!layoutCallback.empty())
			layoutCallback(pos,size);
	}

};

#endif