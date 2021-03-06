#include "TexturePanel.h"


TexturePanel::TexturePanel()
{
	currentTextureId = NULL;
	allowSubPixelRendering = true;
	textureScaleMode = ScaleMode::Fit;
	textureTint = Colors::White;
	horizontalAlignment = AlignmentMode::Center;
	verticalAlignment = AlignmentMode::Center;
}

void TexturePanel::setTextureTint(Color _textureTint)
{
	this->textureTint = _textureTint;
}

Color TexturePanel::getTextureTint()
{
	return textureTint;
}


void TexturePanel::setTextureAlpha(float _textureAlpha)
{
	this->textureTint.colorArray[3] = _textureAlpha;
}

float TexturePanel::getTextureAlpha()
{
	return textureTint.colorArray[3];
}

void TexturePanel::setTextureWindow(Vector textureOffset, Vector scaleVector)
{
	this->textureOffset = textureOffset;
	this->textureScale = scaleVector;
}

void TexturePanel::setScaleMode(int textureScaleMode)
{
	this->textureScaleMode = textureScaleMode;
}

void TexturePanel::setAllowSubPixelRendering(bool allowSubPixel)
{
	this->allowSubPixelRendering = allowSubPixel;
}

bool TexturePanel::getAllowSubPixelRendering()
{
	return this->allowSubPixelRendering;
}


void TexturePanel::getBoundingArea(Vector & drawPosition, float & drawWidth, float & drawHeight)
{	
	PanelBase::getBoundingArea(drawPosition,drawWidth,drawHeight);

	drawPosition += textureOffset;

	if (textureScale.x > 0)
	{
		drawWidth *= textureScale.x;
		drawHeight *= textureScale.x;
		//drawPosition -= Vector(drawWidth/2.0f,drawHeight/2.0f,0);
	}
}

void TexturePanel::drawContent(Vector drawPosition, float drawWidth, float drawHeight)
{		
	if (currentTextureId == NULL)
		return;

	TexturePanel::drawTexture(currentTextureId,drawPosition,drawWidth,drawHeight);	
}


void TexturePanel::drawTexture(GLuint drawTexId, Vector drawPosition, float drawWidth, float drawHeight)
{
	static bool useInt = GlobalConfig::tree()->get<bool>("GraphicsSettings.UseIntForNonSubPixelRender");
	
	glBindTexture(GL_TEXTURE_2D,drawTexId);
	int texWidth,texHeight;
	glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&texWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&texHeight);

	textureWidth =  texWidth;
	textureHeight = texHeight;
	
	if (horizontalAlignment == Center)
		drawPosition.x += floorf(drawWidth/2.0f);	

	if (verticalAlignment == Center)
		drawPosition.y += floorf(drawHeight/2.0f);

	glBindTexture( GL_TEXTURE_2D, drawTexId);
	float drawTextureWidth,drawTextureHeight;
	float tx1 = 0,tx2 = 1,ty1 = 0,ty2 = 1;
	//float tx1 = -.5f,tx2 = .5f,ty1 = -.5f,ty2 = .5f;

	float xScale= drawWidth/textureWidth;
	float yScale = drawHeight/textureHeight;
	
	float ctX = .5f ,ctY = .5f;		

	if (textureScaleMode == ScaleMode::FillUniform || textureScaleMode == ScaleMode::FitHeight || textureScaleMode == ScaleMode::FitWidth)
	{			
		float scale;
		
		if (textureScaleMode = ScaleMode::FillUniform)
			scale = max<float>(xScale,yScale);
		else if (textureScaleMode = ScaleMode::FitWidth)
			scale = xScale;
		else 
			scale = yScale;
		
		drawTextureWidth = min<float>(scale * textureWidth,drawWidth);
		drawTextureHeight = min<float>(scale * textureHeight,drawHeight);		
		
		float texWidth_normalized = xScale/scale;
		float texHeight_normalized = yScale/scale;


		if (!true)
		{
			tx1 = ctX - texWidth_normalized/2.0f;
			tx2 = ctX + texWidth_normalized/2.0f;
			ty1 = ctY - texHeight_normalized/2.0f;
			ty2 = ctY + texHeight_normalized/2.0f;
		}
		else
		{			
			tx1 = ctX - texWidth_normalized/2.0f;
			tx2 = ctX + texWidth_normalized/2.0f;
			ty1 = 0;
			ty2 = texHeight_normalized;
		}
	}
	else if (textureScaleMode == ScaleMode::None)
	{
		drawTextureWidth = textureWidth;
		drawTextureHeight = textureHeight;
	}
	else if (textureScaleMode == ScaleMode::Fit)
	{
		float scale = min<float>(xScale,yScale);

		drawTextureWidth = scale * textureWidth;
		drawTextureHeight = scale * textureHeight;		
	}
	else
	{
		throw "Invalid texture scale mode";
	}

	float x1,y1;

	switch (horizontalAlignment)
	{
	case Center:
		x1 = drawPosition.x - drawTextureWidth/2.0f;
		break;
	case Left:
		x1 = drawPosition.x;
		break;
	case Right:
		x1 = drawPosition.x + (drawWidth - drawTextureWidth);
		break;
	}
	
	switch (verticalAlignment)
	{
	case Center:
		y1 = drawPosition.y - drawTextureHeight/2.0f;
		break;
	case Left:
		y1 = drawPosition.y;
		break;
	case Right:
		y1 = drawPosition.y + (drawHeight - drawTextureHeight);
		break;
	}

	if (!allowSubPixelRendering) y1 = floorf(y1);
	if (!allowSubPixelRendering) x1 = floorf(x1);
	float x2 = x1 + drawTextureWidth;
	float y2 = y1 + drawTextureHeight;



	float z1 = drawPosition.z + .5f;
				
	glColor4fv(textureTint.getFloat());
	glBegin( GL_QUADS );

		if (!allowSubPixelRendering && useInt)
		{			
			glTexCoord2i(tx1,ty1);
			glVertex3i((int)x1,(int)y1,(int)z1); 

			glTexCoord2i(tx2,ty1);
			glVertex3i((int)x2,(int)y1,(int)z1);

			glTexCoord2i(tx2,ty2);
			glVertex3i((int)x2,(int)y2,(int)z1);

			glTexCoord2i(tx1,ty2);
			glVertex3i((int)x1,(int)y2,(int)z1);
		}
		else
		{
			glTexCoord2f(tx1,ty1);
			glVertex3f(x1,y1,z1); 

			glTexCoord2f(tx2,ty1);
			glVertex3f(x2,y1,z1);

			glTexCoord2f(tx2,ty2);
			glVertex3f(x2,y2,z1);

			glTexCoord2f(tx1,ty2);
			glVertex3f(x1,y2,z1);
		}

	glEnd();	


}