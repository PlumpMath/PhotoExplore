#include "ResourceManager.h"
#include "TextureLoader.hpp"
#include "ImageLoader.hpp"
#include "GlobalConfig.hpp"
#include "TexturePool.h"


ResourceManager::ResourceManager()
{
	textureLoadThreshold = 1000;
	imageLoadThreshold = 1000;

	ImageLoader::getInstance().setResourceChangedCallback([this](string resourceId, int statusCode, cv::Mat imgMat){
		updateImageResource(resourceId,statusCode,imgMat);
	});
}

void ResourceManager::updateImageResource(string resourceId, int statusCode, cv::Mat imgMat)
{
	updateTaskMutex.lock();
	updateThreadTasks.push([this,resourceId,statusCode,imgMat](){
		auto resource = resourceCache.get<IdIndex>().find(resourceId);	

		if (resource != resourceCache.get<IdIndex>().end())
		{
			ResourceData * data = resource->Data;
			data->ImageState = statusCode;
			data->image = imgMat;
			updateResource(data);
		}
	});
	updateTaskMutex.unlock();
}

ResourceData * ResourceManager::loadResource(string resourceId, string imageURI, float priority, IResourceWatcher * watcher)
{
	auto resource = resourceCache.get<IdIndex>().find(resourceId);	
	
	bool resourceModified = false;

	ResourceData * data = NULL;
	if (resource == resourceCache.get<IdIndex>().end())
	{
		data = new ResourceData(resourceId, priority, imageURI);
		resourceCache.insert(CachedResource(data));
		data->TextureState = ResourceState::Empty;
		data->ImageState = ResourceState::Empty;
		resourceModified = true;
	}
	else 
	{		
		data = resource->Data;

		if (data->priority != priority)
			resourceModified = true;

		data->priority = priority;		
		resourceCache.get<IdIndex>().replace(resource,CachedResource(data));
	}

	data->callbacks.insert(watcher);
	//if (resourceModified)
		//updateResource(data);


	return data;	
}


ResourceData * ResourceManager::loadResource(string resourceId, cv::Mat & image, float priority, IResourceWatcher * watcher)
{
	auto resource = resourceCache.get<IdIndex>().find(resourceId);	
	
	bool resourceModified = false;

	ResourceData * data = NULL;
	if (resource == resourceCache.get<IdIndex>().end())
	{
		data = new ResourceData(resourceId, priority, image);
		data->TextureState = ResourceState::Empty;
		data->ImageState = ResourceState::ImageLoaded;
		resourceCache.insert(CachedResource(data));
		resourceModified = true;
	}
	else 
	{		
		data = resource->Data;		
		//if (data->priority != priority || data->image.size() != image.size())
		//{
		resourceModified = true;
		data->image = image;
		data->priority = priority;		
		resourceCache.get<IdIndex>().replace(resource,CachedResource(data));
		//}
	}
		

	data->callbacks.insert(watcher);
	if (resourceModified)
		updateResource(data);


	return data;	
}

void ResourceManager::updateTextureState(ResourceData * data, bool load)
{
	if (load)
	{
		switch (data->TextureState)
		{
		case ResourceState::Empty:
			if (data->ImageState == ResourceState::ImageLoaded)
			{
				data->TextureState = ResourceState::TextureLoading;
				TextureLoader::getInstance().loadTextureFromImage(data->resourceId, data->priority, data->image, [this,data](GLuint textureId, int taskStatus){
					this->textureLoaded(data,textureId,taskStatus);
				});
			}
			break;		
		case ResourceState::TextureLoading:
			//TextureLoader::getInstance().updateTask(data->resourceId,data->priority);
			break;
		case ResourceState::TextureLoaded:
			//nothing to do
			break;
		}
	}
	else
	{
		switch (data->TextureState)
		{
		case ResourceState::Empty:
			//nothing to do
			break;		
		case ResourceState::TextureLoading:
			TextureLoader::getInstance().cancelTask(data->resourceId);
			data->TextureState = ResourceState::Empty;
			break;
		case ResourceState::TextureLoaded:
			TexturePool::getInstance().releaseTexture(data->textureId);
			data->TextureState = ResourceState::Empty;
			break;
		}
	}
}

void ResourceManager::updateImageState(ResourceData * data, bool load)
{
	if (load)
	{
		switch(data->ImageState)
		{
		case ResourceState::Empty:
			data->ImageState = ResourceState::ImageLoading;
			ImageLoader::getInstance().startLoadingImage(data->resourceId,data->imageURI,(data->imageURI.find("http") == string::npos) ? 0 : 2,data->priority);
			break;
			
		case ResourceState::ImageLoadError:
			cout << "Error!" << endl;
		case ResourceState::ImageLoaded:
			//nothing to do
			break;
		case ResourceState::ImageLoading:		
			//ImageLoader::getInstance().updateTask(data->resourceId,data->priority);			
			break;
		}
	}
	else
	{
		switch(data->ImageState)
		{
		case ResourceState::ImageLoadError:
		case ResourceState::Empty:			
			//nothing to do
			break;
		case ResourceState::ImageLoaded:
			data->image.release();
			data->ImageState = ResourceState::Empty;
			break;
		case ResourceState::ImageLoading:		
			ImageLoader::getInstance().cancelTask(data->resourceId);			
			data->ImageState = ResourceState::Empty;
			break;
		}
	}

}

void ResourceManager::updateResource(ResourceData * data)
{	
	updateImageState(data, data->priority < imageLoadThreshold);
	updateTextureState(data, data->priority < textureLoadThreshold);
}

int compareResourcePriority(const void * a, const void * b) 
{
	ResourceData * r1 = (ResourceData*)a;
	ResourceData * r2 = (ResourceData*)b;
	if ( r1->priority <  r2->priority ) return -1;
	if ( r1->priority == r2->priority ) return 0;
	if ( r1->priority >  r2->priority ) return 1;
}


void ResourceManager::cleanupCache()
{
	int textureCacheSizeBytes = GlobalConfig::tree()->get<int>("ResourceCache.TextureCacheSize") * BytesToMB;
	int imageLimit = GlobalConfig::tree()->get<int>("ResourceCache.ImageCount");


	//if (size > textureLimit || size > imageLimit)
	{
		int size = resourceCache.size();
		//ResourceData ** resources = new ResourceData*[size];

		vector<ResourceData*> rVector;

		int i=0;
		for (auto cache = resourceCache.get<IdIndex>().begin(); cache != resourceCache.get<IdIndex>().end(); cache++)
		{
			rVector.push_back(cache->Data);
			//resources[i++] = cache->Data;
		}

		//std::qsort(resources,size,sizeof(resources[0]),compareResourcePriority);

		std::sort(rVector.begin(),rVector.end(),[](ResourceData * r1, ResourceData * r2) {
			return r1->priority > r2->priority;
		});

		for (int i=0; i<size;i++)
		{
			ResourceData * data = rVector.at(i);
						
			int dataSize = 160000; //200x200 px @ 4bpp
			
			if (data->image.data != NULL)
				dataSize = data->image.size().area() * 4;

			textureCacheSizeBytes -= dataSize;

			updateImageState(data,imageLimit-- > 0);
			//updateTextureState(data,textureCacheSizeBytes > 0);

			if (imageLimit == 0)
				imageLoadThreshold = data->priority;

			if (textureCacheSizeBytes <= 0) 
				textureLoadThreshold = data->priority;
		}

		//delete resources;
	}
}

void ResourceManager::update()
{
	ImageLoader::getInstance().update();

	updateTaskMutex.lock();
	while (!updateThreadTasks.empty())
	{
		updateThreadTasks.front()();
		updateThreadTasks.pop();
	}
	updateTaskMutex.unlock();

	TextureLoader::getInstance().update();

	cleanupCache();
}

void ResourceManager::textureLoaded(ResourceData * data, GLuint textureId, int taskStatus)
{

	if (taskStatus > 0)
		data->TextureState = ResourceState::TextureLoaded;
	else
		data->TextureState = ResourceState::Empty;				

	data->textureId = textureId;
	
	data->TextureState = ResourceState::TextureLoaded;
	for (auto it = data->callbacks.begin(); it != data->callbacks.end(); it++)
	{
		(*it)->resourceUpdated(data);
	}
}