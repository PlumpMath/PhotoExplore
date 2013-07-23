#include "Panel.h"
#include "FBNode.h"

Panel::Panel()
{	
	width = height = 0;
	initDefaults();
}

Panel::Panel(float width, float height)
{
	this->width = width;
	this->height = height;

	initDefaults();
}

void Panel::initDefaults()
{	
 	offset = position = Vector(0,0,0);
	this->backgroundColor = Colors::White;
	
	this->borderColor = Colors::Transparent;
	this->borderThickness = 0;

	glTextureId = NULL;

	textureHeight = width;
	textureWidth = height;

	dataPriority = 1;

	textPanel = NULL;

	textureScale = Vector(1,1,1);
	textureScaleMode = ScaleMode::FillUniform;
	currentDetailLevel = LevelOfDetail_Types::Preview;

	NudgeAnimationEnabled = true;
	
	fullScreenMode = false;

	loadAnimTimer.start();

	currentResource = NULL;
}
//
//void Panel::drawLoadTimer(Vector drawPosition, float drawWidth, float drawHeight)
//{
//	glBindTexture( GL_TEXTURE_2D, NULL);		
//	//
//	//float beatsPerMinute = 30;
//	//float secondsPerBeat = 60.0f/beatsPerMinute;
//	//float maxAlpha = .5f;
//
//	//float x = fmodf(loadAnimTimer.seconds(),secondsPerBeat);
//	//x /= secondsPerBeat;
//
//	//float bgAlpha = 0;
//
//	//float peak = .6f;
//
//	//if (x < peak)
//	//	bgAlpha = sqrtf(x)/(sqrtf(peak));
//	//else 
//	//	bgAlpha = 1.0f - pow(x-peak,2)/(pow(1.0f-peak,2));
//
//
//	////bgAlpha = max<float>(0,bgAlpha);
//	//
//	//Color loadColor = Colors::White;
//	//loadColor.setAlpha(maxAlpha);
//
//	//drawHeight *= bgAlpha;
//	//drawWidth = 4.0f;
//
//	//float x1 = drawPosition.x - drawWidth/2.0f;
//	//float x2 = x1 + drawWidth;
//	//float y1 = drawPosition.y - drawHeight/2.0f;
//	//float y2 = y1 + drawHeight;
//	//float z1 = drawPosition.z;
//	//		
//	//glColor4fv(loadColor.getFloat());
//
//	//glBindTexture( GL_TEXTURE_2D, NULL);
//
//	//glBegin( GL_QUADS );
//	//	glVertex3f(x1,y1,z1);
//	//	glVertex3f(x2,y1,z1);
//	//	glVertex3f(x2,y2,z1);
//	//	glVertex3f(x1,y2,z1);
//	//glEnd();
//
//
//	Color loadColor = backgroundColor; 
//	loadColor.setAlpha(.5f); //::B;
//	glColor4fv(loadColor.getFloat());
//	//float lineWidth = 1.0f;
//	for (int i = 0;i<5;i++)
//	{		
//		float angle = loadAnimTimer.seconds()/4.0 + ((float)(i*i)/10);
//
//		float x1,x2,x3,x4;
//		float angle2 = fmod(angle,2);
//
//
//		x2 = x1 = (angle2*.5f) * drawWidth;
//
//		float lineWidth =  10.0f * (.5f + sin(angle*GeomConstants::PI_F));
//
//		x4 = x3 = min<float>(x1 + lineWidth,drawWidth);
//
//		float y1,y2,y3,y4;
//
//		y4 = y1 = 0;
//		y3 = y2 = drawHeight;
//
//		float z1 = 0;
//
//		glTranslatef(drawPosition.x,drawPosition.y,drawPosition.z);
//
//		glBegin( GL_QUADS );
//
//		glVertex3f(x1,y1,z1);
//		glVertex3f(x2,y2,z1);
//		glVertex3f(x3,y3,z1);
//		glVertex3f(x4,y4,z1);
//
//		glEnd();
//
//		glTranslatef(-drawPosition.x,-drawPosition.y,-drawPosition.z);
//	}
//
//	//Color loadColor = Colors::DarkBlue;
//	//glColor4fv(loadColor.getFloat());
//	//float lineWidth = 1.0f;
//	//for (int i = 0;i<5;i++)
//	//{		
//	//	float angle = loadAnimTimer.seconds()/2.0 + ((float)(i*i)/10);
//
//	//	float x1,x2,x3,x4;
//	//	float angle2 = fmod(angle,2);
//	//	x2 = x1 = (angle2*.5f) * drawWidth;
//	//	x3 = x4 = x1 + lineWidth;
//
//	//	float y1,y2,y3,y4;
//
//	//	y4 = y1 = -(drawHeight*.4f) * sin(angle*GeomConstants::PI_F*.5) + (drawHeight*.5f);
//	//	y3 = y2 = drawHeight;
//
//	//	float z1 = 0;
//
//	//	glTranslatef(drawPosition.x,drawPosition.y,drawPosition.z);
//
//	//	glBegin( GL_QUADS );
//
//	//	glVertex3f(x1,y1,z1);
//	//	glVertex3f(x2,y2,z1);
//	//	glVertex3f(x3,y3,z1);
//	//	glVertex3f(x4,y4,z1);
//
//	//	glEnd();
//
//	//	glTranslatef(-drawPosition.x,-drawPosition.y,-drawPosition.z);
//	//}
//}
//

void Panel::drawContent(Vector drawPosition, float drawWidth, float drawHeight)
{		
	if (glTextureId != NULL)
	{
		TexturePanel::drawTexture(glTextureId,drawPosition,drawWidth,drawHeight);
	}		
	else
	{	
		
		if (GlobalConfig::tree()->get<bool>("Panel.DebugLoading"))
		{
			if (currentResource == NULL)
			{
				loadAnimationColor = Colors::Green;
				return;
			}
			
			switch (currentResource->ImageState)
			{		
			case ResourceState::Empty:
				loadAnimationColor = Colors::Purple;
				break;
			case ResourceState::ImageLoading:
				loadAnimationColor = Colors::Blue;
				break;
			case ResourceState::ImageLoaded:
				loadAnimationColor = Colors::LimeGreen;
				break;
			case ResourceState::ImageLoadError:
				loadAnimationColor = Colors::Red;
				break;
			}

			drawLoadTimer(drawPosition,drawWidth,drawHeight/2.0f);

			switch (currentResource->TextureState)
			{		
			case ResourceState::Empty:
				loadAnimationColor = Colors::Magenta;
				break;
			case ResourceState::TextureLoading:
				loadAnimationColor = Colors::HoloBlueBright;
				break;
			case ResourceState::TextureLoaded:
				loadAnimationColor = Colors::DarkGreen;
				break;
			case ResourceState::TextureLoadError:
				loadAnimationColor = Colors::OrangeRed;
				break;
			}
			

			drawLoadTimer(drawPosition + Vector(0,drawHeight/2.0f,0),drawWidth,drawHeight/2.0f);
		}
		else
		{
			loadAnimationColor = Colors::DimGray.withAlpha(.3f);
			drawLoadTimer(drawPosition,drawWidth,drawHeight);
		}
	}

	//if (textPanel != NULL)
	//	textPanel->drawPanel(drawPosition,drawWidth, drawHeight);
}

void Panel::drawPanel(Vector drawPosition, float drawWidth, float drawHeight)
{
	if (glTextureId == NULL)
		drawBackground(drawPosition,drawWidth,drawHeight);

	drawContent(drawPosition,drawWidth,drawHeight);	
	
	Color t = getBackgroundColor();
	setBackgroundColor(Colors::HoloBlueBright); //.withAlpha(max<float>(0,2.0f*(.5f - dataPriority))));

	if (GlobalConfig::tree()->get<bool>("Panel.DebugPriority"))
		drawBackground(drawPosition,drawWidth*.1f,drawHeight*(min<float>(1.0f,dataPriority*.1f)));

	setBackgroundColor(t);
}

void Panel::setFullscreenMode(bool _fullScreenMode)
{		
	this->fullScreenMode = _fullScreenMode;
	if (fullScreenMode)
	{	
		setDetailLevel(LevelOfDetail_Types::Full);
	}
	else
	{
		setDetailLevel(LevelOfDetail_Types::Preview);
	}
}

bool Panel::isFullscreenMode()
{
	return fullScreenMode;
}

void Panel::fitPanelToBoundary(Vector targetPosition, float maxWidth, float maxHeight, bool fill)
{
	float targetWidth = maxWidth, targetHeight =maxHeight;


	TexturePanel::setTextureWindow(Vector(),Vector(1,1,1));

	if (!fill)// && (maxWidth < textureWidth || maxHeight < textureHeight))
	{		
		//setDetailLevel(LevelOfDetail_Types::Full);

		float xScale = maxWidth/textureWidth;
		float yScale = maxHeight/textureHeight;

		if (xScale < 1.0f || yScale < 1.0f)
		{
			if (xScale > yScale)
			{
				targetWidth = yScale * textureWidth;
				targetHeight = yScale * textureHeight;
			}
			else 
			{
				targetWidth = xScale * textureWidth;
				targetHeight = xScale * textureHeight;
			}
		}
		else
		{
			targetWidth = textureWidth;
			targetHeight = textureHeight;
		}
	}
	else
	{		
		//setDetailLevel(LevelOfDetail_Types::Preview);
	}
			
	int expandAnimTime = 200, shrinkAnimTime = 200;

	animateToPosition(targetPosition-Vector(targetWidth/2.0f,targetHeight/2.0f,0),expandAnimTime, expandAnimTime);
	animatePanelSize(targetWidth,targetHeight,expandAnimTime);
}

GLuint Panel::getTextureId()
{
	return glTextureId;
}

void Panel::setDetailLevel(int levelOfDetail)
{
	if (levelOfDetail != currentDetailLevel)
	{
		currentDetailLevel = levelOfDetail;

		string imageURI = "";
		cv::Size2i imageSize;

		Facebook::FBNode * fbNode = (Facebook::FBNode*)node;

		if (GlobalConfig::tree()->get<bool>("FakeDataMode.Enable"))
		{
			if (levelOfDetail == LevelOfDetail_Types::Full)
				imageURI = fbNode->Edges.find("fake_uri_high")->Value;

			if (imageURI.size() == 0)
				imageURI = fbNode->Edges.find("fake_uri")->Value;
		}
		else
		{
			string suffix = "_o.jpg";
			if (levelOfDetail == LevelOfDetail_Types::Preview)
				suffix = "_n.jpg";

			auto res = fbNode->Edges.find("images");
			if (res != fbNode->Edges.end())
			{
				vector<json_spirit::Value> objectArray = res->JsonValue.get_array();				

				auto it = std::find_if(objectArray.begin(),objectArray.end(),[suffix](json_spirit::Value item) -> bool {					
					return json_spirit::find_value(item.get_obj(),"source").get_str().find(suffix) != string::npos;
				});

				if (it != objectArray.end())
				{
					imageSize.width = json_spirit::find_value(it->get_obj(),"width").get_int();		
					imageSize.height = json_spirit::find_value(it->get_obj(),"height").get_int();	

					imageURI =  json_spirit::find_value(it->get_obj(),"source").get_str();

					textureWidth = imageSize.width;
					textureHeight = imageSize.height;
				}
			}


			if (imageURI.size() == 0)
			{
				Facebook::FBNode * fbNode = (Facebook::FBNode*)node;
				std::stringstream urlStream;
				urlStream << "https://graph.facebook.com/";
				urlStream << fbNode->getId() << "/picture?width=600&height=600&";
				urlStream << "method=get&redirect=true&access_token=" << GlobalConfig::TestingToken;

				imageURI = urlStream.str();			
				//textureWidth = 600;
				//textureHeight = 600;
			}
		}
		if (currentResource != NULL && currentResource->imageURI.compare(imageURI) != 0 && currentDetailLevel == LevelOfDetail_Types::Full)
			currentResource->priority = 100;

		currentResource = ResourceManager::getInstance().loadResource(node->getURI(),imageURI,dataPriority,this);
	}
}

void Panel::setDataPriority(float dRel)
{	
	if (dataPriority != dRel)
	{
		dataPriority = dRel;
		node->setDataPriority(dRel);
		if (currentResource != NULL)
		{
			currentResource->priority = dataPriority;
			ResourceManager::getInstance().updateResource(currentResource);
		}
		else
		{
			int tmp = currentDetailLevel;
			currentDetailLevel = -2;
			setDetailLevel(tmp);
		}
	}	
}

float Panel::getDataPriority()
{
	return dataPriority;
}

void Panel::setNode(NodeBase * node)
{
	this->node = node;
}

NodeBase * Panel::getNode()
{
	return this->node;
}

void Panel::setVisible(bool visible)
{	
	this->visible = visible;
}

void Panel::resourceUpdated(ResourceData * data)
{
	if (data == NULL)
	{
		glTextureId = NULL;
	}
	else
	{
		glTextureId = data->textureId;
	}
}
