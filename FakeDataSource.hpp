#ifndef LEAPIMAGE_FAKE_DATA_SOURCE_HPP_
#define LEAPIMAGE_FAKE_DATA_SOURCE_HPP_

#include <boost/filesystem.hpp>
#include "FBNode.h"
#include "Types.h"
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <queue>
#include <vector>

#include "GlobalConfig.hpp"
#include "Logger.hpp"

using namespace Facebook;
using namespace boost::filesystem;

class FakeDataSource : public FBDataSource {

public:
	int photoIndex, albumIndex, friendIndex;
	boost::mutex taskMutex;
	std::queue<boost::function<void()> > taskQueue;
	vector<path> dirContents;

	FakeDataSource(string fakeDataPath);

	static void runThread(boost::mutex * taskMutex, std::queue<boost::function<void()> > * taskQueue);

	void loadWithOffset(FBNode * parent, string edge, int limit, int offset);

	void extractLimitOffset(string nodeQuery, string edge, int & offset, int & limit);

	void loadField(FBNode * parent, string nodeQuery, string interpretAs, boost::function<void(FBNode*)> callback);



	void postRequest(string request);
	void load(FBNode * parent, string objectId, string edge);
	void loadQuery(FBNode * parent, string nodeQuery, string interpretAs, boost::function<void(FBNode*)> callback);


};

#endif
