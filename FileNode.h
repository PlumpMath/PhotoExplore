#include "Types.h"
#include "FileManager.h"
#include <boost/filesystem.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include "NodeBase.h"


#ifndef FileNode_H_
#define FileNode_H_

#define ChildRelevanceFactor .5f

using namespace boost::filesystem;
using namespace std;

		
class FileNode;	
struct FileNameIndex{};
const path & getNodeName(const FileNode * nd);

	
typedef boost::multi_index_container
	<
		FileNode*,
		boost::multi_index::indexed_by
		<
			boost::multi_index::ordered_unique
			<
				boost::multi_index::tag<FileNameIndex>,
				boost::multi_index::global_fun<const FileNode*, const path&, &getNodeName>
			>
		>
	> FileNodeList;

class FileNode : public NodeBase {

public:
	path filePath;
	bool isDirectory;
	FileNodeList * children;
	FileNode * parent;
	bool childrenLoadState;

	FileNode(path boostPath);
	FileNode(std::string file);

	bool hasChildren();
	void setNodeDistance(int distance);
	void setDataPriority(float dataPriority);
		
	std::string getURI();

	string getNodeType();

    friend const path& getNodeName(const FileNode* nd);

protected:
	void setNodeDistance(int distance, NodeBase * sender);
	void setDataPriority(float priority, NodeBase * sender);

private:
	void loadMyChildren();

		


};	





#endif
