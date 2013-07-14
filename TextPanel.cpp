#include "TextPanel.h"
#include "TypographyManager.h"

TextPanel::TextPanel(string text) 
{
	this->text = std::string("");
	textureScaleMode = ScaleMode::None;
	textColor = Colors::Black;
	textSize = 6;
	textDirty = fitToText = false;
	currentTextureId = NULL;
	textFitPadding = 0;
	textEnabled = true;
	setText(text);
	setAllowSubPixelRendering(false);
}

TextPanel::TextPanel() 
{
	text = std::string("");
	textureScaleMode = ScaleMode::None;
	textColor = Colors::Black;
	textSize = 6;
	currentTextureId = NULL;
	textDirty = fitToText= false;
	textEnabled = true;
	textFitPadding = 0;
	setAllowSubPixelRendering(false);
}

void TextPanel::resourceUpdated(ResourceData * data)
{
	if (data != NULL)
	{
		if (data->TextureState == ResourceState::TextureLoaded)
		{
			currentTextureId = data->textureId;
			//currentTextImage.release();
		}		
	}
	else
		currentTextureId = NULL;

	//if (fitToText)
	//{
	//	animatePanelSize(textureSize.width + textFitPadding,textureSize.height + textFitPadding,ARB_TEXT_RESIZE_DURATION);
	//}
}


void TextPanel::setTextEnabled(bool _textEnabled)
{
	textEnabled = _textEnabled;
}

bool TextPanel::getTextEnabled()
{
	return textEnabled;
}

void TextPanel::reloadText()
{		
	//if (currentDefinition != NULL)
	//{
	//	//ResourceManager::getInstance().releaseTextResource(*currentDefinition,this);
	//	delete currentDefinition;
	//	currentDefinition = NULL;
	//}

	cv::Size2f textureSize(this->lastSize.width-textFitPadding*2,this->lastSize.height-textFitPadding*2);
	//TextDefinition * currentDefinition = new 
	TextDefinition td(text, textColor, textSize, textureSize);

	currentTextImage = TypographyManager::getInstance()->renderText(text,textColor,textSize,textureSize);	
	if (currentTextImage.data != NULL)
	{
		ResourceManager::getInstance().loadResource(td.getKey(),currentTextImage,0,this);
	}
	textDirty = false;
}

void TextPanel::drawContent(Vector drawPosition, float drawWidth, float drawHeight)
{		
	if (!textEnabled || currentTextureId == NULL)// || !(TextureManager::getInstance()->isTextureLoaded(currentTextureId,TextureManager::TextureType_Font))))
		return;
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	TexturePanel::drawTexture(currentTextureId,drawPosition,drawWidth,drawHeight);	
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void TextPanel::refresh()
{
	reloadText();
}

void TextPanel::setText(string _text)
{
	//if (_text.size() == 0)
	//{
	//	if (currentDefinition != NULL)
	//	{
	//		ResourceManager::getInstance().releaseTextResource(*currentDefinition,this);
	//		delete currentDefinition;
	//		currentDefinition = NULL;
	//	}
	//	currentTextureId = NULL;
	//}
	this->text = _text;	
	textDirty = true;
}
string TextPanel::getText()
{
	return this->text;
}


void TextPanel::setTextColor(Color textColor)
{		
	textDirty = true;
	this->textColor = textColor;
}

void TextPanel::setTextSize(float textSize)
{
	textDirty = true;
	this->textSize = textSize;	
}

void TextPanel::setTextFitMode(bool fitToText)
{
	textDirty = true;
	this->fitToText = fitToText;
}

void TextPanel::layout(Vector position, cv::Size2f size)
{
	if (lastSize != size || textDirty)
	{
		lastSize = size;
		reloadText();
	}
	lastPosition = position;
	PanelBase::layout(position,size);
}

bool TextPanel::getTextFitMode()
{
	return fitToText;
}

void TextPanel::setTextFitPadding(float _textFitPadding)
{
	this->textFitPadding = _textFitPadding;
}

float TextPanel::getTextFitPadding()
{
	return this->textFitPadding;
}


void TextPanel::setVisible(bool v)
{
	PanelBase::setVisible(v);
}