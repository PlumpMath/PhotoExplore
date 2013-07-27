#ifndef LEAPIMAGE_DATA_LIST_VIEW_HPP_
#define LEAPIMAGE_DATA_LIST_VIEW_HPP_

#include "View.hpp"
#include "ActivityView.hpp"
#include "ScrollingView.hpp"
#include "ScrollBar.hpp"
#include "FBDataCursor.hpp"
#include "FBDataView.hpp"

using namespace Facebook;

class DataListActivity : public ActivityView {

private:
	void updateLoading();
	void updatePriorities();

protected: 	
	ViewGroup * itemGroup;
	ScrollingView * itemScroll;	
	ScrollBar * scrollBar;
	View * loadIndicator;

	FBDataCursor * cursor;


	int rowCount;
	float currentRightBoundary,lastUpdatePos;


	map<FBNode*,FBDataView*> items;
	

	virtual void addNode(FBNode * node);
	virtual FBDataView * getDataView(FBNode * node) = 0;

public:
	DataListActivity(int rowCount);
	~DataListActivity();
	
	virtual void show(FBDataCursor * cursor);
	virtual void suspend();

	void layout(Vector position, cv::Size2f size);
	void draw();
	void update();


};


#endif