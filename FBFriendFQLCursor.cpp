#include "FBDataCursor.hpp"
#include "Logger.hpp"

DataNode * FBFriendsFQLCursor::getNext()
{
	if (state == DataCursor::Loading || state == DataCursor::Finished)
		return NULL;

	FBNode * result = nextItem;
	if (nextItem == NULL)
	{
		if (state == DataCursor::Local)
			state = DataCursor::Local;

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
			else if (state == DataCursor::Local)
			{
				loadItems(itemsPerRequest);					
			}
			else if (state == DataCursor::Ended)
			{
				state = DataCursor::Finished;
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
			else if (state == DataCursor::Local)
			{
				loadItems(currentPosition + itemsPerRequest);					
			}
			else if (state == DataCursor::Ended) 
			{
				state = DataCursor::Finished;
			}
		}
		else
		{
			state = DataCursor::Finished;
			result = NULL;
		}
	}		
	if (result != NULL)
		Logger::stream("FBFriendFQLCursor","INFO") << "Returning friend matching '" << searchName << "' with name = " << result->getAttribute("name") << endl;

	return result;
}

void FBFriendsFQLCursor::lookupName(string _lookupName)
{
	std::transform(_lookupName.begin(), _lookupName.end(), _lookupName.begin(), ::tolower);

	if (state == DataCursor::Ended && _lookupName.find(searchName) == 0 && searchName.length() > 0)
		state = DataCursor::Ended;
	else if (state == DataCursor::Ended && searchName.find(_lookupName) == 0 && _lookupName.length() > 0)
		state = DataCursor::Ended;
	else
		state = DataCursor::Local;

	this->searchName = _lookupName;
	currentPosition = 0;
	nextItem = NULL;
}

void FBFriendsFQLCursor::loadItems(int friends)
{
	if (state == DataCursor::Loading || state == DataCursor::Ended)
	{		
		return;
	}

	Logger::stream("FBFriendsFQL","INFO") << "Loading " << friends << " more friends." << endl;
	
	if (friends > currentPosition)
	{
		state = DataCursor::Loading;

		stringstream loadStr;
		int limit = friends-currentPosition;
		int offset = currentPosition;

		loadStr << "/fql?q=SELECT%20name%2Cuid%20from%20user%20where%20uid%20in%20(SELECT%20uid2%20FROM%20friend%20WHERE%20uid1%3Dme())%20AND%20substr(first_name%2C0%2C";
		loadStr << searchName.length() << ")%20%3D%20%22" << searchName << "%22";
		loadStr << "%20LIMIT%20" << limit << "%20OFFSET%20" << offset;
		
		int expectedCount = node->Edges.get<EdgeTypeIndex>().count("friends") + limit;

		FBFriendsFQLCursor * v = this;
		FBDataSource::instance->loadQuery(node,loadStr.str(),"friends",[v,expectedCount](FBNode * _node){

			if (v->state == DataCursor::Loading)
			{
				int newFriendCount = _node->Edges.get<EdgeTypeIndex>().count("friends");
				if (newFriendCount < expectedCount)
				{
					v->state = DataCursor::Ended;
				}
				else
				{		
					v->state = DataCursor::Local;					
				}

				//v->getNext();
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

