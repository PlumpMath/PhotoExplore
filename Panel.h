#include "ResourceManager.h"
#include "NodeBase.h"
#include "TexturePanel.h"
#include "TextPanel.h"
#include <boost/function.hpp>

#ifndef Panel_H_
#define Panel_H_

using namespace std;
using namespace Leap;

class Panel : public TexturePanel, public IResourceWatcher {

private:
	GLuint glTextureId, fullTextureId;
	bool fullScreenMode;
	
	TextPanel * textPanel;

	ResourceData * currentResource;

	//_ResourceLoadCallbackType resourceCallback;

protected:
	int currentDetailLevel;
	float dataPriority;
	NodeBase * node;	

	virtual void initDefaults();
	
	virtual void drawContent(Vector drawPosition, float drawWidth, float drawHeight);
		
public:	
	Panel();
	Panel(float width,float height);
	
	void fitPanelToBoundary(Vector center, float width, float height, bool fill);

	virtual void setFullscreenMode(bool full);
	bool isFullscreenMode();

	void setDetailLevel(int levelOfDetail);
	
	virtual void setNode(NodeBase * node);
	virtual NodeBase * getNode();

	virtual void setVisible(bool visible);
	
	float getDataPriority();
	virtual void setDataPriority(float relevance);
	
	void elementTapped(ScreenTapGesture tapGesture);

	void resourceUpdated(ResourceData * data);

	void drawPanel(Vector drawPosition, float drawWidth, float drawHeight);

	
};

#endif