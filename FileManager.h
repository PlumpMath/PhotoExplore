#include <boost/thread.hpp>
#include <boost/filesystem.hpp>


#ifndef FileManager_H_
#define FileManager_H_

using namespace std;
using namespace boost::filesystem;

class FileManager {

public:
	static FileManager * instance;

	static FileManager * getInstance()
	{
		if (instance == NULL)
			instance = new FileManager();

		return instance;
	}

	FileManager()
	{
		;
	}

	vector<path> * loadDirectory(path directoryPath)
	{
		vector<path> * dirContents = new vector<path>();
		copy(directory_iterator(directoryPath), directory_iterator(), back_inserter(*dirContents));
		return dirContents;
	}





};

#endif