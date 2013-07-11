#ifndef LEAPIMAGE_DATA_VIEW_GENERATOR_HPP_
#define LEAPIMAGE_DATA_VIEW_GENERATOR_HPP_

#include <vector>

#include "FBNode.h"

using namespace std;
using namespace Facebook;

struct DataViewConfig 
{
	vector<map<int,long> > layers;
};

struct SelectionConfig {

	long offset, nodeCount;
	bool include;

	SelectionConfig(long _nodeCount):
		offset(0),
		nodeCount(_nodeCount),
		include(true)
	{}
		
	SelectionConfig(long _nodeCount, long _offset, bool _include):
		offset(_offset),
		nodeCount(_nodeCount),
		include(_include)
	{}

};

struct NodeQuerySpec {

	int maxResultCount;

	vector<map<string,SelectionConfig> > layers;

	NodeQuerySpec()
	{
		;
	}

	NodeQuerySpec(int layerCount) :
		maxResultCount(-1)
	{
		while (layerCount-- > 0)
			layers.push_back(map<string,SelectionConfig>());
	}
	
	NodeQuerySpec(int layerCount, int _maxResultCount) :
		maxResultCount(_maxResultCount)
	{
		while (layerCount-- > 0)
			layers.push_back(map<string,SelectionConfig>());
	}
};

//class DataViewWatcher {
//
//public:
//	virtual void viewChanged(string viewIdentifier, vector<FBNode*> & viewData) = 0;
//};


class DataViewGenerator {

private: 
	static DataViewGenerator * instance;

	multimap<FBNode*,boost::function<void()> > viewCallbacks;

public:
	static DataViewGenerator * getInstance()
	{
		if (instance == NULL)
			instance = new DataViewGenerator();

		return instance;
	}

	void nodeUpdated(FBNode * node);
	void SelectNodes(FBNode * start, const NodeQuerySpec & nodeQuery, vector<FBNode*> & result);
	
public:
	vector<FBNode*>  getDataView(FBNode * start, const NodeQuerySpec & nodeQuery, boost::function<void(vector<FBNode*>&)> callback, bool & loadStarted);

};

#endif