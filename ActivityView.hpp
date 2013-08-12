#ifndef LEAPIMAGE_ACTIVITY_VIEW_HPP_
#define LEAPIMAGE_ACTIVITY_VIEW_HPP_

#include "View.hpp"
#include <boost/function.hpp>

class ActivityView : public ViewGroup {
public:
	enum ViewState {
		New,
		Active,
		Suspended
	};

protected:	
	boost::function<void(std::string)> viewFinishedCallback;
	
	ViewState viewState;

	
public:
	ActivityView() :
	  viewState(New)
	{
	}
	
	virtual void setViewFinishedCallback(boost::function<void(std::string)> _viewFinishedCallback);

	virtual bool onLeapGesture(const Controller & controller, const Gesture & gesture) { return false; }

	virtual void onGlobalGesture(const Controller & controller, std::string gestureType) = 0;
	
	virtual void getTutorialDescriptor(vector<string> & tutorial) = 0;

	virtual void onGlobalFocusChanged(bool isFocused) {}	

	void setViewState(ViewState _viewState)
	{
		this->viewState = _viewState;
	}

	ViewState getViewState()
	{
		return this->viewState;
	}

	static float getMenuHeight();
	static float getTutorialHeight();







};

#endif