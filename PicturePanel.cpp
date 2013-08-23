#include "PicturePanel.hpp"
#include <boost/filesystem.hpp>

using namespace Facebook;



void PicturePanel::prepareFakeResource()
{
	string resourceId = "";
	bool undersized;
	ResourceData * newResource = selectResource(undersized);

	if (newResource != NULL && (currentResource == NULL || (newResource->TextureState == ResourceState::TextureLoaded && !undersized)))
	{
		if (currentResource != NULL)
			currentResource->priority = 100;

		currentResource = newResource;
		pictureSize = currentResource->imageSize;
	}
	else //if (newResource == NULL)
	{
		stringstream rs;
		rs <<  pictureNode->getAttribute("fake_uri");

		boost::function<void(cv::Mat&)> tran;
		int loadingWidth = 0;

		if (!maxResolutionMode)
		{
			loadingWidth = (int)getWidth();

			int targetDimension = max<int>((int)getWidth(),(int)getHeight());

			tran = [targetDimension,this](cv::Mat&imgMat){

				float td = (float)targetDimension;

				float originalWidth = imgMat.size().width;
				float originalHeight = imgMat.size().height;
				
				float scale = max<float>(td/originalWidth,td/originalHeight);

				scale = min<float>(1.0f,scale);

				int adjustedWidth = (int)ceil(scale * originalWidth);
				int adjustedHeight = (int)ceil(scale * originalHeight);

				cv::Size newSize = cv::Size(adjustedWidth,adjustedHeight);
				cv::Mat resized = cv::Mat(newSize, CV_8UC4);
				cv::resize(imgMat,resized,newSize,0,0,cv::INTER_AREA);
				imgMat.release();

				imgMat = resized;
			};

			rs << targetDimension;
		}
		else
		{
			rs << "_max";
			pictureSize.width = 1000;
			pictureSize.height = 1000;
			loadingWidth = pictureSize.width;
		}

		if (resourceMap.count(loadingWidth) == 0)
		{
			resourceId = rs.str();
			ResourceData * loadingResource = ResourceManager::getInstance().loadResourceWithTransform(resourceId,pictureNode->getAttribute("fake_uri"),dataPriority,this,tran);
			resourceMap.insert(make_pair(loadingWidth,loadingResource));
			Logger::stream("FileImagePanel","INFO") << "Loading transformed resource. Width = " << loadingWidth << ", ID = " << resourceId << endl;
		}
	}
}

void PicturePanel::prepareResource()
{	
	if (getWidth() <= 0 || getHeight() <= 0)
		return;

	string newResourceURI = "", resourceId = "";
	cv::Size2i newResourceSize(0,0);
	if (GlobalConfig::tree()->get<bool>("FakeDataMode.Enable"))
	{
		prepareFakeResource();
	}
	else
	{
		auto res = pictureNode->Edges.find("images");
		if (res != pictureNode->Edges.end())
		{
			vector<json_spirit::Value> objectArray = res->JsonValue.get_array();				

			
			map<int,pair<cv::Size2i,string> > areaImageMap;
			for (auto it = objectArray.begin(); it != objectArray.end(); it++)
			{
				cv::Size2i imageSize;
				imageSize.width = json_spirit::find_value(it->get_obj(),"width").get_int();		
				imageSize.height = json_spirit::find_value(it->get_obj(),"height").get_int();	
				string imageURI =  json_spirit::find_value(it->get_obj(),"source").get_str();

				areaImageMap.insert(make_pair(min<int>(imageSize.width,imageSize.height),make_pair(imageSize,imageURI)));
			}

			if (areaImageMap.empty())
			{

			}
			else if (maxResolutionMode)
			{
				newResourceURI = areaImageMap.rbegin()->second.second;
				newResourceSize = areaImageMap.rbegin()->second.first;
			}
			else
			{
				int targetDimension = max<int>((int)getWidth(),(int)getHeight());
				for (auto it = areaImageMap.begin(); it != areaImageMap.end(); it++)
				{
					if (!maxResolutionMode && areaImageMap.size() > 1 && it->second.second.find("_o.jpg") != string::npos)
						continue;

					newResourceURI = it->second.second;
					newResourceSize = it->second.first;
					if (it->first > targetDimension)
					{
						break;
					}
				}
			}
		}
		
		resourceId = newResourceURI;
		if (newResourceURI.size() == 0)
		{						
			if (pictureNode->getAttribute("picture").length() > 0)
			{
				resourceId = pictureNode->getAttribute("picture");
				newResourceURI = resourceId;
			}
			else
			{
				std::stringstream urlStream;
				urlStream << "https://graph.facebook.com/";
				urlStream << pictureNode->getId() << "/picture?width=600&height=600&";
				urlStream << "method=get&redirect=true&access_token=" << GlobalConfig::TestingToken;
				resourceId = pictureNode->getId();
				newResourceURI = urlStream.str();			
			}
		}

		if (currentResource == NULL || currentResource->imageURI.compare(newResourceURI) != 0)
		{
			Logger::stream("PicturePanel","INFO") << "Selected new resource: " << newResourceURI << " with size = [" << newResourceSize.width << "," << newResourceSize.height << "] , my size is [" << getWidth() << "," << getHeight() << "]" << endl;
			if (currentResource != NULL)
			{
				currentResource->priority = 100;
			}

			pictureSize = newResourceSize;
			if (newResourceSize.area() > 0)
			{
				textureWidth = newResourceSize.width;
				textureHeight = newResourceSize.height;
			}

			currentResource = ResourceManager::getInstance().loadResource(resourceId,newResourceURI,dataPriority,this);
		}
	}

}

void PicturePanel::show(FBNode * _pictureNode)
{
	this->pictureNode = _pictureNode;
	prepareResource();
}

FBNode * PicturePanel::getNode()
{
	return this->pictureNode;
}
