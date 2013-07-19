#ifndef LEAPIMAGE_LAYOUT_HPP_
#define LEAPIMAGE_LAYOUT_HPP_

#include <vector>
#include <queue>

#include <Leap.h>
#include <opencv2/opencv.hpp>
#include "PointableElementManager.h"
#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>


using namespace Leap;
using namespace std;

struct LayoutParams {
		
	cv::Size2f size;
	cv::Vec4f padding;

	LayoutParams() :
		size(),
		padding()
	{

	}

	LayoutParams(const LayoutParams & copy) :
		size(copy.size),
		padding(copy.padding)
	{
	}


	LayoutParams(cv::Size2f _size) :
		size(_size)
	{
	}

	LayoutParams(cv::Size2f _size, cv::Vec4f _padding) :
		size(_size),
		padding(_padding)
	{
	}

	LayoutParams & operator= (const LayoutParams & other)
    {
		size = other.size;
		padding = other.padding;
		return *this;
	}

};

class View : public LeapElement {

protected:
	bool layoutDirty, visible, clickable, enabled;
	Leap::Vector lastPosition;
	cv::Size2f lastSize, desiredSize;
	LayoutParams layoutParams;
	
	boost::mutex updateTaskMutex;
	std::queue<boost::function<void()> > updateThreadTasks;

public:
	View();

	virtual void setLayoutParams(cv::Size2f desiredSize);
	virtual void setLayoutParams(LayoutParams params);

	LayoutParams getLayoutParams();

	virtual void measure(cv::Size2f & size);	
	virtual void update();

	virtual void setVisible(bool visible);
	virtual bool isVisible();
	
	virtual void layout(Vector position, cv::Size2f size) = 0;
	virtual void draw() = 0;

	void postTask(boost::function<void()> task);

	cv::Size2f getMeasuredSize();
	Leap::Vector getLastPosition();
};

class ViewGroup : public View {

protected:
	vector<View*> children;

public:	
	virtual void draw();
	virtual bool addChild(View * child);
	virtual void clearChildren();
	virtual void update();
	virtual LeapElement * elementAtPoint(int x, int y, int & elementStateFlags);
	virtual bool remove(View * child);

	vector<View*> * getChildren();

};


#endif