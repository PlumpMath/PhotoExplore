#define LeapImageOut cout
#include "Types.h"
#include "ResourceManagerTypes.h"

#include <boost/thread.hpp>
#include <map>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <opencv2/opencv.hpp>

#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/cef_app.h>
#include <include/cef_urlrequest.h>


#ifndef ImageManager_H_
#define ImageManager_H_

using namespace std;

namespace ImgTaskQueue 
{
	const int FacebookImage = 2;
	const int LocalImage = 0;

	struct ImageLoadTask {
		
		string imageName, imageURI;
		LevelOfDetail levelOfDetail;
		bool complete, success, locked;
		map<int,cv::Mat> loadResults;
		float priority;

		int imageType;
		
		ImageLoadTask(string _imageName, int _imageType, LevelOfDetail _levelOfDetail, string _imageURI, float _priority):
			imageName(_imageName),
			imageURI(_imageURI),
			priority(_priority),
			levelOfDetail(_levelOfDetail)
		{
			complete = success = locked = false;

			if (_imageType >= 0)
				imageType = _imageType;
			else
			{
				if(imageURI.find_first_of('.') < 0)
					imageType = FacebookImage;
				else
					imageType = LocalImage;
			}
		}

	};

	struct NameIndex{};
	struct PriorityIndex{};

	typedef boost::multi_index_container
		<
			ImageLoadTask,
			boost::multi_index::indexed_by
			<	
				boost::multi_index::ordered_non_unique
				<
					boost::multi_index::tag<PriorityIndex>,
					boost::multi_index::member<ImageLoadTask,float,&ImageLoadTask::priority>,
					std::less<float>
				>,
				boost::multi_index::hashed_unique
				<
					boost::multi_index::tag<NameIndex>,
					boost::multi_index::member<ImageLoadTask, std::string, &ImageLoadTask::imageName>
				>	
			>
		> ImageTaskQueue;
	
	typedef ImageTaskQueue::index<PriorityIndex>::type TPriorIdx;
};

struct ImageMetaData {
	
	cv::Size2i Size;
	string URI;

	ImageMetaData() :
		Size(-1,-1),
		URI("")
	{}

	ImageMetaData(cv::Size2i _Size, string _URI) :
		Size(_Size),
		URI(_URI)
	{}
};

class MultiResolutionImage {

private:
	std::map<LevelOfDetail,cv::Mat> imageMap;
	std::map<LevelOfDetail,ImageMetaData> imageMetaData;

	bool isValidImage;
	float dataPriority;
	int minimumDetail;
	
public:
	MultiResolutionImage(string imageFileName, int imageType, bool isValidImage = false);

	cv::Mat getImage(LevelOfDetail detailLevel);

	cv::Size2i getImageSize(LevelOfDetail detailLevel);

	void addImage(ImgTaskQueue::ImageLoadTask & result);

	void addImage(LevelOfDetail detailLevel, cv::Mat image);

	void addImageMetaData(LevelOfDetail detail, ImageMetaData & metaData);

	string getURI(LevelOfDetail detail, bool & uriDefined);

	void setValid(bool valid);
	bool isValid();

	bool isLoaded(LevelOfDetail detailLevel);

	bool hasMetaData(LevelOfDetail detailLevel);

	void setDataPriority(float dataPriority);

	float getDataPriority();

	void setMinimumDetail(int levelOfDetail);

	int getMinimumDetail();


	bool canCleanup();
	
	void cleanup();
	
	std::string getFilename();
	std::string imageFileName;
	int imageType;
};

namespace ImgCache 
{
	struct CachedImage
	{
		CachedImage (MultiResolutionImage * _imgInfo, std::string _imageFileName)
			: imageInfo(_imgInfo),
			imageFileName(_imageFileName)
		{
		}

		MultiResolutionImage * imageInfo;
		std::string imageFileName;

		float getPriority() const;

		bool hasData() const ;

	};

	struct NameIndex{};
	struct RelevanceIndex{};
	//struct RelevanceLoadedIndex{};


	typedef boost::multi_index_container
		<
			CachedImage,
			boost::multi_index::indexed_by
			<
				boost::multi_index::ordered_non_unique
				<
					boost::multi_index::tag<RelevanceIndex>,
					boost::multi_index::const_mem_fun<CachedImage,float,&CachedImage::getPriority> 
				>,
				//boost::multi_index::ordered_non_unique
				//<
				//	boost::multi_index::tag<RelevanceLoadedIndex>,
				//	boost::multi_index::composite_key
				//	<
				//		CachedImage,
				//		boost::multi_index::const_mem_fun<CachedImage,bool,&CachedImage::hasData>,
				//		boost::multi_index::const_mem_fun<CachedImage,float,&CachedImage::getPriority>
				//	>,
				//	boost::multi_index::composite_key_compare
				//	<
				//		std::greater<bool>,
				//		std::greater<float>
				//	>
				//>,
				boost::multi_index::hashed_unique
				<
					boost::multi_index::tag<NameIndex>,
					boost::multi_index::member<CachedImage, std::string, &CachedImage::imageFileName>
				>
			>
		> ImageCache;
	
	typedef ImageCache::index<NameIndex>::type TNameIdx;
};



class ImageManager  {
private:
	int dirtyItems;

public:	
	static ImageManager * getInstance()
	{
		if (instance == NULL)
			instance = new ImageManager();
		
		return instance;
	}

	ImageManager()
	{		
		dirtyItems = 0;
		loadThread = NULL;
		loadThreadRunning = false;
		minimumLoadLevel = new float[1];
		minimumLoadLevel[0] = -1;
	}

	void setResourceChangedCallback(boost::function<void(string resourceId, MultiResolutionImage * affectedImage)> resourceChangedCallback);

	void setImageRelevance(string resourceId, LevelOfDetail levelOfDetail, float priority, int imageType, string imageURI = "",cv::Size2i imageSize = cv::Size2i(-1,-1));
	MultiResolutionImage * loadImage(string filename, LevelOfDetail levelOfDetail);
	void releaseImage(string filename, LevelOfDetail levelOfDetail);
	void cancelLoadTasks();


	int processCompletedTasks(int count  = 1);

	void handleCompletedTask(ImgTaskQueue::ImageLoadTask loadTask, vector<unsigned char> & data);
	
	void loadMockImage(string name, string filename);

	void update();

	void startLoadingImage(MultiResolutionImage * multiResImage, LevelOfDetail levelOfDetail);
	
private:	
	static ImageManager * instance;

	static int maxConcurrentLoads;

	void startLoadingThreads(int count = 1);	

	ImgCache::ImageCache imageCache;
	ImgTaskQueue::ImageTaskQueue imageLoadQueue;
	boost::function<void(string resourceId, MultiResolutionImage * affectedImage)> resourceChangedCallback;

	boost::mutex loadQueueMutex, imageCacheMutex;

	float * minimumLoadLevel;
		
	static void runLoadThread(ImgTaskQueue::ImageTaskQueue * taskQueue,boost::mutex * loadQueueMutex, ImgCache::ImageCache * imageCache, boost::mutex * cacheMutex);
	boost::thread * loadThread;
	bool loadThreadRunning;

	void cleanupCache();

};

#endif
