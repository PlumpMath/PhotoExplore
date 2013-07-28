#include "FBDataCursor.hpp"
#include "Logger.hpp"

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
		else if (canLoad)
		{
			int currentCount =  node->Edges.get<EdgeTypeIndex>().count("friends");
			loadItems(currentCount + itemsPerRequest);					
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		auto friendNodes = node->Edges.get<EdgeTypeIndex>().find(boost::make_tuple("friends",nextItem->getNumericId()));
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
			else if (canLoad)
			{
				int currentCount =  node->Edges.get<EdgeTypeIndex>().count("friends");
				loadItems(currentCount + itemsPerRequest);					
			}
			else
			{
				return NULL;
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
	if (isLoading)
	{
		Logger::stream("FBFriendsCursor","ERROR") << "Currently loading." << endl;
		return;
	}

	int currentFriendCount = node->Edges.get<EdgeTypeIndex>().count("friends");
	Logger::stream("FBFriendsCursor","INFO") << "Loading " << friends << " more friends. CurrentCount = " << currentFriendCount << endl;

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
			{				
				Logger::stream("FBFriendsCursor","ERROR") << "Reached end of items" << endl;
				v->canLoad = false;
			}

			if (v->isLoading)
			{
				v->isLoading = false;
				v->getNext();
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

				if (check != NULL && check->getNodeType().compare("friends") == 0)
				{
					string checkName = check->getAttribute("name");
					string subName = checkName.substr(0,searchName.length());

					std::transform(subName.begin(), subName.end(), subName.begin(), ::tolower);
					if (subName.compare(searchName) == 0)
						break;		
				}
			}

			if (friendNodes.first != friendNodes.second)
			{
				currentPosition++;
				nextItem = friendNodes.first->Node;
			}
			else if (canLoad)
			{
				loadItems(itemsPerRequest);					
			}
			else 
			{
				return NULL;
			}
		}
		else
		{
			loadItems(itemsPerRequest);
		}
	}
	else
	{
		auto friendNodes = node->Edges.get<EdgeTypeIndex>().find(boost::make_tuple("friends",nextItem->getNumericId()));
		if (friendNodes != node->Edges.get<EdgeTypeIndex>().end())
		{
			friendNodes++;
			for (;friendNodes != node->Edges.get<EdgeTypeIndex>().end(); friendNodes++)
			{
				FBNode * check = friendNodes->Node;
				
				if (check != NULL && check->getNodeType().compare("friends") == 0)
				{
					string checkName = check->getAttribute("name");
					string subName = checkName.substr(0,searchName.length());

					std::transform(subName.begin(), subName.end(), subName.begin(), ::tolower);
					if (subName.compare(searchName) == 0)
						break;		
				}
			}

			if (friendNodes != node->Edges.get<EdgeTypeIndex>().end())
			{
				currentPosition++;
				nextItem = (friendNodes)->Node;
			}
			else if (canLoad)
			{
				loadItems(currentPosition + itemsPerRequest);					
			}
			else 
			{
				return NULL;
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
	std::transform(_lookupName.begin(), _lookupName.end(), _lookupName.begin(), ::tolower);

	if (!canLoad && _lookupName.find(searchName) == 0 && searchName.length() > 0)
		canLoad = false;
	else
		canLoad = true;

	this->searchName = _lookupName;
	currentPosition = 0;
	nextItem = NULL;
	isLoading = false;
}

void FBFriendsFQLCursor::loadItems(int friends)
{
	if (isLoading)
	{
		Logger::stream("FBFriendsFQL","ERROR") << "Currently loading." << endl;
		return;
	}

	Logger::stream("FBFriendsFQL","INFO") << "Loading " << friends << " more friends." << endl;
	
	if (friends > currentPosition && canLoad)
	{
		isLoading = true;

		stringstream loadStr;
		int limit = friends-currentPosition;
		int offset = currentPosition;

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
			else
			{
				Logger::stream("FBFriendsCursor","ERROR") << "Load completed while not loading" << endl;
			}
		});
	}
}

