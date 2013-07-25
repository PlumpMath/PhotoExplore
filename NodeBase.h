#include <boost/function.hpp>
#include <iostream>


#ifndef NODE_BASE_H_
#define NODE_BASE_H_

using namespace std;

namespace NodeType
{	
	const static string FacebookFriend("friends");
	const static string FacebookAlbum("albums");
	const static string FacebookImage("photos");

	const static string ImageFile("f");
	const static string ImageDirectory("d");
	
}

class NodeBase {
	
public:
	NodeBase()
	{
	}

	~NodeBase()
	{

	}

	virtual std::string getURI() = 0;	
	virtual string getNodeType() = 0;
	
	virtual std::string toString()
	{
		return std::string();
	}
		
	virtual float getDataPriority()
	{
		return dataPriority;
	}
	
	virtual void setDataPriority(float priority) {};
	virtual void setDataPriority(float priority, NodeBase * sender) {};

protected:	
	float dataPriority;
	




};

#endif