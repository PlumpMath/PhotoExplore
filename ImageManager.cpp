#include "ImageManager.h"
#include <iostream>
#include <fstream>
#include "SDLTimer.h"
#include "Types.h"
#include "FacebookLoader.h"
#include "CefDownloadClient.h"

#include <include/cef_app.h>
#include <include/cef_urlrequest.h>

#include <boost/filesystem.hpp>

#define ImageCacheMaxSize 1000



float ImgCache::CachedImage::getPriority() const
{
	return imageInfo->getDataPriority();
}

bool ImgCache::CachedImage::hasData() const 
{
	return imageInfo->isLoaded(1) || imageInfo->isLoaded(16);
}

ImageManager * ImageManager::instance = NULL;
int ImageManager::maxConcurrentLoads = 8;

void ImageManager::setResourceChangedCallback(boost::function<void(string resourceId, MultiResolutionImage * affectedImage)> _resourceChangedCallback)
{
	this->resourceChangedCallback = _resourceChangedCallback;
}




void ImageManager::loadMockImage(string name, string filename)
{
	maxConcurrentLoads = -100;	
}


void ImageManager::setImageRelevance(string filename, LevelOfDetail levelOfDetail, float priority, int imageType, string imageURI, cv::Size2i imageSize)
{ 
	Timer timer;
	timer.start();

	MultiResolutionImage * multiResImage = NULL;
	
	if (imageType < 0)
	{
		if (filename.find_first_of('.') == -1 && filename.find("fake") == string::npos)
			imageType = 2;
		else
			imageType = 0;
	}

	imageCacheMutex.lock();
	auto imageIndex = imageCache.get<ImgCache::NameIndex>().find(filename);
	if (imageIndex != imageCache.get<ImgCache::NameIndex>().end())
		multiResImage = imageIndex->imageInfo;		
	
	bool imageChanged = false;

	if (multiResImage == NULL)
	{
		multiResImage = new MultiResolutionImage(filename,imageType,true);
		multiResImage->setDataPriority(priority);
		multiResImage->setMinimumDetail(levelOfDetail);

		if (imageSize.width > 0 && imageURI.size() > 0)
			multiResImage->addImageMetaData(levelOfDetail,ImageMetaData(imageSize,imageURI));
		
		imageChanged = true;
		imageCache.insert(ImgCache::CachedImage(multiResImage, filename));
	}
	else if (imageIndex->getPriority() != priority || multiResImage->getMinimumDetail() != levelOfDetail)
	{
		multiResImage->setDataPriority(priority);
		multiResImage->setMinimumDetail(levelOfDetail);
				
		if (imageSize.width > 0 && imageURI.size() > 0)
			multiResImage->addImageMetaData(levelOfDetail,ImageMetaData(imageSize,imageURI));

		imageChanged = true;
		
		imageCache.get<ImgCache::NameIndex>().replace(imageIndex,ImgCache::CachedImage(multiResImage, filename));
	}

	bool load = false;
	if (imageChanged)
	{
		dirtyItems = 1;
		if (!multiResImage->isLoaded(levelOfDetail))
		{	
			load = true;
		}
	}

	imageCacheMutex.unlock();

	if (load)
	{
		//if (levelOfDetail == LevelOfDetail_Types::Full)
		//{
		//	if (!multiResImage->isLoaded(LevelOfDetail_Types::Preview))
		//		levelOfDetail = LevelOfDetail_Types::Full | LevelOfDetail_Types::Preview;
		//}
		startLoadingImage(multiResImage,levelOfDetail);	
	}

}

void ImageManager::startLoadingImage(MultiResolutionImage * multiResImage, LevelOfDetail levelOfDetail)
{
	bool defined;
	string loadURI = multiResImage->getURI(levelOfDetail,defined);

	if (!defined)
	{
		if (multiResImage->imageType == 2) //Image is not yet defined, use FB picture edge
		{
			std::string token = std::string(GlobalConfig::TestingToken);

			std::stringstream urlStream;
			urlStream << "https://graph.facebook.com/";
			urlStream << multiResImage->imageFileName << "/picture?width=600&height=600&";
			urlStream << "method=get&redirect=true&access_token=" << token;
			loadURI = urlStream.str();
		}
		else
		{
			loadURI = multiResImage->imageFileName; //Local image??
		}
	}

	loadQueueMutex.lock();
	auto res = imageLoadQueue.get<ImgTaskQueue::NameIndex>().find(multiResImage->imageFileName);
	if (res == imageLoadQueue.get<ImgTaskQueue::NameIndex>().end())
	{			
		//cout << "Creating load task for image " << multiResImage->imageFileName << ", LoD[" << levelOfDetail <<"] with importance = " <<  multiResImage->getDataPriority() << endl;
		imageLoadQueue.insert(ImgTaskQueue::ImageLoadTask(multiResImage->imageFileName,multiResImage->imageType,levelOfDetail,loadURI,multiResImage->getDataPriority()));			
		startLoadingThreads(2);			
	}
	else //if ( multiResImage->getDataPriority() >= 0)
	{			
		if (!res->locked)
		{
			//cout << "Updating load task for image " << multiResImage->imageFileName << "[" << levelOfDetail <<"]  with priority = " << multiResImage->getDataPriority() << "\n";// and detail " << (levelOfDetail | res->levelOfDetail) << "\n";
			imageLoadQueue.get<ImgTaskQueue::NameIndex>().replace(res, ImgTaskQueue::ImageLoadTask(multiResImage->imageFileName, multiResImage->imageType, levelOfDetail | res->levelOfDetail,loadURI, multiResImage->getDataPriority()));
		}
	}
	loadQueueMutex.unlock();
}

void ImageManager::cleanupCache()
{		
	static int printSeq;
	
	Timer cleanTimer;
	cleanTimer.start();
	
	if (imageCache.get<ImgCache::RelevanceIndex>().size() < ImageCacheMaxSize)
		return;

	//int itemsAdded = 0, itemsRemoved = 0, itemsFailed = 0;
	//int actionCount =15, loadedCount = 0;
	//bool skipDecrement = false;
	//do

	set<string> cleanedImages;
	
	//auto endLoad =  imageCache.get<ImgCache::RelevanceIndex>().rend();//equal_range(boost::make_tuple(true)).second;
	
	int startCount = imageCache.get<ImgCache::RelevanceIndex>().size();
	imageCacheMutex.lock();
	int deleteCount = 100;
	//for (; startLoad != endLoad && deleteCount-- > 0; startLoad++)
	while (imageCache.get<ImgCache::RelevanceIndex>().size() > (ImageCacheMaxSize - deleteCount))
	{			
		auto lastItem = imageCache.get<ImgCache::RelevanceIndex>().rbegin();//equal_range(boost::make_tuple(true)).first;
		++lastItem;
		auto eraseIter = lastItem.base();
    	imageCache.get<ImgCache::RelevanceIndex>().erase(eraseIter);
		//startLoad = ImgCache::ImageCache::index<ImgCache::RelevanceIndex>::type::reverse_iterator(tempIter); 
		//startLoad++;
	}

	cout << "ImageCache trimmed from " << startCount << " to " << imageCache.get<ImgCache::RelevanceIndex>().size() << endl;
	
	//for (; startLoad != endLoad; startLoad++)
	//{				
	//	loadedCount++;
	//	if (startLoad->hasData())
	//	{
	//		if (loadedCount > ImageCacheMaxSize)
	//		{
	//			cleanedImages.insert(startLoad->imageFileName);
	//			actionCount--;
	//			itemsRemoved++;
	//			startLoad->imageInfo->cleanup();
	//			minimumLoadLevel[0] = startLoad->getPriority();
	//		}
	//	}
	//	else
	//	{
	//		if (loadedCount <= ImageCacheMaxSize)
	//		{
	//			startLoadingImage(startLoad->imageInfo,startLoad->imageInfo->getMinimumDetail());
	//		}
	//	}
	//}

	//if (cleanTimer.millis() > 10)
	//	cout << "Clean took " << cleanTimer.millis() << " ms, +" << itemsAdded << ", -" << itemsRemoved << ", >" << itemsFailed << " Total=" << imageCache.size() << endl;//" Actions lef = " << dirtyItems << endl;

	//if (actionCount > 0)
	//	dirtyItems = 0;

	////if (printSeq++ % 5 == 0)
	//if (actionCount < 14)
	//{
	//	cout << "***** Cache state ***** " << endl;
	//	for (auto print = imageCache.get<ImgCache::RelevanceIndex>().begin(); print != imageCache.get<ImgCache::RelevanceIndex>().end(); print++)
	//	{		
	//		if (cleanedImages.count(print->imageFileName) > 0)
	//			cout << "X";
	//		cout << "[" << print->hasData() << "] - " << print->getPriority() << " - " << print->imageFileName << " \n";
	//	}
	//}
	imageCacheMutex.unlock();

}

MultiResolutionImage * ImageManager::loadImage(string filename, LevelOfDetail levelOfDetail)
{
	auto imageIndex = imageCache.get<ImgCache::NameIndex>().find(filename);
	if (imageIndex != imageCache.get<ImgCache::NameIndex>().end())
		return imageIndex->imageInfo;	
	else
		return NULL;
}

void ImageManager::releaseImage(string filename, LevelOfDetail levelOfDetail)
{
	auto imgIt = imageCache.get<ImgCache::NameIndex>().find(filename);
	if (imgIt != imageCache.get<ImgCache::NameIndex>().end())
	{
		if (imgIt->imageInfo->isLoaded(levelOfDetail))
		{		
			imgIt->imageInfo->getImage(levelOfDetail).release();
		}

		imageCache.get<ImgCache::NameIndex>().erase(imgIt);
	}
}

void ImageManager::startLoadingThreads(int threadCount)
{
	if (!loadThreadRunning)
	{
		loadThreadRunning = true;
		for (int i=0;i<threadCount;i++)
		{
			new boost::thread(runLoadThread, &imageLoadQueue, &loadQueueMutex, &imageCache, &imageCacheMutex);
		}		
	}
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
			ImageManager::getInstance()->handleCompletedTask(loadTask, dataVector);
			taskComplete = true;
		}
	}

};

void ImageManager::update()
{
	if (dirtyItems > 0)
	{
		cleanupCache();	
	}
}

void ImageManager::runLoadThread(ImgTaskQueue::ImageTaskQueue * loadQueue, boost::mutex * loadQueueMutex, ImgCache::ImageCache * imageCache, boost::mutex * imageCacheMutex)
{
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
			while (maxConcurrentLoads <= 0)
			{
				boost::this_thread::sleep(boost::posix_time::milliseconds(100));
			}
		
			{
				maxConcurrentLoads--;
				//cout << "Starting load task: " << loadTask.imageFileName << "\n";
				CefRefPtr<CefTaskRunner> runner = CefTaskRunner::GetForThread(TID_IO);
				CefRefPtr<DownloadImageTask> dlTask = new DownloadImageTask(loadTask);
			
				if (runner != NULL)
					runner->PostTask(dlTask.get());
			}
		}
		//else if (!true)
		//{		
		//	CefTaskRunner::GetForThread(TID_FILE).get()->PostTask(new LoadLocalImageTask(loadTask));
		//}
		else
		{
			
			Timer loadTimer;
			loadTimer.start();

			//cout << "Loading image[" << loadTask.levelOfDetail << "]" << loadTask.imageName << " with priority " << loadTask.priority << "\n";
			cv::Mat imgMat  = cv::imread(loadTask.imageURI, -1 );
			//LeapImageOut << "CV Load took " << loadTimer.millis() << " ms @ " << ((imgMat.size().area()*4) / BytesToMB) / loadTimer.seconds() << "MB/s \n";
			
			//boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

			if (imgMat.data != NULL)
			{
				cv::cvtColor(imgMat, imgMat, CV_BGR2RGBA, 4);
				loadTask.success = true;
			}	
		
			if (loadTask.success)
			{
				if (loadTask.levelOfDetail & LevelOfDetail_Types::Preview)
				{
					float targetHeight = 512, targetWidth = 512;
					int adjustedWidth, adjustedHeight;

					float xScale = targetWidth/((float)imgMat.size().width);
					float yScale = targetHeight/((float)imgMat.size().height);

					if (xScale < yScale)	
						xScale = yScale;	
					else 	
						yScale = xScale;


					float originalWidth = imgMat.size().width, originalHeight = imgMat.size().height;

					adjustedWidth = (int)ceil(xScale * originalWidth);
					adjustedHeight = (int)ceil(yScale * originalHeight);

					Timer resizeTimer;
					resizeTimer.start();

					cv::Size newSize = cv::Size(adjustedWidth,adjustedHeight);
					cv::Mat resized = cv::Mat(newSize, CV_8UC4);
					cv::resize(imgMat,resized,newSize,0,0,cv::INTER_AREA);
					loadTask.loadResults[LevelOfDetail_Types::Preview] = resized;

					if (loadTask.levelOfDetail & LevelOfDetail_Types::Full)
					{
						loadTask.loadResults[LevelOfDetail_Types::Full] = imgMat;		
					}
					else
					{
						imgMat.release();			

						int * data = new int[2];
						data[0] = (int)originalWidth;
						data[1] = (int)originalHeight;
						loadTask.loadResults[LevelOfDetail_Types::Full] = cv::Mat(1,2,CV_32S,data);
					}
				}
				else if (loadTask.levelOfDetail & LevelOfDetail_Types::Full)
				{
					loadTask.loadResults[LevelOfDetail_Types::Full] = imgMat;		
				}
			}
			else
			{
				cout << loadTask.imageName << "[" << loadTask.levelOfDetail << "] FAILED TO LOAD\n";
			}
		
			loadQueueMutex->lock();
			auto it = loadQueue->get<ImgTaskQueue::NameIndex>().find(loadTask.imageName);
			loadQueue->get<ImgTaskQueue::NameIndex>().erase(it);
			loadQueueMutex->unlock();
			
			imageCacheMutex->lock();
			auto cacheRes = imageCache->get<ImgCache::NameIndex>().find(loadTask.imageName);
			if (cacheRes != imageCache->get<ImgCache::NameIndex>().end())
			{
				cacheRes->imageInfo->setValid(loadTask.success);
				cacheRes->imageInfo->addImage(loadTask);
			}			
			imageCacheMutex->unlock();
			
			if (!ImageManager::getInstance()->resourceChangedCallback.empty())
				ImageManager::getInstance()->resourceChangedCallback(loadTask.imageName,cacheRes->imageInfo);
		}
	}
}

void ImageManager::handleCompletedTask(ImgTaskQueue::ImageLoadTask loadTask, vector<unsigned char> & dataVector)
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

			loadTask.loadResults[loadTask.levelOfDetail] = imgMat;
			//loadTask.loadResults[LevelOfDetail_Types::Full] = imgMat;
		}	
		else
		{
			//cout << "URL = " << "Image data = " << std::string((const char * )dataVector.data()) << "\n";
			cout << "Invalid image file: " << loadTask.imageName << "\n";;
		}

		loadQueueMutex.lock();
		auto it = imageLoadQueue.get<ImgTaskQueue::NameIndex>().find(loadTask.imageName);
		imageLoadQueue.get<ImgTaskQueue::NameIndex>().erase(it);
		loadQueueMutex.unlock();

		if (loadTask.success)
		{
			imageCacheMutex.lock();
			auto cacheRes = imageCache.get<ImgCache::NameIndex>().find(loadTask.imageName);
			if (cacheRes != imageCache.get<ImgCache::NameIndex>().end())
			{
				cacheRes->imageInfo->setValid(loadTask.success);
				cacheRes->imageInfo->addImage(loadTask);		

				if (!resourceChangedCallback.empty())
					resourceChangedCallback(loadTask.imageName,cacheRes->imageInfo);
			}

			imageCacheMutex.unlock();
			dirtyItems = 1;
		}
	}
	catch (std::exception & e)
	{
		cout << "Exception while decoding image: " << e.what() << endl;
	}
}

int ImageManager::processCompletedTasks(int count)
{
	return 1;	
}
