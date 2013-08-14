#include "ImageLoader.hpp"

using namespace ImgTaskQueue;

void ImageLoadTask::executeLoad()
{
	//Nothing ATM
}

void ImageLoadTask::executeTransform()
{
	cv::cvtColor(image, image, CV_BGR2RGBA, 4);

	if (!this->transformFunction.empty())
	{
		transformFunction(image);
	}

	if (image.data != NULL)
		success = true;
}

void ImageLoadTask::cleanup()
{
	if (image.data != NULL)
		image.release();
}