#ifndef LEAPIMAGE_DATA_LIST_VIEW_HPP_
#define LEAPIMAGE_DATA_LIST_VIEW_HPP_

#include "View.hpp"
#include "ActivityView.hpp"
#include "ScrollingView.hpp"
#include "ScrollBar.hpp"
#include "DataCursor.hpp"
#include "TextPanel.h"
#include "DataNode.hpp"

class DataListActivity : public ActivityView {

protected: 	
	ViewGroup * itemGroup;
	ScrollingView * itemScroll;	
	ScrollBar * scrollBar;
	
	TextPanel * loadIndicator;	
	TextPanel * titlePanel;

	DataCursor * cursor;

	int rowCount;
	float currentRightBoundary,lastUpdatePos;

	map<DataNode*,View*> items;	
	
	Timer loadingTime;
	int loadIndicatorState;
		
	void setTitlePanel(TextPanel * titlePanel);
	void updateLoading();
	void updatePriorities();

	virtual void addNode(DataNode * node);

	virtual View * getDataView(DataNode * node) = 0;
	virtual void setItemPriority(float priority, View * itemView) = 0;
	virtual void refreshDataView(DataNode * node, View * view) = 0;

public:
	DataListActivity(int rowCount);
	~DataListActivity();

	void onGlobalFocusChanged(bool isFocused);
	
	virtual void show(DataCursor * cursor);
	virtual void suspend();
	virtual void resume();

	void setRowCount(int rowCount);
	int getRowCount();

	void layout(Vector position, cv::Size2f size);
	void draw();
	void update();
};


#endif