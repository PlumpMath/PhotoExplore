#include "FileImagePanel.hpp"
#include <boost/filesystem.hpp>

using namespace FileSystem;


void FileImagePanel::show(FileNode * _fileNode)
{
	this->fileNode = _fileNode;
	prepareResource();
}

FileNode * FileImagePanel::getNode()
{
	return this->fileNode;
}


void FileImagePanel::prepareResource()
{	
	if (getWidth() <= 0 || getHeight() <= 0)
		return;

	string newResourceURI = "", resourceId = "";
	cv::Size2i newResourceSize(0,0);


	bool undersized;
	ResourceData * newResource = selectResource(undersized);

	if (newResource != NULL && (currentResource == NULL || (newResource->TextureState == ResourceState::TextureLoaded && !undersized)))
	{
		if (currentResource != NULL)
			currentResource->priority = 100;

		currentResource = newResource;
		pictureSize = currentResource->imageSize;
	}
	else //if (newResource == NULL)
	{
		stringstream rs;
		rs <<  fileNode->filePath.string();

		boost::function<void(cv::Mat&)> tran;
		int loadingWidth = 0;

		if (!maxResolutionMode)
		{
			loadingWidth = (int)getWidth();

			int targetDimension = max<int>((int)getWidth(),(int)getHeight());

			tran = [targetDimension,this](cv::Mat&imgMat){

				float td = (float)targetDimension;

				float originalWidth = imgMat.size().width;
				float originalHeight = imgMat.size().height;

				this->fileNode->Attributes.put<int>("Image.Width",originalWidth);
				this->fileNode->Attributes.put<int>("Image.Height",originalHeight);

				float scale = max<float>(td/originalWidth,td/originalHeight);

				scale = min<float>(1.0f,scale);

				int adjustedWidth = (int)ceil(scale * originalWidth);
				int adjustedHeight = (int)ceil(scale * originalHeight);

				cv::Size newSize = cv::Size(adjustedWidth,adjustedHeight);
				cv::Mat resized = cv::Mat(newSize, CV_8UC4);
				cv::resize(imgMat,resized,newSize,0,0,cv::INTER_AREA);
				imgMat.release();

				imgMat = resized;
			};

			rs << targetDimension;
		}
		else
		{
			rs << "_max";
			pictureSize.width = fileNode->Attributes.get<int>("Image.Width",0);
			pictureSize.height = fileNode->Attributes.get<int>("Image.Height",0);
			loadingWidth = pictureSize.width;
		}

		if (resourceMap.count(loadingWidth) == 0)
		{
			resourceId = rs.str();
			ResourceData * loadingResource = ResourceManager::getInstance().loadResourceWithTransform(resourceId,fileNode->filePath.string(),dataPriority,this,tran);
			resourceMap.insert(make_pair(loadingWidth,loadingResource));
			Logger::stream("FileImagePanel","INFO") << "Loading transformed resource. Width = " << loadingWidth << ", ID = " << resourceId << endl;
		}
	}

}