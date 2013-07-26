#ifndef LEAPIMAGE_FB_DATA_CURSOR_HPP_
#define LEAPIMAGE_FB_DATA_CURSOR_HPP_

#include "FBNode.h"

using namespace Facebook;

class FBDataCursor {
	
//protected:

public:	
	volatile bool isLoading, canLoad;
	boost::function<void()> cursorChangedCallback;

	FBDataCursor() :	
		isLoading(false),
		canLoad(true)
	{
	}

	//bool isLoading()
	//{
	//	return isLoading;
	//}

	//bool canLoad() 
	//{
	//	return canLoad;
	//}

	virtual FBNode * getNext() = 0;

};

class FBFriendsCursor : public FBDataCursor {

private:
	FBNode * node;
	
	FBNode * nextItem;

public:
	FBFriendsCursor(FBNode * _node) :
	   node(_node)
	{
		nextItem = NULL;
	}

	void loadItems(int friends)
	{
		int requestedFriends = node->loadState["friends"].requestedCount;

		stringstream loadstr;
		loadstr << node->getId() << "?fields=";

		if (friends > requestedFriends)
		{
			node->loadState["friends"].requestedCount = friends;
			loadstr << "friends.offset(" << requestedFriends << ").limit(" << friends << ").fields(id,name)";

			isLoading = true;
			FBFriendsCursor * v = this;
			FBDataSource::instance->loadField(node,loadstr.str(),"",[v](FBNode * _node){

				auto fRange = v->node->Edges.get<EdgeTypeIndex>().equal_range("friends");
				for (; fRange.first != fRange.second; fRange.first++)
				{
					int photoCount = fRange.first->Node->Edges.get<EdgeTypeIndex>().count("photos");
					if (photoCount < 2)
					{					
						stringstream load2;
						load2 << fRange.first->Node->getId() << "?fields=photos.fields(id,name,images).limit(2)";

						FBFriendsCursor * v2= v;						
						FBDataSource::instance->loadField(fRange.first->Node,load2.str(),"",[v2,photoCount](FBNode * nn)
						{
							v2->isLoading = false;

							int newPhotoCount = nn->Edges.get<EdgeTypeIndex>().count("photos");
							if (newPhotoCount <= photoCount)
								v2->canLoad = false;

							if (!v2->cursorChangedCallback.empty())
								v2->cursorChangedCallback();
						}); 
					}
				}				
			});
		}
	}


	FBNode * getNext()
	{
		FBNode * result = nextItem;
		if (nextItem == NULL)
		{
			auto friendNodes = node->Edges.get<EdgeTypeIndex>().equal_range(boost::make_tuple("friends"));
			if (friendNodes.first != friendNodes.second)
			{
				nextItem = friendNodes.first->Node;
			}
			else
			{
				loadItems(25);
				nextItem = NULL;
			}
		}
		else
		{
			auto friendNodes = node->Edges.get<EdgeTypeIndex>().find(boost::make_tuple("friends",nextItem->getId()));
			if (friendNodes != node->Edges.get<EdgeTypeIndex>().end())
			{
				friendNodes++;
				if (friendNodes != node->Edges.get<EdgeTypeIndex>().end())
				{
					nextItem = (friendNodes)->Node;
				}
				else
				{
					int currentCount =  node->Edges.get<EdgeTypeIndex>().count("friends");
					loadItems(currentCount + 25);
					nextItem = NULL;
				}
			}
		}		
		return result;
	}


};

#endif