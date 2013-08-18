﻿#include "AlbumCursorView.hpp"
#include "ViewOrchestrator.hpp"

using namespace Facebook;

AlbumCursorView::AlbumCursorView() : DataListActivity(2)
{	
	imageDetailView = new PictureDetailView();
	imageDetailView->setVisible(false);
	imageDetailView->setFinishedCallback([this](string a){
		LeapInput::getInstance()->releaseGlobalGestureFocus(this->imageDetailView);	
		this->imageDetailView->setVisible(false);
		this->layoutDirty = true;						
	});

	addChild(imageDetailView);
}

void AlbumCursorView::setAlbumOwner(FBNode * _albumOwner)
{
	this->albumOwner = _albumOwner;

	FBAlbumPhotosCursor * photosCursor = new FBAlbumPhotosCursor(albumOwner);
	photosCursor->getNext();

	this->show(photosCursor);
	
	TextPanel * albumHeading = dynamic_cast<TextPanel*>(ViewOrchestrator::getInstance()->requestView(albumOwner->getId() + "/name", this));

	if (albumHeading == NULL)
	{
		albumHeading = new TextPanel(albumOwner->getAttribute("name"));
		ViewOrchestrator::getInstance()->registerView(albumOwner->getId() + "/name",albumHeading,this);
	}
	albumHeading->setStyle(GlobalConfig::tree()->get_child("AlbumDetailView.Title"));

	setTitlePanel(albumHeading);
}

FBNode * AlbumCursorView::getAlbumOwner()
{
	return this->albumOwner;
}


void AlbumCursorView::setItemPriority(float priority, View * itemView)
{
	PicturePanel * picture = dynamic_cast<PicturePanel*>(itemView);
	if (picture != NULL)
		picture->setDataPriority(priority);
}

View * AlbumCursorView::getDataView(DataNode * dataNode)
{
	FBNode * node = (FBNode*)dataNode;

	View * v= ViewOrchestrator::getInstance()->requestView(node->getId(), this);

	PicturePanel * item = NULL;
	if (v == NULL)
	{
		item = new PicturePanel();
		item->show(node);
		ViewOrchestrator::getInstance()->registerView(node->getId(),item, this);
	}
	else
	{
		item = dynamic_cast<PicturePanel*>(v);
	}

	item->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(5,5,5,5)));
	item->layout(Vector(lastSize.width-itemScroll->getFlyWheel()->getPosition(),lastSize.height*.5f,-10),cv::Size2f(lastSize.height*(1.0f/((float)rowCount)),10));
	item->setClickable(true);
	item->setVisible(true);

	item->elementClickedCallback = [this,node](LeapElement * clicked){
		this->itemScroll->getFlyWheel()->impartVelocity(0);			

		this->imageDetailView->notifyOffsetChanged(Vector((float)this->itemScroll->getFlyWheel()->getCurrentPosition(),0,0));

		WrappingBDCursor * wrapCursor = new WrappingBDCursor(new FBAlbumPhotosCursor(albumOwner));
		wrapCursor->fastForward(node);

		this->imageDetailView->setCursor(wrapCursor);										
		this->imageDetailView->setVisible(true);
		LeapInput::getInstance()->requestGlobalGestureFocus(this->imageDetailView);
		this->layoutDirty = true;			
	};

	return item;
}


void AlbumCursorView::showPhoto(FBNode * photoNode)
{
	((PicturePanel*)getDataView(photoNode))->elementClicked();
}

void AlbumCursorView::getTutorialDescriptor(vector<string> & tutorial)
{
	tutorial.push_back("swipe");
	tutorial.push_back("point_stop");
	tutorial.push_back("shake");
}

void AlbumCursorView::setFinishedCallback(const boost::function<void(std::string)> & callback)
{
	viewFinishedCallback = callback;
}

void AlbumCursorView::viewOwnershipChanged(View * view, ViewOwner * newOwner)
{	
	auto r1 = std::find(itemGroup->getChildren()->begin(),itemGroup->getChildren()->end(),view);
	if (r1 != itemGroup->getChildren()->end())
		itemGroup->getChildren()->erase(r1);
}


void AlbumCursorView::onGlobalGesture(const Controller & controller, std::string gestureType)
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

void AlbumCursorView::layout(Vector position, cv::Size2f size)
{
	DataListActivity::layout(position,size);
	
	if (imageDetailView->isVisible())
	{
		imageDetailView->layout(position,size);
	}
}