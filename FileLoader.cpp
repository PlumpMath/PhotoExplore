#include "FileLoader.hpp"
#include "GlobalConfig.hpp"
#include "tinydir.h"
#include <regex>

using namespace FileSystem;
using namespace std;

FileLoader::FileLoader()
{
	
	for (int i=0;i<GlobalConfig::tree()->get<int>("FileLoader.NumThreads"); i++)
	{
		new boost::thread([this](){
			runThread(&taskMutex,&taskQueue);
		});
	}
}

void FileLoader::loadFiles(FileNode * directoryNode, int depth, boost::function<void(FileNode*)> callback)
{	
	string filter = GlobalConfig::tree()->get<string>("FileLoader.Filter",".*");

	regex reggie = regex(filter);

	auto task = [directoryNode, callback, reggie](){

		string directoryPath = directoryNode->filePath.string();
		tinydir_dir dir;
		tinydir_open(&dir, directoryPath.c_str());

		while (dir.has_next)
		{
			tinydir_file file;
			tinydir_readfile(&dir, &file);

			string filename = file.name;
			std::smatch match;
			std::regex_search(filename,match,reggie);
			if (file.is_dir || !match.empty())
			{
				FileNode * fileNode = new FileNode(std::string(file.path));
				directoryNode->Files.get<FileSystemSeq>().push_back(File(fileNode));			
			}
			tinydir_next(&dir);
		}
		tinydir_close(&dir);

		callback(directoryNode);
	};	
	taskMutex.lock();
	taskQueue.push(task);
	taskMutex.unlock();
}


void FileLoader::runThread(boost::mutex * taskMutex, std::queue<boost::function<void()> > * taskQueue)
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