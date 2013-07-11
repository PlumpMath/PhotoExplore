#include "FileNode.h"


const path& getNodeName(const FileNode* nd)
{
	return nd->filePath;
}


FileNode::FileNode(path boostPath)
{
	filePath = boostPath;
	isDirectory = is_directory(boostPath);
	childrenLoadState = false;
	children = new FileNodeList();
	parent = NULL;
	dataPriority = 1;
	nodeDistance = 100;
}

FileNode::FileNode(std::string file) 
{
	filePath = path(file);
	isDirectory = is_directory(filePath);
	childrenLoadState = false;
	children = new FileNodeList();
	parent = NULL;
	dataPriority = 1;
	nodeDistance = 100;
}

static bool isImageFile(boost::filesystem::path testPath)
{
	return testPath.extension().compare(std::string(".jpg")) == 0  || testPath.extension().compare(std::string(".png")) == 0;
}


void FileNode::loadMyChildren()
{		
	if (!childrenLoadState)
	{
		vector<path> * childPaths = FileManager::getInstance()->loadDirectory(filePath);

		if (childPaths != NULL)
		{

			for (auto it = childPaths->begin();it != childPaths->end();it++)
			{
				if (is_directory(*it) || isImageFile(*it))
				{
					FileNode * childNode = new FileNode(*it);
					childNode->parent = this;
					children->get<FileNameIndex>().insert(childNode);		

					childNode->setNodeDistance(nodeDistance+1,this);
				}
			}
		}

		childrenLoadState = true;		
	}
}

std::string FileNode::getURI()
{
	return filePath.string();
}

bool FileNode::hasChildren()
{
	return isDirectory;
}

void FileNode::setDataPriority(float _dataPriority, NodeBase * sender)
{
	if (this->dataPriority != _dataPriority)
	{
		this->dataPriority = _dataPriority;

		if (!isDirectory)
		{
			if (nodeDistance <= 1)
				ImageManager::getInstance()->setImageRelevance(filePath.string(),LevelOfDetail_Types::Full,dataPriority,ImgTaskQueue::LocalImage);
			else if (nodeDistance <= 3)
				ImageManager::getInstance()->setImageRelevance(filePath.string(),LevelOfDetail_Types::Preview,dataPriority,ImgTaskQueue::LocalImage);
		}
	}
}

void FileNode::setNodeDistance(int nodeDistance, NodeBase * sender)
{	
	this->nodeDistance = nodeDistance;

	if (parent != NULL && parent != sender)
		parent->setNodeDistance(nodeDistance + 1, this);

	if (isDirectory && nodeDistance <= 3)
	{
		loadMyChildren();

		for (auto it = children->get<FileNameIndex>().begin(); it != children->get<FileNameIndex>().end();it++)
		{
			if (sender != (*it))
			{
				(*it)->setNodeDistance(nodeDistance+1, this);			
			}
		}

		childrenChanged();
	}
	//else
	//{
	//	//if (nodeDistance <= 1)
	//	//	ImageManager::getInstance()->setImageRelevance(filePath.string(),LevelOfDetail_Types::Full,dataPriority+nodeDistance,ImgTaskQueue::LocalImage);
	//	//else 
	//	if (nodeDistance <= 3)
	//		ImageManager::getInstance()->setImageRelevance(filePath.string(),LevelOfDetail_Types::Preview,dataPriority+nodeDistance,ImgTaskQueue::LocalImage);
	//}

}


void FileNode::setDataPriority(float _dataPriority)
{
	setDataPriority(_dataPriority, NULL);
}

void FileNode::setNodeDistance(int _nodeDistance)
{
	setNodeDistance(_nodeDistance, NULL);
}

string FileNode::getNodeType()
{
	if (hasChildren())
		return NodeType::ImageDirectory;
	else
		return NodeType::ImageFile;
}


