#ifndef LEAPIMAGE_ALBUM_CURSOR_VIEW_HPP_
#define LEAPIMAGE_ALBUM_CURSOR_VIEW_HPP_

#include "DataListActivity.hpp"
#include "FBDataCursor.hpp"
#include "PictureDetailView.hpp"
#include "ViewOrchestrator.hpp"

namespace Facebook 
{


class AlbumCursorView : public DataListActivity,public ViewOwner
{
private:
	FBNode * albumOwner;
	FBAlbumPhotosCursor * albumCursor;	
	PictureDetailView * imageDetailView;
	
public:
	AlbumCursorView();

	void setAlbumOwner(FBNode * node);
	void showPhoto(FBNode * photoNode);

	FBNode * getAlbumOwner();
	
	virtual View * getDataView(DataNode * photoNode);
	virtual void setItemPriority(float priority, View * itemView);
	
	void viewOwnershipChanged(View * view, ViewOwner * newOwner);
	void getTutorialDescriptor(vector<string> & tutorial);
	void setFinishedCallback(const boost::function<void(std::string)> & callback);
	
	void onGlobalGesture(const Controller & controller, std::string gestureType);

	void layout(Vector position, cv::Size2f size);
};

}


#endif