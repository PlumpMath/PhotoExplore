#include "PicturePanel.hpp"
#include <boost/filesystem.hpp>

PicturePanel::PicturePanel()
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
}


void PicturePanel::prepareResource()
{	
	if (getWidth() <= 0 || getHeight() <= 0 || !isVisible())
		return;

	string newResourceURI = "", resourceId = "";
	cv::Size2i newResourceSize(0,0);
	if (GlobalConfig::tree()->get<bool>("FakeDataMode.Enable"))
	{
		newResourceURI = pictureNode->Edges.find("fake_uri")->Value;
		resourceId = pictureNode->getId() + boost::filesystem::path(newResourceURI).filename().string();
	}
	else
	{
		auto res = pictureNode->Edges.find("images");
		if (res != pictureNode->Edges.end())
		{
			vector<json_spirit::Value> objectArray = res->JsonValue.get_array();				

			
			map<int,pair<cv::Size2i,string> > areaImageMap;
			for (auto it = objectArray.begin(); it != objectArray.end(); it++)
			{
				cv::Size2i imageSize;
				imageSize.width = json_spirit::find_value(it->get_obj(),"width").get_int();		
				imageSize.height = json_spirit::find_value(it->get_obj(),"height").get_int();	
				string imageURI =  json_spirit::find_value(it->get_obj(),"source").get_str();

				areaImageMap.insert(make_pair(min<int>(imageSize.width,imageSize.height),make_pair(imageSize,imageURI)));
			}

			if (areaImageMap.empty())
			{

			}
			else if (maxResolutionMode)
			{
				newResourceURI = areaImageMap.rbegin()->second.second;
				newResourceSize = areaImageMap.rbegin()->second.first;
			}
			else
			{
				int targetDimension = max<int>((int)getWidth(),(int)getHeight());
				for (auto it = areaImageMap.begin(); it != areaImageMap.end(); it++)
				{
					if (!maxResolutionMode && areaImageMap.size() > 1 && it->second.second.find("_o.jpg") != string::npos)
						continue;

					newResourceURI = it->second.second;
					newResourceSize = it->second.first;
					if (it->first > targetDimension)
					{
						break;
					}
				}
			}
		}
		
		resourceId = newResourceURI;
		if (newResourceURI.size() == 0)
		{						
			if (pictureNode->getAttribute("picture").length() > 0)
			{
				resourceId = pictureNode->getAttribute("picture");
				newResourceURI = resourceId;
			}
			else
			{
				std::stringstream urlStream;
				urlStream << "https://graph.facebook.com/";
				urlStream << pictureNode->getId() << "/picture?width=600&height=600&";
				urlStream << "method=get&redirect=true&access_token=" << GlobalConfig::TestingToken;
				resourceId = pictureNode->getId();
				newResourceURI = urlStream.str();			
			}
		}
	}
	if (currentResource == NULL || currentResource->imageURI.compare(newResourceURI) != 0)
	{
		Logger::stream("PicturePanel","INFO") << "Selected new resource: " << newResourceURI << " with size = [" << newResourceSize.width << "," << newResourceSize.height << "] , my size is [" << getWidth() << "," << getHeight() << "]" << endl;
		if (currentResource != NULL)
		{
			currentResource->priority = 100;
		}
		
		pictureSize = newResourceSize;
		if (newResourceSize.area() > 0)
		{
			textureWidth = newResourceSize.width;
			textureHeight = newResourceSize.height;
		}

		currentResource = ResourceManager::getInstance().loadResource(resourceId,newResourceURI,dataPriority,this);
	}
}

void PicturePanel::show(FBNode * _pictureNode)
{
	this->pictureNode = _pictureNode;
	prepareResource();
}

FBNode * PicturePanel::getNode()
{
	return this->pictureNode;
}


void PicturePanel::setMaxResolutionMode(bool maxResMode)
{
	this->maxResolutionMode = maxResMode;
	prepareResource();
}

bool PicturePanel::isMaxResolutionMode()
{
	return this->maxResolutionMode;
}

void PicturePanel::setDataPriority(float dRel)
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

float PicturePanel::getDataPriority()
{
	return dataPriority;
}

void PicturePanel::resourceUpdated(ResourceData * data)
{
	if (data == NULL)
	{
		currentTextureId = NULL;
	}
	else
	{
		currentTextureId = data->textureId;

		glBindTexture(GL_TEXTURE_2D,currentTextureId);
		int texWidth,texHeight;
		glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&texWidth);
		glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&texHeight);

		pictureSize = cv::Size2i((int)texWidth,(int)texHeight);
	}
}

void PicturePanel::layout(Vector position, cv::Size2f size)
{
	bool sizeChanged = size.width != this->getWidth() || size.height != this->getHeight();
	TexturePanel::layout(position,size);

	if (sizeChanged)
		prepareResource();
}

void PicturePanel::fitPanelToBoundary(Vector targetPosition, float maxWidth, float maxHeight, bool fill)
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
			
	int expandAnimTime = 200, shrinkAnimTime = 200;

	animateToPosition(targetPosition-Vector(targetWidth/2.0f,targetHeight/2.0f,0),expandAnimTime, expandAnimTime);
	animatePanelSize(targetWidth,targetHeight,expandAnimTime);
}



void PicturePanel::drawContent(Vector drawPosition, float drawWidth, float drawHeight)
{		
	

	//if (currentTextureId != NULL)
	if (currentResource != NULL && currentResource->textureId != NULL)
	{
		currentTextureId = currentResource->textureId;
		TexturePanel::drawTexture(currentTextureId,drawPosition,drawWidth,drawHeight);
	}		
	else
	{			
		if (GlobalConfig::tree()->get<bool>("PicturePanel.DebugLoading"))
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


void PicturePanel::drawPanel(Vector drawPosition, float drawWidth, float drawHeight)
{
	static bool priorityDebug = GlobalConfig::tree()->get<bool>("PicturePanel.DebugPriority");

	if (currentTextureId == NULL)
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
