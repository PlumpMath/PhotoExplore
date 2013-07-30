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
	
	
	auto labels = GlobalConfig::tree()->get_child("Tutorial.Labels");

	Color backgroundColor = Color(GlobalConfig::tree()->get_child("Tutorial.Background"));
		
	Color textBackground = Color(GlobalConfig::tree()->get_child("Tutorial.TextBackground"));
	Color labelColor = Color(GlobalConfig::tree()->get_child("Tutorial.TextColor"));
	Color invertedLabelColor = Color(GlobalConfig::tree()->get_child("Tutorial.InvertedTextColor"));


	float tutorialTextSize = labels.get<float>("FontSize");
	float textPadding = labels.get<float>("TextPadding");
	string fontName = labels.get<string>("FontName");

	cv::Size2f imagePanelSize = cv::Size2f(300,GlobalConfig::tree()->get<float>("Tutorial.Height")*.6f);
	
	auto tutorialIcons = GlobalConfig::tree()->get_child("Tutorial.Icons");

	for (auto tutIt = tutorialIcons.begin(); tutIt != tutorialIcons.end(); tutIt++)
	{

		ImagePanel  * tutorialImage = new ImagePanel(tutIt->second.get<string>("RightImage"));
		tutorialImage->setScaleMode(ScaleMode::None);
		tutorialImage->setAllowSubPixelRendering(false);

		tutorialImage->setBackgroundColor(backgroundColor);

		TextPanel  * tutorialText = new TextPanel(tutIt->second.get<string>("Text"));
		tutorialText->setTextFitPadding(textPadding);
		tutorialText->setTextColor(tutIt->second.get<bool>("InvertColor") ? invertedLabelColor : labelColor);
		tutorialText->setBackgroundColor(textBackground);
		tutorialText->setTextSize(tutorialTextSize,false);
		tutorialText->setFontName(fontName);

		vector<RowDefinition> gridDefinition;	
		gridDefinition.push_back(RowDefinition(.6f));
		gridDefinition.push_back(RowDefinition(.4f));
		gridDefinition[0].ColumnWidths.push_back(1);
		gridDefinition[1].ColumnWidths.push_back(1);

		CustomGrid * tutorialLayout = new CustomGrid(gridDefinition);
		tutorialLayout->addChild(tutorialImage);
		tutorialLayout->addChild(tutorialText);

		tutorialPanels.insert(make_pair(tutIt->second.get<string>("Name"),tutorialLayout));	
	}

	tutorialLayout = new FixedAspectGrid(cv::Size2f(0,1),GlobalConfig::tree()->get<float>("Tutorial.AspectRatio"));
	tutorialPanel = new ContentPanel(tutorialLayout);

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

	if (drawHeight == 0 || (debugVisual->screenPoint.x == 0 && debugVisual->screenPoint.y == 0))
		return;

	static float vertices = GlobalConfig::tree()->get<float>("Overlay.VertexCount");
	static float cornerAngle = GeomConstants::DegToRad*GlobalConfig::tree()->get<float>("Overlay.CornerAngle");
	static float angleOffset = GeomConstants::DegToRad*GlobalConfig::tree()->get<float>("Overlay.AngleOffset");

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