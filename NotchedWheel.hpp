#ifndef LEAPIMAGE_NEWPANEL_NOTCHEDWHEEL_HPP_
#define LEAPIMAGE_NEWPANEL_NOTCHEDWHEEL_HPP_

#include "Flywheel.h"
#include <boost/function.hpp>

class NotchedWheel : public FlyWheel {

private:
	double notchOffset, notchSpacing;
	double targetNotch, currentNotchIndex;

	bool hasTarget;

	boost::function<void(int,int)> notchChangedListener;

public:
	NotchedWheel(double notchOffset, double notchSpacing);

	void setNotches(double offset, double spacing);
	double getNotchingSpacing();
	double getNotchOffset();
	
	void update(double elapsed);

	void flingWheel(double flingVelocity);

	void overrideValue(double position);

	void setNotchChangedListener(boost::function<void(int,int)>);
	void setCurrentNotchIndex(int notchIndex);


};


#endif