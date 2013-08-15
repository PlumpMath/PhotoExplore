#ifndef LEAPIMAGE_FACEBOOK_DYNAMIC_IMAGE_HPP_
#define LEAPIMAGE_FACEBOOK_DYNAMIC_IMAGE_HPP_

#include "TexturePanel.h"
#include "ResourceManager.h"
#include <map>

class DynamicImagePanel : public TexturePanel, public IResourceWatcher   {

protected:
	ResourceData * currentResource;
	float dataPriority;
	void initDefaults();	
	void drawContent(Vector drawPosition, float drawWidth, float drawHeight);
	
	bool maxResolutionMode;

	cv::Size2i pictureSize;

	map<int,ResourceData*> resourceMap;

	ResourceData * selectResource(bool & undersized);
		
	virtual void prepareResource() = 0;

public:	
	DynamicImagePanel();

	void fitPanelToBoundary(Vector center, float width, float height, bool fill);

	void setMaxResolutionMode(bool maxResolutionMode);
	bool isMaxResolutionMode();

	float getDataPriority();
	void setDataPriority(float relevance);


	void resourceUpdated(ResourceData * data);

	void drawPanel(Vector drawPosition, float drawWidth, float drawHeight);

	void layout(Vector position, cv::Size2f size);

};



#endif