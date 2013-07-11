#ifndef LEAPIMAGE_RADIAL_MENU_HPP_
#define LEAPIMAGE_RADIAL_MENU_HPP_

#include <vector>
#include "View.hpp"
#include <Leap.h>


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

	ViewGroup * rootView, * buttonView;
	void itemClicked(string id);
	
public:	
	vector<RadialMenuItem> items;

	RadialMenu(vector<RadialMenuItem> & items);
		
	boost::function<bool(string)> ItemClickedCallback;

	void setItems(vector<RadialMenuItem> & items);

	bool onLeapGesture(const Controller & controller, const Gesture & gesture);
	void onGlobalGesture(const Controller & controller, std::string gestureType);

	void layout(Vector pos, cv::Size2f size);

	void setVisible(bool visible);
	void show();
	
	void draw();

	bool checkMenuOpenGesture(const Gesture & circle);


};

#endif