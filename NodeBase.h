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
		childrenChangedFlag = false;
	}

	~NodeBase()
	{

	}


	boost::function<void(NodeBase * node)> childrenChangedCallback;

	virtual std::string getURI() = 0;	
	virtual string getNodeType() = 0;

	virtual void update()
	{
		if (childrenChangedFlag)
		{
			if (!childrenChangedCallback.empty())
			{
				childrenChangedFlag = false;
				childrenChangedCallback(this);
			}
		}
	}

	virtual std::string toString()
	{
		/*std::stringstream ss;
		ss << "NodeBase@" << this;
		return ss.str();*/
		return std::string();
	}
		
	virtual int getNodeDistance()
	{
		return nodeDistance;
	}

	virtual float getDataPriority()
	{
		return dataPriority;
	}

	void childrenChanged()
	{
		childrenChangedFlag = true;
	}


	virtual void setDataPriority(float priority) {};
	virtual void setNodeDistance(int distance) {};
	virtual void setNodeDistance(int distance, NodeBase * sender) {};
	virtual void setDataPriority(float priority, NodeBase * sender) {};

protected:	
	int nodeDistance;
	float dataPriority;
	



private:
	bool childrenChangedFlag;


};

#endif