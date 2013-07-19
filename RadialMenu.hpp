#ifndef LEAPIMAGE_RADIAL_MENU_HPP_
#define LEAPIMAGE_RADIAL_MENU_HPP_

#include <vector>
#include "View.hpp"
#include <Leap.h>
#include "Button.hpp"


struct RadialMenuItem {

	string label;
	string id;
	Color buttonColor;

	RadialMenuItem(string _label) :
		label(_label),
		id(_label)
	{
		
	}
	
	RadialMenuItem(string _label, string _id, Color _color) :
		label(_label),
		id(_id),
		buttonColor(_color)
	{
		
	}

};

class RadialMenu : public ViewGroup, public GlobalGestureListener {

private:

	ViewGroup * buttonView;
	void itemClicked(string id);

	int state;

	View * menuLaunchButton;

	View * privacyInfoBox;

	bool blurWasEnabled;
	
public:		
	static RadialMenu * instance;

	const static int MenuState_ButtonOnly = 0;
	const static int MenuState_DisplayFull= 1;
	const static int MenuState_ShowingDialog = 2;

	vector<RadialMenuItem> items;

	RadialMenu(vector<RadialMenuItem> & items);
		
	boost::function<bool(string)> ItemClickedCallback;

	void setItems(vector<RadialMenuItem> & items);

	bool onLeapGesture(const Controller & controller, const Gesture & gesture);
	void onGlobalGesture(const Controller & controller, std::string gestureType);	
	void getTutorialDescriptor(vector<string> & tutorial);

	void layout(Vector pos, cv::Size2f size);

	float getZValue();

	void show();
	void dismiss();
	
	void draw();

	bool checkMenuOpenGesture(const Gesture & circle);

	LeapElement * elementAtPoint(int x, int y, int & state);


};

#endif