#ifndef LEAPIMAGE_OVERLAY_LEAPCURSOR_HPP_
#define LEAPIMAGE_OVERLAY_LEAPCURSOR_HPP_

#include <Leap.h>
#include "GLImport.h"
#include "Types.h"

using namespace Leap;


class OverlayVisual {

public:	
	virtual void onFrame(const Controller & controller) { }
	virtual void update() { }
	virtual void draw() { }
};

class PointableCursor : public OverlayVisual {
		
protected:
	Vector drawPoint;

public:	
	int trackPointableId;

	Color fillColor,lineColor;
	double size;
	float depth, lineWidth;

	PointableCursor(Pointable _trackPointable, double size, Color fillColor)
	{
		this->trackPointableId = _trackPointable.id();
		this->size = size;
		this->fillColor = fillColor;
		this->depth= 10;
		this->lineWidth = 1;
	}

	PointableCursor(double size, Color fillColor)
	{
		trackPointableId = -1;
		this->size = size;
		this->fillColor = fillColor;
		this->depth= 10;
		this->lineWidth = 1;
	}

	Vector getDrawPoint();

	virtual void onFrame(const Controller & controller);
	virtual void update();
	virtual void draw();
};


class MouseCursor : public PointableCursor {

public:
	MouseCursor(double size, Color fillColor);

	void update();

};

class ArrowCursor : public PointableCursor {
	
private:
	float cursorDirection, squareFactor;
	
public:
	ArrowCursor(float,Color);

	void setDirection(float direction);
	float getDirection();

	void onFrame(const Controller & controller);
	void draw();


};

class OpposingArrowCursor : public OverlayVisual {

private:
	ArrowCursor * c1, * c2;

public:
	OpposingArrowCursor(float,Color);

	void setPointables(Pointable p1, Pointable p2);

	void setSize(float size);
	
	void onFrame(const Controller & controller);
	void draw();

};


#endif