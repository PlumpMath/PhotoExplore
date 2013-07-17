#include "TexturePanel.h"
#include "ResourceManager.h"

#ifndef TEXT_PANEL_H_
#define TEXT_PANEL_H_

#define ARB_TEXT_RESIZE_DURATION 150



class TextDefinition {
	
private:
	std::string key;

public:	
	const std::string text;
	const Color textColor;
	const float fontSize;
	const cv::Size2f textWrapSize;

	TextDefinition() : 
		fontSize(0)
	{		
	}

	TextDefinition(TextDefinition & copy) : 
		text(copy.text),
		textColor(copy.textColor),
		fontSize(copy.fontSize),
		textWrapSize(copy.textWrapSize),
		key(copy.key)
	{		
	}

	TextDefinition(std::string _text, Color _textColor, float _textSize, cv::Size2f _textWrapSize) :
		text(_text),
		textColor(_textColor),
		fontSize(_textSize),
		textWrapSize(_textWrapSize)
	{
		std::stringstream ss;
		ss << _text << _textColor.idValue() << _textSize << _textWrapSize.width << _textWrapSize.height;
		key = ss.str();
	}

public:
	std::string getKey()
	{
		return key;
	}
};


class TextPanel : public TexturePanel, public IResourceWatcher {

private:
	string text;
	Color textColor;
	float textSize, textFitPadding;
	
	bool fitToText;
	bool textDirty;
	bool textEnabled;

	//TextDefinition * currentDefinition;
	
	cv::Mat currentTextImage;

	void init();

public:
	TextPanel(string text);
	TextPanel();	
	void reloadText();

	void drawContent(Vector drawPosition, float drawWidth, float drawHeight);
	void refresh();
	
	void setText(string text);
	string getText();

	void setTextEnabled(bool enableText);
	bool getTextEnabled();

	void setTextColor(Color textColor);
	void setTextSize(float textSize, bool relative = true);
	void setTextFitMode(bool fitToText);		
	bool getTextFitMode();
	void setTextFitPadding(float textFitPadding);
	float getTextFitPadding();
	void setVisible(bool v);

	void layout(Vector position, cv::Size2f size);

	void resourceUpdated(ResourceData * resourceData);
};

#endif