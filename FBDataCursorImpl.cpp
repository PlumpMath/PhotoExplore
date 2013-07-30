#include "FBDataCursor.hpp"
#include "Logger.hpp"

FBNode * FBSimpleEdgeCursor::getNext()
{
	if (state == FBDataCursor::Loading || state == FBDataCursor::Finished)
		return NULL;

	FBNode * result = nextItem;
	if (nextItem == NULL)
	{
		if (state = FBDataCursor::Local)
			state = FBDataCursor::Local;	

		auto itemNodes = node->Edges.get<EdgeTypeIndex>().equal_range(boost::make_tuple(edgeName));
		if (itemNodes.first != itemNodes.second)
		{
			for (;itemNodes.first != itemNodes.second; itemNodes.first++)
			{
				FBNode * check = itemNodes.first->Node;
				if (check != NULL)
					break;		
			}
			nextItem = itemNodes.first->Node;
		}
		else if (state == FBDataCursor::Local)
		{
			int currentCount =  node->Edges.get<EdgeTypeIndex>().count(edgeName);
			loadItems(currentCount + itemsPerRequest);					
		}
		else if (state == FBDataCursor::Ended)
		{
			state = FBDataCursor::Finished;
		}
	}
	else
	{
		auto itemNodes = node->Edges.get<EdgeTypeIndex>().find(boost::make_tuple(edgeName,nextItem->getNumericId()));
		if (itemNodes != node->Edges.get<EdgeTypeIndex>().end())
		{
			itemNodes++;
			for (;itemNodes != node->Edges.get<EdgeTypeIndex>().end(); itemNodes++)
			{
				FBNode * check = itemNodes->Node;
				if (check != NULL && check->getNodeType().compare(edgeName) == 0)
					break;		
			}

			if (itemNodes != node->Edges.get<EdgeTypeIndex>().end())
			{
				nextItem = (itemNodes)->Node;
			}
			else if (state == FBDataCursor::Local)
			{
				int currentCount =  node->Edges.get<EdgeTypeIndex>().count(edgeName);
				loadItems(currentCount + itemsPerRequest);					
			}
			else if (state == FBDataCursor::Ended)
			{
				state = FBDataCursor::Finished;
			}
		}
		else
		{
			result = NULL;
		}
	}		
	return result;
}

void FBSimpleEdgeCursor::reset()
{
	nextItem = NULL;
	state = FBDataCursor::Local;
}

void FBFriendsCursor::loadItems(int friends)
{
	if (state == FBDataCursor::Loading || state == FBDataCursor::Ended)
	{		
		return;
	}

	int currentFriendCount = node->Edges.get<EdgeTypeIndex>().count("friends");
	Logger::stream("FBFriendsCursor","INFO") << "Loading " << friends << " more friends. CurrentCount = " << currentFriendCount << endl;

	if (friends > currentFriendCount)
	{		
		state = FBDataCursor::Loading;

		stringstream loadstr;
		loadstr << node->getId() << "?fields=";
		loadstr << "friends.offset(" << currentFriendCount << ").limit(" << (friends-currentFriendCount) << ").fields(id,name)";

		FBFriendsCursor * v = this;
		FBDataSource::instance->loadField(node,loadstr.str(),"",[v,currentFriendCount](FBNode * _node){
						
			if (v->state == FBDataCursor::Loading)
			{
				int newFriendCount = _node->Edges.get<EdgeTypeIndex>().count("friends");
				if (newFriendCount <= currentFriendCount)
				{				
					Logger::stream("FBFriendsCursor","ERROR") << "Reached end of items" << endl;
					v->state = FBDataCursor::Ended;
				}
				else
				{
					v->state = FBDataCursor::Local;
					v->getNext();
				}

				if (!v->cursorChangedCallback.empty())
					v->cursorChangedCallback();
			}
			else
			{
				Logger::stream("FBFriendsCursor","ERROR") << "Load completed while not loading" << endl;
			}
		});
	}
}


void FBUserAlbumsCursor::loadItems(int albums)
{
	if (state == FBDataCursor::Loading || state == FBDataCursor::Ended)
	{		
		return;
	}

	int currentCount = node->Edges.get<EdgeTypeIndex>().count("albums");

	if (albums > currentCount)
	{		
		state = FBDataCursor::Loading;

		stringstream loadstr;
		loadstr << node->getId() << "?fields=";
		loadstr << "albums.offset(" << currentCount << ").limit(" << (albums-currentCount) << ").fields(id,name,photos.fields(id,name,images,album).limit(4))";

		FBUserAlbumsCursor * v = this;
		FBDataSource::instance->loadField(node,loadstr.str(),"",[v,currentCount](FBNode * _node){
						
			if (v->state == FBDataCursor::Loading)
			{
				int newFriendCount = _node->Edges.get<EdgeTypeIndex>().count("albums");
				if (newFriendCount <= currentCount)
				{				
					Logger::stream("FBUserAlbumsCursor","ERROR") << "Reached end of items" << endl;
					v->state = FBDataCursor::Ended;
				}
				else
				{
					v->state = FBDataCursor::Local;
					v->getNext();
				}

				if (!v->cursorChangedCallback.empty())
					v->cursorChangedCallback();
			}
			else
			{
				Logger::stream("FBUserAlbumsCursor","ERROR") << "Load completed while not loading" << endl;
			}
		});
	}
}


void FBAlbumPhotosCursor::loadItems(int photos)
{
	if (state == FBDataCursor::Loading || state == FBDataCursor::Ended)
	{		
		return;
	}

	int currentCount = node->Edges.get<EdgeTypeIndex>().count("photos");

	if (photos > currentCount)
	{		
		state = FBDataCursor::Loading;

		stringstream loadstr;
		loadstr << node->getId() << "?fields=";
		loadstr << "photos.offset(" << currentCount << ").limit(" << (photos-currentCount) << ").fields(id,name,images,album)";

		FBAlbumPhotosCursor * v = this;
		FBDataSource::instance->loadField(node,loadstr.str(),"",[v,currentCount](FBNode * _node){
						
			if (v->state == FBDataCursor::Loading)
			{
				int newFriendCount = _node->Edges.get<EdgeTypeIndex>().count("albums");
				if (newFriendCount <= currentCount)
				{				
					Logger::stream("FBAlbumPhotosCursor","ERROR") << "Reached end of items" << endl;
					v->state = FBDataCursor::Ended;
				}
				else
				{
					v->state = FBDataCursor::Local;
					v->getNext();
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
