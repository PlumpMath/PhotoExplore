#include "LeapDebug.h"
#include "FixedAspectGrid.hpp"
#include "ContentPanel.hpp"
#include "CustomGrid.hpp"
#include "ImagePanel.hpp"
#include "TextPanel.h"

LeapDebug::LeapDebug()
{
	initDebugBox();
	this->backgroundColor = Colors::Black;
	backgroundColor.setAlpha(.5);

	TextPanel * leapDisconnectedPanel = new TextPanel();
	leapDisconnectedPanel->setLayoutParams(LayoutParams(cv::Size2f(400,200)));
	leapDisconnectedPanel->setText(GlobalConfig::tree()->get<string>("Strings.LeapOverlay.DisconnectedMessage"));
	leapDisconnectedPanel->setTextColor(Colors::White);
	leapDisconnectedPanel->setTextSize(8);
	leapDisconnectedPanel->setBackgroundColor(Colors::DarkRed.withAlpha(.7f));
	leapDisconnectedPanel->setPosition(Vector(GlobalConfig::ScreenWidth - 450, GlobalConfig::ScreenHeight - 100, 1));
	leapDisconnectedPanel->layout(Vector(GlobalConfig::ScreenWidth - 500, GlobalConfig::ScreenHeight - 300, 10),cv::Size2f(400,150));

	this->leapDisconnectedPanel = leapDisconnectedPanel;

	TextPanel * leapNotFocusedPanel = new TextPanel();
	leapNotFocusedPanel->setLayoutParams(LayoutParams(cv::Size2f(400,200)));
	leapNotFocusedPanel->setText(GlobalConfig::tree()->get<string>("Strings.LeapOverlay.NotFocusedMessage"));
	leapNotFocusedPanel->setTextColor(Colors::White);
	leapNotFocusedPanel->setTextSize(8);
	leapNotFocusedPanel->setPosition(Vector(GlobalConfig::ScreenWidth - 450, GlobalConfig::ScreenHeight - 100, 1));
	leapNotFocusedPanel->layout(Vector(GlobalConfig::ScreenWidth - 500, GlobalConfig::ScreenHeight - 300, 10),cv::Size2f(400,150));
	leapNotFocusedPanel->setBackgroundColor(Colors::LeapGreen.withAlpha(.7f));
	leapNotFocusedPanel->setVisible(false);

	this->leapNotFocusedPanel = leapNotFocusedPanel;
	
	auto labels = GlobalConfig::tree()->get_child("Tutorial.Labels");

	Color backgroundColor = Color(GlobalConfig::tree()->get_child("Tutorial.Background"));
		
	Color textBackground = Color(GlobalConfig::tree()->get_child("Tutorial.TextBackground"));
	Color labelColor = Color(GlobalConfig::tree()->get_child("Tutorial.TextColor"));
	Color invertedLabelColor = Color(GlobalConfig::tree()->get_child("Tutorial.InvertedTextColor"));


	float tutorialTextSize = labels.get<float>("TextSize");
	float textPadding = labels.get<float>("TextPadding");
	string fontName = labels.get<string>("FontName");

	float imgHeight = GlobalConfig::tree()->get<float>("Tutorial.ImageHeight") / GlobalConfig::tree()->get<float>("Tutorial.Height");
	
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
		gridDefinition.push_back(RowDefinition(imgHeight));
		gridDefinition.push_back(RowDefinition(1.0f -imgHeight));
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


#if defined(_WIN32) 

void LeapDebug::initDebugBox()
{
	auto config = GlobalConfig::tree()->get_child("WindowsDebugPanel");

	if (config.get<bool>("Enable"))
	{

		vector<HWND> handles = WinHelper::getToplevelWindows();
		HWND hwnd = handles.at(0);

		if (config.get<bool>("DesktopParent"))
			hwnd = 0;

		debugTextBox = CreateWindowEx(
			0, "EDIT",   // predefined class 
			NULL,         // no window title 
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
			ES_LEFT | ES_MULTILINE | ES_WANTRETURN, 
			0, 0, config.get<int>("Width"), config.get<int>("Height"),   // set size in WM_SIZE message 
			hwnd,         // parent window 
			(HMENU) NULL,   // edit control ID 
			(HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), 
			NULL);        // pointer not needed 
	}
}

#endif

void LeapDebug::updateDebugBox()
{
	
	stringstream ss;

	for (auto it = debugValues.begin(); it != debugValues.end(); it++)
	{
		ss << it->first.c_str() << "\t= " << it->second.c_str() << "\r\n";
	}
	TCHAR * lolString = new TCHAR[ss.str().length()+1];
	
	int i=0;
	for (;i<ss.str().length();i++)
	{
		lolString[i] = ss.str().at(i);
	}
	lolString[i] = '\0';
	
	SendMessage(debugTextBox, WM_SETTEXT, 0, (LPARAM) lolString); 

	delete lolString;
}

void LeapDebug::showValue(string key, string value)
{
	debugValues[key] = value;
	updateDebugBox();
}


void LeapDebug::showValue(string key, double value)
{
	stringstream ss;
	ss << value;

	debugValues[key] = ss.str();
	updateDebugBox();
}

void LeapDebug::plotValue(string key, Color color, float value)
{
	static int maxSize = GlobalConfig::tree()->get<int>("DebugPlot.Width");

	plots[key].values.push_back(value);
	plots[key].color = color;

	if (plots[key].values.size() > maxSize)
		plots[key].values.pop_front();
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

	for (int i=0;i<persistentVisuals.size();i++)
	{
		persistentVisuals.at(i)->onFrame(controller);
	}

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
		

	for (int i = 0;i< persistentVisuals.size();i++)
	{
		persistentVisuals.at(i)->draw();
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

	bool debugPlotEnabled = true;
	static float yOffset = GlobalConfig::tree()->get<float>("DebugPlot.YOffset");
	static float xOffset = GlobalConfig::tree()->get<float>("DebugPlot.XOffset");
	static float defaultLineWidth = GlobalConfig::tree()->get<float>("DebugPlot.LineWidth");

	if (debugPlotEnabled)
	{
		glBindTexture(GL_TEXTURE_2D,NULL);
		float z = 40;
		for (auto it = plots.begin(); it != plots.end(); it++)
		{
			float x = xOffset;

			glColor4fv(it->second.color.withAlpha(.7f).getFloat());
			glLineWidth((it->second.lineWidth) > 0 ? it->second.lineWidth : defaultLineWidth);
			glBegin(GL_LINE_STRIP);
			
			z++;
			for (auto point = it->second.values.begin(); point != it->second.values.end(); point++)
			{
				float xD = x++;
				float yD = yOffset - *point;
				glVertex3f(xD,yD,z);
			}
			glEnd();
		}
	}

	glPopMatrix();	
}
