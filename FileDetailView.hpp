#ifndef LEAPIMAGE_FACEBOOK_PICTUREDETAIL_HPP_
#define LEAPIMAGE_FACEBOOK_PICTUREDETAIL_HPP_

#include "ImageDetailView.hpp"
#include "FileImagePanel.hpp"

namespace FileSystem {
	
	class FileDetailView : public ImageDetailView {

	private:	
		FileImagePanel * picturePanel;
		FileNode * imageNode;		
		
	public:
		void setPicturePanel(FileImagePanel * imagePanel);
		FileImagePanel * getPicturePanel();
	};

}

#endif