#ifndef LEAPIMAGE_FILESYSTEM_DIRECTORY_PANEL_HPP_
#define LEAPIMAGE_FILESYSTEM_DIRECTORY_PANEL_HPP_

#include "ContentPanel.hpp"
#include "FileNode.hpp"
#include "ViewOrchestrator.hpp"
#include "TextPanel.h"

namespace FileSystem 
{

	class DirectoryPanel : public ContentPanel, public ViewOwner {

	private:
		ViewGroup * fileGroup;
		TextPanel * nameText;
		FileNode * directoryNode;

		float currentDataPriority;

	public:
		DirectoryPanel();
		
		void setDataPriority(float dataPriority);
		void setDirectory(FileNode * node);
		FileNode * getDirectory();
		void viewOwnershipChanged(View * view, ViewOwner * newOwner);

	};
}

#endif