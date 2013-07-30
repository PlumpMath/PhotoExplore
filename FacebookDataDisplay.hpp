#ifndef LEAPIMAGE_FACEBOOK_DATA_DISPLAY_HPP_
#define LEAPIMAGE_FACEBOOK_DATA_DISPLAY_HPP_

#include "FBNode.h"

class FacebookDataDisplay {


public:
	static FacebookDataDisplay * instance;

	virtual void displayNode(Facebook::FBNode * newNode, string action) = 0;

	static FacebookDataDisplay * getInstance()
	{
		return instance;
	}


};


#endif