#include "LeapDebug.h"


LeapDebug::LeapDebug(HandProcessor * handProcessor)
{
	this->backgroundColor = Colors::Black;
	backgroundColor.setAlpha(.5);
	this->handProcessor = handProcessor;
	leapDisconnectedPanel = new TextPanel();
	leapDisconnectedPanel->setLayoutParams(LayoutParams(cv::Size2f(400,150)));
	leapDisconnectedPanel->setText("Leap disconnected");
	leapDisconnectedPanel->setTextColor(Colors::White);
	leapDisconnectedPanel->setTextSize(10);
	leapDisconnectedPanel->setBackgroundColor(Colors::DarkRed.withAlpha(.7f));

	leapDisconnectedPanel->setPosition(Vector(GlobalConfig::ScreenWidth - 450, GlobalConfig::ScreenHeight - 100, 1));
	leapDisconnectedPanel->layout(Vector(GlobalConfig::ScreenWidth - 500, GlobalConfig::ScreenHeight - 300, 10),cv::Size2f(400,150));


	leapNotFocusedPanel = new TextPanel();
	leapNotFocusedPanel->setLayoutParams(LayoutParams(cv::Size2f(400,150)));
	leapNotFocusedPanel->setText("Leap not focused");
	leapNotFocusedPanel->setTextColor(Colors::White);
	leapNotFocusedPanel->setTextSize(10);
	leapNotFocusedPanel->setPosition(Vector(GlobalConfig::ScreenWidth - 450, GlobalConfig::ScreenHeight - 100, 1));
	leapNotFocusedPanel->layout(Vector(GlobalConfig::ScreenWidth - 500, GlobalConfig::ScreenHeight - 300, 10),cv::Size2f(400,150));
	leapNotFocusedPanel->setBackgroundColor(Colors::LeapGreen.withAlpha(.7f));
}


void LeapDebug::onFrame(const Controller& controller)
{

	if (controller.isConnected() != isControllerConnected)
	{
		isControllerConnected = controller.isConnected();
		isControllerFocused = controller.hasFocus();

		if (isControllerConnected)
		{
			leapDisconnectedPanel->setVisible(false);
			
			leapNotFocusedPanel->setVisible(false);
		}
		else
		{			
			leapNotFocusedPanel->setVisible(false);
			leapDisconnectedPanel->setVisible(true);


			//leapDisconnectedPanel->layout(Vector(GlobalConfig::ScreenWidth - 500, GlobalConfig::ScreenHeight - 300, 10),cv::Size2f(400,150));
		}
	}

	if (controller.isConnected() && isControllerFocused != controller.hasFocus())
	{
		isControllerFocused = controller.hasFocus();
		if (isControllerFocused)
		{
			leapDisconnectedPanel->setVisible(false);
			//leapDisconnectedPanel->layout(Vector(GlobalConfig::ScreenWidth - 500, GlobalConfig::ScreenHeight - 300, 10),cv::Size2f(0,150));

			leapNotFocusedPanel->setVisible(false);
			//leapNotFocusedPanel->layout(Vector(GlobalConfig::ScreenWidth - 500, GlobalConfig::ScreenHeight - 300, 10),cv::Size2f(0,150));
		}
		else
		{
			leapNotFocusedPanel->setVisible(true);
			leapDisconnectedPanel->setVisible(false);
			
			//leapNotFocusedPanel->layout(Vector(GlobalConfig::ScreenWidth - 500, GlobalConfig::ScreenHeight - 300, 10),cv::Size2f(400,150));
		}
	}

	
	Frame frame = controller.frame();

	if (frame.id() == lastFrame.id())
		return;

	lastFrame = frame;
	static map<int,LeapDebugVisual*> ldv_intent;

	//HandModel * intentModel = handProcessor->lastModel();

	//if (false && intentModel->HandId < 0)
	//{
	//	Pointable pointable = frame.pointable(intentModel->IntentFinger);
	//	if (pointable.isValid())
	//	{
	//		Vector screenPoint = LeapHelper::FindScreenPoint(controller,pointable);

	//		if (ldv_intent.find(-2) == ldv_intent.end())
	//		{	
	//			LeapDebugVisual * ldv =new LeapDebugVisual(screenPoint,1,LeapDebugVisual::LiveForever,41,Colors::Black.withAlpha(.5f));
	//			ldv_intent.insert(make_pair(-2,ldv));
	//			ldv->lineColor = Colors::LimeGreen;
	//			addDebugVisual(ldv);	
	//		}
	//		else
	//		{
	//			ldv_intent.find(-2)->second->screenPoint = Point2f(screenPoint.x,screenPoint.y);
	//		}	

	//		for (auto it = ldv_intent.begin();it != ldv_intent.end();it++)
	//		{
	//			if (it->first != -2)
	//			{
	//				it->second->wayOfLife = LeapDebugVisual::LiveByTime;
	//				it->second->timeToLive = 0;

	//				it = ldv_intent.erase(it);
	//				if (it == ldv_intent.end())
	//					break;
	//			}
	//		}
	//	}
	//	else
	//	{
	//		//auto it = ldv_intent.find(-2);
	//		//if (it != ldv_intent.end())
	//		//{
	//		//	it->second->wayOfLife = LeapDebugVisual::LiveByTime;
	//		//	it->second->timeToLive = 0;

	//		//	ldv_intent.erase(it);
	//		//}
	//		for (auto it = ldv_intent.begin();it != ldv_intent.end();it++)
	//		{
	//			if (!controller.frame().hand(it->first).isValid())
	//			{
	//				it->second->wayOfLife = LeapDebugVisual::LiveByTime;
	//				it->second->timeToLive = 0;

	//				it = ldv_intent.erase(it);
	//				if (it == ldv_intent.end())
	//					break;
	//			}
	//		}
	//	}
	//}
	//else
	//{
	//	for (int h=0;h < frame.hands().count();h++)
	//	{
	//		Hand hand = frame.hands()[h];
	//		HandModel * handModel = handProcessor->lastModel(hand.id());

	//		Color color = Colors::Black;		
	//		if (frame.hands().count() > 1 && hand.id() == frame.hands().leftmost().id())
	//			color = Colors::Red;
	//		else
	//			break;

	//		color.setAlpha(.5f);

	//		Pointable pointable = hand.pointable(handModel->IntentFinger);
	//		if (pointable.isValid())
	//		{
	//			Vector screenPoint = LeapHelper::FindScreenPoint(controller,pointable);

	//			if (ldv_intent.find(hand.id()) == ldv_intent.end())
	//			{	
	//				LeapDebugVisual * ldv =new LeapDebugVisual(screenPoint,1,LeapDebugVisual::LiveForever,41,color);
	//				ldv_intent.insert(make_pair(hand.id(),ldv));
	//				ldv->lineColor = Colors::HoloBlueBright;
	//				addDebugVisual(ldv);	
	//			}
	//			else
	//			{
	//				ldv_intent.find(hand.id())->second->screenPoint = Point2f(screenPoint.x,screenPoint.y);
	//			}			
	//		}
	//	}

	//	for (auto it = ldv_intent.begin();it != ldv_intent.end();it++)
	//	{
	//		if (!controller.frame().hand(it->first).isValid())
	//		{
	//			it->second->wayOfLife = LeapDebugVisual::LiveByTime;
	//			it->second->timeToLive = 0;

	//			it = ldv_intent.erase(it);
	//			if (it == ldv_intent.end())
	//				break;
	//		}
	//	}
	//}
}

void LeapDebug::showValue(string key, double value)
{	
	//std::stringstream textStream;		
	//textStream.precision(2);
	//textStream.setf(std::ios_base::fixed);
	//textStream << key;
	//textStream << " = ";
	//textStream << value;
	//LeapImageOut << textStream.str() << '\n';
}

void LeapDebug::addDebugVisual(LeapDebugVisual * ldv)
{
	persistentVisuals.push_back(ldv);
}

static void drawHand(Vector handCenter, Hand hand, HandModel * handModel)
{	
	glLineWidth(2);
	for (int i=0;i< hand.fingers().count();i++)
	{
		Finger f = hand.fingers()[i];
		
		float angle = LeapHelper::GetFingerTipAngle(hand,f);
		Vector drawFinger = Vector(200 * cos(angle), 200 * sin(angle), 0) + handCenter;

		if (handModel->IntentFinger == f.id())
			glColor4fv(Colors::Lime.getFloat());
		else 
			glColor4fv(Colors::OrangeRed.getFloat());

		glBegin(GL_LINES);
			glVertex3f(handCenter.x, handCenter.y, handCenter.z);
			glVertex3f(drawFinger.x, drawFinger.y, drawFinger.z);
		glEnd();
	}

}

void LeapDebug::draw()
{
	glPushMatrix();
	glLoadIdentity();
	
	if (leapDisconnectedPanel->isVisible())
		leapDisconnectedPanel->draw();
	
	if (leapNotFocusedPanel->isVisible())
		leapNotFocusedPanel->draw();
		
	for (auto it = pointableList.begin(); it != pointableList.end(); it++)
	{
		drawPointer(&(*it).second);
	}
	
	for (int i = 0;i< persistentVisuals.size();i++)
	{
		LeapDebugVisual * ldv = persistentVisuals.at(i);

		drawPointer(ldv);

		ldv->iterateLife();

		if (!ldv->isAlive())
		{
			persistentVisuals.erase(persistentVisuals.begin()+i);
			i--;
			delete ldv;
		}
	}
	
	//for (int i=0;i<lastFrame.hands().count();i++)
	//{
	//	Hand hand = lastFrame.hands()[i];
	//	HandModel * handModel = handProcessor->lastModel(hand.id());
	//	drawHand(Vector(100+(i*200),100,10),hand,handModel);
	//}
	glPopMatrix();
	
}

void LeapDebug::drawPointer(LeapDebugVisual * debugVisual)
{
	float drawWidth,drawHeight,x1,x2,y1,y2, z1;

	z1 = debugVisual->depth;
		
	drawHeight = drawWidth = debugVisual->size;	

	x1 = debugVisual->screenPoint.x - floorf(drawWidth/2.0f);
	x2 = debugVisual->screenPoint.x + floorf(drawWidth/2.0f);
	y1 = debugVisual->screenPoint.y - floorf(drawHeight/2.0f);
	y2 = debugVisual->screenPoint.y + floorf(drawHeight/2.0f);
		
	//cout << "Drawing dbg vis at " << debugVisual->screenPoint.x << ","<<debugVisual->screenPoint.y << endl;

	glColor4fv(debugVisual->fillColor.getFloat());
	glBindTexture( GL_TEXTURE_2D, NULL);
	glLineWidth(0);
	glBegin(GL_QUADS);
		glVertex3f(x1,y1,z1);
		glVertex3f(x2,y1,z1);
		glVertex3f(x2,y2,z1);
		glVertex3f(x1,y2,z1);
	glEnd();

	glColor4fv(debugVisual->lineColor.getFloat());
	glBindTexture( GL_TEXTURE_2D, NULL);
	glLineWidth(1);
	glBegin(GL_LINE_LOOP);
		glVertex3f(x1,y1,z1);
		glVertex3f(x2,y1,z1);
		glVertex3f(x2,y2,z1);
		glVertex3f(x1,y2,z1);
	glEnd();
}