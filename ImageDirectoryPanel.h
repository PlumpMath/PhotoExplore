#include "PanelUniformGrid.h"
#include "FileNode.h"

#ifndef IMAGE_DIR_PANEL_H_
#define IMAGE_DIR_PANEL_H_

class ImageDirectoryPanel : public PanelUniformGrid{

public:
	ImageDirectoryPanel(float width, float height) : PanelUniformGrid(width,height,3,3)
	{
		this->setBorderColor(Colors::DarkGray);
		this->setBorderThickness(1);
		this->BorderSelectionThickness = 0;
	}

protected:
	void layoutChild(int index, float drawWidth, float drawHeight, Vector & childPosition, float & childWidth, float & childHeight)
	{	
		if (panels.at(index)->panelId.compare("name") == 0)
		{
			childPosition = Vector(0,-drawHeight/2.0f,0);
			
			childWidth = drawWidth;
			childHeight = drawHeight * .1f;
			childPosition.y += childHeight/2.0f;
		}
		else
		{
			PanelUniformGrid::layoutChild(index,drawWidth,drawHeight*.9f,childPosition,childWidth,childHeight);
			childPosition.y += drawHeight *.05f;
		}
	}


};

#endif