#ifndef LEAPIMAGE_LINEAR_LAYOUT_HPP_
#define LEAPIMAGE_LINEAR_LAYOUT_HPP_

#include "View.hpp"

class LinearLayout : public ViewGroup {
			
private:	
	bool horizontal;
	vector<cv::Size2f> childSizes;

public:
	LinearLayout(bool horizontal = true);
	
	void measure(cv::Size2f & measuredSize);

	void layout(Vector position, cv::Size2f size);
	
	cv::Rect_<int> getHitRect();

};



#endif