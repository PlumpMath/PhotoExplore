#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include "ImageManager.h"
#include "TextureManager.h"
#include "ResourceManagerTypes.h"

#include <set>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/hashed_index.hpp>


struct IdIndex{};


struct Resource
{
	set<IResourceWatcher*> callbacks;
	LevelOfDetail requestedDetail;
	string resourceId;

	float priority;

	//string resourceURI;
	//cv::Size2f resourceSize;

	string getResourceId() const
	{
		return resourceId;
	}

};


typedef boost::multi_index_container
	<
		Resource,
		boost::multi_index::indexed_by
		<
			boost::multi_index::hashed_unique
			<
				boost::multi_index::tag<IdIndex>,
				boost::multi_index::const_mem_fun<Resource,string,&Resource::getResourceId>
			>
		>
	> ResourceCache;



class ResourceManager {

private:
	ImageManager * imgMan;
	TextureManager * tMan;
	
	ResourceManager();
	static ResourceManager * instance;

	PendingTaskQueue textureOverflowQueue;
	ResourceCache resourceCache;

	Resource * insertResource(string resourceId, LevelOfDetail levelOfDetail, float loadPriority, IResourceWatcher * loadCallback);
	//Resource * insertResource(FBNode * resourceNode, LevelOfDetail levelOfDetail, float loadPriority, IResourceWatcher * loadCallback);

	bool resourcesChanged;

	queue<Resource*> modifiedResources;
	boost::mutex resourceMutex;

public:

	void textureResourceChanged(string resourceId, LevelOfDetail levelOfDetail, GLuint glTextureId);
	void imageResourceChanged(string resourceId, MultiResolutionImage * affectedImage);


	static ResourceManager * getInstance();

	void loadTextResource(TextDefinition textDefinition, _PriorityType loadPriority, IResourceWatcher * resourceOwner); 
	Resource * loadResource(string resourceId, LevelOfDetail _levelOfDetail, _PriorityType loadPriority, IResourceWatcher * resourceOwner); 
	void releaseTextResource(TextDefinition textDefinition, IResourceWatcher * resourceOwner);
	void releaseResource(string resourceId, LevelOfDetail _levelOfDetail, IResourceWatcher * resourceOwner); 

	bool getResourceDimensions(string resourceId, LevelOfDetail _levelOfDetail, cv::Size2i & imageSize);

	void updateResource(Resource * resource);
	void updateResource(string resourceId, LevelOfDetail _levelOfDetail);

	void update();

};


#endif