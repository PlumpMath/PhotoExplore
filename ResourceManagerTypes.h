#ifndef LEAPIMG_RESOURCE_MANAGER_TYPES_H_
#define LEAPIMG_RESOURCE_MANAGER_TYPES_H_

#include <boost/function.hpp>
#include <opencv2/opencv.hpp>
#include "GLImport.h"

typedef float _PriorityType;
typedef int ResourceType;
typedef int LevelOfDetail;

using namespace std;

#define LowestPriority 100

struct ResourceData;

struct IResourceWatcher {

	virtual void resourceUpdated(ResourceData * data) = 0;

};


namespace ResourceState
{
	const static int ImageLoading = 1;
	const static int ImageLoaded = 2;
	const static int ImageLoadError = -1;

	const static int TextureLoading = 1;
	const static int TextureLoaded = 2;
	const static int TextureLoadError = -1;

	const static int Empty = 0;
}

struct ResourceData
{	
	const string resourceId, imageURI;
	float priority;
	cv::Mat image;	
	GLuint textureId;	
	set<IResourceWatcher*> callbacks;	

	int ImageState;
	int TextureState;

	ResourceData(string _resourceId, float _priority, string _imageURI) :
		resourceId(_resourceId),
		priority(_priority),
		imageURI(_imageURI),
		textureId(0)
	{

	}
	
	ResourceData(string _resourceId, float _priority, cv::Mat & _image) :
		resourceId(_resourceId),
		priority(_priority),
		image(_image),
		textureId(0)
	{

	}

	ResourceData(string _resourceId, float _priority, cv::Mat & _image, GLuint _textureId) :
		resourceId(_resourceId),
		priority(_priority),
		image(_image),
		textureId(_textureId)
	{

	}

	void updateCallbacks()
	{
		for (auto it = callbacks.begin(); it != callbacks.end(); it++)
		{
			(*it)->resourceUpdated(this);
		}
	}

};


#endif