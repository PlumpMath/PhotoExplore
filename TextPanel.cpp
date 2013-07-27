#include "TextPanel.h"
#include "TypographyManager.h"

TextPanel::TextPanel(string text) 
{
	init();
	setText(text);	
}

TextPanel::TextPanel() 
{
	init();
	textEnabled = false;
}

void TextPanel::init()
{	
	fontName = "Default";
	textColor = Colors::Black;
	textSize = 6;
	textDirty = fitToText = false;
	currentTextureId = NULL;
	textFitPadding = 0;
	textEnabled = true;
	this->text = std::string("");
	textureScaleMode = ScaleMode::None;
	setAllowSubPixelRendering(false); //fontDefinition.get<bool>("SubPixelTextRendering"));
	textAlignment = 1;
}

void TextPanel::resourceUpdated(ResourceData * data)
{
	if (data != NULL)
	{
		if (data->TextureState == ResourceState::TextureLoaded)
		{
			currentTextureId = data->textureId;
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
	cv::Size2f textureSize(this->lastSize.width-textFitPadding*2,this->lastSize.height-textFitPadding*2);
	TextDefinition td(text, textColor, textSize, textureSize);

	ResourceData * textResource = ResourceManager::getInstance().watchResource(td.getKey(),this);	
	if (textResource == NULL)
	{
		TextLayoutConfig config(textAlignment,textureSize.width);
		currentTextImage = TypographyManager::getInstance()->renderText(text,fontName,textColor,textSize,config);	
		if (currentTextImage.data != NULL)
		{
			ResourceManager::getInstance().loadResource(td.getKey(),currentTextImage,-1,this);
		}
	}
	else if (textResource->TextureState == ResourceState::TextureLoaded)
		currentTextureId = textResource->textureId;

	textDirty = false;
}

void TextPanel::drawContent(Vector drawPosition, float drawWidth, float drawHeight)
{		
	if (!textEnabled || currentTextureId == NULL)// || !(TextureManager::getInstance()->isTextureLoaded(currentTextureId,TextureManager::TextureType_Font))))
		return;
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	
	//glBindTexture(GL_TEXTURE_2D,currentTextureId);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	TexturePanel::drawTexture(currentTextureId,drawPosition+Vector(0,0,1),drawWidth,drawHeight);	
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void TextPanel::refresh()
{
	if (textDirty)
		reloadText();
}

void TextPanel::setFontName(string _fontName)
{
	this->fontName = _fontName;
}

string TextPanel::getFontName()
{
	return fontName;
}

void TextPanel::setTextAlignment(int a)
{
	this->textAlignment = a;
}

void TextPanel::setText(string _text)
{
	textEnabled = _text.length() > 0;		

	if (!textEnabled)
		currentTextureId = NULL;

	textDirty = textDirty || (_text.compare(this->text) != 0);
	this->text = _text;		
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

void TextPanel::setTextSize(float _textSize, bool relative)
{
	textDirty = true;
	if (relative)
		this->textSize = (_textSize * (GlobalConfig::ScreenHeight/1440.0f));
	else
		this->textSize = _textSize;	
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