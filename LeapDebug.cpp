#include "LeapDebug.h"
#include "FixedAspectGrid.hpp"
#include "ContentPanel.hpp"
#include "CustomGrid.hpp"
#include "ImagePanel.hpp"

LeapDebug::LeapDebug(HandProcessor * handProcessor)
{
	this->backgroundColor = Colors::Black;
	backgroundColor.setAlpha(.5);
	this->handProcessor = handProcessor;
	leapDisconnectedPanel = new TextPanel();
	leapDisconnectedPanel->setLayoutParams(LayoutParams(cv::Size2f(400,200)));
	leapDisconnectedPanel->setText(GlobalConfig::tree()->get<string>("Strings.LeapOverlay.DisconnectedMessage"));
	leapDisconnectedPanel->setTextColor(Colors::White);
	leapDisconnectedPanel->setTextSize(8);
	leapDisconnectedPanel->setBackgroundColor(Colors::DarkRed.withAlpha(.7f));

	leapDisconnectedPanel->setPosition(Vector(GlobalConfig::ScreenWidth - 450, GlobalConfig::ScreenHeight - 100, 1));
	leapDisconnectedPanel->layout(Vector(GlobalConfig::ScreenWidth - 500, GlobalConfig::ScreenHeight - 300, 10),cv::Size2f(400,150));


	leapNotFocusedPanel = new TextPanel();
	leapNotFocusedPanel->setLayoutParams(LayoutParams(cv::Size2f(400,200)));
	leapNotFocusedPanel->setText(GlobalConfig::tree()->get<string>("Strings.LeapOverlay.NotFocusedMessage"));
	leapNotFocusedPanel->setTextColor(Colors::White);
	leapNotFocusedPanel->setTextSize(8);
	leapNotFocusedPanel->setPosition(Vector(GlobalConfig::ScreenWidth - 450, GlobalConfig::ScreenHeight - 100, 1));
	leapNotFocusedPanel->layout(Vector(GlobalConfig::ScreenWidth - 500, GlobalConfig::ScreenHeight - 300, 10),cv::Size2f(400,150));
	leapNotFocusedPanel->setBackgroundColor(Colors::LeapGreen.withAlpha(.7f));

	auto p = GlobalConfig::tree()->get_child("Tutorial.RightHandImages");

	if (GlobalConfig::tree()->get<bool>("Leap.PreferLeftHand"))
		p = GlobalConfig::tree()->get_child("Tutorial.LeftHandImages");

	
	Color labelColor = Colors::White;
	auto labels = GlobalConfig::tree()->get_child("Tutorial.Labels");

	auto colorConfig = GlobalConfig::tree()->get_child("Tutorial.Background");
	Color backgroundColor = Color(colorConfig.get<int>("R"),colorConfig.get<int>("G"),colorConfig.get<int>("B"),colorConfig.get<int>("A"));

	float tutorialTextSize = labels.get<float>("FontSize");

	float textPadding = labels.get<float>("TextPadding");

	ImagePanel  * shakeGesturePanel_img = new ImagePanel(p.get<string>("ShakeGesture"));
	shakeGesturePanel_img->setBackgroundColor(backgroundColor);

	TextPanel  * shakeGesturePanel_text = new TextPanel(labels.get<string>("ShakeGesture"));
	shakeGesturePanel_text->setTextFitPadding(textPadding);
	shakeGesturePanel_text->setTextColor(labelColor);
	shakeGesturePanel_text->setBackgroundColor(backgroundColor);
	shakeGesturePanel_text->setTextSize(tutorialTextSize);
	
	ImagePanel  * swipeGesturePanel_img = new ImagePanel(p.get<string>("SwipeGesture"));
	swipeGesturePanel_img->setBackgroundColor(backgroundColor);

	TextPanel  * swipeGesturePanel_text = new TextPanel(labels.get<string>("SwipeGesture"));
	swipeGesturePanel_text->setTextFitPadding(textPadding);
	swipeGesturePanel_text->setTextColor(labelColor);
	swipeGesturePanel_text->setBackgroundColor(backgroundColor);
	swipeGesturePanel_text->setTextSize(tutorialTextSize);

	ImagePanel  * pointGesturePanel_img = new ImagePanel(p.get<string>("PointGesture"));
	pointGesturePanel_img->setBackgroundColor(backgroundColor);

	TextPanel  * pointGesturePanel_text = new TextPanel(labels.get<string>("PointGesture"));
	pointGesturePanel_text->setTextFitPadding(textPadding);
	pointGesturePanel_text->setTextColor(labelColor);
	pointGesturePanel_text->setBackgroundColor(backgroundColor);
	pointGesturePanel_text->setTextSize(tutorialTextSize);
	

	ImagePanel  * pointStopGesturePanel_img = new ImagePanel(p.get<string>("PointStopGesture"));
	pointStopGesturePanel_img->setBackgroundColor(backgroundColor);

	TextPanel  * pointStopGesturePanel_text = new TextPanel(labels.get<string>("PointStopGesture"));
	pointStopGesturePanel_text->setTextFitPadding(textPadding);
	pointStopGesturePanel_text->setTextColor(labelColor);
	pointStopGesturePanel_text->setBackgroundColor(backgroundColor);
	pointStopGesturePanel_text->setTextSize(tutorialTextSize);


	ImagePanel  * stretchGesturePanel_img = new ImagePanel(p.get<string>("StretchGesture"));
	stretchGesturePanel_img->setBackgroundColor(backgroundColor);

	TextPanel  * stretchGesturePanel_text = new TextPanel(labels.get<string>("StretchGesture"));
	stretchGesturePanel_text->setTextFitPadding(textPadding);
	stretchGesturePanel_text->setTextColor(labelColor);
	stretchGesturePanel_text->setBackgroundColor(backgroundColor);
	stretchGesturePanel_text->setTextSize(tutorialTextSize);

			
	vector<RowDefinition> gridDefinition;	
	gridDefinition.push_back(RowDefinition(.6f));
	gridDefinition.push_back(RowDefinition(.4f));
	gridDefinition[0].ColumnWidths.push_back(1);
	gridDefinition[1].ColumnWidths.push_back(1);

	CustomGrid * cg1 = new CustomGrid(gridDefinition);
	cg1->addChild(shakeGesturePanel_img);
	cg1->addChild(shakeGesturePanel_text);
		
	CustomGrid * cg2 = new CustomGrid(gridDefinition);
	cg2->addChild(swipeGesturePanel_img);
	cg2->addChild(swipeGesturePanel_text);
		
	CustomGrid * cg3 = new CustomGrid(gridDefinition);
	cg3->addChild(pointGesturePanel_img);
	cg3->addChild(pointGesturePanel_text);
	
	CustomGrid * cg4 = new CustomGrid(gridDefinition);
	cg4->addChild(pointStopGesturePanel_img);
	cg4->addChild(pointStopGesturePanel_text);
		
	CustomGrid * cg5 = new CustomGrid(gridDefinition);
	cg5->addChild(stretchGesturePanel_img);
	cg5->addChild(stretchGesturePanel_text);
	
	tutorialPanels.insert(make_pair("shake",cg1));	
	tutorialPanels.insert(make_pair("swipe",cg2));	
	tutorialPanels.insert(make_pair("point",cg3));	
	tutorialPanels.insert(make_pair("point_stop",cg4));	
	tutorialPanels.insert(make_pair("stretch",cg5));	

	tutorialLayout = new FixedAspectGrid(cv::Size2f(0,1),GlobalConfig::tree()->get<float>("Tutorial.AspectRatio"));

	tutorialPanel = new ContentPanel(tutorialLayout);
	//tutorialPanel->setBackgroundColor(Colors::DimGray.withAlpha(.2f));

	//for (auto it = tutorialPanels.begin(); it != tutorialPanels.end(); it++)
	//{
	//	it->second->layout(Vector(-300,GlobalConfig::ScreenHeight-200,10),cv::Size2f(100,1));
	//	//tutorialLayout->addChild(it->second);
	//}
		
	cv::Size2f size = cv::Size2f(300,GlobalConfig::tree()->get<float>("Tutorial.Height"));
	tutorialLayout->measure(size);
	tutorialLayout->layout(Vector(0,GlobalConfig::ScreenHeight+100,10),size);
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

void LeapDebug::setTutorialImages(vector<string> names)
{	
	if (!GlobalConfig::tree()->get<bool>("Tutorial.Enabled"))
		names.clear();

	for (auto it = tutorialPanels.begin(); it != tutorialPanels.end(); it++)
	{
		if (std::find(names.begin(),names.end(),it->first) == names.end())
			it->second->layout(Vector(it->second->getLastPosition().x,GlobalConfig::ScreenHeight+100,10),it->second->getMeasuredSize());
	}
	tutorialLayout->clearChildren();
	for (auto it = names.begin(); it != names.end(); it++)
	{
		auto gesture = tutorialPanels.find(*it);
		if (gesture != tutorialPanels.end())
			tutorialLayout->addChild(gesture->second);
	}
	cv::Size2f size = cv::Size2f(300,GlobalConfig::tree()->get<float>("Tutorial.Height"));
	tutorialPanel->measure(size);
	tutorialPanel->layout(Vector(0,GlobalConfig::ScreenHeight-size.height,10),size);
	
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

	if (tutorialPanel->isVisible())
	{
		tutorialLayout->setVisible(false);
		tutorialPanel->draw();
		tutorialLayout->setVisible(true);

		for (auto it = tutorialPanels.begin(); it != tutorialPanels.end(); it++)
		{
			it->second->draw();
		}
	}
		//tutorialPanel->draw();

	//for (auto it = tutorialPanels.begin(); it != tutorialPanels.end(); it++)
	//{
	//	if (it->second->isVisible())
	//		it->second->draw();
	//}
	//
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