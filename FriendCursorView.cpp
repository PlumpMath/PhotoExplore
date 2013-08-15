#include "FriendCursorView.hpp"
#include "AlbumPanel.hpp"
#include "FacebookDataDisplay.hpp"

using namespace Facebook;

FriendCursorView::FriendCursorView() : AlbumCursorView()
{
}

void FriendCursorView::setFriendNode(FBNode * _friendNode)
{
	this->friendNode = _friendNode;

	FBUserAlbumsCursor * userAlbumsCursor = new FBUserAlbumsCursor(friendNode);	
	FBUserPhotosCursor * userPhotosCursor = new FBUserPhotosCursor(friendNode);
		
	InterleavingCursor * iCursor = new InterleavingCursor(userAlbumsCursor,userPhotosCursor);

	userAlbumsCursor->cursorChangedCallback = [iCursor](){

		if (!iCursor->cursorChangedCallback.empty())
			iCursor->cursorChangedCallback();
	};

	userPhotosCursor->cursorChangedCallback = [iCursor](){

		if (!iCursor->cursorChangedCallback.empty())
			iCursor->cursorChangedCallback();
	};
	
	this->show(iCursor);

	userAlbumsCursor->getNext();
	userPhotosCursor->getNext();


	TextPanel * friendNameHeading = dynamic_cast<TextPanel*>(ViewOrchestrator::getInstance()->requestView(friendNode->getId() + "/name", this));

	if (friendNameHeading == NULL)
	{
		friendNameHeading = new TextPanel(friendNode->getAttribute("name"));
		ViewOrchestrator::getInstance()->registerView(friendNode->getId() + "/name",friendNameHeading,this);
	}
	friendNameHeading->setStyle(GlobalConfig::tree()->get_child("FriendDetailView.Title"));

	setTitlePanel(friendNameHeading);
}

FBNode * FriendCursorView::getFriendNode()
{
	return this->friendNode;
}


void FriendCursorView::setItemPriority(float priority, View * itemView)
{
	AlbumPanel * album = dynamic_cast<AlbumPanel*>(itemView);
	if (album != NULL)
		album->setDataPriority(priority);
	else
		AlbumCursorView::setItemPriority(priority,itemView);
}



View * FriendCursorView::getDataView(DataNode * dnode)
{
	FBNode * node = (FBNode*)dnode;
	if (node->getNodeType() == NodeType::FacebookImage)
		return AlbumCursorView::getDataView(node);
	else
	{		
		View * item = ViewOrchestrator::getInstance()->requestView(node->getId(),this);

		AlbumPanel * ap = dynamic_cast<AlbumPanel*>(item);

		if (ap == NULL)
		{
			ap = new AlbumPanel();
			ViewOrchestrator::getInstance()->registerView(node->getId(),ap,this);
		}

		ap->show(node);				

		ap->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(5,5,5,5)));
		ap->elementClickedCallback = [node](LeapElement * element){
			FacebookDataDisplay::getInstance()->displayNode(node,"");
		};

		return ap;
	}
}


void FriendCursorView::showPhoto(FBNode * photoNode)
{
	((PicturePanel*)getDataView(photoNode))->elementClicked();
}
