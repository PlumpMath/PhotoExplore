#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_


#include <set>
#include <queue>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/hashed_index.hpp>

#include <boost/thread/mutex.hpp>

#include "ResourceManagerTypes.h"
#include <opencv2/opencv.hpp>
#include "SDLTimer.h"


struct IdIndex{};

using namespace std;


struct CachedResource
{
	ResourceData * Data;

	string resourceId;

	CachedResource(ResourceData * _data) :
		Data(_data),		
		resourceId(_data->resourceId)
	{
	}
};


typedef boost::multi_index_container
	<
		CachedResource,
		boost::multi_index::indexed_by
		<
			boost::multi_index::hashed_unique
			<
				boost::multi_index::tag<IdIndex>,
				boost::multi_index::member<CachedResource,string,&CachedResource::resourceId>
			>
		>
	> ResourceCache;



class ResourceManager {

private:	
	ResourceManager();
	ResourceManager(ResourceManager const&);
	void operator=(ResourceManager const&); 

	void cleanupCache(bool updateAll);	
	void textureLoaded(string resourceId, GLuint textureId, int taskStatus);

	void updateImageState(ResourceData * data, bool load);	
	void updateTextureState(ResourceData * data, bool load);
	void updateImageResource(string resourceId, int statusCode, cv::Mat imgMat);


	float calculateResourceSize(ResourceData * data);


	//Fields
	Timer cacheCleanupTimer;

	ResourceCache resourceCache;
	bool resourcesChanged;	
	boost::mutex resourceMutex;
	boost::mutex updateTaskMutex;
	queue<boost::function<void()> > updateThreadTasks;
	
	float maxTextureLoadPriority;
	float maxImageLoadPriority;
	
	float textureLoadThreshold, imageLoadThreshold;
	double currentTextureCacheSize, currentImageCacheSize;

	bool texturesEnabled;

public:		
	static ResourceManager& getInstance()
	{
		static ResourceManager instance; 
		return instance;
	}
	
	void unloadTextures();
	void reloadTextures();

	void destroyResourceIfEmpty(string resourceId, IResourceWatcher * watcher);
	void releaseResource(string resourceId,IResourceWatcher * watcher);
	ResourceData * watchResource(string resourceId, IResourceWatcher * watcher);

	ResourceData * loadResource(string resourceId, string imageURI, float priority, IResourceWatcher * watcher);	
	ResourceData * loadResource(string resourceId, cv::Mat & image, float priority, IResourceWatcher * watcher);

	ResourceData * loadResourceWithTransform(string resourceId, string imageURI, float priority, IResourceWatcher * watcher, boost::function<void(cv::Mat&)> transform);
	
	void updateResource(ResourceData * resource);

	void update();

};


#endif