#ifndef LEAPIMAGE_ALBUM_CURSOR_VIEW_HPP_
#define LEAPIMAGE_ALBUM_CURSOR_VIEW_HPP_

#include "DataListActivity.hpp"
#include "FBDataCursor.hpp"
#include "ImageDetailView.hpp"
#include "ViewOrchestrator.hpp"

class AlbumCursorView : public DataListActivity,public ViewOwner
{
private:
	FBNode * albumOwner;
	FBAlbumPhotosCursor * albumCursor;	
	ImageDetailView * imageDetailView;

public:
	AlbumCursorView();

	void setAlbumOwner(FBNode * node);
	FBNode * getAlbumOwner();
	FBDataView * getDataView(FBNode * photoNode);

	
	void viewOwnershipChanged(View * view, ViewOwner * newOwner);
	void getTutorialDescriptor(vector<string> & tutorial);
	void setFinishedCallback(const boost::function<void(std::string)> & callback);
	
	void onGlobalGesture(const Controller & controller, std::string gestureType);
};


#endif