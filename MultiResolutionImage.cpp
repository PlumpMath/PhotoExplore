#include "ImageManager.h"


MultiResolutionImage::MultiResolutionImage(string imageFileName, int _imageType, bool isValidImage)
{
	this->imageType = _imageType;
	this->imageFileName = imageFileName;
	this->isValidImage = isValidImage;
	dataPriority = 0;
}

cv::Mat MultiResolutionImage::getImage(LevelOfDetail detailLevel)
{
	auto res = imageMap.find(detailLevel);
	if (res != imageMap.end())
		return res->second;
	else
		return cv::Mat();
}

cv::Size2i MultiResolutionImage::getImageSize(LevelOfDetail detailLevel)
{
	auto res = imageMap.find(detailLevel);
	if (res != imageMap.end())
	{			
		if (res->second.rows > 1)
		{
			return res->second.size();
		}
		else if (res->second.cols > 1)
		{
			auto m1 = imageMetaData.find(detailLevel);
			if (m1 != imageMetaData.end())
				return m1->second.Size;

			cv::Mat sizeMat = res->second;
			int * data = (int*) sizeMat.data;
			return cv::Size2i(data[0],data[1]);
		}
	}
	else
		return cv::Size2i(-1,-1);
}

void MultiResolutionImage::addImage(ImgTaskQueue::ImageLoadTask & result)
{
	for (auto it = result.loadResults.begin(); it != result.loadResults.end(); it++)
	{
		addImage(it->first,it->second);
	}
}

void MultiResolutionImage::addImage(LevelOfDetail detailLevel, cv::Mat image)
{
	auto res = imageMap.find(detailLevel);
	if (res != imageMap.end())
	{
		if (res->second.rows <= 1)
		{
			//cout << "Upgrading image, LoD = " << detailLevel << " file = " << imageFileName << "\n";
			imageMap[detailLevel] = image;
		}
		else if (res->second.size().area() < image.size().area())
		{
			imageMap[detailLevel] = image;
		}
	}
	else
	{
		//cout << "Adding image, LoD = " << detailLevel << " file = " << imageFileName << "\n";
		imageMap[detailLevel] = image;
	}
}

void MultiResolutionImage::addImageMetaData(LevelOfDetail detail, ImageMetaData & metaData)
{
	auto it = imageMetaData.find(detail);
	if (it == imageMetaData.end())
		imageMetaData.insert(make_pair(detail,metaData));
	else
	{
		imageMetaData[detail] = metaData;
	}	
}

string MultiResolutionImage::getURI(LevelOfDetail detail, bool & uriDefined)
{
	uriDefined = false;
	auto it = imageMetaData.find(detail);
	if (it == imageMetaData.end())
		return imageFileName;
	else
	{
		uriDefined = true;
		return it->second.URI;
	}
}

void MultiResolutionImage::setValid(bool valid)
{
	this->isValidImage = valid;
}

bool MultiResolutionImage::isValid()
{
	return isValidImage;
}

bool MultiResolutionImage::isLoaded(LevelOfDetail detailLevel)
{
	auto res = imageMap.find(detailLevel);
	return (res != imageMap.end() && res->second.rows > 1);
}

bool MultiResolutionImage::hasMetaData(LevelOfDetail detailLevel)
{
	auto m1 = imageMetaData.find(detailLevel);
	if (m1 != imageMetaData.end())
		return true;

	auto res = imageMap.find(detailLevel);
	return (res != imageMap.end());
}

void MultiResolutionImage::setDataPriority(float dataPriority)
{
	this->dataPriority = dataPriority;
}

float MultiResolutionImage::getDataPriority()
{
	return dataPriority;
}

void MultiResolutionImage::setMinimumDetail(int levelOfDetail)
{
	this->minimumDetail = levelOfDetail;
}

int MultiResolutionImage::getMinimumDetail()
{
	return this->minimumDetail;
}


bool MultiResolutionImage::canCleanup()
{
	if (minimumDetail == LevelOfDetail_Types::Preview && isLoaded(LevelOfDetail_Types::Full))
		return true;
	else if (minimumDetail == NULL && isLoaded(LevelOfDetail_Types::Preview))
		return true;
	else
		return false;
}

void MultiResolutionImage::cleanup()
{
	for (auto it = imageMap.begin(); it != imageMap.end();it++)
	{
		//if (it->first > minimumDetail)
		{
			//cout << "Released image with resource ID = " << imageFileName << endl;
			it->second.release();
		}
	}
}	

std::string MultiResolutionImage::getFilename()
{
	return imageFileName;
}