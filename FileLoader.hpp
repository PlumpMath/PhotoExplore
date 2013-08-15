#ifndef LEAPIMAGE_FILESYSTEM_FILELOADER_HPP_
#define LEAPIMAGE_FILESYSTEM_FILELOADER_HPP_

#include "FileNode.hpp"
#include <boost/thread.hpp>
#include <queue>
#include <boost/function.hpp>

namespace FileSystem 
{
	class FileLoader {

	private:
		FileLoader();
		FileLoader(FileLoader const&);
		void operator=(FileLoader const&); 

		boost::mutex taskMutex;
		std::queue<boost::function<void()> > taskQueue;

		void runThread(boost::mutex * taskMutex, std::queue<boost::function<void()> > * taskQueue);

	public:		
		static FileLoader& getInstance()
		{
			static FileLoader instance; 
			return instance;
		}

		
		void loadFiles(FileNode * directoryNode, int depth, boost::function<void(FileNode*)> callback);



	};
}


#endif