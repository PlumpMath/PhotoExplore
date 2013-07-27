#include "FBDataCursor.hpp"


FBNode * FBFriendsCursor::getNext()
{
	if (isLoading)
		return NULL;

	FBNode * result = nextItem;
	if (nextItem == NULL)
	{
		auto friendNodes = node->Edges.get<EdgeTypeIndex>().equal_range(boost::make_tuple("friends"));
		if (friendNodes.first != friendNodes.second)
		{
			for (;friendNodes.first != friendNodes.second; friendNodes.first++)
			{
				FBNode * check = friendNodes.first->Node;
				if (check != NULL)
					break;		
			}
			nextItem = friendNodes.first->Node;
		}
		else
		{
			loadItems(25);
		}
	}
	else
	{
		auto friendNodes = node->Edges.get<EdgeTypeIndex>().find(boost::make_tuple("friends",nextItem->getId()));
		if (friendNodes != node->Edges.get<EdgeTypeIndex>().end())
		{
			friendNodes++;
			for (;friendNodes != node->Edges.get<EdgeTypeIndex>().end(); friendNodes++)
			{
				FBNode * check = friendNodes->Node;
				if (check != NULL && check->getNodeType().compare("friends") == 0)
					break;		
			}

			if (friendNodes != node->Edges.get<EdgeTypeIndex>().end())
			{
				nextItem = (friendNodes)->Node;
			}
			else
			{
				int currentCount =  node->Edges.get<EdgeTypeIndex>().count("friends");
				loadItems(currentCount + 25);					
			}
		}
		else
		{
			result = NULL;
		}
	}		
	return result;
}

void FBFriendsCursor::reset()
{
	nextItem = NULL;
	canLoad = true;
	isLoading = false;
}

void FBFriendsCursor::loadItems(int friends)
{

	int currentFriendCount = node->Edges.get<EdgeTypeIndex>().count("friends");
	if (friends > currentFriendCount && canLoad)
	{		
		isLoading = true;	

		stringstream loadstr;
		loadstr << node->getId() << "?fields=";
		loadstr << "friends.offset(" << currentFriendCount << ").limit(" << (friends-currentFriendCount) << ").fields(id,name)";

		FBFriendsCursor * v = this;
		FBDataSource::instance->loadField(node,loadstr.str(),"",[v,currentFriendCount](FBNode * _node){

			int newFriendCount = _node->Edges.get<EdgeTypeIndex>().count("friends");
			if (newFriendCount <= currentFriendCount)
				v->canLoad = false;

			//if (v->isLoading)
			{
				v->isLoading = false;
				//v->getNext();
				if (!v->cursorChangedCallback.empty())
					v->cursorChangedCallback();
			}
		});
	}
}

FBNode * FBFriendsFQLCursor::getNext()
{
	if (isLoading)
		return NULL;

	FBNode * result = nextItem;
	if (nextItem == NULL)
	{
		auto friendNodes = node->Edges.get<EdgeTypeIndex>().equal_range(boost::make_tuple("friends"));
		if (friendNodes.first != friendNodes.second)
		{
			for (;friendNodes.first != friendNodes.second; friendNodes.first++)
			{
				FBNode * check = friendNodes.first->Node;

				if (check->getAttribute("name").substr(0,searchName.length()).compare(searchName) == 0 && check->getNodeType().compare("friends") == 0)
					break;		
			}

			if (friendNodes.first != friendNodes.second)
			{
				currentPosition++;
				nextItem = friendNodes.first->Node;
			}
		}
		else
		{
			loadItems(25);
		}
	}
	else
	{
		auto friendNodes = node->Edges.get<EdgeTypeIndex>().find(boost::make_tuple("friends",nextItem->getId()));
		if (friendNodes != node->Edges.get<EdgeTypeIndex>().end())
		{
			friendNodes++;
			for (;friendNodes != node->Edges.get<EdgeTypeIndex>().end(); friendNodes++)
			{
				FBNode * check = friendNodes->Node;
				
				if (check != NULL && check->getAttribute("name").substr(0,searchName.length()).compare(searchName) == 0 && check->getNodeType().compare("friends") == 0)
					break;		
			}

			if (friendNodes != node->Edges.get<EdgeTypeIndex>().end())
			{
				currentPosition++;
				nextItem = (friendNodes)->Node;
			}
			else
			{
				loadItems(currentPosition + 25);					
			}
		}
		else
		{
			result = NULL;
		}
	}		
	return result;
}

void FBFriendsFQLCursor::lookupName(string _lookupName)
{
	this->searchName = _lookupName;
	currentPosition = 0;
	nextItem = NULL;
	canLoad = true;
	isLoading = false;
	loadItems(10);
}

void FBFriendsFQLCursor::loadItems(int friends)
{
	
	if (friends > currentPosition && canLoad)
	{
		isLoading = true;

		stringstream loadStr;
		int limit = 25,offset = currentPosition;

		loadStr << "/fql?q=SELECT%20name%2Cuid%20from%20user%20where%20uid%20in%20(SELECT%20uid2%20FROM%20friend%20WHERE%20uid1%3Dme())%20AND%20substr(first_name%2C0%2C";
		loadStr << searchName.length() << ")%20%3D%20%22" << searchName << "%22";
		loadStr << "%20LIMIT%20" << limit << "%20OFFSET%20" << offset;
		
		int expectedCount = node->Edges.get<EdgeTypeIndex>().count("friends") + limit;

		FBFriendsFQLCursor * v = this;
		FBDataSource::instance->loadQuery(node,loadStr.str(),"friends",[v,expectedCount](FBNode * _node){

			int newFriendCount = _node->Edges.get<EdgeTypeIndex>().count("friends");
			if (newFriendCount < expectedCount)
				v->canLoad = false;

			//v->currentPosition = newFriendCount;

			if (v->isLoading)
			{
				v->isLoading = false;
				v->getNext();
				if (!v->cursorChangedCallback.empty())
					v->cursorChangedCallback();
			}
		});
	}
}

