#include "FBDataCursor.hpp"
#include "Logger.hpp"

FBNode * FBSimpleEdgeCursor::getNext()
{
	if (ascending)
		return getNextAsc();
	else
		return getNextDesc();
}

typedef EdgeTypeRowNumIndex AscendingIndexType;

FBNode * FBSimpleEdgeCursor::getNextAsc()
{
	if (state == FBDataCursor::Loading || state == FBDataCursor::Finished)
		return NULL;

	FBNode * result = nextItem;
	if (nextItem == NULL)
	{
		if (state == FBDataCursor::Local)
			state = FBDataCursor::Local;	

		auto itemNodes = node->Edges.get<AscendingIndexType>().equal_range(boost::make_tuple(edgeName));
		if (itemNodes.first != itemNodes.second)
		{
			for (;itemNodes.first != itemNodes.second; itemNodes.first++)
			{
				FBNode * check = itemNodes.first->Node;
				if (check != NULL)
					break;		
			}
			nextItemNumber = itemNodes.first->RowNumber;
			nextItem = itemNodes.first->Node;
		}
		else if (state == FBDataCursor::Local)
		{
			int currentCount =  node->Edges.get<AscendingIndexType>().count(edgeName);
			loadItems(currentCount + itemsPerRequest);					
		}
		else if (state == FBDataCursor::Ended)
		{
			state = FBDataCursor::Finished;
		}
	}
	else
	{
		auto itemNodes = node->Edges.get<AscendingIndexType>().find(boost::make_tuple(edgeName,nextItemNumber));//nextItem->getTimestamp(), nextItem->getNumericId()));
		
		if (itemNodes != node->Edges.get<AscendingIndexType>().end())
		{
			itemNodes++;
			for (;itemNodes != node->Edges.get<AscendingIndexType>().end(); itemNodes++)
			{
				FBNode * check = itemNodes->Node;
				if (check != NULL && check->getNodeType().compare(edgeName) == 0)
					break;		
			}

			if (itemNodes != node->Edges.get<AscendingIndexType>().end())
			{
				nextItemNumber = itemNodes->RowNumber;
				nextItem = (itemNodes)->Node;
			}
			else if (state == FBDataCursor::Local)
			{
				int currentCount =  node->Edges.get<AscendingIndexType>().count(edgeName);
				loadItems(currentCount + itemsPerRequest);					
			}
			else if (state == FBDataCursor::Ended)
			{
				state = FBDataCursor::Finished;
			}
		}
		else
		{
			if (state == FBDataCursor::Ended)
			{
				state = FBDataCursor::Finished;
			}
			result = NULL;
		}
	}		
	return result;
}

FBNode * FBSimpleEdgeCursor::getNextDesc()
{
	if (state == FBDataCursor::Loading || state == FBDataCursor::Finished)
		return NULL;

	FBNode * result = nextItem;
	if (nextItem == NULL)
	{
		if (state == FBDataCursor::Local)
			state = FBDataCursor::Local;	

		auto itemNodes = node->Edges.get<TimeOrderedEdgeType>().equal_range(boost::make_tuple(edgeName));
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
			int currentCount =  node->Edges.get<TimeOrderedEdgeType>().count(edgeName);
			loadItems(currentCount + itemsPerRequest);					
		}
		else if (state == FBDataCursor::Ended)
		{
			state = FBDataCursor::Finished;
		}
	}
	else
	{
		auto itemNodes = node->Edges.get<TimeOrderedEdgeType>().find(boost::make_tuple(edgeName,nextItem->getTimestamp(), nextItem->getNumericId()));
		
		if (itemNodes != node->Edges.get<TimeOrderedEdgeType>().end())
		{
			itemNodes++;
			for (;itemNodes != node->Edges.get<TimeOrderedEdgeType>().end(); itemNodes++)
			{
				FBNode * check = itemNodes->Node;
				if (check != NULL && check->getNodeType().compare(edgeName) == 0)
					break;		
			}

			if (itemNodes != node->Edges.get<TimeOrderedEdgeType>().end())
			{
				nextItem = (itemNodes)->Node;
			}
			else if (state == FBDataCursor::Local)
			{
				int currentCount =  node->Edges.get<TimeOrderedEdgeType>().count(edgeName);
				loadItems(currentCount + itemsPerRequest);					
			}
			else if (state == FBDataCursor::Ended)
			{
				state = FBDataCursor::Finished;
			}
		}
		else
		{
			if (state == FBDataCursor::Ended)
			{
				state = FBDataCursor::Finished;
			}
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

FBNode * InterleavingCursor::getNext()
{
	Logger::stream("InterleavingCursor","DEBUG") << "getNext()" << endl;
	FBNode * result = NULL;
	if (lastCursorIndex == 0)
	{
		if (cursor0->getState() != State::Finished)
		{
			result = cursor0->getNext();			
			Logger::stream("InterleavingCursor","DEBUG") << "0_C0 = " << ((result==NULL) ? "NULL" : result->getId()) << endl;
		}
		
		if (result == NULL && cursor0->getState() == State::Finished && cursor1->getState() != State::Finished)
		{
			result = cursor1->getNext();
			Logger::stream("InterleavingCursor","DEBUG") << "0_C1 = " << ((result==NULL) ? "NULL" : result->getId()) << endl;
		}

	}
	else
	{
		if (cursor1->getState() != State::Finished)
		{	result = cursor1->getNext();			
			Logger::stream("InterleavingCursor","DEBUG") << "1_C1 = " << ((result==NULL) ? "NULL" : result->getId()) << endl;
		}
		
		if (result == NULL && cursor1->getState() == State::Finished && cursor0->getState() != State::Finished)
		{
			result = cursor0->getNext();		
			Logger::stream("InterleavingCursor","DEBUG") << "1_C0 = " << ((result==NULL) ? "NULL" : result->getId()) << endl;
		}
	}

	if (result != NULL)
		lastCursorIndex = (lastCursorIndex+1)%2;

	return result;
}

FBDataCursor::State InterleavingCursor::getState()
{
	FBDataCursor::State s0 = cursor0->getState();
	FBDataCursor::State s1 = cursor1->getState();

	if (s0 == Loading || s1 == Loading)
		return Loading;

	if (s0 == Finished && s1 == Finished)
		return Finished;

	return Local;
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
				int newItemCount = _node->Edges.get<EdgeTypeIndex>().count("friends");
				if (newItemCount <= currentFriendCount)
				{				
					//if (newItemCount == 0)
					//	v->state = FBDataCursor::Finished;
					//else
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
		loadstr << "albums.offset(" << currentCount << ").limit(" << (albums-currentCount) << ").fields(id,name,updated_time,photos.fields(id,name,images,album).limit(4))";

		FBUserAlbumsCursor * v = this;
		FBDataSource::instance->loadField(node,loadstr.str(),"",[v,currentCount](FBNode * _node){
						
			if (v->state == FBDataCursor::Loading)
			{
				int newItemCount = _node->Edges.get<EdgeTypeIndex>().count("albums");
				if (newItemCount <= currentCount)
				{				
					//if (newItemCount == 0)
					//	v->state = FBDataCursor::Finished;
					//else
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
				int newItemCount = _node->Edges.get<EdgeTypeIndex>().count("photos");
				if (newItemCount <= currentCount)
				{				
					//if (newItemCount == 0)
					//	v->state = FBDataCursor::Finished;
					//else
						v->state = FBDataCursor::Ended;
				}
				else
				{
					v->state = FBDataCursor::Local;
					//v->getNext();
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