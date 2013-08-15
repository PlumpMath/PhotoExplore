#ifndef LEAPIMAGE_FACEBOOK_PICTUREDETAIL_HPP_
#define LEAPIMAGE_FACEBOOK_PICTUREDETAIL_HPP_

#include "ImageDetailView.hpp"
#include "PicturePanel.hpp"

namespace Facebook {


	class PictureDetailView : public ImageDetailView {

	private:	
		PicturePanel * picturePanel;
		Button * likeButton, * alreadyLikedButton;
		TextPanel * photoComment;

		FBNode * imageNode;		

		void setImageMetaData();		
		void initLikeButton(Facebook::FBNode * node);

	public:
		PictureDetailView();

		void setPicturePanel(PicturePanel * imagePanel);
		PicturePanel * getPicturePanel();

		void layout(Vector position, cv::Size2f size);

	};

}

#endif