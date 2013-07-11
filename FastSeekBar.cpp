#include "FastSeekBar.hpp"
#include "GLImport.h"

FastSeekBar::FastSeekBar(ScrollingView * _scrollView) : scrollView(_scrollView)
{
	scrollBarHeight = 100;
	scrollThumbWidth = 200;
	scrollThumb = new Button("< >");
	scrollThumb->setTextColor(Colors::White);
	scrollThumb->setTextSize(12);
	scrollThumb->setBackgroundColor(Colors::HoloBlueDark);
	addChild(scrollThumb);
	state = FastSeekBar::StateWaiting;
	lastScrollDelta = 0;

	scrollThumb->pointableEnterCallback = [this](LeapElement * node, Pointable & pointable){
		this->capturedHand = pointable.hand();
		if (this->capturedHand.isValid())
		{
			selectTimer.start();
			this->state = FastSeekBar::StateSelecting;
		}
	};

	scrollThumb->elementClickedCallback = [this](LeapElement * element)	{
		this->startScroll();
	};
}

void FastSeekBar::startScroll()
{
	if (state == FastSeekBar::StateSelecting && capturedHand.isValid())
	{
		state = FastSeekBar::StateScrolling;
		startPosition = capturedHand.palmPosition();
		flywheelStartPosition = scrollView->getFlyWheel()->getPosition();
		scrollThumb->setBackgroundColor(Colors::HoloBlueBright);
		moveTimer.start();
	}
}

void FastSeekBar::endScroll()
{
	if (state == StateScrolling)
	{
		state = FastSeekBar::StateWaiting;
		scrollThumb->setBackgroundColor(Colors::HoloBlueDark);
		scrollView->getFlyWheel()->impartVelocity(0);
	}
	else if (state == StateSelecting)
	{		
		scrollThumb->setBackgroundColor(Colors::HoloBlueDark);
		state = FastSeekBar::StateWaiting;
	}
	scrollThumb->setPosition(lastPosition + Vector((lastSize.width-scrollThumbWidth)*.5f,lastSize.height - scrollBarHeight,0));
}

void FastSeekBar::layout(Vector position, cv::Size2f size)
{
	this->lastPosition = position;
	this->lastSize = size;

	scrollThumb->layout(position + Vector((size.width-scrollThumbWidth)*.5f,lastSize.height - scrollBarHeight,0) ,cv::Size2f(scrollThumbWidth,scrollBarHeight));
}

void FastSeekBar::update()
{
	//View::update();

	//if (state == FastSeekBar::StateSelecting && selectTimer.seconds() > 1.0)
	//{
	//	state == FastSeekBar::StateScrolling;
	//}

}

void FastSeekBar::draw()
{

	if (state == FastSeekBar::StateScrolling)
	{
		glColor4fv(Colors::HoloBlueLight.withAlpha(.4f).getFloat());
	}
	else
	{
		glColor4fv(Colors::HoloBlueLight.withAlpha(.1f).getFloat());
	}

	Vector pos = lastPosition;

	float x1 = pos.x, x2 = pos.x + lastSize.width;
	float y1 = pos.y + (lastSize.height - scrollBarHeight), y2 = pos.y + lastSize.height;
	float z1 = pos.z;

	glBegin( GL_QUADS );
	glVertex3f(x1,y1,z1);
	glVertex3f(x1,y2,z1);
	glVertex3f(x2,y2,z1);
	glVertex3f(x2,y1,z1);
	glEnd();

	
	//cv::Size2f thumbSize = cv::Size2f(scrollThumbWidth,scrollBarHeight);
	//pos = Vector((lastSize.width-thumbSize.width)*.5f,lastSize.height - thumbSize.height,0);


	//glTranslatef(lastScrollDelta,0,0);
	scrollThumb->draw(); 
	//glTranslatef(-lastScrollDelta,0,0);
	
	
	//(pos,thumbSize.width,thumbSize.height);

	//if (state == FastSeekBar::StateScrolling)
	//{
	//	glColor4fv(Colors::HoloBlueBright.getFloat());
	//}
	//else
	//{
	//	glColor4fv(Colors::HoloBlueDark.getFloat());
	//}
	//
	//x1 = pos.x, x2 = pos.x + thumbSize.width;
	//y1 = pos.y + (thumbSize.height - scrollBarHeight), y2 = pos.y + thumbSize.height;
	//z1 = pos.z+1;
	//
	//glBegin( GL_QUADS );
	//	glVertex3f(x1,y1,z1);
	//	glVertex3f(x1,y2,z1);
	//	glVertex3f(x2,y2,z1);
	//	glVertex3f(x2,y1,z1);
	//glEnd();


	//if (state == FastSeekBar::StateSelecting)
	//{		
	//	thumbSize = cv::Size2f(scrollThumbWidth*selectTimer.seconds(),scrollBarHeight);
	//	pos = lastPosition + Vector((lastSize.width-thumbSize.width)*.5f,lastSize.height - thumbSize.height,0);
	//	glColor4fv(Colors::Red.getFloat());

	//	x1 = pos.x, x2 = pos.x + thumbSize.width;
	//	y1 = pos.y + (lastSize.height - thumbSize.height), y2 = pos.y + thumbSize.height;
	//	z1 = pos.z+2;

	//	glBegin( GL_QUADS );
	//	glVertex3f(x1,y1,z1);
	//	glVertex3f(x1,y2,z1);
	//	glVertex3f(x2,y2,z1);
	//	glVertex3f(x2,y1,z1);
	//	glEnd();
	//}
}

void FastSeekBar::onFrame(const Controller & controller)
{
	//this->update();
	capturedHand = controller.frame().hand(capturedHand.id());
	if (capturedHand.isValid())
	{
		if (state == FastSeekBar::StateScrolling)
		{
			float handX = capturedHand.palmPosition().x;
			//handX = LeapHelper::lowpass(lastHandX,handX,100,moveTimer.millis());
			//moveTimer.start();
			//lastHandX = handX;
			
			double delta = handX - startPosition.x;

			//cout << "StartX = " << startPosition.x << ", Delta = " << delta << endl;

			scrollView->getFlyWheel()->impartVelocity(delta*-30);
			//lastScrollDelta = delta;

			scrollThumb->setPosition(lastPosition + Vector((lastSize.width-scrollThumbWidth)*.5f - delta,lastSize.height - scrollBarHeight,0));
		}
		else if (state == FastSeekBar::StateSelecting)
		{			
		}
	}
	else
	{
		endScroll();
	}
}