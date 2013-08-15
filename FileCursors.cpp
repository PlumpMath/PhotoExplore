#include "FileDataCursors.hpp"
#include "FileLoader.hpp"
#include "Logger.hpp"

using namespace FileSystem;

DataNode * DirectoryCursor::getNext()
{
	if (state == DataCursor::Loading || state == DataCursor::Finished)
		return NULL;

	FileNode * result = nextItem;
	if (nextItem == NULL)
	{
		if (state == DataCursor::Local)
			state = DataCursor::Local;	

		auto itemNodes = node->Files.get<RandFileIndex>().begin();

		if (itemNodes != node->Files.get<RandFileIndex>().end())
		{
			nextItem = itemNodes->Node;
			nextItemIndex = 0;
		}
		else if (state == DataCursor::Local)
		{
			int currentCount =  node->Files.size();
			loadItems(currentCount + filesPerLoad);					
		}
		else if (state == DataCursor::Ended)
		{
			state = DataCursor::Finished;
		}
	}
	else
	{
		
		if (nextItemIndex < node->Files.get<RandFileIndex>().size())
		{			
			nextItemIndex++;

			if (nextItemIndex < node->Files.get<RandFileIndex>().size())
			{
				nextItem =  node->Files.get<RandFileIndex>().at(nextItemIndex).Node;
			}
			else if (state == DataCursor::Local)
			{
				int currentCount =  node->Files.size();
				loadItems(currentCount + filesPerLoad);					
			}
			else if (state == DataCursor::Ended)
			{
				state = DataCursor::Finished;
			}
		}
		else
		{
			if (state == DataCursor::Ended)
			{
				state = DataCursor::Finished;
			}
			result = NULL;
		}
	}		
	return result;
}


void DirectoryCursor::reset()
{
	nextItem = NULL;
	nextItemIndex = 0;
	state = DataCursor::Local;
}



void DirectoryCursor::loadItems(int photos)
{
	if (state == DataCursor::Loading || state == DataCursor::Ended)
	{		
		return;
	}

	int currentCount = node->Files.size();

	if (photos > currentCount)
	{		
		state = DataCursor::Loading;

		DirectoryCursor * v = this;
		FileLoader::getInstance().loadFiles(node,0,[v,currentCount](FileNode * _node){
					
			if (v->state == DataCursor::Loading)
			{
				int newItemCount = _node->Files.size();
				if (newItemCount <= currentCount)
				{				
					v->state = DataCursor::Ended;
				}
				else
				{
					v->state = DataCursor::Local;					
				}

				if (!v->cursorChangedCallback.empty())
					v->cursorChangedCallback();
			}
			else
			{
				Logger::stream("FBAlbumPhotosCursor","ERROR") << "Load completed while not loading" << endl;
			}
		});
	}
}