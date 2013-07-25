#ifndef LEAPIMAGE_PUNCHGESTUREDETECTOR_HPP_
#define	LEAPIMAGE_PUNCHGESTUREDETECTOR_HPP_

#include <Leap.h>
#include <boost/function.hpp>

using namespace Leap;

class PunchGestureDetector : public Listener {

public:
	static PunchGestureDetector& getInstance();
	
private:
	PunchGestureDetector();
	PunchGestureDetector(PunchGestureDetector const&);
	void operator=(PunchGestureDetector const&); 
	~PunchGestureDetector();
		
	boost::function<void(Hand punchingHand)> punchListener;

	Hand punchingHand;

	int state;


public:
	void onFrame(const Controller & controller);


};


#endif