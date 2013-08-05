#ifndef LEAPIMAGE_FACEBOOK_PICTURE_PANEL_HPP_
#define LEAPIMAGE_FACEBOOK_PICTURE_PANEL_HPP_

#include "TexturePanel.h"
#include "FBNode.h"
#include "ResourceManager.h"
#include "FBDataView.hpp"

using namespace Facebook;

class PicturePanel : public TexturePanel, public IResourceWatcher, public FBDataView   {
		
private:
	ResourceData * currentResource;

	float dataPriority;
	FBNode * pictureNode;	

	void initDefaults();	
	void drawContent(Vector drawPosition, float drawWidth, float drawHeight);

	void prepareResource();

	bool maxResolutionMode;

	cv::Size2i pictureSize;
		
public:	
	PicturePanel();

	void fitPanelToBoundary(Vector center, float width, float height, bool fill);

	void setMaxResolutionMode(bool maxResolutionMode);
	bool isMaxResolutionMode();

	float getDataPriority();
	void setDataPriority(float relevance);

	void show(FBNode * node);
	FBNode * getNode();

	void resourceUpdated(ResourceData * data);

	void drawPanel(Vector drawPosition, float drawWidth, float drawHeight);

	void layout(Vector position, cv::Size2f size);
	
};


#endif