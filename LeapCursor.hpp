#ifndef LEAPIMAGE_OVERLAY_LEAPCURSOR_HPP_
#define LEAPIMAGE_OVERLAY_LEAPCURSOR_HPP_

#include <Leap.h>
#include "GLImport.h"
#include "Types.h"

using namespace Leap;


class OverlayVisual {

private:
	bool visible;

public:	
	OverlayVisual() : visible(true) {}

	virtual void setVisible(bool visible) {this->visible = visible;}
	virtual bool isVisible() { return visible; }

	virtual void onFrame(const Controller & controller) { }
	virtual void update() { }
	virtual void draw() { }
};

class PointableCursor : public OverlayVisual {
		
protected:
	Vector drawPoint;
	Color cursorBaseColor;
	float fillAlpha, borderAlpha;
	
	Color fillColor,lineColor;
	
public:	
	int trackPointableId;

	double size;
	float depth, lineWidth;

	PointableCursor(float size, Color cursorBaseColor);

	void setBorderAlpha(float borderAlpha);
	void setFillAlpha(float fillAlpha);
	void setCursorColor(Color cursorColor);

	Vector getDrawPoint();

	virtual void onFrame(const Controller & controller);
	virtual void update();
	virtual void draw();	

	static float getDefaultSize()
	{
		float cursorDimension = (((float)GlobalConfig::ScreenWidth) / 2560.0f) *  GlobalConfig::tree()->get<float>("Overlay.BaseCursorSize");
		return cursorDimension;
	}
};


class MouseCursor : public PointableCursor {

public:
	MouseCursor(double size, Color fillColor);

	void update();

};

class PointableTouchCursor : public OverlayVisual {


private:
	PointableCursor * c1, * c2;
	float minSize, maxSize, touchDistanceThreshold;

public:
	PointableTouchCursor(float minSize, float maxSize, Color cursorColor);	

	void setPointable(Pointable & pointable);
	void setColor(Color color);

	void onFrame(const Controller & controller);
	void draw();

};

class ArrowCursor : public PointableCursor {
	
private:
	float cursorDirection, squareFactor;
	float filledTouchDist, triangleTouchDist, squareTouchDist;
	
public:
	ArrowCursor(float,Color);
	
	void setDirection(float direction);
	float getDirection();

	void onFrame(const Controller & controller);
	void draw();


};

class OpposingArrowCursor : public OverlayVisual {

private:
	//ArrowCursor * c1, * c2, * c3, * c4;
	vector<ArrowCursor*> arrows;

public:
	OpposingArrowCursor(float,Color);

	void setPointables(Pointable p1, Pointable p2);

	void setSize(float size);
	
	void onFrame(const Controller & controller);
	void draw();

};


#endif