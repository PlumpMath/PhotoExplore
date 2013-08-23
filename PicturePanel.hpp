#ifndef LEAPIMAGE_FACEBOOK_PICTURE_PANEL_HPP_
#define LEAPIMAGE_FACEBOOK_PICTURE_PANEL_HPP_

#include "DynamicImagePanel.hpp"
#include "FBNode.h"

namespace Facebook {

	class PicturePanel : public DynamicImagePanel  {
		
	protected:
		FBNode * pictureNode;	

		void prepareResource();
		void prepareFakeResource();

	public:	
		void show(FBNode * node);
		FBNode * getNode();

	};

}


#endif