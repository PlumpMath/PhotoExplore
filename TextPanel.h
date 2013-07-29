#include "TexturePanel.h"
#include "ResourceManager.h"
#include <boost/property_tree/ptree.hpp>

#ifndef TEXT_PANEL_H_
#define TEXT_PANEL_H_

#define ARB_TEXT_RESIZE_DURATION 150





class TextPanel : public TexturePanel, public IResourceWatcher {

private:
	Color textColor;
	float textSize, textFitPadding;
	
	bool fitToText;
	bool textDirty;
	bool textEnabled;
		
	int textAlignment;

	cv::Mat currentTextImage;

	string fontName;
	void init();

protected:
	string text;
	ResourceData * currentResource;
	cv::Rect_<float> currentTextRect;

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
	Color getTextColor();

	void setTextSize(float textSize, bool relative = true);
	void setTextFitMode(bool fitToText);		
	bool getTextFitMode();
	void setTextFitPadding(float textFitPadding);
	float getTextFitPadding();
	void setVisible(bool v);

	int getTextAlignment();
	void setTextAlignment(int alignment);

	void setFontName(string fontName);
	string getFontName();

	void layout(Vector position, cv::Size2f size);

	void resourceUpdated(ResourceData * resourceData);
};

#endif