#include "ImagePanel.hpp"

ImagePanel::ImagePanel(string path)
{
	ResourceManager::getInstance().loadResource(path,path,-1,this);
}

void ImagePanel::resourceUpdated(ResourceData * data)
{
	if (data != NULL)
		currentTextureId = data->textureId;
}


