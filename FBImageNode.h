#ifndef FB_IMAGE_NODE_H
#define FB_IMAGE_NODE_H

#include "ImageManager.h"
#include "FBNode.h"

using namespace std;

namespace Facebook {

	class FBImageNode : public FBNode {

	private:
		bool isLoaded;

	public:
		FBImageNode(string id) : FBNode(id)
		{
			isLoaded = false;
		}

		void doLoading()
		{
			if (dataPriority < 1 && !isLoaded)
			{
				if (nodeDistance <= 2)
					ImageManager::getInstance()->setImageRelevance(id,LevelOfDetail_Types::Preview,dataPriority, ImgTaskQueue::FacebookImage);
			
				isLoaded = true;
			}
		}

		void setDataPriority(float _dataPriority)
		{
			if (this->dataPriority != _dataPriority)
			{
				this->dataPriority = _dataPriority;
			
				doLoading();
			}
		}

		void setNodeDistance(int nodeDistance)
		{
			this->nodeDistance = nodeDistance;		
		}

		int getNodeType()
		{
			return NodeType::FacebookImage;
		}
	
		FBNode * buildChildFromJSON(string childId, string edge, vector<json_spirit::Pair> & obj);


	};
} 


#endif