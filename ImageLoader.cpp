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


int ImageLoader::maxConcurrentLoads = 8;

void ImageLoader::setResourceChangedCallback(boost::function<void(string resourceId,int statusCode, cv::Mat imgMat)> _resourceChangedCallback)
{
	this->resourceChangedCallback = _resourceChangedCallback;
}

void ImageLoader::startLoadingImage(string resourceId, string loadURI, int imageType, float priority)
{
	loadQueueMutex.lock();
	auto res = imageLoadQueue.get<ImgTaskQueue::NameIndex>().find(resourceId);
	if (res == imageLoadQueue.get<ImgTaskQueue::NameIndex>().end())
	{	
		imageLoadQueue.insert(ImgTaskQueue::ImageLoadTask(resourceId,loadURI,imageType, priority));			
		startLoadingThreads(2);			
	}
	else 
	{			
		if (!res->locked)
		{			
			imageLoadQueue.get<ImgTaskQueue::NameIndex>().replace(res, ImgTaskQueue::ImageLoadTask(resourceId,loadURI,imageType, priority));			
		}
	}
	loadQueueMutex.unlock();
}

void ImageLoader::startLoadingThreads(int threadCount)
{
	if (!loadThreadRunning)
	{
		loadThreadRunning = true;
		for (int i=0;i<threadCount;i++)
		{
			new boost::thread(runLoadThread, &imageLoadQueue, &loadQueueMutex);
		}		
	}
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

void ImageLoader::update()
{
}

void ImageLoader::runLoadThread(ImgTaskQueue::ImageTaskQueue * loadQueue, boost::mutex * loadQueueMutex) 
{
	static int maxImageSize = GlobalConfig::tree()->get<int>("ResourceCache.MaxImageSize");

	while (true)
	{		
		loadQueueMutex->lock();
				
		if (loadQueue->get<ImgTaskQueue::PriorityIndex>().size() == 0)
		{
			loadQueueMutex->unlock();
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
			continue;
		}

		auto front = loadQueue->get<ImgTaskQueue::PriorityIndex>().begin();
		while (front != loadQueue->get<ImgTaskQueue::PriorityIndex>().end() && front->locked)
			front++;

		if (front == loadQueue->get<ImgTaskQueue::PriorityIndex>().end())
		{
			loadQueueMutex->unlock();
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
			continue;
		}
		
		ImgTaskQueue::ImageLoadTask loadTask =  (*front);
		loadTask.locked = true;
		loadQueue->get<ImgTaskQueue::PriorityIndex>().replace(front,loadTask);

		loadQueueMutex->unlock();
		

		if (loadTask.imageType == ImgTaskQueue::FacebookImage)
		{
			//while (maxConcurrentLoads <= 0)
			//{
			//	boost::this_thread::sleep(boost::posix_time::milliseconds(100));
			//}
		
			//{
				//maxConcurrentLoads--;
				//cout << "Starting load task: " << loadTask.imageFileName << "\n";
				CefRefPtr<CefTaskRunner> runner = CefTaskRunner::GetForThread(TID_IO);
				CefRefPtr<DownloadImageTask> dlTask = new DownloadImageTask(loadTask);
			
				if (runner != NULL)
					runner->PostTask(dlTask.get());
			//}
		}
		else
		{			
			Timer loadTimer;
			loadTimer.start();
			cv::Mat imgMat;
			
			try
			{
				imgMat  = cv::imread(loadTask.imageURI, -1 );

				if (imgMat.data != NULL)
				{
					cv::cvtColor(imgMat, imgMat, CV_BGR2RGBA, 4);

					if (4 * imgMat.size().area() >  maxImageSize * BytesToMB)
					{
						float scale = (float)(maxImageSize * BytesToMB)/((float)imgMat.size().area()*4.0f);
						
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
					{
						ImageLoader::getInstance().resourceChangedCallback(loadTask.resourceId, ResourceState::ImageLoaded, imgMat);
					}
				}
			}
			catch (std::exception & e)
			{
				if (imgMat.data != NULL)
					imgMat.release();
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

	maxConcurrentLoads++;
	//cout <<"Concurrent load count = " << maxConcurrentLoads << "\n";

	try
	{
		//cout << "Decoding image[" << loadTask.imageFileName << "]. Size = " << dataVector.size() << "b \n";
		cv::Mat imgMat  = cv::imdecode(dataVector,-1);
		//LeapImageOut << "CV decode took " << loadTimer.millis() << " ms @ " << ((imgMat.size().area()*4) / BytesToMB) / loadTimer.seconds() << "MB/s \n";

		if (imgMat.data != NULL)
		{
			cv::cvtColor(imgMat, imgMat, CV_BGR2RGBA, 4);

			loadTask.complete = true;
			loadTask.success = true;
		}	
		else
		{
			//cout << "URL = " << "Image data = " << std::string((const char * )dataVector.data()) << "\n";
			Logger::stream("ImageLoader","ERROR") << "Invalid image file: " << loadTask.resourceId << endl;		
		}

		loadQueueMutex.lock();
		auto it = imageLoadQueue.get<ImgTaskQueue::NameIndex>().find(loadTask.resourceId);
		imageLoadQueue.get<ImgTaskQueue::NameIndex>().erase(it);
		loadQueueMutex.unlock();
		
		resourceChangedCallback(loadTask.resourceId,(loadTask.success) ? ResourceState::ImageLoaded : ResourceState::ImageLoadError,imgMat);

	}
	catch (std::exception & e)
	{		
		Logger::stream("ImageLoader","ERROR") << "Exception decoding image: " << e.what() << endl;				
		resourceChangedCallback(loadTask.resourceId,ResourceState::ImageLoadError,cv::Mat());
	}
}

int ImageLoader::processCompletedTasks(int count)
{
	return 1;	
}

				//if (loadTask.levelOfDetail & LevelOfDetail_Types::Preview)
				//{
				//	float targetHeight = 512, targetWidth = 512;
				//	int adjustedWidth, adjustedHeight;

				//	float xScale = targetWidth/((float)imgMat.size().width);
				//	float yScale = targetHeight/((float)imgMat.size().height);

				//	if (xScale < yScale)	
				//		xScale = yScale;	
				//	else 	
				//		yScale = xScale;


				//	float originalWidth = imgMat.size().width, originalHeight = imgMat.size().height;

				//	adjustedWidth = (int)ceil(xScale * originalWidth);
				//	adjustedHeight = (int)ceil(yScale * originalHeight);

				//	Timer resizeTimer;
				//	resizeTimer.start();

				//	cv::Size newSize = cv::Size(adjustedWidth,adjustedHeight);
				//	cv::Mat resized = cv::Mat(newSize, CV_8UC4);
				//	cv::resize(imgMat,resized,newSize,0,0,cv::INTER_AREA);
				//	loadTask.loadResults[LevelOfDetail_Types::Preview] = resized;

				//	if (loadTask.levelOfDetail & LevelOfDetail_Types::Full)
				//	{
				//		loadTask.loadResults[LevelOfDetail_Types::Full] = imgMat;		
				//	}
				//	else
				//	{
				//		imgMat.release();			

				//		int * data = new int[2];
				//		data[0] = (int)originalWidth;
				//		data[1] = (int)originalHeight;
				//		loadTask.loadResults[LevelOfDetail_Types::Full] = cv::Mat(1,2,CV_32S,data);
				//	}
				//}
				//else if (loadTask.levelOfDetail & LevelOfDetail_Types::Full)
				//{
				//	loadTask.loadResults[LevelOfDetail_Types::Full] = imgMat;		
				//}		