#include <math.h>
#include "SDLTimer.h"
#include "Types.h"

#ifndef Animation_H
#define Animation_H

class Interpolator {

public:
	virtual double getValue(double normalizedPosition)
	{
		return -1;
	}

};


class LinearInterpolator : public Interpolator {

public:
	double getValue(double normalizedPosition)
	{
		return normalizedPosition;
	}
};


class AccelerateDecelerateInterpolator : public Interpolator {
	
public:
	double getValue(double normalizedPosition)
	{
		return (cos((normalizedPosition + 1.0) * GeomConstants::PI) / 2.0) + 0.5;
	}
};


class DoubleAnimation {


private: 
	//double velocity;
	double startValue, endValue;
	double delta, duration;
	int state;
	bool autoReverse, fillAfter;
	Interpolator * interpolator;
	Timer timer;

public:
	DoubleAnimation();
	DoubleAnimation(double start, double end, long duration, Interpolator * interpolator = NULL, bool autoReverse = false, bool fillAfter = false);
	
	void setAutoReverse(bool autoReverse);
	void setFillAfter(bool fillAfter);

	void setInterpolator(Interpolator * interpolator);

	void start();
	void stop();
	//void update();
	double getValue();
	bool isRunning();

	double getElapsedTime();

};

class VectorAnimation {



};




namespace AnimatorState
{
	const static int New = 0;
	const static int Forward = 1;
	const static int Reverse = 2;
	const static int Finished = 3;
	const static int FinishedHolding = 4;
}


#endif