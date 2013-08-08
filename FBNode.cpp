#include "FBNode.h"
#include "Logger.hpp"
#include <boost/date_time.hpp>

using namespace json_spirit;
using namespace Facebook;

string Edge::id() const
{
	if (Node != NULL)
		return Node->getId();
	else
		return "";
}

unsigned long long Edge::numericId() const 
{
	if (Node != NULL)
		return Node->getNumericId();
	else
		return 0;
}

facebook_time Edge::timestamp() const
{
	if (Node != NULL)
		return Node->getTimestamp();
	else
		return 0;
}

FBNode::FBNode(string id)
{
	this->id = id;
	if (id.length() > 0)
	{
		try
		{
			this->numericId = boost::lexical_cast<unsigned long long>(id);
		}
		catch (boost::bad_lexical_cast & b)
		{
			this->numericId = 0;
		}
	}
	else
		this->numericId = 0;
}

unsigned long long FBNode::getNumericId() 
{
	return numericId;
}

facebook_time FBNode::getTimestamp()
{
	return timestamp;
}

//namespace bt = boost::posix_time;
//const std::locale formats[] = { 
//	//2012-05-10T11:15:51+0000
////std::locale(std::locale::classic(),new bt::time_input_facet("%Y-%m-%d %H:%M:%S")),
////std::locale(std::locale::classic(),new bt::time_input_facet("%Y/%m/%d %H:%M:%S")),
////std::locale(std::locale::classic(),new bt::time_input_facet("%d.%m.%Y %H:%M:%S")),
//std::locale(std::locale::classic(),new bt::time_input_facet("%Y-%m-%dT%H:%M:%S"))
//};
//const size_t formats_n = sizeof(formats)/sizeof(formats[0]);

//static std::time_t pt_to_time_t(const bt::ptime& pt)
//{
//    bt::ptime timet_start(boost::gregorian::date(1970,1,1));
//    bt::time_duration diff = pt - timet_start;
//    return diff.ticks()/bt::time_duration::rep_type::ticks_per_second;
//
//}
//static facebook_time seconds_from_epoch(const std::string& s)
//{
//    bt::ptime pt;
//    for(size_t i=0; i<formats_n; ++i)
//    {
//        std::istringstream is(s);
//        is.imbue(formats[i]);
//        is >> pt;
//        if(pt != bt::ptime()) break;
//    }
//    //std::cout << " ptime is " << pt << '\n';
//    //std::cout << " seconds from epoch are " << pt_to_time_t(pt) << '\n';
//
//	return pt_to_time_t(pt);
//}


string FBNode::tryExtractId(vector<Pair> & obj, bool & success, facebook_time & childTimestamp)
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

	json_spirit::Value timeVal = find_value(obj,"updated_time");
	if (timeVal.is_null())
		timeVal = find_value(obj,"created_time");

	if (timeVal.type() == json_spirit::int_type)
	{
		childTimestamp =  timeVal.get_int64();
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

void FBNode::setTimestamp(facebook_time _timestamp)
{
	this->timestamp = _timestamp;
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
	facebook_time childTimestamp;
	string childId = tryExtractId(obj,result,childTimestamp);
	if (!result)
		return NULL;

	FBNode * newChild = new FBNode(childId);
	newChild->setTimestamp(childTimestamp);
	newChild->setNodeType(edge);

	newChild->addJSON("",obj);

	return newChild;	
}

void FBNode::addJSON(string edge, vector<json_spirit::Pair> & obj)
{	
	bool result;
	facebook_time childTimestamp;
	string childId = tryExtractId(obj,result,childTimestamp);

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
			child->ReverseEdges.insert(Edge(this->getNodeType(),this,0));
			long rowNum = Edges.count(child->getNodeType());
			Edges.insert(Edge(edge,child,rowNum));
		}
	}
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