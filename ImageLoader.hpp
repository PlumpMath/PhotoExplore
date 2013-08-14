#ifndef LEAPIMAGE_RESOURCE_MANAGER_IMAGELOADER_HPP_
#define LEAPIMAGE_RESOURCE_MANAGER_IMAGELOADER_HPP_

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
#include <boost/function.hpp>

using namespace std;

namespace ImgTaskQueue 
{
	const int FacebookImage = 2;
	const int LocalImage = 0;

	struct ImageLoadTask {
		
		string resourceId, imageURI;
		bool complete, success, locked;
		cv::Mat image;
		float priority;

		int imageType;

		boost::function<void(cv::Mat&)> transformFunction;
		
		ImageLoadTask(string _resourceId, string _imageURI, int _imageType, float _priority,boost::function<void(cv::Mat&)> _transformFunction):
			resourceId(_resourceId),
			imageURI(_imageURI),
			priority(_priority),
			imageType(_imageType),
			transformFunction(_transformFunction)
		{
			complete = success = locked = false;
		}

		void executeLoad();
		void executeTransform();

		void cleanup();

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
					boost::multi_index::member<ImageLoadTask, std::string, &ImageLoadTask::resourceId>
				>	
			>
		> ImageTaskQueue;
	
	typedef ImageTaskQueue::index<PriorityIndex>::type TPriorIdx;
};


class ImageLoader  {

private:
	ImageLoader();
	ImageLoader(ImageLoader const&);
	void operator=(ImageLoader const&); 
		
public:
	static ImageLoader& getInstance()
	{
		static ImageLoader instance; 
		return instance;
	}
	
	void setResourceChangedCallback(boost::function<void(string resourceId, int statusCode, cv::Mat imgMat)> resourceChangedCallback);

	void cancelTask(string resourceId);
	void updateTask(string resourceId, float priority);

	int processCompletedTasks(int count  = 1);
	void handleCompletedTask(ImgTaskQueue::ImageLoadTask loadTask, vector<unsigned char> & data);
	
	void startLoadingImage(string resourceId, string loadURI, int imageType, float priority,boost::function<void(cv::Mat&)> transformFunction);
	
private:	
	static ImageLoader * instance;
	static int maxConcurrentLoads;

	boost::mutex loadQueueMutex, transformQueueMutex;
	ImgTaskQueue::ImageTaskQueue imageLoadQueue;
	ImgTaskQueue::ImageTaskQueue imageTransformQueue;
	bool loadThreadRunning;

	boost::function<void(string resourceId, int statusCode, cv::Mat result)> resourceChangedCallback;
		
	void postTransformTask(const ImgTaskQueue::ImageLoadTask & task);
	
	static void runLoadThread(ImgTaskQueue::ImageTaskQueue * taskQueue,boost::mutex * loadQueueMutex);
	static void runTransformThread(ImgTaskQueue::ImageTaskQueue * transformQueue,boost::mutex * queueMutex);
};

#endif
