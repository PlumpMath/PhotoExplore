#include "FileBrowser.hpp"
#include <boost/filesystem.hpp>
#include "GraphicContext.hpp"
#include "LinearLayout.hpp"

#include "DataCursor.hpp"
#include <vector>

using namespace FileSystem;


FileBrowser::FileBrowser()
{
	directoryView = new DirectoryView();

	//FileNode * testNode = new FileNode(boost::filesystem::path("C:\\Users\\Adam\\Pictures"));

	//directoryView->setDirectory(testNode);

	auto fakeDataPaths = GlobalConfig::tree()->get_child("FakeDataMode.ImageDirectories");

	std::vector<DataNode*> dataVector;

	for (auto it=fakeDataPaths.begin(); it != fakeDataPaths.end(); it++)
	{
		string fakeDataPath = it->second.data();

		FileNode * dataNode = new FileNode(fakeDataPath);

		dataVector.push_back(dataNode);
	}
	
	directoryView->show(new VectorCursor(dataVector));
	directoryView->setViewFinishedCallback([](std::string s){});

	menuGroup = new LinearLayout();

	this->layoutDirty = true;
	layout(Vector(),cv::Size2f(GlobalConfig::ScreenWidth, GlobalConfig::ScreenHeight));
}


void FileBrowser::draw()
{
	directoryView->draw();
}

void FileBrowser::layout(Vector position, cv::Size2f size)
{	
	float menuHeight = ActivityView::getMenuHeight();
	float tutorialHeight = ActivityView::getTutorialHeight();

	Vector menuBarPosition = Vector();
	cv::Size2f menuBarSize = cv::Size2f(GlobalConfig::ScreenWidth-menuHeight, menuHeight);

	menuGroup->measure(menuBarSize);
	menuGroup->layout(menuBarPosition,menuBarSize);

	//homeButton->layout(homeButton->getLastPosition(),cv::Size2f(homeButton->getMeasuredSize().width,menuHeight));


	Vector contentPosition = Vector(0,menuHeight,0);
	cv::Size2f contentSize = cv::Size2f(GlobalConfig::ScreenWidth, GlobalConfig::ScreenHeight-(tutorialHeight+menuHeight));

	directoryView->layout(contentPosition,contentSize);

	if (!GraphicsContext::getInstance().IsBlurCurrentPass)
	{	
		menuGroup->draw();		
	}
}


void FileBrowser::update()
{
	View::update();

	directoryView->update();
}

LeapElement * FileBrowser::elementAtPoint(int x, int y, int & state)
{	
	LeapElement * hit = NULL;
	
	if (!GraphicsContext::getInstance().BlurRenderEnabled) //TODO: Unhack this
		hit = menuGroup->elementAtPoint(x,y,state);

	if (hit != NULL)
		return hit;

	if (directoryView != NULL)
		return directoryView->elementAtPoint(x,y,state);
	
	return NULL;
}