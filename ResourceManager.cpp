#include "ResourceManager.h"
#include "TextureLoader.hpp"
#include "ImageLoader.hpp"
#include "GlobalConfig.hpp"
#include "TexturePool.h"


ResourceManager::ResourceManager()
{
	currentImageCacheSize = 0;
	currentTextureCacheSize = 0;
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
			currentImageCacheSize += data->image.size().area() * 4;
		}
	});
	updateTaskMutex.unlock();
}

ResourceData * ResourceManager::watchResource(string resourceId, IResourceWatcher * watcher)
{
	auto resource = resourceCache.get<IdIndex>().find(resourceId);	
	
	ResourceData * data = NULL;
	if (resource != resourceCache.get<IdIndex>().end())
	{
		data = resource->Data;		
		data->callbacks.insert(watcher);
	}
	return data;	
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
	if (resourceModified)
		updateResource(data);

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
		data->image = image;
		data->priority = priority;

		resourceModified = true;		
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
			TextureLoader::getInstance().updateTask(data->resourceId,data->priority);
			break;
		case ResourceState::TextureLoaded:
			//nothing to do
			break;
		}
	}
	else
	{
		int texWidth,texHeight;
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

			//glBindTexture(GL_TEXTURE_2D,data->textureId);
			//glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&texWidth);
			//glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&texHeight);
			//TexturePool::getInstance().releaseTexture(data->textureId);
			//currentTextureCacheSize -= (texWidth*texHeight*4);	
			//glBindTexture(GL_TEXTURE_2D,NULL);

			data->TextureState = ResourceState::Empty;			
			TexturePool::getInstance().releaseTexture(data->textureId);
			data->textureId = NULL;

			for (auto it = data->callbacks.begin(); it != data->callbacks.end(); it++)
			{
				(*it)->resourceUpdated(data);
			}
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
			break;
		case ResourceState::ImageLoaded:
			//nothing to do
			break;
		case ResourceState::ImageLoading:		
			ImageLoader::getInstance().updateTask(data->resourceId,data->priority);			
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


float ResourceManager::calculateResourceSize(ResourceData * data)
{
	if (data->ImageState == ResourceState::ImageLoaded)
	{
		return data->image.size().area() * 4;
	}

	if (data->TextureState == ResourceState::TextureLoaded)
	{
		int texWidth,texHeight;
		glBindTexture(GL_TEXTURE_2D,data->textureId);
		glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&texWidth);
		glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&texHeight);	
		return texWidth*texHeight*4;
	}

	return 0;
}

void ResourceManager::cleanupCache()
{
	double textureCacheMaxSize = GlobalConfig::tree()->get<double>("ResourceCache.TextureCacheSize") * BytesToMB;
	double imageCacheMaxSize = GlobalConfig::tree()->get<double>("ResourceCache.ImageCacheSize")* BytesToMB;
	

	textureCacheMaxSize *= GlobalConfig::tree()->get<double>("ResourceCache.TextureCacheGCCollectRatio");
	imageCacheMaxSize *= GlobalConfig::tree()->get<double>("ResourceCache.ImageCacheGCCollectRatio");

	Logger::stream("ResourceManager","INFO") << "Starting Cache Clean. TexStorage = " << currentTextureCacheSize/BytesToMB  << " ImageCacheSize = " << currentImageCacheSize/BytesToMB << endl;
	
	Timer cacheTimer;
	cacheTimer.start();

	//ResourceData ** resources = new ResourceData*[size];
	vector<ResourceData*> rVector;
	for (auto cache = resourceCache.get<IdIndex>().begin(); cache != resourceCache.get<IdIndex>().end(); cache++)
	{
		rVector.push_back(cache->Data);
		//resources[i++] = cache->Data;
	}

	//std::qsort(resources,size,sizeof(resources[0]),compareResourcePriority);

	std::sort(rVector.begin(),rVector.end(),[](ResourceData * r1, ResourceData * r2) {
		return r1->priority < r2->priority;
	});

	if (cacheTimer.millis() > 0)
		Logger::stream("ResourceManager","TIME") << "Sort took " << cacheTimer.millis() << " ms" << endl;

	long imageCacheSize = 0, textureCacheSize = 0;
	bool imgCacheFull = false, textureCacheFull = false;

	for (int i=0; i<rVector.size();i++)
	{
		//string changed = "";
		ResourceData * data = rVector.at(i);
		//ResourceData * data = resources[i];

		float resourceSize = calculateResourceSize(data);

		if (data->ImageState == ResourceState::ImageLoaded)
		{	
			if (imgCacheFull || imageCacheSize  + resourceSize >= imageCacheMaxSize)
			{
				updateImageState(data,false);
				if (!imgCacheFull)
				{					
					imageLoadThreshold = data->priority;
					imgCacheFull = true;
					//changed += "[-I!]";
				}
					//changed += "[-I]";
				//Logger::stream("ResourceManager","DEBUG") << "Unloading image for resource " << data->resourceId << " with priority " << data->priority << endl;
			}
			else
			{
				imageCacheSize += resourceSize;
			}
		}
		else if (!imgCacheFull && imageCacheSize < imageCacheMaxSize)
		{ 
			//changed += "[+I]";
			updateImageState(data,true);
			imageCacheSize += resourceSize;
		}

		if (data->TextureState == ResourceState::TextureLoaded)
		{				
			if (textureCacheFull || textureCacheSize + resourceSize >= textureCacheMaxSize)
			{
				updateTextureState(data,false);
				if (!textureCacheFull)
				{
					textureLoadThreshold = data->priority;
					textureCacheFull = true;
					//changed += " (-T!)";
				}
				//else
					//changed += " (-T)";
				//Logger::stream("ResourceManager","DEBUG") << "Unloading texture for resource " << data->resourceId << " with priority " << data->priority << endl;
			}
			else
			{
				textureCacheSize += resourceSize;
			}
		}
		else if (!textureCacheFull && textureCacheSize < textureCacheMaxSize)
		{
			updateTextureState(data,true);
			textureCacheSize += resourceSize;
			//changed = true;
			//changed += " (+T)";
		}

		//if (GlobalConfig::tree()->get<bool>("ResourceCache.DebugLogging"))
			//Logger::stream("ResourceManager","DEBUG") << changed << "  IMG[" << data->ImageState << "] TEX[" << data->TextureState << "] \t\t SIZE[" << resourceSize/BytesToMB << "] \t\t P [" << data->priority << "] " << data->resourceId << endl;
	}	
	currentTextureCacheSize = textureCacheSize;
	currentImageCacheSize = imageCacheSize;
	
	Logger::stream("ResourceManager","TIME") << "Cache clean took " << cacheTimer.millis() << " ms" << endl;
	Logger::stream("ResourceManager","INFO") << "Clean complete. TexCacheSize= " << currentTextureCacheSize/BytesToMB  << " ImageCacheSize = " << currentImageCacheSize/BytesToMB << endl;
	//Logger::stream("ResourceManager","INFO") << "Texture threshold set to: " << textureLoadThreshold << " Img threshold set to : " << imageLoadThreshold << endl;

	//delete resources;
	//}
}

void ResourceManager::update()
{
	Timer updateTimer;
	updateTimer.start();
	ImageLoader::getInstance().update();
	
	if (updateTimer.millis() > 0)
		Logger::stream("ResourceManager","TIME") << "Image manager update took " << updateTimer.millis() << " ms " << endl;

	
	updateTimer.start();
	updateTaskMutex.lock();
	while (!updateThreadTasks.empty())
	{
		updateThreadTasks.front()();
		updateThreadTasks.pop();
	}
	updateTaskMutex.unlock();
	if (updateTimer.millis() > 0)
		Logger::stream("ResourceManager","TIME") << "Task updates took " << updateTimer.millis() << " ms " << endl;


	updateTimer.start();
	TextureLoader::getInstance().update();
	if (updateTimer.millis() > 0)
		Logger::stream("ResourceManager","TIME") << "TextureLoader update took " << updateTimer.millis() << " ms " << endl;
	
	long textureCacheMaxSize = GlobalConfig::tree()->get<int>("ResourceCache.TextureCacheSize") * BytesToMB;
	long imageCacheMaxSize = GlobalConfig::tree()->get<int>("ResourceCache.ImageCacheSize")* BytesToMB;
	bool cacheFull = (currentTextureCacheSize >= textureCacheMaxSize || currentImageCacheSize > imageCacheMaxSize);

	if (cacheCleanupTimer.seconds() > GlobalConfig::tree()->get<int>("ResourceCache.CacheGCFrequency") || 
		(cacheFull && GlobalConfig::tree()->get<int>("ResourceCache.FullCacheGCFrequency")))
	{
		cleanupCache();
		cacheCleanupTimer.start();
	}

}

void ResourceManager::textureLoaded(ResourceData * data, GLuint textureId, int taskStatus)
{
	//if (data->TextureState == ResourceState::TextureLoaded)
	//{
	//	int texWidth,texHeight;
	//	glBindTexture(GL_TEXTURE_2D,data->textureId);
	//	glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&texWidth);
	//	glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&texHeight);
	//	TexturePool::getInstance().releaseTexture(data->textureId);
	//	currentTextureStorageSize -= (texWidth*texHeight*4);	
	//}

	data->TextureState = taskStatus;

	if (data->TextureState == ResourceState::TextureLoaded)
	{
		data->textureId = textureId;
		currentTextureCacheSize += data->image.size().area() * 4;
		//int texWidth,texHeight;
		//glBindTexture(GL_TEXTURE_2D,data->textureId);
		//glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&texWidth);
		//glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&texHeight);	
		//currentTextureStorageSize += (texWidth*texHeight*4);
	}

	for (auto it = data->callbacks.begin(); it != data->callbacks.end(); it++)
	{
		(*it)->resourceUpdated(data);
	}
}