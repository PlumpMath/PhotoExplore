#ifndef LEAPIMAGE_CONTENT_PANEL_HPP_
#define LEAPIMAGE_CONTENT_PANEL_HPP_

#include "PanelBase.h"

class ContentPanel : public PanelBase {
	
protected:
	View * contentView;
	bool eatEvents;

public:	
	ContentPanel();
	ContentPanel(View * contentView);
	void drawContent(Vector drawPosition, float drawWidth, float drawHeight);

	void setContentView(View * contentView);
	View * getContentView();

	void setEventHandlingMode(bool eatEvents);
	bool getEventHandlingMode();

	void measure(cv::Size2f & measuredSize);

	void layout(Vector layoutPosition, cv::Size2f layoutSize);
	
	LeapElement * elementAtPoint(int x, int y, int & elementStateFlags);
};


#endif