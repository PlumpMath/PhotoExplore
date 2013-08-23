#include "FakeDataSource.hpp"
#include "tinydir.h"
#include <boost/date_time/posix_time/posix_time.hpp>

FakeDataSource::FakeDataSource()
{
	srand(boost::posix_time::microsec_clock::local_time().time_of_day().total_microseconds());


	photoIndex = albumIndex = friendIndex = 0;
//	boost::filesystem::path fakePath = boost::filesystem::path(
	auto fakeDataPaths = GlobalConfig::tree()->get_child("FakeDataMode.ImageDirectories");
	
	for (auto it=fakeDataPaths.begin(); it != fakeDataPaths.end(); it++)
	{
		string fakeDataPath = it->second.data();
		tinydir_dir dir;
		tinydir_open(&dir, fakeDataPath.c_str());
		Logger::stream("FAKEDATA","INFO") << "Loading images from " << fakeDataPath << endl;
		
		while (dir.has_next)
		{
			tinydir_file file;
			tinydir_readfile(&dir, &file);

//			Logger::stream("FAKEDATA","INFO") << file.name << endl;
			dirContents.push_back(boost::filesystem::path(file.path));
			
			tinydir_next(&dir);
		}
		
		tinydir_close(&dir);
	}
	
	
	new boost::thread([this](){
		runThread(&taskMutex,&taskQueue);
	});
}

void FakeDataSource::runThread(boost::mutex * taskMutex, std::queue<boost::function<void()> > * taskQueue)
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

void FakeDataSource::loadWithOffset(FBNode * parent, string edge, int limit, int offset)
{
	int delay = GlobalConfig::tree()->get<int>("FakeDataMode.RequestDelay");
	int delayVariance = GlobalConfig::tree()->get<int>("FakeDataMode.DelayVariance");

	delay += (rand() & 2*delayVariance) - delayVariance;

	boost::this_thread::sleep(boost::posix_time::milliseconds(delay+delayVariance));

	if (edge.compare("photos") == 0)
	{
		for (int i=0;i<limit;i++)
		{

			while ((is_directory(this->dirContents.at(photoIndex%dirContents.size())) ||  
				this->dirContents.at(photoIndex%dirContents.size()).extension().string().size() < 3 || (
				this->dirContents.at(photoIndex%dirContents.size()).extension().string().find("jpg") == string::npos && 
				this->dirContents.at(photoIndex%dirContents.size()).extension().string().find("png") == string::npos)
				)) photoIndex++;


			stringstream ss2;
			ss2 << (10000 + photoIndex);

			FBNode * n2 = new FBNode(ss2.str());
			n2->setNodeType("photos");

			n2->Edges.insert(Edge("fake_uri",this->dirContents.at(photoIndex%dirContents.size()).string()));
			n2->Edges.insert(Edge("fake_uri_high",""));

			n2->Edges.insert(Edge("name","Fake photo name " + ss2.str()));
			
			n2->ReverseEdges.insert(Edge(parent->getNodeType(),parent,-1));

			parent->Edges.insert(Edge("photos",n2,photoIndex));

			if (photoIndex >= GlobalConfig::tree()->get<int>("FakeDataMode.MaxPhotos"))
				parent->loadState["photos"].hasReachedEnd = true;
				
			photoIndex++;
		}
	} else if (edge.compare("albums") == 0)
	{
		if (albumIndex < GlobalConfig::tree()->get<int>("FakeDataMode.MaxAlbums"))
		{
			for (int i=0;i<limit;i++)
			{
				stringstream ss2;
				ss2 << (30000 + this->albumIndex);

				FBNode * n2 = new FBNode(ss2.str());
				n2->setNodeType("albums");

				stringstream name;
				name << "Good album with a very long name"  << albumIndex;
				n2->Edges.insert(Edge("name",name.str()));
						
				n2->ReverseEdges.insert(Edge(parent->getNodeType(),parent,-1));
				parent->Edges.insert(Edge("albums",n2,albumIndex));
				albumIndex++;
			}
		}
	}else if (edge.compare("friends") == 0)
	{		
		if (friendIndex < GlobalConfig::tree()->get<int>("FakeDataMode.MaxFriends"))
		{
			for (int i=0;i<limit;i++)
			{
				while ((is_directory(this->dirContents.at(friendIndex%dirContents.size())) ||  
					this->dirContents.at(friendIndex%dirContents.size()).extension().string().size() < 3 || (
					this->dirContents.at(friendIndex%dirContents.size()).extension().string().find("jpg") == string::npos && 
					this->dirContents.at(friendIndex%dirContents.size()).extension().string().find("png") == string::npos)
					)) friendIndex++;

				stringstream ss2;
				ss2 << (40000 + this->friendIndex);

				FBNode * n2 = new FBNode(ss2.str());
				n2->setNodeType("friends");

				string billy [] = {"King","Idell","Yun","Felix","Micki","Celine","Blanch","Maira","Fidel",
					"Carolee","Angle","Bell","Maybelle","Bertram","Jolie","Miguel","Weston","Milo","Hedy","Raphael","Catina","Maryrose","Kathryne","Dallas","Lavenia","Lael","Suellen","Jermaine","Marguerita","Pamala"};

				stringstream name;				
				name << billy[rand()%20] << " " << billy[rand()%20];

				n2->Edges.insert(Edge("name",name.str()));
				n2->Edges.insert(Edge("fake_uri",this->dirContents.at(friendIndex%dirContents.size()).string()));

				n2->ReverseEdges.insert(Edge(parent->getNodeType(),parent,0));
				parent->Edges.insert(Edge("friends",n2,friendIndex));

				friendIndex++;
			}
		}
	}
}

void FakeDataSource::extractLimitOffset(string nodeQuery, string edge, int & limit, int & offset)
{	
	int photoPos = nodeQuery.find(edge);
	int limitPos = nodeQuery.find("limit(",photoPos);
	if (limitPos == string::npos)
	{
		throw new std::runtime_error("Needs limit");
	}
	int limitEnd = nodeQuery.find(")",limitPos);
	limit = (int)atoi(nodeQuery.substr(limitPos+6,(limitPos+6)- limitEnd).c_str());

	int offsetPos = nodeQuery.find("offset(",photoPos);
	if (offsetPos == string::npos)
	{
		offset = 0;
	}
	else
	{
		int offsetEnd = nodeQuery.find(")",offsetPos);
		offset = (int)atoi(nodeQuery.substr(offsetPos+7,(offsetPos+7)- offsetEnd).c_str());
	}
}

void FakeDataSource::loadField(FBNode * parent, string nodeQuery, string interpretAs, boost::function<void(FBNode*)> callback)
{

	int photoLimit =0 , photoOffset =0, albumLimit = 0, albumOffset = 0, friendLimit = 0, friendOffset = 0;

	if (nodeQuery.find("photos") != string::npos)
	{
		extractLimitOffset(nodeQuery,"photos",photoLimit,photoOffset);
	}

	if (nodeQuery.find("albums") != string::npos)
	{
		extractLimitOffset(nodeQuery,"albums",albumLimit,albumOffset);
	
	}

	if (nodeQuery.find("friends") != string::npos)
	{
		extractLimitOffset(nodeQuery,"friends",friendLimit,friendOffset);
	}



	Logger::stream("FakeData","INFO") << "Loading. Albums=" << albumLimit << " +" << albumOffset << " , Photos=" << photoLimit << " +" << photoOffset << " , Friends = " << friendLimit << " +" << friendOffset << endl;

	taskMutex.lock();
	taskQueue.push([this,photoLimit, photoOffset, albumLimit, albumOffset,friendLimit,friendOffset,parent,callback](){

		if (albumLimit != 0)
			loadWithOffset(parent,"albums",albumLimit,albumOffset);

		if (photoLimit != 0)
			loadWithOffset(parent,"photos",photoLimit,photoOffset);

		if (friendLimit != 0)
		{
			loadWithOffset(parent,"friends",friendLimit, friendOffset);

			//auto fRange = parent->Edges.get<EdgeTypeIndex>().equal_range("friends");
			//for (; fRange.first != fRange.second; fRange.first++)
			//{
			//	if (fRange.first->Node->Edges.get<EdgeTypeIndex>().count("photos") < 3)
			//		loadWithOffset(fRange.first->Node,"photos",3, 0);
			//}
		}		
		callback(parent);
	});
	taskMutex.unlock();


}




void FakeDataSource::postRequest(string request){		
	cout << "Not posting obviously fake request: " << request << endl;
}
void FakeDataSource::load(FBNode * parent, string objectId, string edge){
	cout << "FAKE LOAD NOT IMPLANTED\n";
}
void FakeDataSource::loadQuery(FBNode * parent, string nodeQuery, string interpretAs, boost::function<void(FBNode*)> callback){
	
	Logger::stream("FakeData","INFO") << "LoadQuery=" << nodeQuery << endl;

	if (nodeQuery.find("friend") != string::npos)
	{
		int friendLimit = 50, friendOffset = 0;

		taskMutex.lock();
		taskQueue.push([this,friendLimit,friendOffset,parent,callback](){
		
			if (friendLimit != 0)
			{
				loadWithOffset(parent,"friends",friendLimit, friendOffset);
			}		
			callback(parent);
		});
		taskMutex.unlock();
	}
}