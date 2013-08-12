#include "TextureLoader.hpp"
#include "GlobalConfig.hpp"
#include "PixelBufferPool.hpp"
#include "ResourceManagerTypes.h"

TextureLoader::TextureLoader()
{
	maxConcurrentTasks = GlobalConfig::tree()->get<int>("TextureLoading.MaxConcurrentTasks");

	PixelBufferPool::getInstance().initBuffers(GlobalConfig::tree()->get<int>("TextureLoading.PixelBufferCount"));
}

void TextureLoader::loadTextureFromImage(std::string resourceId, float priority, cv::Mat & image, boost::function<void(GLuint textureId, int taskStatus)> callback)
{
	auto resource = loadQueue.get<NameIndex>().find(resourceId);

	if (resource == loadQueue.get<NameIndex>().end())
		loadQueue.insert(TextureLoadTask(resourceId, priority, image, callback));
	else
		loadQueue.get<NameIndex>().replace(resource, TextureLoadTask(resourceId, priority, image, callback));
}


void TextureLoader::update()
{	
	//for (auto task = activeTasks.begin(); task != activeTasks.end(); task++)
	for (int i=0;i<activeTasks.size();i++)
	{		
		activeTasks[i].update();

		if (activeTasks[i].getState() == LoadTaskState::Error || activeTasks[i].getState() == LoadTaskState::Complete)
		{
			activeTasks[i].callback(activeTasks[i].getTextureId(),(activeTasks[i].getState() == LoadTaskState::Complete) ? ResourceState::TextureLoaded : ResourceState::TextureLoadError);
			activeTasks.erase(activeTasks.begin() + i);
			i--;
		}
	}
	
	
	while (activeTasks.size() < maxConcurrentTasks && loadQueue.size() > 0)
	{
		auto load = loadQueue.get<PriorityIndex>().begin();
		activeTasks.push_back(*load);
		loadQueue.get<PriorityIndex>().erase(load);
	}
}

void TextureLoader::updateTask(string resourceId, float priority)
{
	auto update = loadQueue.get<NameIndex>().find(resourceId);
	if (update != loadQueue.get<NameIndex>().end())
	{
		TextureLoadTask current = *update;		
		current.setPriority(priority);
		loadQueue.get<NameIndex>().replace(update,current);
	}
}


void TextureLoader::cancelTask(std::string resourceId)
{
	auto remove = loadQueue.get<NameIndex>().find(resourceId);
	if (remove != loadQueue.get<NameIndex>().end())
	{
		loadQueue.get<NameIndex>().erase(remove);
	}
}

void TextureLoader::cancelAllTasks()
{
	//for (auto it = loadQueue.get<NameIndex>().begin(); it != loadQueue.get<NameIndex>().end(); it++)
	//{
	//	it->
	//}
}