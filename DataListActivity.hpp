#ifndef LEAPIMAGE_DATA_LIST_VIEW_HPP_
#define LEAPIMAGE_DATA_LIST_VIEW_HPP_

#include "View.hpp"
#include "ActivityView.hpp"
#include "ScrollingView.hpp"
#include "ScrollBar.hpp"
#include "FBDataCursor.hpp"
#include "FBDataView.hpp"
#include "TextPanel.h"

using namespace Facebook;

class DataListActivity : public ActivityView {

protected: 	
	ViewGroup * itemGroup;
	ScrollingView * itemScroll;	
	ScrollBar * scrollBar;
	
	TextPanel * loadIndicator;	
	TextPanel * titlePanel;

	FBDataCursor * cursor;

	int rowCount;
	float currentRightBoundary,lastUpdatePos;

	map<FBNode*,FBDataView*> items;	

	virtual void addNode(FBNode * node);
	virtual FBDataView * getDataView(FBNode * node) = 0;
	
	void setTitlePanel(TextPanel * titlePanel);
	void updateLoading();
	void updatePriorities();

public:
	DataListActivity(int rowCount);
	~DataListActivity();

	void onGlobalFocusChanged(bool isFocused);
	
	virtual void show(FBDataCursor * cursor);
	virtual void suspend();
	virtual void resume();

	void layout(Vector position, cv::Size2f size);
	void draw();
	void update();
};


#endif