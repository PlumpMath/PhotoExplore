#include "FileNode.hpp"

using namespace FileSystem;
using namespace boost::filesystem;

string File::Filename() const 
{
	return Node->filename;
}


bool File::IsDirectory() const 
{
	return Node->isDirectory;
}


FileNode::FileNode(path _filePath) :
	filePath(_filePath),
	Parent(NULL)
{
	isDirectory = is_directory(filePath);
	filename = filePath.filename().string();
}

FileNode::FileNode(string _filePath)
{
	Parent = NULL;
	filePath = path(_filePath);
	isDirectory = is_directory(filePath);
	filename = filePath.filename().string();
}

