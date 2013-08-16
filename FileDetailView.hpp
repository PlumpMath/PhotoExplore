#ifndef LEAPIMAGE_FILESYSTEM_FILE_DETAIL_HPP_
#define LEAPIMAGE_FILESYSTEM_FILE_DETAIL_HPP_

#include "ImageDetailView.hpp"
#include "FileImagePanel.hpp"

namespace FileSystem {
	
	class FileDetailView : public ImageDetailView {

	private:	
		FileImagePanel * picturePanel;
		FileNode * imageNode;		

		int containerOffset;
		FileNode * parentNode;

	protected:
		DynamicImagePanel * getSiblingByOffset(int offset);
		
	public:
		void setPicturePanel(FileImagePanel * imagePanel);
		FileImagePanel * getPicturePanel();
	};

}

#endif