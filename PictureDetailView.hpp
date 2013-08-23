#ifndef LEAPIMAGE_FACEBOOK_PICTUREDETAIL_HPP_
#define LEAPIMAGE_FACEBOOK_PICTUREDETAIL_HPP_

#include "ImageDetailView.hpp"
#include "PicturePanel.hpp"
#include "ViewOrchestrator.hpp"

namespace Facebook {


	class PictureDetailView : public ImageDetailView, public ViewOwner {

	private:	
		Button * likeButton, * alreadyLikedButton;
		TextPanel * photoComment;
		
		void initLikeButton(Facebook::FBNode * node);
		void setImageMetaData();

	protected:

		void setMainPanel(DynamicImagePanel * mainPanel);
		DynamicImagePanel * getDetailedDataView(DataNode * node);

	public:
		PictureDetailView();


		void layout(Vector position, cv::Size2f size);

	};

}

#endif