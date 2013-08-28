#include "LeapDebug.h"
#include "FixedAspectGrid.hpp"
#include "ContentPanel.hpp"
#include "CustomGrid.hpp"
#include "ImagePanel.hpp"
#include "TextPanel.h"
#include "LinearLayout.hpp"


static float getTutorialHeight()
{	
	float relativeHeight = GlobalConfig::tree()->get<float>("Tutorial.RelativeHeight") * GlobalConfig::ScreenHeight;
	float minHeight = GlobalConfig::tree()->get<float>("Tutorial.MinimumHeight");	
	return max<float>(minHeight,relativeHeight);
}

LeapDebug::LeapDebug()
{
#ifdef _WIN32
	initDebugBox();
#endif

	debugPlotEnabled = GlobalConfig::tree()->get<bool>("DebugPlot.Enable");


	this->backgroundColor = Colors::Black;
	backgroundColor.setAlpha(.5);
	
	auto labels = GlobalConfig::tree()->get_child("Tutorial.Labels");

	Color backgroundColor = Color(GlobalConfig::tree()->get_child("Tutorial.Background"));
		
	Color textBackground = Color(GlobalConfig::tree()->get_child("Tutorial.TextBackground"));
	Color labelColor = Color(GlobalConfig::tree()->get_child("Tutorial.TextColor"));
	Color invertedLabelColor = Color(GlobalConfig::tree()->get_child("Tutorial.InvertedTextColor"));


	float tutorialHeight = getTutorialHeight();
	float tutorialTextSize = labels.get<float>("TextSize");
	float textPadding = labels.get<float>("TextPadding");
	string fontName = labels.get<string>("FontName");

	float imgHeight = GlobalConfig::tree()->get<float>("Tutorial.ImageHeight");
		
	float defaultIconWidth = tutorialHeight;
	auto tutorialIcons = GlobalConfig::tree()->get_child("Tutorial.Icons");	

	cv::Size2f tutorialSize = cv::Size2f(0,tutorialHeight);

	for (auto tutIt = tutorialIcons.begin(); tutIt != tutorialIcons.end(); tutIt++)
	{		
		float iconWidth = tutIt->second.get<float>("IconWidth", defaultIconWidth);
		ImagePanel  * tutorialImage = NULL;

/*		if (tutorialHeight == GlobalConfig::tree()->get<float>("Tutorial.MinimumHeight"))
		{
			tutorialImage = new ImagePanel(tutIt->second.get<string>("RightImage"));
			tutorialImage->setScaleMode(ScaleMode::None);
		}
		else
		{*/			
		tutorialImage = new ImagePanel(tutIt->second.get<string>("RightImage_High"),cv::Size2f(defaultIconWidth,imgHeight*tutorialHeight));
		tutorialImage->setScaleMode(ScaleMode::Fit);
		//}

		tutorialImage->setAllowSubPixelRendering(false);
		tutorialImage->setBackgroundColor(backgroundColor);

		TextPanel  * tutorialText = new TextPanel(tutIt->second.get<string>("Text"));
		tutorialText->setTextFitPadding(textPadding);
		tutorialText->setTextColor(tutIt->second.get<bool>("InvertColor") ? invertedLabelColor : labelColor);
		tutorialText->setBackgroundColor(textBackground);
		tutorialText->setTextSize(tutorialTextSize,false);
		tutorialText->setFontName(fontName);
		
		vector<RowDefinition> gridDefinition;	
		gridDefinition.push_back(RowDefinition(GlobalConfig::tree()->get<float>("Tutorial.PaddingHeight")));
		gridDefinition.push_back(RowDefinition(imgHeight));
		gridDefinition.push_back(RowDefinition(1.0f -imgHeight));
		gridDefinition[0].ColumnWidths.push_back(1);
		gridDefinition[1].ColumnWidths.push_back(1);
		gridDefinition[2].ColumnWidths.push_back(1);
		
		CustomGrid * tutorialIconLayout = new CustomGrid(gridDefinition);
		tutorialIconLayout->addChild(new TextPanel());
		tutorialIconLayout->addChild(tutorialImage);
		tutorialIconLayout->addChild(tutorialText);
		tutorialIconLayout->setLayoutParams(LayoutParams(cv::Size2f(iconWidth,tutorialSize.height),cv::Vec4f(0,0,0,0)));

		tutorialPanels.insert(make_pair(tutIt->second.get<string>("Name"),tutorialIconLayout));	
	}

	tutorialLayout = new LinearLayout();
	tutorialPanel = new ContentPanel(tutorialLayout);
	tutorialLayout->measure(tutorialSize);
	tutorialLayout->layout(Vector(0,GlobalConfig::ScreenHeight+tutorialHeight,10),tutorialSize);


	if (debugPlotEnabled)
		plotLegend = new LinearLayout();
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
#else

void LeapDebug::showValue(string key, string value)
{

}

void LeapDebug::showValue(string key, double value)
{

}
#endif

void LeapDebug::plotValue(string key, Color color, float value)
{
	if (debugPlotEnabled)
	{
		static int maxSize = GlobalConfig::tree()->get<int>("DebugPlot.Width");

		if (plots.count(key) == 0)
		{
			float yOffset = GlobalConfig::tree()->get<float>("DebugPlot.YOffset");
			float xOffset = GlobalConfig::tree()->get<float>("DebugPlot.XOffset");

			TextPanel * legendText = new TextPanel(key);
			legendText->setTextColor(color);
			((ViewGroup*)plotLegend)->addChild(legendText);
			legendText->layout(Vector(),cv::Size2f(40,80));
			cv::Size2f legendSize(0,40);
			plotLegend->measure(legendSize);
			plotLegend->layout(Vector(xOffset,yOffset,40),legendSize);
		}

		plots[key].values.push_back(value);
		plots[key].color = color;

		if (plots[key].values.size() > maxSize)
			plots[key].values.pop_front();
	}
}

void LeapDebug::onFrame(const Controller& controller)
{
	
	Frame frame = controller.frame();

	if (frame.id() == lastFrame.id())
		return;

	for (int i=0;i<persistentVisuals.size();i++)
	{
		persistentVisuals.at(i)->onFrame(controller);
	}

	lastFrame = frame;
	static map<int,PointableCursor*> ldv_intent;
}

void LeapDebug::addDebugVisual(OverlayVisual * ldv)
{
	persistentVisuals.push_back(ldv);
}

void LeapDebug::setTutorialImages(vector<string> names)
{	
	if (!GlobalConfig::tree()->get<bool>("Tutorial.Enabled"))
		names.clear();

	tutorialLayout->clearChildren();
	for (auto it = names.begin(); it != names.end(); it++)
	{ 
		auto gesture = tutorialPanels.find(*it);
		if (gesture != tutorialPanels.end())
			tutorialLayout->addChild(gesture->second);
	}
	layoutTutorial();
}

void LeapDebug::layoutTutorial()
{		
	for (auto it = tutorialPanels.begin(); it != tutorialPanels.end(); it++)
	{
		auto res = std::find(tutorialLayout->getChildren()->begin(), tutorialLayout->getChildren()->end(),it->second);
		
		if (res == tutorialLayout->getChildren()->end())
			it->second->layout(Vector(it->second->getLastPosition().x,GlobalConfig::ScreenHeight+getTutorialHeight(),10),it->second->getMeasuredSize());
	}
	
	cv::Size2f size = cv::Size2f(300,getTutorialHeight());
	tutorialPanel->measure(size);
	tutorialPanel->layout(Vector(0,GlobalConfig::ScreenHeight-size.height,10),size);	
	//tutorialLayout->layout(Vector(0,GlobalConfig::ScreenHeight-tutorialHeight,10),size);
}

void LeapDebug::update()
{
	for (int i=0;i<persistentVisuals.size();i++)
	{
		persistentVisuals.at(i)->update();
	}
}

void LeapDebug::draw()
{
	glPushMatrix();
	glLoadIdentity();

	for (int i = 0;i< persistentVisuals.size();i++)
	{
		if (persistentVisuals.at(i)->isVisible())
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
	if (debugPlotEnabled)
	{		
		static float yOffset = GlobalConfig::tree()->get<float>("DebugPlot.YOffset");
		static float xOffset = GlobalConfig::tree()->get<float>("DebugPlot.XOffset");
		static float defaultLineWidth = GlobalConfig::tree()->get<float>("DebugPlot.LineWidth");


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
		
		plotLegend->draw();
	}

	glPopMatrix();	
}
