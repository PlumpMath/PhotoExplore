#ifndef LEAPIMAGE_FAKE_DATA_SOURCE_HPP_
#define LEAPIMAGE_FAKE_DATA_SOURCE_HPP_

#include <boost/filesystem.hpp>
#include "FBNode.h"
#include "Types.h"
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>


using namespace Facebook;
using namespace boost::filesystem;

class FakeDataSource : public FBDataSource {

public:
	//int photoIndex;
	bool isRunning;
	boost::mutex taskMutex;
	std::queue<boost::function<void()> > taskQueue;

	FakeDataSource(string fakeDataPath)
	{
		isRunning=false;
		
		new boost::thread([this](){
			runThread(&taskMutex,&taskQueue);
		});
	}

	static void runThread(boost::mutex * taskMutex, std::queue<boost::function<void()> > * taskQueue)
	{
		while (true)
		{
			taskMutex->lock();
			if (!taskQueue->empty())
			{
				auto x = taskQueue->front();
				taskQueue->pop();
				taskMutex->unlock();
				x();
			}
			else
			{
				taskMutex->unlock();
				boost::this_thread::sleep(boost::posix_time::milliseconds(10));
			}
		}
	}

	void loadWithOffset(FBNode * parent, string edge, int limit, int offset, boost::function<void(FBNode*)> callback)
	{


		//taskMutex.lock();
		//taskQueue.push([this,limit,parent,callback](){

		//	boost::this_thread::sleep(boost::posix_time::milliseconds(GlobalConfig::tree()->get<int>("FakeDataMode.RequestDelay")));

		//	string fakeSource = parent->getAttribute("fake_data_dir");			
		//	if (fakeSource.length ==  0 && parent->getId().compare("me") == 0)
		//		parent->Edges.insert(Edge("fake_data_dir",GlobalConfig::tree()->get<string>("FakeDataMode.SourceDataDirectory")));

		//	
		//	vector<path> dirContents;
		//	boost::filesystem::path fakePath = boost::filesystem::path(fakeSource);
		//	copy(directory_iterator(fakePath), directory_iterator(), back_inserter(dirContents));

		//	if (edge.compare("photos") == 0)
		//	{
		//		int photoIndex = offset;
		//		for (int i=0;i<limit;i++)
		//		{

		//			while ((is_directory(this->dirContents.at(photoIndex%dirContents.size())) ||  
		//				this->dirContents.at(photoIndex%dirContents.size()).extension().string().size() < 3 || (
		//				this->dirContents.at(photoIndex%dirContents.size()).extension().string().find("jpg") == string::npos && 
		//				this->dirContents.at(photoIndex%dirContents.size()).extension().string().find("png") == string::npos)
		//				)) photoIndex++;


		//			stringstream ss2;
		//			ss2 << "fake_IMG_" << photoIndex;

		//			FBNode * n2 = new FBNode(ss2.str());
		//			n2->setNodeType("photos");

		//			n2->Edges.insert(Edge("fake_uri",this->dirContents.at(photoIndex%dirContents.size()).string()));
		//			n2->Edges.insert(Edge("fake_uri_high",""));

		//			photoIndex++;

		//			parent->Edges.insert(Edge("photos",n2));

		//			if (photoIndex >= GlobalConfig::tree()->get<int>("FakeDataMode.MaxPhotos"))
		//				parent->loadState["photos"].hasReachedEnd = true;

		//			callback(parent);
		//		}
		//	} else if (edge.compare("albums") == 0)
		//	{
		//		int albumIndex = 0;
		//		for (int i=0;i<limit;i++)
		//		{

		//			while (!is_directory(this->dirContents.at(photoIndex%dirContents.size())))
		//				albumIndex++;


		//			stringstream ss2;
		//			ss2 << "Fake_Album_" << this->photoIndex;

		//			FBNode * n2 = new FBNode(ss2.str());
		//			n2->setNodeType("albums");

		//			n2->Edges.insert(Edge("fake_uri",this->dirContents.at(photoIndex%dirContents.size()).string()));
		//			n2->Edges.insert(Edge("fake_uri_high",""));

		//			this->photoIndex++;

		//			parent->Edges.insert(Edge("photos",n2));
		//			//parent->childrenChanged();

		//			if (photoIndex >= GlobalConfig::tree()->get<int>("FakeDataMode.MaxPhotos"))
		//				parent->loadState["photos"].hasReachedEnd = true;

		//			callback(parent);
		//		}
		//	}
		//});
		//taskMutex.unlock();
	}

	void extractLimitOffset(string nodeQuery, string edge, int & offset, int & limit)
	{	
		int photoPos = nodeQuery.find(edge);
		int limitPos = nodeQuery.find("limit(",photoPos);
		if (limitPos == string::npos)
		{
			throw new std::exception("Needs limit");
		}
		int limitEnd = nodeQuery.find(")",limitPos);
		limit = (int)atoi(nodeQuery.substr(limitPos+6,(limitPos+6)- limitEnd).c_str());

		int offsetPos = nodeQuery.find("offset(",photoPos);
		if (offsetPos == string::npos)
		{
			throw new std::exception("Needs offset");
		}
		int offsetEnd = nodeQuery.find(")",offsetPos);
		offset = (int)atoi(nodeQuery.substr(offsetPos+7,(offsetPos+7)- offsetEnd).c_str());
	}

	void loadField(FBNode * parent, string nodeQuery, string interpretAs, boost::function<void(FBNode*)> callback)
	{
		if (nodeQuery.find("photos") != string::npos)
		{
			int limit, offset;
			extractLimitOffset(nodeQuery,"photos",limit,offset);
			loadWithOffset(parent,"photos",limit,offset,callback);
		}
				
		if (nodeQuery.find("albums") != string::npos)
		{
			int limit, offset;
			extractLimitOffset(nodeQuery,"albums",limit,offset);
			loadWithOffset(parent,"albums",limit,offset,callback);
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
