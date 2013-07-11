#ifndef LEAPIMAGE_PANEL_FACTORY_HPP_
#define LEAPIMAGE_PANEL_FACTORY_HPP_

#include "Panel.h"
#include "TextPanel.h"
#include "NodeBase.h"

typedef int Style_;

namespace TextStyles {

	const static Style_ Heading1 = 1;
	const static Style_ Heading2 = 2;
	const static Style_ Title = 3;
	const static Style_ Body = 4;
}


class PanelFactory
{
public:
	static PanelFactory& getInstance()
	{
		static PanelFactory instance; // Guaranteed to be destroyed.
		// Instantiated on first use.
		return instance;
	}
private:
	PanelFactory();
	PanelFactory(PanelFactory const&);
	void operator=(PanelFactory const&); 

public:
	Panel * buildImagePanel(NodeBase * imageNode);

	TextPanel * buildTextPanel(string text, Style_ style = 0);
	void setStyle(TextPanel * target, Style_ style);

};

#endif