#include "FBDataCursor.hpp"
#include "Logger.hpp"

FBNode * FBFriendsCursor::getNext()
{
	if (state == FBDataCursor::Loading || state == FBDataCursor::Finished)
		return NULL;

	FBNode * result = nextItem;
	if (nextItem == NULL)
	{
		if (state = FBDataCursor::Local)
			state = FBDataCursor::Local;	

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
		else if (state == FBDataCursor::Local)
		{
			int currentCount =  node->Edges.get<EdgeTypeIndex>().count("friends");
			loadItems(currentCount + itemsPerRequest);					
		}
		else if (state == FBDataCursor::Ended)
		{
			state = FBDataCursor::Finished;
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
			else if (state == FBDataCursor::Local)
			{
				int currentCount =  node->Edges.get<EdgeTypeIndex>().count("friends");
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

void FBFriendsCursor::reset()
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

FBNode * FBFriendsFQLCursor::getNext()
{
	if (state == FBDataCursor::Loading || state == FBDataCursor::Finished)
		return NULL;

	FBNode * result = nextItem;
	if (nextItem == NULL)
	{
		if (state == FBDataCursor::Local)
			state = FBDataCursor::Local;

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
			else if (state == FBDataCursor::Local)
			{
				loadItems(itemsPerRequest);					
			}
			else if (state == FBDataCursor::Ended)
			{
				state = FBDataCursor::Finished;
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
			else if (state == FBDataCursor::Local)
			{
				loadItems(currentPosition + itemsPerRequest);					
			}
			else if (state == FBDataCursor::Ended) 
			{
				state = FBDataCursor::Finished;
			}
		}
		else
		{
			state = FBDataCursor::Finished;
			result = NULL;
		}
	}		
	return result;
}

void FBFriendsFQLCursor::lookupName(string _lookupName)
{
	std::transform(_lookupName.begin(), _lookupName.end(), _lookupName.begin(), ::tolower);

	if (state == FBDataCursor::Ended && _lookupName.find(searchName) == 0 && searchName.length() > 0)
		state = FBDataCursor::Ended;
	else if (state == FBDataCursor::Ended && searchName.find(_lookupName) == 0 && _lookupName.length() > 0)
		state = FBDataCursor::Ended;
	else
		state = FBDataCursor::Local;

	this->searchName = _lookupName;
	currentPosition = 0;
	nextItem = NULL;
}

void FBFriendsFQLCursor::loadItems(int friends)
{
	if (state == FBDataCursor::Loading || state == FBDataCursor::Ended)
	{		
		return;
	}

	Logger::stream("FBFriendsFQL","INFO") << "Loading " << friends << " more friends." << endl;
	
	if (friends > currentPosition)
	{
		state = FBDataCursor::Loading;

		stringstream loadStr;
		int limit = friends-currentPosition;
		int offset = currentPosition;

		loadStr << "/fql?q=SELECT%20name%2Cuid%20from%20user%20where%20uid%20in%20(SELECT%20uid2%20FROM%20friend%20WHERE%20uid1%3Dme())%20AND%20substr(first_name%2C0%2C";
		loadStr << searchName.length() << ")%20%3D%20%22" << searchName << "%22";
		loadStr << "%20LIMIT%20" << limit << "%20OFFSET%20" << offset;
		
		int expectedCount = node->Edges.get<EdgeTypeIndex>().count("friends") + limit;

		FBFriendsFQLCursor * v = this;
		FBDataSource::instance->loadQuery(node,loadStr.str(),"friends",[v,expectedCount](FBNode * _node){

			if (v->state == FBDataCursor::Loading)
			{
				int newFriendCount = _node->Edges.get<EdgeTypeIndex>().count("friends");
				if (newFriendCount < expectedCount)
				{
					v->state = FBDataCursor::Ended;
				}
				else
				{		
					v->state = FBDataCursor::Local;					
				}

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

