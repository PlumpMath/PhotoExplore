#include "ImagePanel.hpp"

ImagePanel::ImagePanel(string path) :
	softwareResize(false),
	imagePath(path)
{
	currentResource = ResourceManager::getInstance().loadResource(path,path,-2,this);
}

ImagePanel::ImagePanel(string path, cv::Size2f panelSize) :
	softwareResize(true),
	imagePath(path)
{	
	currentResource = NULL;
	loadImage(panelSize);
}

void ImagePanel::loadImage(cv::Size2f panelSize)
{
	try
	{
		cv::Mat imgMat  = cv::imread(imagePath, -1);
		cv::cvtColor(imgMat, imgMat, CV_BGR2RGBA, 4);

		float originalWidth = imgMat.size().width, originalHeight = imgMat.size().height;

		float scale = min<float>(panelSize.width/originalWidth, panelSize.height/originalHeight);

		int adjustedWidth = (int)ceil(scale * originalWidth);
		int adjustedHeight = (int)ceil(scale * originalHeight);
		cv::Size newSize = cv::Size(adjustedWidth,adjustedHeight);
		cv::Mat resized = cv::Mat(newSize, CV_8UC4);
		cv::resize(imgMat,resized,newSize,0,0,cv::INTER_AREA);
		imgMat.release();

		if (currentResource != NULL)
		{
			expiredResources.push_back(currentResource);
		}
		stringstream resKey;
		resKey << imagePath << panelSize.width << panelSize.height;
		currentResource = ResourceManager::getInstance().loadResource(resKey.str(),resized,-2,this);
	}
	catch (cv::Exception & e)
	{
		Logger::stream("ImagePanel","ERROR") << "Exception loading image: " << imagePath << endl;
	}
}

void ImagePanel::resourceUpdated(ResourceData * data)
{
	if (data != NULL && data == currentResource)
	{
		if (data->TextureState == ResourceState::TextureLoaded)
		{
			while (!expiredResources.empty())
			{
				if (expiredResources.front() != data)
					ResourceManager::getInstance().destroyResourceIfEmpty(expiredResources.front()->resourceId,this);

				expiredResources.pop_front();
			}

			currentTextureId = data->textureId;

			//glBindTexture(GL_TEXTURE_2D,currentTextureId);
			//int texWidth,texHeight;
			//glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&texWidth);
			//glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&texHeight);

			//textureWidth =  texWidth;
			//textureHeight = texHeight;
			//glBindTexture(GL_TEXTURE_2D,NULL);
		}		
	}
	//else
	//	currentTextureId = NULL;


/*	if (data == NULL)
		currentR

	if (data->TextureState == ResourceState::TextureLoaded)
	{
		if (data->TextureState == ResourceState::TextureLoaded)
		{
			while (!expiredResources.empty())
			{
				if (expiredResources.front() != data)
					ResourceManager::getInstance().destroyResourceIfEmpty(expiredResources.front()->resourceId,this);

				expiredResources.pop_front();
			}
		}


		glBindTexture(GL_TEXTURE_2D,data->textureId);
		int texWidth,texHeight;
		glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&texWidth);
		glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&texHeight);

		textureWidth =  texWidth;
		textureHeight = texHeight;
		glBindTexture(GL_TEXTURE_2D,NULL);
	}	*/	
}

void ImagePanel::layout(Vector position, cv::Size2f size)
{
	if (softwareResize && (size.height != lastSize.height || size.width != lastSize.width))
	{
		loadImage(size);
	}
		
	lastSize = size;
	lastPosition = position;

	TexturePanel::layout(position,size);
}

