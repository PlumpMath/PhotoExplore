#include "DataViewGenerator.hpp"
#include <queue>

DataViewGenerator * DataViewGenerator::instance = NULL;


void DataViewGenerator::SelectNodes(FBNode * start, const NodeQuerySpec & nodeQuery, vector<FBNode*> & result)
{	
	
	vector<map<string,SelectionConfig> > layers = nodeQuery.layers;

	queue<FBNode*> nodeQueue;
	map<FBNode*, int> visited;

	nodeQueue.push(start);
	visited.insert(make_pair(start,0));

	FBNode * current;
	int lastDepth = 0;

	while (nodeQueue.size() > 0)
	{
		current = nodeQueue.front();
		nodeQueue.pop();

		int depth = visited[current];

		result.push_back(current);
			
		if (depth < layers.size())
		{
			map<string,SelectionConfig> thisLayer = map<string,SelectionConfig>(layers[depth].begin(),layers[depth].end());

			for (auto layer = thisLayer.begin(); layer != thisLayer.end();layer++)
			{
				auto edgesOfType = current->Edges.get<EdgeTypeIndex>().equal_range(layer->first);

				long resultCount = layer->second.nodeCount;
				long offset = layer->second.offset;
				
				//adam::advance
				for (;offset > 0 && edgesOfType.first != edgesOfType.second; offset--,edgesOfType.first++);

				for(;edgesOfType.first != edgesOfType.second; edgesOfType.first++)
				{
					if (visited.count(edgesOfType.first->Node) == 0)
					{
						visited.insert(make_pair(edgesOfType.first->Node,depth+1));
						if (resultCount > 0)
						{
							nodeQueue.push(edgesOfType.first->Node);
							resultCount--;
						}
					}
				}
			}
		}
	}
}

void DataViewGenerator::nodeUpdated(FBNode * node)
{
	queue<boost::function<void()> > callbackQueue;
	
	auto range = viewCallbacks.equal_range(node);
	//for (;range.first != range.second; range.first++)
	while (range.first != range.second)
	{
		callbackQueue.push(range.first->second);
		viewCallbacks.erase(range.first++);
	}

	while(callbackQueue.size() > 0)
	{
		callbackQueue.front()(); //<-- lol C++
		callbackQueue.pop();
	}
}


vector<FBNode*>  DataViewGenerator::getDataView(FBNode * start, const NodeQuerySpec & nodeQuery, boost::function<void(vector<FBNode*>&)> callback, bool & loadStarted)
{
	loadStarted = false;
	vector<FBNode*> result;
	vector<map<string,SelectionConfig> > layers = nodeQuery.layers;

	queue<pair<FBNode*,bool> > nodeQueue;
	map<FBNode*, int> visited;

	nodeQueue.push(make_pair(start,false));

	visited.insert(make_pair(start,0));

	FBNode * current;
	int lastDepth = 0;

	while (nodeQueue.size() > 0)
	{
		current = nodeQueue.front().first;
		bool includeCurrent = nodeQueue.front().second;

		nodeQueue.pop();

		int depth = visited[current];

		if (includeCurrent)
			result.push_back(current);

		if (nodeQuery.maxResultCount >= 0 && result.size() >= nodeQuery.maxResultCount)
			break;
					
		if (depth < layers.size())
		{
			map<string,SelectionConfig> thisLayer = map<string,SelectionConfig>(layers[depth].begin(),layers[depth].end());
			
			for (auto layer = thisLayer.begin(); layer != thisLayer.end();layer++)
			{
				auto edgesOfType = current->Edges.get<EdgeTypeIndex>().equal_range(layer->first);

				long resultCount = layer->second.nodeCount;
				if (edgesOfType.first != edgesOfType.second && resultCount > 0)
				{
					long offset = layer->second.offset;

					//adam::advance
					for (;offset > 0 && edgesOfType.first != edgesOfType.second; offset--,edgesOfType.first++);

					for(;edgesOfType.first != edgesOfType.second; edgesOfType.first++)
					{
						if (visited.count(edgesOfType.first->Node) == 0)
						{
							visited.insert(make_pair(edgesOfType.first->Node,depth+1));
							if (resultCount > 0)
							{								
								nodeQueue.push(make_pair(edgesOfType.first->Node,layer->second.include));
								resultCount--;
							}
						}
					}
				}
//				
//				if (resultCount > 0)
//				{
//					long loadCount = layer->second.nodeCount+layer->second.offset;
//					//cout << "Not enough data, requesting edge: " << layer->first << " load to " << loadCount << endl;
//					
//					if (current->setLoadTarget(layer->first,loadCount))
//					{
//						current->childrenChangedCallback = [this](NodeBase * nb){
//							this->nodeUpdated((FBNode*)nb);
//						};
//						
//						viewCallbacks.insert(make_pair(current,[this,start,nodeQuery,callback](){
//							bool started;
//							callback(this->getDataView(start,nodeQuery,callback,started));
//						}));
//						current->startLoad();
//						loadStarted = true;
//					}
//				}
			}
		}
	}
	return result;
}
