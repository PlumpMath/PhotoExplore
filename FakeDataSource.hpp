#ifndef LEAPIMAGE_FAKE_DATA_SOURCE_HPP_
#define LEAPIMAGE_FAKE_DATA_SOURCE_HPP_

#include <boost/filesystem.hpp>
#include "FBNode.h"
#include "Types.h"
#include <boost/thread.hpp>

using namespace Facebook;
using namespace boost::filesystem;

class FakeDataSource : public FBDataSource {

public:
	vector<path> dirContents;
	int photoIndex;
	bool isRunning;

	FakeDataSource(string fakeDataPath)
	{
		isRunning=false;
		photoIndex = 0;
		boost::filesystem::path testPath_mine = boost::filesystem::path(fakeDataPath);
		copy(directory_iterator(testPath_mine), directory_iterator(), back_inserter(dirContents));
	}

	void loadField(FBNode * parent, string nodeQuery, string interpretAs)
	{

		int photoPos = nodeQuery.find("photos");
		if (photoPos != string::npos)
		{
			int limitPos = nodeQuery.find("limit(",photoPos);
			if (limitPos == string::npos)
			{
				cout << "no limit! = " + nodeQuery << endl;
				return;
			}
			int limitEnd = nodeQuery.find(")",limitPos);
			int limit = (int)atoi(nodeQuery.substr(limitPos+6,(limitPos+6)- limitEnd).c_str());
			
			cout << "limit is: " << limit << " from : " << nodeQuery << endl;

			new boost::thread([parent,this, limit](){
				
				while (this->isRunning) 
					boost::this_thread::sleep(boost::posix_time::milliseconds(10));

				this->isRunning = true;
				
				boost::this_thread::sleep(boost::posix_time::milliseconds(300));
				for (int i=0;i<limit;i++)
				{
					
					while ((is_directory(this->dirContents.at(photoIndex%dirContents.size())) ||  
						this->dirContents.at(photoIndex%dirContents.size()).extension().string().size() < 3 || (
						this->dirContents.at(photoIndex%dirContents.size()).extension().string().find("jpg") == string::npos && 
						this->dirContents.at(photoIndex%dirContents.size()).extension().string().find("png") == string::npos)
						)) photoIndex++;
					
					
					stringstream ss2;
					ss2 << "fake_IMG_" << this->photoIndex;

					FBNode * n2 = new FBNode(ss2.str());
					n2->setNodeType("photos");

					ImageManager::getInstance()->setImageRelevance(ss2.str(),16,1,10,this->dirContents.at(photoIndex%dirContents.size()).string(), cv::Size2i(500,500));
					this->photoIndex++;

					parent->Edges.insert(Edge("photos",n2));
					parent->childrenChanged();
				}
				this->isRunning = false;
			});
		}
	}
	



	void postRequest(string request){		
		cout << "Not posting obviously fake request: " << request << endl;
	}
	void load(FBNode * parent, string objectId, string edge){
		cout << "FAKE LOAD NOT IMPLANTED\n";
	}
	void loadQuery(FBNode * parent, string nodeQuery, string interpretAs){
		cout << "FAKE QUERY NOT IMPLANTED\n";
	}


};

#endif
