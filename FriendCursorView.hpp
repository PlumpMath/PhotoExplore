#ifndef LEAPIMAGE_FRIEND_CURSOR_VIEW_HPP_
#define LEAPIMAGE_FRIEND_CURSOR_VIEW_HPP_

#include "AlbumCursorView.hpp"

class FriendCursorView : public AlbumCursorView {


private:
	FBNode * friendNode;
	FBAlbumPhotosCursor * albumCursor;	
	ImageDetailView * imageDetailView;

public:
	FriendCursorView();

	void setFriendNode(FBNode * node);
	void showPhoto(FBNode * photoNode);

	FBNode * getFriendNode();
	FBDataView * getDataView(FBNode * photoNode);
	
};

#endif