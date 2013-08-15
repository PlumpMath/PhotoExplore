#ifndef LEAPIMAGE_FILESYSTEM_FILEIMAGE_PANEL_HPP_
#define LEAPIMAGE_FILESYSTEM_FILEIMAGE_PANEL_HPP_

#include "DynamicImagePanel.hpp"
#include "FileNode.hpp"

namespace FileSystem {

	class FileImagePanel : public DynamicImagePanel  {
		
	protected:
		FileNode * fileNode;	

		void prepareResource();

	public:	
		void show(FileNode * node);
		FileNode * getNode();

	};

}


#endif