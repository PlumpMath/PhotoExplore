#ifndef LEAPIMAGE_FILESYSTEM_FILENODE_HPP_
#define LEAPIMAGE_FILESYSTEM_FILENODE_HPP_


// This is the FileNode class, which maintains a memory representation of relevant portions of the local filesystem
// 
// Give that a filesystem is strictly hierarchal, strict parent-child relationships can be maintained
//
// Children are stored using a Boost.MultiIndex container. This container has the following indices:
//  1. Name-based unique UNSORTED index. Case sensitivity depends on filesytem - this index is used only for element access and maintaining uniqueness.
//	2. Random access index for other types of user-defined sorts (date,size, tags, etc)
//
// note: use MultiIndex rearrange for sorting


#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include <boost/filesystem.hpp>

#include "DataNode.hpp"

using namespace std;

namespace FileSystem 
{

	class FileNode; //Predeclare

	//This is the container structure
	struct File {

		FileNode * Node;

		File(FileNode * _Node) : 
			Node(_Node)
		{}
					
		string Filename() const;
		bool IsDirectory() const;

	};

	
	struct FileNameIndex{};
	struct RandFileIndex{};
	struct FileSystemSeq{};

	typedef boost::multi_index_container
	<
		File,
		boost::multi_index::indexed_by
		<
			boost::multi_index::hashed_unique
			<
				boost::multi_index::tag<FileNameIndex>,
				boost::multi_index::const_mem_fun<File,string, &File::Filename>
				//TODO: Case-insensitive compare 
			>,
			boost::multi_index::sequenced
			<
				boost::multi_index::tag<FileSystemSeq>
			>,
			boost::multi_index::random_access
			<
				boost::multi_index::tag<RandFileIndex>
			>
		>
	> FileContainer;


	class FileNode : public DataNode {

	public:
		FileContainer Files;
		string filename;
		boost::filesystem::path filePath;
		bool isDirectory;

		FileNode(boost::filesystem::path filePath);
		FileNode(string filePath);
	};

}
#endif