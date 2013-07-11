#include "TextureManager.h"

using std::string;

TextureManager::TextureManager()
{
	initialize();
}

void TextureManager::dispose()
{	
	texturePoolMap[TextureType_Font]->releaseAll();
	texturePoolMap[TextureType_Image]->releaseAll();
		
	delete texturePoolMap[TextureType_Image];
	delete texturePoolMap[TextureType_Font];

	while (!bufferQueue.empty())
	{
		glDeleteBuffersARB(1,&bufferQueue.front());
		bufferQueue.pop();
	}

	for (auto it = textureCache.get<NameDetailIndex>().begin(); it != textureCache.get<NameDetailIndex>().end();it++)
	{
		string deletedURI = it->imageFileName;
		int deletedDetail = it->levelOfDetail;
		it = textureCache.get<NameDetailIndex>().erase(it);
		if (!resourceChangedCallback.empty())
		{
			resourceChangedCallback(deletedURI,deletedDetail,0);
		}

		if (it == textureCache.get<NameDetailIndex>().end())
			break;
	}
	textureCache.clear();
}

void TextureManager::initialize()
{
	bufferCount = 4;
	MaxTextureCount = 180;
	dirtyItems = 0;

	texturePoolMap[TextureType_Font] = new TexturePool(100);
	texturePoolMap[TextureType_Image] = new TexturePool(200);

	initBuffers(bufferCount);
}


void TextureManager::initBuffers(int bufferCount)
{			
	GLuint * pboIds = new GLuint[bufferCount];
	glGenBuffersARB(bufferCount,pboIds);

	for (int i=0;i<bufferCount;i++)
	{
		bufferQueue.push(pboIds[i]);
	}

	GLenum errorCode = glGetError();
	if (errorCode != GL_NO_ERROR)
	{
		cout << "PixelBuffer generation error: " << errorCode << "\n";;
	}
}

void TextureManager::freeBuffers(int count)
{
	if (textureTaskQueue.size() > 0)
	{
		textureTaskQueue.front()->forceComplete();
		checkTaskComplete();
	}
}


void TextureManager::setTexturePriority(string textureURI, int levelOfDetail, int textureType, float priority)
{
	TextureCache * currentCache = getCacheForTextureType(textureType);
	auto it = currentCache->get<NameDetailIndex>().find(boost::make_tuple(textureURI, levelOfDetail));

	bool dirty = false, exists = false;
	if (it != currentCache->get<NameDetailIndex>().end())
	{
		CachedTexture c = *it;
		if (c.priority != priority)
		{
			c.priority = priority;
			currentCache->get<NameDetailIndex>().replace(it, c);		
			dirty = true;
		}
	}
	else
	{
		dirty = true;
		currentCache->insert(CachedTexture(0,textureURI, levelOfDetail,-1,-1,CachedTexture::LoadState_Empty, priority));
	}

	if (textureType == TextureType_Font)
		fontCacheDirty = fontCacheDirty|| dirty;
	else if (textureType == TextureType_Image && dirty)
		dirtyItems = 1;
}


void TextureManager::destroyTexture(string textureURI, int levelOfDetail, int textureType)
{	
	TextureCache * currentCache = getCacheForTextureType(textureType);

	auto imageIt = currentCache->get<NameDetailIndex>().find(boost::make_tuple(textureURI, levelOfDetail));
	if (imageIt != currentCache->get<NameDetailIndex>().end())
	{
		GLuint releaseId = imageIt->textureId;
		std::remove_if(pendingTextures.begin(), pendingTextures.end(), [imageIt](PendingLoadTask & t) { return t.image->imageFileName.compare(imageIt->imageFileName) == 0;});
		std::for_each(textureTaskQueue.begin(), textureTaskQueue.end(), [releaseId](TextureLoadTask * t) { if (t->getTextureId() == releaseId) t->cancel();});
		
		//cout << "Destroyed texture " << textureURI << "[" << levelOfDetail << "] ID = " << releaseId << " \n";
		currentCache->get<NameDetailIndex>().erase(imageIt);			
		texturePoolMap[textureType]->releaseTexture(releaseId);
		
		if (!resourceChangedCallback.empty())
		{
			resourceChangedCallback(textureURI,levelOfDetail,0);
		}
	}
}

bool TextureManager::isTextureLoaded(string textureURI, int levelOfDetail)
{
	auto texIt = textureCache.get<NameDetailIndex>().find(boost::make_tuple(textureURI, levelOfDetail));
	if (texIt != textureCache.get<NameDetailIndex>().end())
		return (texIt->loaded == CachedTexture::LoadState_Complete);
	else
		return false;
}

bool TextureManager::isTextureLoaded(GLuint textureId, int textureType)
{
	TextureCache * currentCache = getCacheForTextureType(textureType);

	auto texIt = currentCache->get<TextureIdIndex>().find(textureId);
	if (texIt != currentCache->get<TextureIdIndex>().end())
		return texIt->loaded == 2;	
	else
	{
		return false;
	}
}



void TextureManager::unloadTexture(GLuint textureId, int textureType)
{			
	TextureCache * currentCache = getCacheForTextureType(textureType);
	auto imageIt = currentCache->get<TextureIdIndex>().find(textureId);
	if (imageIt != currentCache->get<TextureIdIndex>().end())
	{
		std::remove_if(pendingTextures.begin(), pendingTextures.end(), [imageIt](PendingLoadTask & t){ return t.image->imageFileName.compare(imageIt->imageFileName) == 0;});
		std::for_each(textureTaskQueue.begin(), textureTaskQueue.end(), [imageIt](TextureLoadTask * t) { if (t->getTextureId() == imageIt->textureId) t->cancel();});

		CachedTexture ct = (*imageIt);
		ct.loaded = CachedTexture::LoadState_Empty;
		ct.textureId = 0;		
				
		texturePoolMap[textureType]->releaseTexture(imageIt->textureId);
		currentCache->get<TextureIdIndex>().replace(imageIt, ct);			
				
		if (!resourceChangedCallback.empty())
			resourceChangedCallback(imageIt->imageFileName,imageIt->levelOfDetail,0);	
	}
}

void TextureManager::setResourceChangedCallback(boost::function<void(string,LevelOfDetail,GLuint)> _resourceChangedCallback)
{
	this->resourceChangedCallback = _resourceChangedCallback;
}


void TextureManager::cleanupFontCache()
{
	static int printSeq;
	
	Timer cleanTimer;
	cleanTimer.start();
	
	TextureCache * currentCache = getCacheForTextureType(TextureType_Font);

	if (currentCache->size() <= 0)
		return;
	
	//auto startLoad = currentCache->get<PriorityIndex>().end();
	//startLoad--;

	//while (startLoad != currentCache->get<PriorityIndex>().begin())
		
	for (auto startLoad = currentCache->get<PriorityIndex>().begin(); startLoad != currentCache->get<PriorityIndex>().end(); startLoad++)
	{
		if (startLoad->priority >= LowestPriority)
		{			
			std::remove_if(pendingTextures.begin(), pendingTextures.end(), [startLoad](PendingLoadTask & t) { return t.image->imageFileName.compare(startLoad->imageFileName) == 0;});
			std::for_each(textureTaskQueue.begin(), textureTaskQueue.end(), [startLoad](TextureLoadTask * t) { if (t->getTextureId() == startLoad->textureId) t->cancel();});
			
			cout << "Cleaning up font " << startLoad->imageFileName << endl;
			startLoad = currentCache->get<PriorityIndex>().erase(startLoad);

			if (startLoad == currentCache->get<PriorityIndex>().end())
				break;
		}
	}
}


void TextureManager::cleanupTextureCache()
{
	static int printSeq;
	
	Timer cleanTimer;
	cleanTimer.start();
	
	if (textureCache.get<PriorityIndex>().size() <= 0)
		return;

	int itemsAdded = 0, itemsRemoved = 0, itemsFailed = 0;
	int actionCount =15, loadedCount = 0;
	bool skipDecrement = false;
	//do
	
	for (auto startLoad = textureCache.get<PriorityIndex>().begin(); startLoad != textureCache.get<PriorityIndex>().end(); startLoad++)
	{		
		loadedCount++;

		if (loadedCount < MaxTextureCount)
		{
			if (!startLoad->isLoaded())
			{
				MultiResolutionImage * imageInfo = ImageManager::getInstance()->loadImage(startLoad->imageFileName,startLoad->levelOfDetail);
				if (imageInfo != NULL && imageInfo->isValid() && imageInfo->isLoaded(startLoad->levelOfDetail))
				{	
					loadTextureFromImage(imageInfo,startLoad->levelOfDetail,LoadImportance::Anytime,startLoad->priority, TextureType_Image);
				}
				else
				{
					//cout << "Tried to load " << startLoad->imageFileName << "[" << startLoad->levelOfDetail << "] but it was too late." << endl;
					loadedCount--;
					itemsFailed++;
					startLoad = textureCache.get<PriorityIndex>().erase(startLoad);
					skipDecrement = true;
					if (startLoad == textureCache.get<PriorityIndex>().end())
						break;
					else
						continue;
				}				
				itemsAdded++;
				actionCount--;
			}
		}
		else
		{
			if (startLoad->isLoaded())
			{
				actionCount--;
				itemsRemoved++;
				unloadTexture(startLoad->textureId, TextureType_Image);
			}
		}
	}

	//cout << "Clean took " << cleanTimer.millis() << " ms, +" << itemsAdded << ", -" << itemsRemoved << ", >" << itemsFailed << " Total=" << textureCache.size() << endl;//" Actions lef = " << dirtyItems << endl;

	if (actionCount > 0)
		dirtyItems = 0;

	//if ((itemsAdded != 0 || itemsRemoved != 0))// && printSeq++ % 5 == 0)
	//{
	//	cout << "***** Cache state ***** " << endl;
	//	auto startLoad = textureCache.get<PriorityIndex>().end();
	//	while (startLoad-- != textureCache.get<PriorityIndex>().begin())
	//	{
	//		cout << "[" << startLoad->isLoaded() << "] - " << startLoad->priority << " - " << startLoad->imageFileName << " \n";
	//	}

	//}

}


void TextureManager::update(int stepCount)
{
	if (pendingTextures.size() > 0)
	{
		for (int i=0;i<pendingTextures.size();i++)
		{
			PendingLoadTask value = pendingTextures.at(i);

			MultiResolutionImage * oldInfo = value.image;
			LevelOfDetail levelOfDetail = value.levelOfDetail;
			if (startLoadTask(value, LoadImportance::Anytime))
			{
				pendingTextures.erase(pendingTextures.begin() + i);
			}				
			break;			
		}
	}
	
	if (textureTaskQueue.size() > 0)
	{
		textureTaskQueue.front()->onFrame();
		checkTaskComplete();
	}

	if (dirtyItems > 0)
	{
		cleanupTextureCache();
		//dirtyItems = 0;
	}

	if (fontCacheDirty)
	{
		cleanupFontCache();
		fontCacheDirty = false;
	}
}


void TextureManager::checkTaskComplete()
{
	if (textureTaskQueue.front()->isComplete())
	{				
		bufferQueue.push(textureTaskQueue.front()->getSourceBuffer()); 

		TextureCache * currentCache = getCacheForTextureType(textureTaskQueue.front()->textureType);
		auto textureIt = currentCache->get<TextureIdIndex>().find(textureTaskQueue.front()->getTextureId());			
		if (textureIt != currentCache->get<TextureIdIndex>().end())
		{
			CachedTexture c = (*textureIt);
			c.loaded = CachedTexture::LoadState_Complete;
			currentCache->get<TextureIdIndex>().replace(textureIt,c);
			resourceChangedCallback(c.imageFileName,c.levelOfDetail,c.textureId);
		}
		//else if (textureTaskQueue.front()->state > 0)
		//{
		//	cout << "Task completed for unknown texture " << textureTaskQueue.front()->getTextureId() << ", Type = " << textureTaskQueue.front()->textureType << endl;
		//}

		delete textureTaskQueue.front();
		textureTaskQueue.pop_front();
	}
}


void TextureManager::loadTextureFromImage(string key, LevelOfDetail levelOfDetail, int textureType, float priority, cv::Mat & imgMat)
{
	TextureCache * currentCache = getCacheForTextureType(textureType);
	
	auto texIt = currentCache->get<NameDetailIndex>().find(boost::make_tuple(key, levelOfDetail));
	if (texIt != currentCache->get<NameDetailIndex>().end() && texIt->isLoaded())
	{		
		if (texIt->priority != priority)
		{
			CachedTexture c = (*texIt);
			//cout << "Restored " << key << " from " << c.priority << " to " << priority << endl;
			c.priority = priority;
			currentCache->get<NameDetailIndex>().replace(texIt,c);
		}

		return;
	}

	MultiResolutionImage * imageInfo = new MultiResolutionImage(key,1,true);
	imageInfo->addImage(levelOfDetail,imgMat);

	if (imageInfo != NULL && imageInfo->isValid() && imageInfo->isLoaded(levelOfDetail))
	{	
		loadTextureFromImage(imageInfo,levelOfDetail,LoadImportance::Anytime,priority, textureType);
	}
}

GLuint TextureManager::getLoadedTexture(string imageURI, LevelOfDetail levelOfDetail, int textureType)
{
	TextureCache * currentCache = getCacheForTextureType(textureType);
	auto texIt = currentCache->get<NameDetailIndex>().find(boost::make_tuple(imageURI, levelOfDetail));
	if (texIt != currentCache->get<NameDetailIndex>().end())
	{	
		return texIt->textureId;
	}
	else
		return NULL;
}

bool TextureManager::loadTexture(string imageURI, LevelOfDetail levelOfDetail, float loadImportance, float priority)
{		
	auto texIt = textureCache.get<NameDetailIndex>().find(boost::make_tuple(imageURI, levelOfDetail));
	if (texIt != textureCache.get<NameDetailIndex>().end() && texIt->isLoaded())
	{		
		return true;
	}

	MultiResolutionImage * imageInfo = ImageManager::getInstance()->loadImage(imageURI,levelOfDetail);

	if (imageInfo != NULL && imageInfo->isValid() && imageInfo->isLoaded(levelOfDetail))
	{	
		loadTextureFromImage(imageInfo,levelOfDetail,loadImportance,priority, TextureType_Image);
		return true;
	}
	return false;
}

bool TextureManager::startLoadTask(PendingLoadTask  & loadTask, int loadImportance)
{	
	GLuint sourceBuffer = 0;

	if (bufferQueue.size() ==  0)
	{
		//We need more buffers cap'n!			
		if (loadImportance == LoadImportance::Soon)
		{
			freeBuffers(1);
		}
		else
		{	
			return false;
		}
	}		
	
	sourceBuffer = bufferQueue.front();
	bufferQueue.pop();		

	//cout << "Starting load task: ID=" << loadTask.textureInfo.textureId << " URI = " << loadTask.image->imageFileName << "[" << loadTask.levelOfDetail << "]" << endl;
	TextureLoadTask * task = new TextureLoadTask(sourceBuffer,loadTask.image->getImage(loadTask.levelOfDetail),loadTask.textureInfo,loadTask.textureType == TextureType_Font,loadTask.textureType);
	textureTaskQueue.push_back(task);	
	return true;
}

TextureCache * TextureManager::getCacheForTextureType(int textureType)
{
	switch (textureType)
	{
	case TextureType_Font:
		return &fontTextureCache;
	case TextureType_Image:
		return &textureCache;
	default:
		return NULL;
	}
}

TextureInfo TextureManager::getNewTextureInfo(MultiResolutionImage * imageInfo, LevelOfDetail levelOfDetail, float priority, int textureType)
{
	static Timer startTimer;
	startTimer.start();

	cv::Mat img = imageInfo->getImage(levelOfDetail);

	GLint bytesPerPixel = 4;
	GLsizei imageWidth = img.size().width;
	GLsizei imageHeight = img.size().height;
	GLenum pixelFormat = 6408;

	GLuint textureId;

	TextureCache * currentCache = getCacheForTextureType(textureType);

	if (currentCache)
	{
		auto texIt = currentCache->get<NameDetailIndex>().find(boost::make_tuple(imageInfo->imageFileName, levelOfDetail));
		if (texIt != currentCache->get<NameDetailIndex>().end())
		{		
			CachedTexture c = (*texIt);

			if (c.textureId <= 0)
				c.textureId = texturePoolMap[textureType]->getTexture(0);
			
			textureId = c.textureId;
			c.textureWidth = imageWidth;
			c.textureHeight = imageHeight;
			c.priority = priority;
			c.loaded = CachedTexture::LoadState_Ongoing;

			currentCache->get<NameDetailIndex>().replace(texIt,c);
		}		
		else
		{
			textureId = texturePoolMap[textureType]->getTexture(imageWidth*imageHeight*bytesPerPixel);
			currentCache->insert(CachedTexture(textureId,imageInfo->imageFileName, levelOfDetail,imageWidth,imageHeight,CachedTexture::LoadState_Ongoing, priority));
		}
	}
	else
	{
		cout << "Unknown texture type: " << textureType << endl;
		textureId = texturePoolMap[textureType]->getTexture(imageWidth*imageHeight*bytesPerPixel);
	}
	
	return TextureInfo(textureId,pixelFormat, bytesPerPixel,imageWidth,imageHeight);	
}


void TextureManager::loadTextureFromImage(MultiResolutionImage * imageInfo, LevelOfDetail levelOfDetail, int loadImportance, float priority, int textureType)
{				
	TextureInfo texInfo;
	
	texInfo = getNewTextureInfo(imageInfo,levelOfDetail,priority, textureType);

	PendingLoadTask tryLoad = PendingLoadTask(imageInfo,texInfo,levelOfDetail, textureType);

	if (!startLoadTask(tryLoad,loadImportance))
	{
		pendingTextures.push_back(tryLoad);
	}
}


