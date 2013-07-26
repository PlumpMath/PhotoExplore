#ifndef LEAPIMAGE_NEWPANEL_FBDATAVIEW_HPP_
#define LEAPIMAGE_NEWPANEL_FBDATAVIEW_HPP_

#include "FBNode.h"

class FBDataView {
	
public:
	virtual void show(Facebook::FBNode * node) = 0;
	virtual Facebook::FBNode * getNode() = 0;	
	virtual void setDataPriority(float dataPriority) = 0;

};

#endif