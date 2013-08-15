#ifndef LEAPIMAGE_FIELSYSTEM_FILE_DATA_CURSOR_HPP_
#define LEAPIMAGE_FIELSYSTEM_FILE_DATA_CURSOR_HPP_

#include "DataCursor.hpp"
#include "FileNode.hpp"

namespace FileSystem 
{

	class DirectoryCursor : public DataCursor {

	protected:
		FileNode * node;	
		FileNode * nextItem;
		int nextItemIndex;

		int filesPerLoad;

	public:
		DirectoryCursor(FileNode * _directoryNode) :
			node(_directoryNode)
		{
			filesPerLoad = 2000;
			nextItemIndex = 0;
		}

		void loadItems(int items);
		void reset();

		DataNode * getNext();
	};
}

#endif