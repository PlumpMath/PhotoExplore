#include "DirectoryView.hpp"
#include "DirectoryPanel.hpp"
#include "FileDataCursors.hpp"

using namespace FileSystem;

DirectoryView::DirectoryView() : DataListActivity(2)
{	
	childDirectory = NULL;

	imageDetailView = new FileDetailView();
	imageDetailView->setVisible(false);
	imageDetailView->setFinishedCallback([this](string a){
		LeapInput::getInstance()->releaseGlobalGestureFocus(this->imageDetailView);	
		this->imageDetailView->setVisible(false);
		this->layoutDirty = true;						
	});

	addChild(imageDetailView);
}

void DirectoryView::setDirectory(FileNode * _directory)
{
	this->directoryNode = _directory;

	DataCursor * photosCursor = new DirectoryCursor(directoryNode);
	photosCursor->getNext();

	this->show(photosCursor);
	
	TextPanel * directoryName = dynamic_cast<TextPanel*>(ViewOrchestrator::getInstance()->requestView(directoryNode->filePath.string() + "///name", this));

	if (directoryName == NULL)
	{
		directoryName = new TextPanel(directoryNode->filename);
		ViewOrchestrator::getInstance()->registerView(directoryNode->filePath.string() + "///name",directoryName,this);
	}

	directoryName->setStyle(GlobalConfig::tree()->get_child("AlbumDetailView.Title"));

	setTitlePanel(directoryName);
}

FileNode * DirectoryView::getDirectory()
{
	return this->directoryNode;
}


void DirectoryView::setItemPriority(float priority, View * itemView)
{
	FileImagePanel * picture = dynamic_cast<FileImagePanel*>(itemView);
	if (picture != NULL)
		picture->setDataPriority(priority);
}

View * DirectoryView::getDataView(DataNode * dataNode)
{
	FileNode * node = (FileNode*)dataNode;

	if (node->isDirectory)
	{
		View * v= ViewOrchestrator::getInstance()->requestView(node->filePath.string(), this);

		DirectoryPanel * dirPanel = dynamic_cast<DirectoryPanel*>(v);
		
		if (dirPanel == NULL)
		{
			dirPanel = new DirectoryPanel();
			dirPanel->setDirectory(node);
			ViewOrchestrator::getInstance()->registerView(node->filePath.string(),dirPanel, this);
		}

		dirPanel->elementClickedCallback = [node,this](LeapElement * clicked){

			if (childDirectory == NULL)
			{
				childDirectory = new DirectoryView();
				DirectoryView * me = this;
				childDirectory->setViewFinishedCallback([me](string s){
					me->childDirectory->setVisible(false);
					me->layoutDirty = true;
				});
			}
			childDirectory->setVisible(true);
			childDirectory->setDirectory(node);			
			childDirectory->layout(this->lastPosition,this->lastSize);
		};

		return dirPanel;
	}
	else
	{
		View * v= ViewOrchestrator::getInstance()->requestView(node->filePath.string(), this);

		FileImagePanel * item = NULL;
		if (v == NULL)
		{
			item = new FileImagePanel();
			item->show(node);
			ViewOrchestrator::getInstance()->registerView(node->filePath.string(),item, this);
		}
		else
		{
			item = dynamic_cast<FileImagePanel*>(v);
		}

		item->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(5,5,5,5)));
		item->layout(Vector(lastSize.width-itemScroll->getFlyWheel()->getPosition(),lastSize.height*.5f,-10),cv::Size2f(lastSize.height*(1.0f/((float)rowCount)),10));
		item->setClickable(true);
		item->setVisible(true);

		item->elementClickedCallback = [this,node](LeapElement * clicked){
			this->itemScroll->getFlyWheel()->impartVelocity(0);			

			this->imageDetailView->notifyOffsetChanged(Vector((float)this->itemScroll->getFlyWheel()->getCurrentPosition(),0,0));

			DirectoryCursor * dc = new DirectoryCursor(this->directoryNode);
			dc->getNext();

			WrappingBDCursor * wrap = new WrappingBDCursor(dc);
			wrap->fastForward(node);
			
			DirectoryCursor * revDc = new DirectoryCursor(this->directoryNode);
			revDc->getNext();

			WrappingBDCursor * revWrap = new WrappingBDCursor(revDc);
			revWrap->fastForward(node);

			this->imageDetailView->setCursor(revWrap,wrap);
			this->imageDetailView->setVisible(true);
			LeapInput::getInstance()->requestGlobalGestureFocus(this->imageDetailView);
			this->layoutDirty = true;			
		};

		return item;
	}
}


void DirectoryView::showPhoto(FileNode * photoNode)
{
	((FileImagePanel*)getDataView(photoNode))->elementClicked();
}

void DirectoryView::getTutorialDescriptor(vector<string> & tutorial)
{
	tutorial.push_back("swipe");
	tutorial.push_back("point_stop");
	tutorial.push_back("shake");
}

void DirectoryView::setFinishedCallback(const boost::function<void(std::string)> & callback)
{
	viewFinishedCallback = callback;
}

void DirectoryView::viewOwnershipChanged(View * view, ViewOwner * newOwner)
{	
	if (newOwner != imageDetailView)
	{
		itemGroup->remove(view);
	}
}


void DirectoryView::onGlobalGesture(const Controller & controller, std::string gestureType)
{
	if (gestureType.compare("shake") == 0)
	{
		viewFinishedCallback("done");
	} 
	else if (gestureType.compare("pointing") == 0)
	{
		itemScroll->getFlyWheel()->impartVelocity(0);
	}
}

void DirectoryView::layout(Vector position, cv::Size2f size)
{
	if (childDirectory != NULL && childDirectory->isVisible())
	{
		childDirectory->layout(position,size);
		return;
	}

	DataListActivity::layout(position,size);
	
	if (imageDetailView->isVisible())
	{
		imageDetailView->layout(position,size);
	}
}


void DirectoryView::draw()
{
	if (childDirectory != NULL && childDirectory->isVisible())
	{
		childDirectory->draw();		
	}
	else
	{
		if (imageDetailView->isVisible())
		{		
			imageDetailView->draw();
		}

		DataListActivity::draw();	
	}
}

void DirectoryView::update()
{
	if (childDirectory != NULL && childDirectory->isVisible())
	{
		childDirectory->update();		
	}
	else
	{
		DataListActivity::update();	
	}
}

LeapElement * DirectoryView::elementAtPoint(int x, int y, int & state)
{
	if (childDirectory != NULL && childDirectory->isVisible())
	{
		return childDirectory->elementAtPoint(x,y,state);
	}

	return DataListActivity::elementAtPoint(x,y,state);
}