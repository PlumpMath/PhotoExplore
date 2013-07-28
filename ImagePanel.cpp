#include "ImagePanel.hpp"

ImagePanel::ImagePanel(string path)
{
	ResourceManager::getInstance().loadResource(path,path,-2,this);
}

ImagePanel::ImagePanel(string path, cv::Size2f panelSize)
{
	try
	{
		cv::Mat imgMat  = cv::imread(path, -1);
		cv::cvtColor(imgMat, imgMat, CV_BGR2RGBA, 4);

		float originalWidth = imgMat.size().width, originalHeight = imgMat.size().height;

		float scale = min<float>(panelSize.width/originalWidth, panelSize.height/originalHeight);

		int adjustedWidth = (int)ceil(scale * originalWidth);
		int adjustedHeight = (int)ceil(scale * originalHeight);
		cv::Size newSize = cv::Size(adjustedWidth,adjustedHeight);
		cv::Mat resized = cv::Mat(newSize, CV_8UC4);
		cv::resize(imgMat,resized,newSize,0,0,cv::INTER_AREA);
		imgMat.release();

		ResourceManager::getInstance().loadResource(path,resized,-2,this);
	}
	catch (cv::Exception & e)
	{
		Logger::stream("ImagePanel","ERROR") << "Exception loading image: " << path << endl;
	}

}

void ImagePanel::resourceUpdated(ResourceData * data)
{
	if (data != NULL)
		currentTextureId = data->textureId;

	if (data->textureId != NULL)
	{
		glBindTexture(GL_TEXTURE_2D,data->textureId);
		int texWidth,texHeight;
		glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&texWidth);
		glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&texHeight);

		textureWidth =  texWidth;
		textureHeight = texHeight;
		glBindTexture(GL_TEXTURE_2D,NULL);
	}		
}


