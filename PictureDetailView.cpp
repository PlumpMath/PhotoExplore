#include "PictureDetailView.hpp"
#include "FacebookLoader.h"

using namespace Facebook;

PictureDetailView::PictureDetailView() 
{
	
	
	likeButton = new Button("Like this photo?");
	likeButton->setVisible(false);
	likeButton->setAnimateOnLayout(false);
	likeButton->setStyle(GlobalConfig::tree()->get_child("PictureDetailView.LikeButton"));

	
	alreadyLikedButton = new Button("You like this.");
	alreadyLikedButton->setStyle(GlobalConfig::tree()->get_child("PictureDetailView.LikeButton"));
	alreadyLikedButton->setBackgroundColor(Colors::HoloBlueBright.withAlpha(.5f));
	alreadyLikedButton->setVisible(false);
	alreadyLikedButton->setAnimateOnLayout(false);

	photoComment = new TextPanel("");
	photoComment->setLayoutParams(LayoutParams(cv::Size2f(400,100),cv::Vec4f(20,20,20,80)));
	photoComment->setAnimateOnLayout(false);
	photoComment->setTextColor(GlobalConfig::tree()->get_child("PictureDetailView.Comment.TextColor"));
	photoComment->setBackgroundColor(Colors::Transparent);
	photoComment->setTextSize(GlobalConfig::tree()->get<float>("PictureDetailView.Comment.TextSize"));
	photoComment->setTextFitPadding(12);
	

	addChild(photoComment);
	addChild(likeButton);
	addChild(alreadyLikedButton);
}

void PictureDetailView::initLikeButton(FBNode * node)
{
	alreadyLikedButton->setVisible(false);
	likeButton->setVisible(true);
	PictureDetailView * me = this;
	FBNode * nodeToLike = imageNode;
	likeButton->elementClickedCallback = [nodeToLike ,me](LeapElement * clicked){

		me->alreadyLikedButton->setVisible(true);				
		me->likeButton->setVisible(false);
		//nodeToLike->Edges.insert(Facebook::Edge("user_likes","1"));
		auto likeIt = nodeToLike->Edges.get<EdgeTypeIndex>().find("user_likes");
		if (likeIt != nodeToLike->Edges.get<EdgeTypeIndex>().end())
			nodeToLike->Edges.replace(likeIt,Facebook::Edge("user_likes","1"));
		else
			nodeToLike->Edges.insert(Facebook::Edge("user_likes","1"));
		((FacebookLoader*)Facebook::FBDataSource::instance)->postRequest(nodeToLike->getId() + "/likes");
	};	
}


void PictureDetailView::setImageMetaData()
{
	likeButton->setVisible(false);
	alreadyLikedButton->setVisible(false);
	photoComment->setVisible(false);

	if (this->imagePanel != NULL)
	{		
		if (imageNode != NULL)
		{
			if (imageNode->getAttribute("user_likes").compare("1") == 0)
			{
				alreadyLikedButton->setVisible(true);
				likeButton->setVisible(false);
			}
			else 
			{				
				initLikeButton(imageNode);
			}

			//Validate state and re-init
			Facebook::FBDataSource::instance->loadQuery(imageNode,"fql?q=SELECT%20like_info%20FROM%20photo%20where%20object_id%3D" + imageNode->getId(),"",[this](FBNode * node){
												
				if (imageNode->getAttribute("user_likes").compare("1") == 0)
				{
					alreadyLikedButton->setVisible(true);
					likeButton->setVisible(false);
				}
				else 
				{				
					initLikeButton(node);
				}
				imageNode->clearLoadCompleteDelegate();
			});
			
			if (imageNode->getAttribute("name").size() != 0)
			{
				photoComment->setVisible(true);
				string comment = imageNode->getAttribute("name");

				//DEBUG
				//stringstream texIdStream;
				//texIdStream << "TexID = " << imagePanel->getTextureId();
				//comment = texIdStream.str();
				//END

				int maxLength = GlobalConfig::tree()->get<int>("PictureDetailView.MaxCommentLength");
				if (comment.length() > maxLength)
				{
					comment = comment.substr(0,maxLength) + "...";

				}

				photoComment->setText(comment);
				photoComment->reloadText();
				this->layoutDirty = true;
				//photoComment->layout(photoComment->getPosition(),cv::Size2f(400,100));
			}
			else
				photoComment->setVisible(false);
		}
		else
		{
			likeButton->setVisible(false);
		}
	}
}

void PictureDetailView::layout(Vector position, cv::Size2f size)
{
	if (imagePanel != NULL && size.width > 0 && size.height > 0)
	{
		lastSize = size;
		lastPosition = position;

		
		imagePanel->fitPanelToBoundary(-hostOffset + Vector(size.width*.5f,size.height*.5f,5),size.width,size.height*.8f, false);
	
		Vector pos;
		float w1,h1;
		imagePanel->getBoundingArea(pos,w1,h1);		
		photoComment->layout(-hostOffset + Vector(size.width*.3f,imagePanel->getPosition().y + imagePanel->getHeight(),10),cv::Size2f(size.width*.4f,size.height*.2f));	
		
		float buttonPadding = 50;
		cv::Size2f buttonSize = cv::Size2f(size.height*.2f,size.height*.15f);
		Vector buttonPos = Vector(imagePanel->getWidth() + imagePanel->getPosition().x + (buttonPadding),imagePanel->getPosition().y + imagePanel->getHeight()*.5f,10);
		
		likeButton->layout(buttonPos,buttonSize);
		alreadyLikedButton->layout(buttonPos,buttonSize);
		
		layoutDirty = false;
	}
}

void PictureDetailView::setPicturePanel(PicturePanel * picturePanel)
{
	imageNode = dynamic_cast<Facebook::FBNode*>(picturePanel->getNode());
	setImagePanel(picturePanel);
	setImageMetaData();
}

