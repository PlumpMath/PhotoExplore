#include <cstdio>
#include <vector>
#include <opencv2/opencv.hpp>
#include "LeapInput.hpp"
#include "Animation.h"
#include "Types.h"
#include "View.hpp"
#include "GLImport.h"

#ifndef Panel_Base_H_
#define Panel_Base_H_

class PanelBase : public View {
private:	
	cv::Rect_<float> boundingRect;

	float beforeSelectionBorderThickness;

protected:
	float width,height;
	Color backgroundColor, borderColor;
	float borderThickness;
	bool useLineBorder;

	Vector position, offset;
	
	long layoutDuration;
	Timer loadAnimTimer;

	bool panelDirty;
	bool drawingLoadAnimation;
	bool animateOnLayout;


	PanelBase();

	virtual void layout(cv::Size2f targetSize);


	
	DoubleAnimation xAnimation,yAnimation,zAnimation;
	DoubleAnimation xPosAnimation, yPosAnimation, zPosAnimation;

	DoubleAnimation heightAnimation, widthAnimation;
	DoubleAnimation scaleYAnimation, scaleXAnimation;

	virtual void getBoundingArea(Vector & position, float & drawWidth, float & drawHeight);
	virtual void drawBackground(Vector drawPosition, float drawWidth, float drawHeight);
	virtual void drawContent(Vector drawPosition, float drawWidth, float drawHeight) = 0;

public:
	bool NudgeAnimationEnabled;	
	float BorderSelectionThickness, ScaleOnEnter;
	
	void requestLayout();

	virtual void setPanelSize(float width, float height, float padding = 0);
	virtual void animatePanelSize(float width, float height, long duration = -1);
	virtual void animatePanelSize(cv::Size2f targetSize, long duration);

	virtual void setPosition(Vector position);
	virtual void animateToPosition(Vector position, long durationX = -1, long durationY = -1);
	
	void setLayoutDuration(long duration);

	void setAnimateOnLayout(bool animateOnLayout);
	bool isAnimateOnLayout();
	
	void setBorderThickness(float borderThickness);
	void setBorderColor(Color borderColor);

	void setUseLineBorder(bool useLineBorder);
	bool isUseLineBorder();

	//void setLayoutParams(cv::Size2f & desiredSize);
	//void setLayoutParams(LayoutParams & _params);

	void setBackgroundColor(Color color);
	Color getBackgroundColor();
	
	void draw();

	void drawLoadTimer(Vector pos, float w, float h);

	virtual void measure(cv::Size2f & measuredSize);

	virtual void layout(Vector layoutPosition, cv::Size2f layoutSize);

	virtual void drawPanel(Vector drawPosition, float drawWidth, float drawHeight);
	
	virtual void update(double deltaTicks);

	Color loadAnimationColor;
	void setDrawLoadAnimation(bool drawLoad);
		
	Vector getCenter();

	Vector getPosition();
	float getWidth();
	float getHeight();

	string panelId;
	
	//LeapElement
	cv::Rect_<int> getHitRect();

	virtual void OnPointableEnter(Pointable & pointable);
	virtual void OnPointableExit(Pointable & pointable);

	float getZValue();
	
	void abortAnimations();
};


#endif
