#include <boost/function.hpp>
#include <iostream>


#ifndef NODE_BASE_H_
#define NODE_BASE_H_

using namespace std;



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
		

};

#endif