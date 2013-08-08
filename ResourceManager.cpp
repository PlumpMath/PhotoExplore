#include "ResourceManager.h"
#include "TextureLoader.hpp"
#include "ImageLoader.hpp"
#include "GlobalConfig.hpp"
#include "TexturePool.h"
#include "LeapDebug.h"
#include <boost/filesystem.hpp>


ResourceManager::ResourceManager()
{
	currentImageCacheSize = 0;
	currentTextureCacheSize = 0;
	textureLoadThreshold = 1000;
	imageLoadThreshold = 1000;

	cacheCleanupTimer.start();
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


void ResourceManager::releaseResource(string resourceId, IResourceWatcher * watcher)
{
	auto resource = resourceCache.get<IdIndex>().find(resourceId);	
	
	ResourceData * data = NULL;
	if (resource != resourceCache.get<IdIndex>().end())
	{
		data = resource->Data;		
		data->callbacks.erase(watcher);
	}

	if (data->callbacks.empty())
		data->priority = 100;
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
	if (data->priority <= imageLoadThreshold)
		updateImageState(data, true);
	
	if (data->priority <= textureLoadThreshold)
		updateTextureState(data, true);

	resourcesChanged = true;
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
	static bool debugLogging = GlobalConfig::tree()->get<bool>("ResourceCache.DebugLogging");



	textureCacheMaxSize *= GlobalConfig::tree()->get<double>("ResourceCache.TextureCacheGCCollectRatio");
	imageCacheMaxSize *= GlobalConfig::tree()->get<double>("ResourceCache.ImageCacheGCCollectRatio");

	Logger::stream("ResourceManager","INFO") << "Starting Cache Clean. TexStorage = " << currentTextureCacheSize/BytesToMB  << " ImageCacheSize = " << currentImageCacheSize/BytesToMB << endl;
	
	Timer cacheTimer;
	cacheTimer.start();


	int permObjects = 0;
	int transientObjects = 0;
	int loadedImages = 0, loadedTextures = 0;
	int priorityZeroObjects = 0;

	//ResourceData ** resources = new ResourceData*[size];
	vector<ResourceData*> rVector;
	for (auto cache = resourceCache.get<IdIndex>().begin(); cache != resourceCache.get<IdIndex>().end(); cache++)
	{

		if (cache->Data->priority >= 0)
			rVector.push_back(cache->Data);
		else
			permObjects++;
		//resources[i++] = cache->Data;
	}

	transientObjects = rVector.size();

	//std::qsort(resources,size,sizeof(resources[0]),compareResourcePriority);

	std::sort(rVector.begin(),rVector.end(),[](ResourceData * r1, ResourceData * r2) {
		return r1->priority < r2->priority;
	});

	if (cacheTimer.millis() > 0)
		Logger::stream("ResourceManager","TIME") << "Sort took " << cacheTimer.millis() << " ms" << endl;

	long imageCacheSize = 0, textureCacheSize = 0;
	bool imgCacheFull = false, textureCacheFull = false;

	stringstream debugStream;
	debugStream.precision(2);
	debugStream.width(4);

	for (int i=0; i<rVector.size();i++)
	{
		string changed = "";
		ResourceData * data = rVector.at(i);
		//ResourceData * data = resources[i];

		float resourceSize = calculateResourceSize(data);

		if (data->ImageState == ResourceState::ImageLoaded || data->ImageState == ResourceState::ImageLoading)
		{				
			if (imageCacheSize < imageCacheMaxSize)
			{ 
				loadedImages++;
				imageCacheSize += resourceSize;
			}
			else 
			{
				updateImageState(data,false);
				if (!imgCacheFull)
				{					
					imageLoadThreshold = data->priority;
					imgCacheFull = true;
					if (debugLogging) changed += "X(-I)X";

				} else if (debugLogging)
				{
					changed += "(-I)";
				}
			}
		}
		else if (imageCacheSize < imageCacheMaxSize)
		{ 
			if (debugLogging) changed += "(+I)";
			loadedImages++;
			updateImageState(data,true);
			imageCacheSize += resourceSize;
		}

		if (data->TextureState == ResourceState::TextureLoaded || data->TextureState == ResourceState::TextureLoading)
		{			
			if (textureCacheFull || textureCacheSize >= textureCacheMaxSize)
			{
				updateTextureState(data,false);
				if (!textureCacheFull)
				{
					textureLoadThreshold = data->priority;
					textureCacheFull = true;
					if (debugLogging) changed += " X(-T)X";
				}
				else if (debugLogging)
					changed += " (-T)";
				//Logger::stream("ResourceManager","DEBUG") << "Unloading texture for resource " << data->resourceId << " with priority " << data->priority << endl;
			}
			else
			{
				if (data->priority == 100)
					priorityZeroObjects++;
				loadedTextures++;
				textureCacheSize += resourceSize;
			}
		}
		else if (!textureCacheFull && textureCacheSize  < textureCacheMaxSize)
		{
			if (data->priority == 100)
				priorityZeroObjects++;
			loadedTextures++;
			updateTextureState(data,true);
			textureCacheSize += resourceSize;			
			if (debugLogging) changed += " (+T)";
		}

		if (debugLogging)
		{			
			debugStream.setf( std::ios::fixed, std:: ios::floatfield );
			debugStream << changed << "  IMG[" << data->ImageState << "] TEX[" << data->TextureState << "] \t\t SIZE[" << resourceSize/BytesToMB << "] \t\t P [" << data->priority << "] " << data->resourceId
			<<" I[" << ((double)imageCacheSize/BytesToMB) << "] Tc[" << ((double)textureCacheSize/BytesToMB) << "]" 
			<< "\r\n";
		}
	}	
	currentTextureCacheSize = textureCacheSize;
	currentImageCacheSize = imageCacheSize;
	
	LeapDebug::instance->showValue("1. Loaded images",loadedImages);
	LeapDebug::instance->showValue("2. Loaded textures",loadedTextures);
	LeapDebug::instance->showValue("3. Perm objects",permObjects);
	LeapDebug::instance->showValue("4. Temp objects",transientObjects);
	LeapDebug::instance->showValue("5. P100 objects",priorityZeroObjects);

	LeapDebug::instance->showValue("6. Total objects",permObjects+transientObjects);

	stringstream ss;
	ss << currentTextureCacheSize/BytesToMB;
	LeapDebug::instance->showValue("7. Texture cache", ss.str());
	
	stringstream ss2;
	ss2 <<  currentImageCacheSize/BytesToMB;
	LeapDebug::instance->showValue("8. Image cache",ss2.str());

	
	LeapDebug::instance->showValue("9. Img PT",imageLoadThreshold);
	LeapDebug::instance->showValue("10. Tex PT",textureLoadThreshold);

	
	LeapDebug::instance->showValue("99. CacheState \r\n",debugStream.str());

	Logger::stream("ResourceManager","TIME") << "Cache clean took " << cacheTimer.millis() << " ms" << endl;
	Logger::stream("ResourceManager","INFO") << "Clean complete. TexCacheSize= " << currentTextureCacheSize/BytesToMB  << " ImageCacheSize = " << currentImageCacheSize/BytesToMB << endl;
	//Logger::stream("ResourceManager","INFO") << "Texture threshold set to: " << textureLoadThreshold << " Img threshold set to : " << imageLoadThreshold << endl;

	//delete resources;
	//}
}

void ResourceManager::update()
{	
	static double textureCacheMaxSize = GlobalConfig::tree()->get<double>("ResourceCache.TextureCacheSize") * BytesToMB;
	static double imageCacheMaxSize = GlobalConfig::tree()->get<double>("ResourceCache.ImageCacheSize")* BytesToMB;

	Timer updateTimer;
	updateTimer.start();
	ImageLoader::getInstance().update();
	LeapDebug::instance->plotValue("ImgCache",Colors::LimeGreen,updateTimer.millis() * 20);

	
	updateTimer.start();
	updateTaskMutex.lock();
	while (!updateThreadTasks.empty())
	{
		updateThreadTasks.front()();
		updateThreadTasks.pop();
	}
	updateTaskMutex.unlock();
	if (updateTimer.millis() > 0)
	{
		Logger::stream("ResourceManager","TIME") << "Task updates took " << updateTimer.millis() << " ms " << endl;
	}

	updateTimer.start();
	TextureLoader::getInstance().update();	
	LeapDebug::instance->plotValue("Tex",Colors::Yellow,updateTimer.millis() * 20);
	
	//if (updateTimer.millis() > 0)
	//	Logger::stream("ResourceManager","TIME") << "TextureLoader update took " << updateTimer.millis() << " ms " << endl;
	
	bool cacheFull = (currentTextureCacheSize >= textureCacheMaxSize || currentImageCacheSize > imageCacheMaxSize);

	if ((resourcesChanged && cacheCleanupTimer.millis() > GlobalConfig::tree()->get<double>("ResourceCache.CacheGCFrequency")) || 
		(cacheFull && cacheCleanupTimer.millis() > GlobalConfig::tree()->get<double>("ResourceCache.FullCacheGCFrequency")))
	{
		resourcesChanged = false;
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