#include "ResourceManager.h"

ResourceManager * ResourceManager::instance = NULL;

ResourceManager::ResourceManager()
{
	tMan = TextureManager::getInstance();
	tMan->setResourceChangedCallback(boost::bind(&ResourceManager::textureResourceChanged, this, _1,_2,_3));
	imgMan = ImageManager::getInstance();
	imgMan->setResourceChangedCallback(boost::bind(&ResourceManager::imageResourceChanged, this, _1,_2));
	resourcesChanged = false;
}


ResourceManager * ResourceManager::getInstance()
{
	if (instance == NULL)
		instance = new ResourceManager();

	return instance;
}

bool ResourceManager::getResourceDimensions(string resourceId, LevelOfDetail _levelOfDetail, cv::Size2i & imageSize)
{
	MultiResolutionImage * imageInfo = imgMan->loadImage(resourceId,_levelOfDetail);
	if (imageInfo != NULL && imageInfo->hasMetaData(_levelOfDetail))
	{
		imageSize = imageInfo->getImageSize(_levelOfDetail);
		return true;
	}
	else
		return false;
}

void ResourceManager::loadTextResource(TextDefinition textDefinition, _PriorityType loadPriority, IResourceWatcher * resourceOwner)
{
	cv::Mat textImage = TypographyManager::getInstance()->renderText(textDefinition.text,textDefinition.textColor,textDefinition.fontSize,textDefinition.textWrapSize);
	
	//cv::imwrite("G:\\FontDebug\\"+textDefinition.getKey()+".png",textImage);

	if (insertResource(textDefinition.getKey(),LevelOfDetail_Types::Full, loadPriority, resourceOwner))	
	{
		cout << "Loading text with key = " << textDefinition.getKey() << endl;
		tMan->loadTextureFromImage(textDefinition.getKey(),LevelOfDetail_Types::Full,TextureManager::TextureType_Font,loadPriority,textImage);
	}	
	else
	{
		resourceOwner->resourceUpdated(textDefinition.getKey(),true);
	}
}

//void ResourceManager::updateResource(Resource * resource)
//{
//	modifiedResources.push(resource);
//}


void ResourceManager::releaseTextResource(TextDefinition textDefinition, IResourceWatcher * resourceOwner)
{
	bool update = false;

	auto res = resourceCache.get<IdIndex>().find(textDefinition.getKey());
	if (res != resourceCache.get<IdIndex>().end())
	{
		Resource r = (*res);		
		if (r.callbacks.count(resourceOwner))
		{
			r.callbacks.erase(resourceOwner);
			update = r.callbacks.size() == 0;
			if (update)
				resourceCache.get<IdIndex>().erase(res);
			else
				resourceCache.get<IdIndex>().replace(res, r);
		}
	}

	if (update)
	{
		//cout << "Releasing text with key = " << textDefinition.getKey() << endl;
		tMan->setTexturePriority(textDefinition.getKey(),LevelOfDetail_Types::Full,TextureManager::TextureType_Font,LowestPriority);
		//tMan->destroyTexture(textDefinition.getKey(), LevelOfDetail_Types::Full, TextureManager::TextureType_Font);
	}
}


Resource * ResourceManager::insertResource(string resourceId, LevelOfDetail levelOfDetail, float loadPriority, IResourceWatcher * loadCallback)
{
	Resource * updatedResource = NULL;

	auto res = resourceCache.get<IdIndex>().find(resourceId);
	if (res != resourceCache.get<IdIndex>().end())
	{
		Resource r = (*res);		

		if (loadCallback != NULL && r.callbacks.count(loadCallback) == 0)
		{
			r.callbacks.insert(loadCallback);
		}
		
		bool update = (r.priority != loadPriority);
		update = update || (r.requestedDetail != levelOfDetail);

		r.resourceId = resourceId;
		r.priority = loadPriority;
		r.requestedDetail = levelOfDetail;		
		resourceCache.get<IdIndex>().replace(res, r);

		if (update)
			updatedResource = (Resource*)(&(*res));
	}
	else
	{
		Resource r = Resource();
		r.resourceId = resourceId;

		if (loadCallback != NULL)
			r.callbacks.insert(loadCallback);

		r.priority = loadPriority;
		r.requestedDetail = levelOfDetail;		
		resourceCache.insert(r);

		res = resourceCache.get<IdIndex>().find(resourceId);
		updatedResource = (Resource*)(&(*res));
	}
	return updatedResource;
}

Resource * ResourceManager::loadResource(string resourceId, LevelOfDetail levelOfDetail, _PriorityType loadPriority, IResourceWatcher * loadCallback)
{
	Timer time;
	time.start();
	Resource * updatedResource = insertResource(resourceId, levelOfDetail, loadPriority, loadCallback);
	if (updatedResource != NULL)	
	{		
		modifiedResources.push(updatedResource);
	}
	return updatedResource;
}


void ResourceManager::updateResource(Resource * updatedResource)
{
	modifiedResources.push(updatedResource);
}


void ResourceManager::releaseResource(string resourceId, LevelOfDetail levelOfDetail, IResourceWatcher * resourceOwner)
{
	bool update = false;

	auto res = resourceCache.get<IdIndex>().find(resourceId);
	if (res != resourceCache.get<IdIndex>().end())
	{
		Resource r = (*res);		
		if (r.callbacks.count(resourceOwner))
		{
			r.callbacks.erase(resourceOwner);
			resourceCache.get<IdIndex>().replace(res, r);
			update = r.callbacks.size() == 0;
		}
	}

	if (update)
	{
		imgMan->setImageRelevance(resourceId,levelOfDetail,LowestPriority,-1);
		MultiResolutionImage * loadedImage = imgMan->loadImage(resourceId,levelOfDetail);
		tMan->setTexturePriority(resourceId, levelOfDetail,TextureManager::TextureType_Image, LowestPriority);
	}
}

void ResourceManager::textureResourceChanged(string resourceId, LevelOfDetail levelOfDetail, GLuint glTextureId)
{	
	//cout << "Locking resource manager for TEX callback\n";
	//resourceMutex.lock();
	auto res = resourceCache.get<IdIndex>().find(resourceId);
	if (res != resourceCache.get<IdIndex>().end())
	{
		for (auto callback = res->callbacks.begin(); callback != res->callbacks.end(); callback++)
		{
			(*callback)->resourceUpdated(res->getResourceId(),true);
		}
	}
	//else
	//{
	//	cout << "Error! Got callback for non-existent texture " << resourceId << "[ " << levelOfDetail << "]\n";
	//}
	//resourceMutex.unlock();
	//cout << "Unlocked resource manager for TEX callback\n";
}

void ResourceManager::imageResourceChanged(string resourceId, MultiResolutionImage * affectedImage)
{		
	//cout << "Locking resource manager for IMG callback\n";
	//resourceMutex.lock();
	auto res = resourceCache.get<IdIndex>().find(resourceId);
	if (res != resourceCache.get<IdIndex>().end())
	//{
	//	modifiedResources.push((Resource*)&(*res));	
	//}
	//resourceMutex.unlock();
	//cout << "Unlocked resource manager for IMG callback\n";
	{
		if (affectedImage->isValid() && affectedImage->isLoaded(res->requestedDetail))
		{
			tMan->setTexturePriority(resourceId,res->requestedDetail,TextureManager::TextureType_Image, res->priority);
		}
		else
		{
			for (auto callback = res->callbacks.begin(); callback != res->callbacks.end(); callback++)
			{
				(*callback)->resourceUpdated(res->getResourceId(),false);
			}
		}
	}
}

void ResourceManager::update()
{
	//cout << "Locking resource manager\n";
	//resourceMutex.lock();
	int updateCount = 10;
	Timer updateTimer;
	updateTimer.start();
	while(!modifiedResources.empty() && (updateTimer.millis() < 2 || updateCount-- > 0))
	{		
		Resource * r = modifiedResources.front();
		modifiedResources.pop();

		imgMan->setImageRelevance(r->getResourceId(),r->requestedDetail,r->priority,-1);

		MultiResolutionImage * loadedImage = imgMan->loadImage(r->resourceId,r->requestedDetail);
		
		if (loadedImage == NULL || loadedImage->isValid()) 
			tMan->setTexturePriority(r->resourceId, r->requestedDetail, TextureManager::TextureType_Image, r->priority);
	}	
	//resourceMutex.unlock();
	//cout << "Unlocking Resource Manager"<<endl;
}