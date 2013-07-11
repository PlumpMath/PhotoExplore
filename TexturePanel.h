#include "PanelBase.h"

#ifndef TEXTURE_PANEL_H_
#define TEXTURE_PANEL_H_

class TexturePanel : public PanelBase {



protected:
	TexturePanel();

	int textureScaleMode;
	GLuint currentTextureId;
	float textureWidth,textureHeight;
	bool allowSubPixelRendering;
	
	
	void drawTexture(GLuint texId, Vector drawPosition, float drawWidth, float drawHeight);
	
	virtual void drawContent(Vector drawPosition, float drawWidth, float drawHeight);


public:
	TexturePanel(GLuint textureId)
	{
		currentTextureId = textureId;
	}

	Vector textureOffset, textureScale;
	void setTextureWindow(Leap::Vector textureOffset, Leap::Vector scaleVector);
	void setScaleMode(int scaleMode);

	void setAllowSubPixelRendering(bool allowSubPixel);
	bool getAllowSubPixelRendering();

	virtual void getBoundingArea(Vector & position, float & drawWidth, float & drawHeight);


};

#endif

