#include "FBNode.h"

using namespace json_spirit;
using namespace Facebook;

string Edge::id() const
{
	if (Node != NULL)
		return Node->getId();
	else
		return "";
}

FBNode::FBNode(string id)
{
	this->loadCompleteFlag = false;
	this->id = id;
	this->dataPriority = 1;
}

string FBNode::tryExtractId(vector<Pair> & obj, bool & success)
{
	string childId;
	json_spirit::Value idVal = find_value(obj,"id");
	if (idVal.is_null())
		idVal = find_value(obj,"uid");

	//If ID is null, then the object being added is a terminating edge of this object (aka a simple attribute)
	if (idVal.is_null())
	{
		success = false;
	}
	else if (idVal.type() == json_spirit::int_type)
	{
		stringstream ss;
		ss << idVal.get_int();
		childId = ss.str();
		success = true;
	}
	else if (idVal.type() == json_spirit::str_type)
	{
		childId = idVal.get_str();
		success = true;
	}
	return childId;
}

bool FBNode::edgeLimitReached(string edge)
{
	auto it = loadState.find(edge);

	if (it != loadState.end())
	{
		return it->second.hasReachedEnd;
	}
	else
		return false;
}

string FBNode::getNodeType()
{
	return nodeType;
}

void FBNode::setNodeType(string _nodeType)
{
	this->nodeType = _nodeType;
}

string FBNode::getAttribute(string key)
{
	auto res = Edges.get<EdgeTypeIndex>().find(key);
	if (res != Edges.get<EdgeTypeIndex>().end())
		return res->Value;
	return "";
}

FBNode * FBNode::buildChildFromJSON(string edge, vector<json_spirit::Pair> & obj)
{
	bool result;
	string childId = tryExtractId(obj,result);
	if (!result)
		return NULL;

	FBNode * newChild = new FBNode(childId);
	newChild->setNodeType(edge);

	newChild->addJSON("",obj);

	return newChild;	
}

void FBNode::addJSON(string edge, vector<json_spirit::Pair> & obj)
{	
	bool result;
	string childId = tryExtractId(obj,result);

	if (result && obj.size() == 1)
	{
		//cout << "Empty object returned for edge: " << edge << endl;
		loadState[edge].hasReachedEnd = true;
	}

	if (id.compare("me") == 0 && result)
		id = childId;
	
	if (!result || childId.compare(id) == 0)
	{
		for (auto it = obj.begin(); it != obj.end(); it++)
		{
			string name = it->name_;
			json_spirit::Value value = it->value_;

			FBNode * newChild = NULL;
			stringstream ss;

			switch (value.type())
			{
			case str_type:
				Edges.insert(Edge(name,value.get_str()));
				break;
			case obj_type:			
				if (name.compare("data") == 0)
					addJSON(edge,value.get_obj());
				else
					addJSON(name,value.get_obj());
				break;
			case array_type:
				if (name.compare("data") == 0)
					addJSONArray(edge,value.get_array());
				else if (name.compare("images") == 0)		
					Edges.insert(Edge(name,value));	
					//addJSONArray(name,value.get_array());
					//cout << "Array type that isn't data?? Name = " << name << endl;
				break;
			case int_type:
				ss << value.get_int();
				Edges.insert(Edge(name,ss.str()));			
				break;
			case bool_type:
				ss << value.get_bool();
				Edges.insert(Edge(name,ss.str()));
				break;
			default:
				cout << "Unexpected JSON type: " << value.type() << endl;
			}		
		}
	}
	else
	{
		FBNode * child = buildChildFromJSON(edge,obj);

		if (child != NULL)
		{
			Edges.insert(Edge(edge,child));
			childrenChanged();
		}
	}
	loadCompleteFlag = true;
}


void FBNode::addJSONArray(string edge, vector<json_spirit::Value> & objectArray)
{
	if (objectArray.size() == 0)
	{
		loadState[edge].hasReachedEnd = true;
	}

	//if (edge.compare("images") == 0)
	//{
	//	vector<map<string,string> >
	//}
	//else
	//{
	for (auto it = objectArray.begin(); it != objectArray.end(); it++)
	{
		addJSON(edge, it->get_obj());
	}
	//}
}


void FBNode::startLoad()
{
	loadCompleteFlag = false;
	stringstream loadString;
	string targetEdge = "";
	for (auto it = loadState.begin(); it != loadState.end(); it++)
	{
		EdgeLoadSpec * loadSpec = &((*it).second);
		if (loadSpec->requestedCount != loadSpec->loadedCount && !loadSpec->hasReachedEnd)
		{			
			targetEdge = it->first;
			stringstream fields;
			fields << ".fields(";
			if (targetEdge.compare("photos") == 0)
			{
				fields << "id,name,images";
			}
			//else if (targetEdge.compare("friends") == 0)
			//{
			//	fields << "id,name,photos.fields(id,name,images).limit(5)";
			//}
			else
			{
				fields << "id,name";
			}
			fields << ")";

			if (loadSpec->requestedCount == LONG_MAX)
				loadString << targetEdge << fields.str() << ",";
			else
			{
				int limit = loadSpec->requestedCount - loadSpec->loadedCount;
				loadString << targetEdge << fields.str() << ".offset(" << loadSpec->loadedCount << ").limit(" << limit << "),";		
			}
			loadSpec->loadedCount = loadSpec->requestedCount;
		}
	}

	if (loadString.str().size() > 0)
	{
		string ls = getId()+ "?fields=" + loadString.str();
		ls.pop_back(); //Remove ending ','
		//cout << "Loading node to target level: " << ls << endl;
		FBDataSource::instance->loadField(this,ls, targetEdge);
	}
}

void FBNode::clearLoadCompleteDelegate()
{
	this->loadCompleteDelegate = [](){};
}

void FBNode::setLoadCompleteDelegate(boost::function<void()> _loadCompleteDelegate)
{
	this->loadCompleteDelegate = _loadCompleteDelegate;
}


bool FBNode::setLoadTarget(string edge, long count)
{
	if (!loadState[edge].hasReachedEnd && loadState[edge].loadedCount < count && loadState[edge].requestedCount < count)
	{
		loadState[edge].requestedCount = count;
		return true;
	}
	return false;
}

void FBNode::setDataPriority(float priority)
{
	this->dataPriority = priority;
}

void FBNode::update()
{
	if (loadCompleteFlag)
	{
		if (!loadCompleteDelegate.empty())
			loadCompleteDelegate();
		loadCompleteFlag = false;
	}
	NodeBase::update();
		

	for (auto it=Edges.begin(); it!=Edges.end();it++)
	{
		if (it->Node != NULL)
			it->Node->update();
	}
}