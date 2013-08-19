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

	if (newResource == NULL || undersized)
	{
		stringstream rs;
		rs <<  fileNode->filePath.string();

		boost::function<void(cv::Mat&)> tran;

		if (!maxResolutionMode)
		{
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
			pictureSize.width = fileNode->Attributes.get<int>("Image.Width");
			pictureSize.height = fileNode->Attributes.get<int>("Image.Height");
		}

		resourceId = rs.str();

		ResourceManager::getInstance().loadResourceWithTransform(resourceId,fileNode->filePath.string(),dataPriority,this,tran);
	}
	else
	{
		currentResource = newResource;
		pictureSize = newResource->imageSize;

		if (currentResource->TextureState == ResourceState::TextureLoaded)
			currentTextureId = currentResource->textureId;
	}

}