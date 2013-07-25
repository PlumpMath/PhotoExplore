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

	
	auto labels = GlobalConfig::tree()->get_child("Tutorial.Labels");

	Color backgroundColor = Color(GlobalConfig::tree()->get_child("Tutorial.Background"));
		
	Color textBackground = Color(GlobalConfig::tree()->get_child("Tutorial.TextBackground"));
	Color labelColor = Color(GlobalConfig::tree()->get_child("Tutorial.TextColor"));
	Color invertedLabelColor = Color(GlobalConfig::tree()->get_child("Tutorial.InvertedTextColor"));


	float tutorialTextSize = labels.get<float>("FontSize");// / (GlobalConfig::ScreenHeight/1440.0f);

	float textPadding = labels.get<float>("TextPadding");

	string fontName = labels.get<string>("FontName");

	cv::Size2f imagePanelSize = cv::Size2f(300,GlobalConfig::tree()->get<float>("Tutorial.Height")*.6f);

	ImagePanel  * shakeGesturePanel_img = new ImagePanel(p.get<string>("ShakeGesture"),imagePanelSize);
	shakeGesturePanel_img->setBackgroundColor(backgroundColor);

	TextPanel  * shakeGesturePanel_text = new TextPanel(labels.get<string>("ShakeGesture"));
	shakeGesturePanel_text->setTextFitPadding(textPadding);
	shakeGesturePanel_text->setTextColor(labelColor);
	shakeGesturePanel_text->setBackgroundColor(textBackground);
	shakeGesturePanel_text->setTextSize(tutorialTextSize,false);
	shakeGesturePanel_text->setFontName(fontName);
	
	TextPanel  * shakeGesturePanel_text_inv = new TextPanel(labels.get<string>("ShakeGesture"));
	shakeGesturePanel_text_inv->setTextFitPadding(textPadding);
	shakeGesturePanel_text_inv->setTextColor(invertedLabelColor);
	shakeGesturePanel_text_inv->setBackgroundColor(textBackground);
	shakeGesturePanel_text_inv->setTextSize(tutorialTextSize,false);
	shakeGesturePanel_text_inv->setFontName(fontName);
	
	ImagePanel  * swipeGesturePanel_img = new ImagePanel(p.get<string>("SwipeGesture"),imagePanelSize);
	swipeGesturePanel_img->setBackgroundColor(backgroundColor);

	TextPanel  * swipeGesturePanel_text = new TextPanel(labels.get<string>("SwipeGesture"));
	swipeGesturePanel_text->setTextFitPadding(textPadding);
	swipeGesturePanel_text->setTextColor(labelColor);
	swipeGesturePanel_text->setBackgroundColor(textBackground);
	swipeGesturePanel_text->setTextSize(tutorialTextSize,false);
	swipeGesturePanel_text->setFontName(fontName);

	ImagePanel  * pointGesturePanel_img = new ImagePanel(p.get<string>("PointGesture"),imagePanelSize);
	pointGesturePanel_img->setBackgroundColor(backgroundColor);

	TextPanel  * pointGesturePanel_text = new TextPanel(labels.get<string>("PointGesture"));
	pointGesturePanel_text->setTextFitPadding(textPadding);
	pointGesturePanel_text->setTextColor(labelColor);
	pointGesturePanel_text->setBackgroundColor(textBackground);
	pointGesturePanel_text->setTextSize(tutorialTextSize,false);
	pointGesturePanel_text->setFontName(fontName);

	TextPanel  * pointGesturePanel_text_inv = new TextPanel(labels.get<string>("PointGesture"));
	pointGesturePanel_text_inv->setTextFitPadding(textPadding);
	pointGesturePanel_text_inv->setTextColor(invertedLabelColor);
	pointGesturePanel_text_inv->setBackgroundColor(textBackground);
	pointGesturePanel_text_inv->setTextSize(tutorialTextSize,false);
	pointGesturePanel_text_inv->setFontName(fontName);


	ImagePanel  * pointStopGesturePanel_img = new ImagePanel(p.get<string>("PointStopGesture"),imagePanelSize);
	pointStopGesturePanel_img->setBackgroundColor(backgroundColor);

	TextPanel  * pointStopGesturePanel_text = new TextPanel(labels.get<string>("PointStopGesture"));
	pointStopGesturePanel_text->setTextFitPadding(textPadding);
	pointStopGesturePanel_text->setTextColor(labelColor);
	pointStopGesturePanel_text->setBackgroundColor(textBackground);
	pointStopGesturePanel_text->setTextSize(tutorialTextSize,false);
	pointStopGesturePanel_text->setFontName(fontName);


	ImagePanel  * stretchGesturePanel_img = new ImagePanel(p.get<string>("StretchGesture"),imagePanelSize);
	stretchGesturePanel_img->setBackgroundColor(backgroundColor);

	TextPanel  * stretchGesturePanel_text = new TextPanel(labels.get<string>("StretchGesture"));
	stretchGesturePanel_text->setTextFitPadding(textPadding);
	stretchGesturePanel_text->setTextColor(invertedLabelColor);
	stretchGesturePanel_text->setBackgroundColor(textBackground);
	stretchGesturePanel_text->setTextSize(tutorialTextSize,false);
	stretchGesturePanel_text->setFontName(fontName);

			 
	vector<RowDefinition> gridDefinition;	
	gridDefinition.push_back(RowDefinition(.6f));
	gridDefinition.push_back(RowDefinition(.4f));
	gridDefinition[0].ColumnWidths.push_back(1);
	gridDefinition[1].ColumnWidths.push_back(1);

	CustomGrid * cg1 = new CustomGrid(gridDefinition);
	cg1->addChild(shakeGesturePanel_img);
	cg1->addChild(shakeGesturePanel_text);

	
	CustomGrid * cg1_inv = new CustomGrid(gridDefinition);
	cg1_inv->addChild(shakeGesturePanel_img);
	cg1_inv->addChild(shakeGesturePanel_text_inv);
		
		
	CustomGrid * cg2 = new CustomGrid(gridDefinition);
	cg2->addChild(swipeGesturePanel_img);
	cg2->addChild(swipeGesturePanel_text);
		
	CustomGrid * cg3 = new CustomGrid(gridDefinition);
	cg3->addChild(pointGesturePanel_img);
	cg3->addChild(pointGesturePanel_text);

	
	CustomGrid * cg3_inv = new CustomGrid(gridDefinition);
	cg3_inv->addChild(pointGesturePanel_img);
	cg3_inv->addChild(pointGesturePanel_text_inv);
	
	CustomGrid * cg4 = new CustomGrid(gridDefinition);
	cg4->addChild(pointStopGesturePanel_img);
	cg4->addChild(pointStopGesturePanel_text);
		
	CustomGrid * cg5 = new CustomGrid(gridDefinition);
	cg5->addChild(stretchGesturePanel_img);
	cg5->addChild(stretchGesturePanel_text);
	
	tutorialPanels.insert(make_pair("shake",cg1));	
	tutorialPanels.insert(make_pair("shake_inv",cg1_inv));	
	tutorialPanels.insert(make_pair("swipe",cg2));	
	tutorialPanels.insert(make_pair("point",cg3));	
	tutorialPanels.insert(make_pair("point_inv",cg3_inv));	
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

	glPopMatrix();	
}

void LeapDebug::drawPointer(LeapDebugVisual * debugVisual)
{
	float drawWidth,drawHeight,x1,x2,y1,y2, z1;

	z1 = debugVisual->depth + 60;
		
	drawHeight = drawWidth = debugVisual->size;	

	if (drawHeight == 0)
		return;

	static float vertices = GlobalConfig::tree()->get<float>("Overlay.VertexCount");
	static float cornerAngle = GeomConstants::DegToRad*GlobalConfig::tree()->get<float>("Overlay.CornerAngle");
	static float angleOffset = GeomConstants::DegToRad*GlobalConfig::tree()->get<float>("Overlay.AngleOffset");

	//if (vertices == 4 && angleOffset == 0)
	//{
	//	x1 = debugVisual->screenPoint.x - floorf(drawWidth/2.0f);
	//	x2 = debugVisual->screenPoint.x + floorf(drawWidth/2.0f);
	//	y1 = debugVisual->screenPoint.y - floorf(drawHeight/2.0f);
	//	y2 = debugVisual->screenPoint.y + floorf(drawHeight/2.0f);
	//	
	//	glColor4fv(debugVisual->fillColor.getFloat());
	//	glBindTexture( GL_TEXTURE_2D, NULL);
	//	glLineWidth(0);
	//	glBegin(GL_QUADS);
	//		glVertex3f(x1,y1,z1);
	//		glVertex3f(x2,y1,z1);
	//		glVertex3f(x2,y2,z1);
	//		glVertex3f(x1,y2,z1);
	//	glEnd();

	//	glColor4fv(debugVisual->lineColor.getFloat());
	//	glBindTexture( GL_TEXTURE_2D, NULL);
	//	glLineWidth(1);
	//	glBegin(GL_LINE_LOOP);
	//		glVertex3f(x1,y1,z1);
	//		glVertex3f(x2,y1,z1);
	//		glVertex3f(x2,y2,z1);
	//		glVertex3f(x1,y2,z1);
	//	glEnd();
	//}
	//else
	//{
	float length = drawHeight*.5f;
	float anglePerVertex = (Leap::PI*2.0f)/vertices;


	glColor4fv(debugVisual->fillColor.getFloat());
	glBindTexture( GL_TEXTURE_2D, NULL);
	glLineWidth(0);
	glTranslatef(debugVisual->screenPoint.x,debugVisual->screenPoint.y,0);
	glBegin(GL_POLYGON);
	for (float v=0;v<vertices;v++)
	{
		float angle = v*anglePerVertex;
		angle += angleOffset;
		glVertex3f(sinf(angle)*length,cosf(angle)*length,z1);
	}
	glEnd();

	float alphaScale = debugVisual->lineColor.colorArray[3];

	float lineWidth [] = {debugVisual->lineWidth*3.0f,debugVisual->lineWidth*2.0f,debugVisual->lineWidth};
	float * lineColor [] = {debugVisual->lineColor.withAlpha(.2f*alphaScale).getFloat(),debugVisual->lineColor.withAlpha(.4f*alphaScale).getFloat(),debugVisual->lineColor.withAlpha(1.0f*alphaScale).getFloat()};

	for (int i=0; i < 3; i++)
	{
		glColor4fv(lineColor[i]);
		glLineWidth(lineWidth[i]);

		glBegin(GL_LINE_LOOP);
		for (float v=0;v<vertices;v++)
		{
			float angle = v*anglePerVertex;
			angle += angleOffset;
			glVertex3f(sinf(angle-cornerAngle)*length,cosf(angle-cornerAngle)*length,z1+ ((float)i * .1f));	
			glVertex3f(sinf(angle+cornerAngle)*length,cosf(angle+cornerAngle)*length,z1+ ((float)i * .1f));		
		}
		glEnd();		
	}
	glTranslatef(-debugVisual->screenPoint.x,-debugVisual->screenPoint.y,0);

	//}
}