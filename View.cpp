#include "View.hpp"
#include "GraphicContext.hpp"
#include "GLImport.h"

View::View()
{
	desiredSize = cv::Size2f(0,0);
	layoutDirty = false;
	visible = true;
	enabled = true;
}

void View::setLayoutParams(cv::Size2f _desiredSize)
{
	this->desiredSize = _desiredSize;
}

void View::setLayoutParams(LayoutParams _params)
{
	this->desiredSize = _params.size;
	this->layoutParams = LayoutParams(_params);
}

LayoutParams View::getLayoutParams()
{
	return layoutParams;
}

void View::measure(cv::Size2f & size)
{
	if (desiredSize.width != 0 || desiredSize.height != 0)
		size = desiredSize;
}

void View::update()
{
	
	updateTaskMutex.lock();
	while (!updateThreadTasks.empty())
	{
		updateThreadTasks.front()();
		updateThreadTasks.pop();
	}
	updateTaskMutex.unlock();

	if (layoutDirty)
	{
		if (lastSize.width != 0 && lastSize.height != 0)
			layout(lastPosition,lastSize);
		//else if (desiredSize.width != 0 && desiredSize.height != 0)
			//layout(lastPosition,desiredSize);
		layoutDirty = false;
	}
}

void View::setVisible(bool _visible)
{
	this->visible = _visible;
}

bool View::isVisible()
{
	return this->visible;
}

cv::Size2f View::getMeasuredSize()
{
	return this->lastSize;
}

Leap::Vector View::getLastPosition()
{
	return this->lastPosition;
}

void View::postTask(boost::function<void()> task)
{
	updateTaskMutex.lock();
	updateThreadTasks.push(task);
	updateTaskMutex.unlock();
}

//ViewGroup implementation
void ViewGroup::draw()
{
	bool found;
	float visibleWidth = GraphicsContext::getInstance().getDrawHint("VisibleWidth", found);
	float offset = GraphicsContext::getInstance().getDrawHint("Offset", found);

	for (auto it = children.begin();it != children.end();it++)
	{
		if (found)
		{
			float lastX = (*it)->getLastPosition().x;
			float lastWidth = (*it)->getMeasuredSize().width;
			if ((lastWidth + lastX + offset < 0 || (lastX + offset) > visibleWidth))
				continue;
		}

		//int count = 0;
		//GLenum glError;
		//while ((glError = glGetError()) != GL_NO_ERROR)
		//{
		//	cout << "[" << count << "]" << "ViewGroup draw ERROR:" << glError << endl;
		//}

		if ((*it)->isVisible())
			(*it)->draw();

		//count = 0;
		//while ((glError = glGetError()) != GL_NO_ERROR)
		//{
		//	cout << "[" << count << "]" << "ViewGroup draw ERROR:" << glError << endl;
		//}
	}
}

bool ViewGroup::addChild(View * child)
{
	if (std::count(children.begin(),children.end(),child) == 0)
		children.push_back(child);
	else
		return false;
	return true;
}

void ViewGroup::clearChildren()
{
	this->lastSize = cv::Size2f(0,0);
	children.clear();
}

void ViewGroup::update()
{
	View::update();

	for (auto it = children.begin();it != children.end();it++)
	{
		if ((*it)->isVisible())
			(*it)->update();
	}
}

LeapElement * ViewGroup::elementAtPoint(int x, int y, int & elementStateFlags)
{
	LeapElement * hit = NULL;

	multimap<float,LeapElement*> lazyDepthMap;
	for (auto it = children.begin();it != children.end();it++)
	{
		View * test = (*it);
		if (test->isVisible() && test->isEnabled())
		{
			hit = test->elementAtPoint(x,y,elementStateFlags);

			if (hit != NULL)
			{
				float depth = hit->getZValue();
				lazyDepthMap.insert(make_pair(depth,hit));
			}
		}
	}

	if (lazyDepthMap.size() > 0)
		return (--lazyDepthMap.end())->second;
	else
		return View::elementAtPoint(x,y,elementStateFlags);

	//return hit;
}

vector<View*> * ViewGroup::getChildren()
{
	return &(this->children);
}

bool ViewGroup::remove(View * child)
{	
	auto r1 = std::find(children.begin(),children.end(),child);
	if (r1 != children.end())
	{
		children.erase(r1);
		return true;
	}
	return false;
}