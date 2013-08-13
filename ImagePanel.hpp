#ifndef LEAPIMAGE_PANEL_IMAGEPANEL_HPP_
#define LEAPIMAGE_PANEL_IMAGEPANEL_HPP_

#include "TexturePanel.h"
#include "ResourceManager.h"

class ImagePanel : public TexturePanel, public IResourceWatcher {

private:
	string imagePath;
	cv::Mat currentImage;

	bool softwareResize;

	void loadImage(cv::Size2f panelSize);

	ResourceData * currentResource;
	list<ResourceData*> expiredResources;

public:
	ImagePanel(string imagePath);
	ImagePanel(string imagePath, cv::Size2f panelSize);
	void resourceUpdated(ResourceData * data);

	void layout(Vector position, cv::Size2f size);
};



#endif