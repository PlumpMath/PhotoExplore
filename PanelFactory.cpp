#include "PanelFactory.hpp"


PanelFactory::PanelFactory()
{
	;
}

PicturePanel * PanelFactory::buildImagePanel(NodeBase * imageNode)
{
	PicturePanel * p = new PicturePanel();
	return p;
}


TextPanel * PanelFactory::buildTextPanel(string text, Style_ style)
{
	TextPanel * tp = new TextPanel(text);
	if (style != 0)
		setStyle(tp,style);
	return tp;
}


void PanelFactory::setStyle(TextPanel * target, Style_ style)
{	
	if (target != NULL)
	{
		//Color c = Colors::HoloBlueBright;
		//c.setAlpha(.5f);
		switch (style)
		{
		case TextStyles::Title:
			target->setTextSize(20.0f);
			target->setTextColor(Colors::Black);		
			target->setBackgroundColor(Colors::White);	
			break;	
		case TextStyles::Heading1:
			target->setTextSize(12.0f);
			target->setTextColor(Colors::Black);		
			target->setBackgroundColor(Colors::White);	
			break;		
		case TextStyles::Heading2:
			target->setTextSize(8.0f);	
			target->setBackgroundColor(Colors::White);	
			target->setTextColor(Colors::Black);		
			target->setBorderThickness(0);
			target->setTextFitPadding(10);
			break;
		default:
			return;
		}
		target->refresh();
	}
}