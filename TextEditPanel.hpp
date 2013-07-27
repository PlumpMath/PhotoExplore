#ifndef LEAPIMAGE_TEXT_EDIT_PANEL_HPP_
#define LEAPIMAGE_TEXT_EDIT_PANEL_HPP_

#include "TextPanel.h"
#include <hash_map>

using namespace std;

class TextEditPanel : public TextPanel {

private:
	hash_map<int,int> keyStateMap;
	boost::function<void(string newText)> textChangedCallback;


public:
	TextEditPanel();

	void setTextChangedCallback(boost::function<void(string newText)> callback);

	void update();


};

#endif