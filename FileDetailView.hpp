#ifndef LEAPIMAGE_FILESYSTEM_FILE_DETAIL_HPP_
#define LEAPIMAGE_FILESYSTEM_FILE_DETAIL_HPP_

#include "ImageDetailView.hpp"
#include "FileImagePanel.hpp"
#include "ViewOrchestrator.hpp"

namespace FileSystem {
	
	class FileDetailView : public ImageDetailView, public ViewOwner {

	private:	
		FileImagePanel * picturePanel;
		FileNode * imageNode;		

		int containerOffset;
		FileNode * parentNode;

	protected:
		DynamicImagePanel * getDetailedDataView(DataNode * node);
	
	};

}

#endif