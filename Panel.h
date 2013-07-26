#include "ResourceManager.h"
#include "NodeBase.h"
#include "TexturePanel.h"
#include "FBDataView.hpp"
#include <boost/function.hpp>

#ifndef Panel_H_
#define Panel_H_

using namespace std;
using namespace Leap;
using namespace Facebook;

class Panel : public TexturePanel, public IResourceWatcher, public FBDataView {

private:
	GLuint glTextureId;
	bool fullScreenMode;

	ResourceData * currentResource;

protected:
	int currentDetailLevel;
	float dataPriority;
	FBNode * node;	

	virtual void initDefaults();
	
	virtual void drawContent(Vector drawPosition, float drawWidth, float drawHeight);
		
public:	
	Panel();
	Panel(float width,float height);
	
	void fitPanelToBoundary(Vector center, float width, float height, bool fill);

	virtual void setFullscreenMode(bool full);
	bool isFullscreenMode();

	void setDetailLevel(int levelOfDetail);
	
	virtual void setVisible(bool visible);
	
	float getDataPriority();

	virtual void setDataPriority(float relevance);
	virtual void show(FBNode * node);
	virtual FBNode * getNode();
	
	void elementTapped(ScreenTapGesture tapGesture);

	void resourceUpdated(ResourceData * data);

	void drawPanel(Vector drawPosition, float drawWidth, float drawHeight);

	GLuint getTextureId();

	
};

#endif