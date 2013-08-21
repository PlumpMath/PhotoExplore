#include "DynamicImagePanel.hpp"
#include <boost/filesystem.hpp>


DynamicImagePanel::DynamicImagePanel()
{
 	offset = position = Vector(0,0,0);
	this->backgroundColor = Colors::White;
	
	this->borderColor = Colors::Transparent;
	this->borderThickness = 0;

	textureHeight = width;
	textureWidth = height;

	dataPriority = 100;

	textureScale = Vector(1,1,1);
	textureScaleMode = ScaleMode::FillUniform;

	NudgeAnimationEnabled = true;
	
	loadAnimTimer.start();

	currentResource = NULL;

	maxResolutionMode = false;

	currentResource = NULL;
	currentTextureId = NULL;
}

void DynamicImagePanel::setMaxResolutionMode(bool maxResMode)
{
	this->maxResolutionMode = maxResMode;
	prepareResource();
}

bool DynamicImagePanel::isMaxResolutionMode()
{
	return this->maxResolutionMode;
}

void DynamicImagePanel::setDataPriority(float dRel)
{	
	if (dataPriority != dRel)
	{
		dataPriority = dRel;
		if (currentResource != NULL)
		{
			currentResource->priority = dataPriority;
			ResourceManager::getInstance().updateResource(currentResource);
		}
	}	
}

float DynamicImagePanel::getDataPriority()
{
	return dataPriority;
}

ResourceData * DynamicImagePanel::selectResource(bool & undersized)
{
	undersized = true;

	if (resourceMap.empty())
		return NULL;

	ResourceData * data = NULL;
	int currentArea = (int)(getWidth()); // * getHeight());
	
	if (maxResolutionMode)
	{
		data = resourceMap.rbegin()->second;
	}
	else
	{
		for (auto it = resourceMap.begin(); it != resourceMap.end(); it++)
		{
			if (resourceMap.size() > 1 && it->first == resourceMap.rbegin()->first)
			{
				break;
			}

			data = it->second;

			if (it->first >= currentArea)
			{
				undersized = false;
				break;
			}
		}
	}

	if (data != NULL && data->resourceId.find("_max") != string::npos)
		undersized = false;

	return data;
}

void DynamicImagePanel::resourceUpdated(ResourceData * data)
{
	bool undersized;
	ResourceData * newResource = selectResource(undersized);
	if (newResource != NULL && newResource != currentResource)
	{
		if (currentResource != NULL)
			currentResource->priority = 100;

		currentResource = newResource;
		currentResource->priority = this->getDataPriority();
	}
}

void DynamicImagePanel::layout(Vector position, cv::Size2f size)
{
	bool sizeChanged = size.width != this->getWidth() || size.height != this->getHeight();
	TexturePanel::layout(position,size);

	if (sizeChanged)
	{
		prepareResource();
	}
}

void DynamicImagePanel::fitPanelToBoundary(Vector targetPosition, float maxWidth, float maxHeight, bool fill)
{
	float targetWidth = maxWidth, targetHeight =maxHeight;

	TexturePanel::setTextureWindow(Vector(),Vector(1,1,1));

	float textureWidth = pictureSize.width,textureHeight = pictureSize.height;

	if (!fill)
	{		
		float xScale = maxWidth/textureWidth;
		float yScale = maxHeight/textureHeight;

		if (xScale < 1.0f || yScale < 1.0f)
		{
			if (xScale > yScale)
			{
				targetWidth = yScale * textureWidth;
				targetHeight = yScale * textureHeight;
			}
			else 
			{
				targetWidth = xScale * textureWidth;
				targetHeight = xScale * textureHeight;
			}
		}
		else
		{
			targetWidth = textureWidth;
			targetHeight = textureHeight;
		}
	}

	layout(targetPosition-Vector(targetWidth/2.0f,targetHeight/2.0f,0), cv::Size2f(targetWidth,targetHeight));
}



void DynamicImagePanel::drawContent(Vector drawPosition, float drawWidth, float drawHeight)
{		
	if (currentResource != NULL && currentResource->TextureState == ResourceState::TextureLoaded && currentResource->textureId != NULL) // && currentTextureId != NULL)
	{
		TexturePanel::drawTexture(currentResource->textureId,drawPosition,drawWidth,drawHeight);
	}		
	else
	{			
		if (GlobalConfig::tree()->get<bool>("DynamicImagePanel.DebugLoading"))
		{
			if (currentResource == NULL)
			{
				loadAnimationColor = Colors::Green;
				return;
			}
			
			switch (currentResource->ImageState)
			{		
			case ResourceState::Empty:
				loadAnimationColor = Colors::Purple;
				break;
			case ResourceState::ImageLoading:
				loadAnimationColor = Colors::Blue;
				break;
			case ResourceState::ImageLoaded:
				loadAnimationColor = Colors::LimeGreen;
				break;
			case ResourceState::ImageLoadError:
				loadAnimationColor = Colors::Red;
				break;
			}

			drawLoadTimer(drawPosition,drawWidth,drawHeight/2.0f);

			switch (currentResource->TextureState)
			{		
			case ResourceState::Empty:
				loadAnimationColor = Colors::Magenta;
				break;
			case ResourceState::TextureLoading:
				loadAnimationColor = Colors::HoloBlueBright;
				break;
			case ResourceState::TextureLoaded:
				loadAnimationColor = Colors::DarkGreen;
				break;
			case ResourceState::TextureLoadError:
				loadAnimationColor = Colors::OrangeRed;
				break;
			}			

			drawLoadTimer(drawPosition + Vector(0,drawHeight/2.0f,0),drawWidth,drawHeight/2.0f);
		}
		else
		{
			loadAnimationColor = Colors::DimGray.withAlpha(.3f);
			drawLoadTimer(drawPosition,drawWidth,drawHeight);
		}
	}
}


void DynamicImagePanel::drawPanel(Vector drawPosition, float drawWidth, float drawHeight)
{
	static bool priorityDebug = GlobalConfig::tree()->get<bool>("DynamicImagePanel.DebugPriority");

	if (currentResource == NULL)
		drawBackground(drawPosition,drawWidth,drawHeight);

	drawContent(drawPosition,drawWidth,drawHeight);	

	if (priorityDebug)
	{
		Color t = getBackgroundColor();
		setBackgroundColor(Colors::HoloBlueBright);
		drawBackground(drawPosition+Vector(0,0,2),drawWidth*.1f,drawHeight*(min<float>(1.0f,dataPriority*.1f)));
		setBackgroundColor(t);
	}
}
