#include "ImageLoader.hpp"
#include <iostream>
#include <fstream>
#include "SDLTimer.h"
#include "Types.h"
#include "FacebookLoader.h"
#include "CefDownloadClient.h"

#include <include/cef_app.h>
#include <include/cef_urlrequest.h>

#include <boost/filesystem.hpp>

using namespace ImgTaskQueue;
int ImageLoader::maxConcurrentLoads = 8;

ImageLoader::ImageLoader()
{
	for (int i=0;i<GlobalConfig::tree()->get<int>("ImageLoader.IOThreadCount");i++)
	{
		new boost::thread(runLoadThread, &imageLoadQueue, &loadQueueMutex);
	}		

	for (int i=0;i<GlobalConfig::tree()->get<int>("ImageLoader.TransformThreadCount");i++)
	{
		new boost::thread(runTransformThread, &imageTransformQueue, &transformQueueMutex);
	}	
}

void ImageLoader::setResourceChangedCallback(boost::function<void(string resourceId,int statusCode, cv::Mat imgMat)> _resourceChangedCallback)
{
	this->resourceChangedCallback = _resourceChangedCallback;
}

void ImageLoader::startLoadingImage(string resourceId, string loadURI, int imageType, float priority, boost::function<void(cv::Mat&)> transform)
{
	loadQueueMutex.lock();
	auto res = imageLoadQueue.get<ImgTaskQueue::NameIndex>().find(resourceId);
	if (res == imageLoadQueue.get<ImgTaskQueue::NameIndex>().end())
	{	
		imageLoadQueue.insert(ImgTaskQueue::ImageLoadTask(resourceId,loadURI,imageType, priority,transform));		
	}
	else 
	{			
		if (!res->locked)
		{			
			imageLoadQueue.get<ImgTaskQueue::NameIndex>().replace(res, ImgTaskQueue::ImageLoadTask(resourceId,loadURI,imageType, priority,transform));			
		}
	}
	loadQueueMutex.unlock();
}

void ImageLoader::cancelTask(string resourceId)
{	
	loadQueueMutex.lock();
	auto res = imageLoadQueue.get<ImgTaskQueue::NameIndex>().find(resourceId);
	if (res != imageLoadQueue.get<ImgTaskQueue::NameIndex>().end())
	{					
		if (!res->locked)
		{			
			imageLoadQueue.get<ImgTaskQueue::NameIndex>().erase(res);
		}
	}
	loadQueueMutex.unlock();
}

void ImageLoader::updateTask(string resourceId, float newPriority)
{
	loadQueueMutex.lock();
	auto res = imageLoadQueue.get<ImgTaskQueue::NameIndex>().find(resourceId);
	if (res != imageLoadQueue.get<ImgTaskQueue::NameIndex>().end())
	{					
		if (!res->locked)
		{			
			ImgTaskQueue::ImageLoadTask task = *res;
			task.priority = newPriority;

			imageLoadQueue.get<ImgTaskQueue::NameIndex>().replace(res,task);		
		}
	}
	loadQueueMutex.unlock();
}


class DownloadImageTask : public FacebookDownloadTask
{
public:
	ImgTaskQueue::ImageLoadTask loadTask;


	DownloadImageTask(ImgTaskQueue::ImageLoadTask _loadTask) : 
	FacebookDownloadTask(),
		loadTask(_loadTask)
	{

	}

	string getTaskURL()
	{
		return loadTask.imageURI;
	}

	string getRequestedObject()
	{
		return "";
	}

	void processResult(vector<unsigned char> dataVector)
	{		
		if (!taskComplete)
		{
			ImageLoader::getInstance().handleCompletedTask(loadTask, dataVector);
			taskComplete = true;
		}
	}

};

void ImageLoader::postTransformTask(const ImageLoadTask & task)
{
	transformQueueMutex.lock();
	imageTransformQueue.insert(task);
	transformQueueMutex.unlock();
}

void ImageLoader::runTransformThread(ImgTaskQueue::ImageTaskQueue * transformQueue, boost::mutex * queueMutex) 
{
	while (true)
	{		
		queueMutex->lock();
				
		if (transformQueue->get<ImgTaskQueue::PriorityIndex>().size() == 0)
		{
			queueMutex->unlock();
			boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			continue;
		}

		auto first = transformQueue->get<PriorityIndex>().begin();
		ImageLoadTask loadTask(*first); //Copy
		transformQueue->get<PriorityIndex>().erase(first);

		queueMutex->unlock();

		try
		{
			loadTask.executeTransform();
		}
		catch (cv::Exception & e)
		{
			loadTask.cleanup();
			Logger::stream("ImageLoader","ERROR") << "Exception loading image: " << e.what() << endl;
			loadTask.success = false;
		}

		if (loadTask.success)
			ImageLoader::getInstance().resourceChangedCallback(loadTask.resourceId, ResourceState::ImageLoaded, loadTask.image);		
		else
			ImageLoader::getInstance().resourceChangedCallback(loadTask.resourceId, ResourceState::ImageLoadError,cv::Mat());		
	}
}




void ImageLoader::runLoadThread(ImgTaskQueue::ImageTaskQueue * loadQueue, boost::mutex * loadQueueMutex) 
{
	static float maxImageSize = (float)(GlobalConfig::tree()->get<double>("ResourceCache.MaxImageSize")*BytesToMB);

	while (true)
	{		
		loadQueueMutex->lock();
				
		if (loadQueue->get<ImgTaskQueue::PriorityIndex>().size() == 0)
		{
			loadQueueMutex->unlock();
			boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			continue;
		}

		auto front = loadQueue->get<ImgTaskQueue::PriorityIndex>().begin();
		while (front != loadQueue->get<ImgTaskQueue::PriorityIndex>().end() && front->locked)
			front++;

		if (front == loadQueue->get<ImgTaskQueue::PriorityIndex>().end())
		{
			loadQueueMutex->unlock();
			boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			continue;
		}
		
		ImgTaskQueue::ImageLoadTask loadTask =  (*front);
		loadTask.locked = true;
		loadQueue->get<ImgTaskQueue::PriorityIndex>().replace(front,loadTask);

		loadQueueMutex->unlock();
		

		if (loadTask.imageType == ImgTaskQueue::FacebookImage)
		{
			CefRefPtr<CefTaskRunner> runner = CefTaskRunner::GetForThread(TID_IO);
			CefRefPtr<DownloadImageTask> dlTask = new DownloadImageTask(loadTask);

			if (runner != NULL)
				runner->PostTask(dlTask.get());
		}
		else
		{			
			Timer loadTimer;
			loadTimer.start();			
			try
			{
				loadTask.image = cv::imread(loadTask.imageURI, -1 );

				if (loadTask.image.data != NULL)
				{			
					ImageLoader::getInstance().postTransformTask(loadTask);
				}
			}
			catch( cv::Exception& e )
			{
				if (loadTask.image.data != NULL)
					loadTask.image.release();
				Logger::stream("ImageLoader","ERROR") << "Exception loading image: " << e.what() << endl;
				ImageLoader::getInstance().resourceChangedCallback(loadTask.resourceId, ResourceState::ImageLoadError,cv::Mat());
			}
							
			loadQueueMutex->lock();

			auto it = loadQueue->get<ImgTaskQueue::NameIndex>().find(loadTask.resourceId);			
			if (it != loadQueue->get<ImgTaskQueue::NameIndex>().end())
				loadQueue->get<ImgTaskQueue::NameIndex>().erase(it);

			loadQueueMutex->unlock();			
		}
	}
}

void ImageLoader::handleCompletedTask(ImgTaskQueue::ImageLoadTask loadTask, vector<unsigned char> & dataVector)
{	
	Timer loadTimer;
	loadTimer.start();

	try
	{
		loadTask.image  = cv::imdecode(dataVector,-1);
		if (loadTask.image.data != NULL)
		{
			ImageLoader::getInstance().postTransformTask(loadTask);
		}	
		else
		{
			Logger::stream("ImageLoader","ERROR") << "Invalid image file: " << loadTask.resourceId << endl;		
		}

		loadQueueMutex.lock();
		auto it = imageLoadQueue.get<ImgTaskQueue::NameIndex>().find(loadTask.resourceId);
		imageLoadQueue.get<ImgTaskQueue::NameIndex>().erase(it);
		loadQueueMutex.unlock();
	}
	catch( cv::Exception& e )
	{
		Logger::stream("ImageLoader","ERROR") << "Exception decoding image: " << e.what() << endl;				
		resourceChangedCallback(loadTask.resourceId,ResourceState::ImageLoadError,cv::Mat());
	} catch (std::exception & e)
	{
		Logger::stream("ImageLoader","ERROR") << "Exception decoding image: " << e.what() << endl;				
		resourceChangedCallback(loadTask.resourceId,ResourceState::ImageLoadError,cv::Mat());
	}
	catch (...)
	{
		Logger::stream("ImageLoader","ERROR") << "Unexpected exception" << endl;			
		resourceChangedCallback(loadTask.resourceId,ResourceState::ImageLoadError,cv::Mat());
	}
}

int ImageLoader::processCompletedTasks(int count)
{
	return 1;	
}

	/*	if (4 * imgMat.size().area() >  maxImageSize)
					{
						float scale = (float)(maxImageSize)/((float)imgMat.size().area()*4.0f);
						
						float originalWidth = imgMat.size().width, originalHeight = imgMat.size().height;

						int adjustedWidth = (int)ceil(scale * originalWidth);
						int adjustedHeight = (int)ceil(scale * originalHeight);

						cv::Size newSize = cv::Size(adjustedWidth,adjustedHeight);
						cv::Mat resized = cv::Mat(newSize, CV_8UC4);
						cv::resize(imgMat,resized,newSize,0,0,cv::INTER_AREA);
						imgMat.release();
						
						ImageLoader::getInstance().resourceChangedCallback(loadTask.resourceId, ResourceState::ImageLoaded, resized);
					}
					else
					{*/