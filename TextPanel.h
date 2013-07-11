#include "TexturePanel.h"
#include "ResourceManager.h"

#ifndef TEXT_PANEL_H_
#define TEXT_PANEL_H_

#define ARB_TEXT_RESIZE_DURATION 150

class TextPanel : public TexturePanel, public IResourceWatcher {

private:
	string text;
	Color textColor;
	float textSize, textFitPadding;
	
	bool fitToText;
	bool textDirty;
	bool textEnabled;

	TextDefinition * currentDefinition;

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
	void setTextSize(float textSize);
	void setTextFitMode(bool fitToText);		
	bool getTextFitMode();
	void setTextFitPadding(float textFitPadding);
	float getTextFitPadding();
	void setVisible(bool v);

	void layout(Vector position, cv::Size2f size);

	void resourceUpdated(string resourceId, bool loaded);
};

#endif