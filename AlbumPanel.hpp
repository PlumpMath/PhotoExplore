#ifndef LEAPIMAGE_ALBUM_PANEL_HPP_
#define LEAPIMAGE_ALBUM_PANEL_HPP_

#include "DataViewGenerator.hpp"
#include "ContentPanel.hpp"
#include "UniformGrid.hpp"
#include "FBNode.h"
#include "ViewOrchestrator.hpp"
#include "PicturePanel.hpp"
#include "TextPanel.h"

class AlbumPanel : public ContentPanel, public ViewOwner {

private:
	ViewGroup * albumGroup;
	TextPanel * nameText;
	FBNode * albumNode;

	float currentDataPriority;

public:
	AlbumPanel();

	void viewChanged(string viewIdentifier, vector<FBNode*> & viewData);
	
	void setDataPriority(float dataPriority);
	void show(FBNode * node);
	FBNode * getNode();
	void viewOwnershipChanged(View * view, ViewOwner * newOwner);

};

#endif