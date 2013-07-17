#ifndef LEAPIMAGE_NEWPANEL_EMPTY_VIEW_HPP_
#define LEAPIMAGE_NEWPANEL_EMPTY_VIEW_HPP_

#include "View.hpp"

class EmptyView : public View {

public:	
	EmptyView(){}
	void layout(Vector position, cv::Size2f size) {}
	void draw() {}

};


#endif