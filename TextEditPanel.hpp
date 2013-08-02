#ifndef LEAPIMAGE_TEXT_EDIT_PANEL_HPP_
#define LEAPIMAGE_TEXT_EDIT_PANEL_HPP_

#include "TextPanel.h"
#include <unordered_map>

using namespace std;

class TextEditPanel : public TextPanel {

private:
	unordered_map<int,pair<int,long> > keyStateMap;
	boost::function<void(string newText)> textChangedCallback;

	Timer cursorAnimateTimer;
	bool cursorOn;
	int maxLength;

	Timer keyRepeatTimer;
	
	bool checkKey(int key, double updateTime);

public:
	TextEditPanel();

	void setTextChangedCallback(boost::function<void(string newText)> callback);

	void update();
	void drawContent(Vector position, float width, float height);

	void setMaxLength(int maxLength);
	


};

#endif