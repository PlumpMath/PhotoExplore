#ifndef LEAPIMAGE_FRIEND_CURSOR_VIEW_HPP_
#define LEAPIMAGE_FRIEND_CURSOR_VIEW_HPP_

#include "AlbumCursorView.hpp"

namespace Facebook
{
	class FriendCursorView : public AlbumCursorView {


	private:
		FBNode * friendNode;
		FBAlbumPhotosCursor * albumCursor;	

	public:
		FriendCursorView();

		void setFriendNode(FBNode * node);
		void showPhoto(FBNode * photoNode);

		void childPanelClicked(FBNode * childNode);

		FBNode * getFriendNode();
		View * getDataView(DataNode * photoNode);
		void setItemPriority(float priority, View * itemView);
	
	};
}

#endif