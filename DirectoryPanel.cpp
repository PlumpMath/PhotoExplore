#include "DirectoryPanel.hpp"
#include "UniformGrid.hpp"
#include "FileImagePanel.hpp"
#include "FileDataCursors.hpp"
#include "FileLoader.hpp"

using namespace FileSystem;

DirectoryPanel::DirectoryPanel() : ContentPanel()
{
	fileGroup = new UniformGrid(cv::Size2i(2,2));
		
	this->setContentView(fileGroup);

	this->NudgeAnimationEnabled = true;

	setBorderThickness(0);
	setBackgroundColor(Colors::White);

	this->setEventHandlingMode(true);
}

FileNode * DirectoryPanel::getDirectory()
{
	return directoryNode;
}

void DirectoryPanel::setDirectory(FileNode * node)
{	
	if (node->Files.size() < 4)
	{
		FileLoader::getInstance().loadFiles(node,0,[this](FileNode * nnode){	
			DirectoryPanel * me = this;
			postTask([me,nnode](){

				int count = 0;
				for (auto it = nnode->Files.get<RandFileIndex>().begin(); it != nnode->Files.get<RandFileIndex>().end() && count < 4; it++)
				{
					if (!it->IsDirectory())
					{
						FileNode * imageNode = it->Node;
						View * v= ViewOrchestrator::getInstance()->requestView(imageNode->filePath.string(), me);

						FileImagePanel * item = dynamic_cast<FileImagePanel*>(v);
						if (item == NULL)
						{
							item = new FileImagePanel();
							item->show(imageNode);
							ViewOrchestrator::getInstance()->registerView(imageNode->filePath.string(),item, me);
						}

						me->fileGroup->addChild(item);
						count++;
						me->layoutDirty = true;
					}
				}
			});
		});
	}

}

static cv::Size2i calculatePanelSize(int count)
{	
	int numRows, numColumns;
	
	switch (count)
	{
	case 0:
		numRows = numColumns = 1;
		break;
	case 1:
		numRows = numColumns = 1;
		break;
	case 2:
	case 3:		
		numRows = 2;
		numColumns = 1;
		break;		
	case 4:
	case 5:
		numRows = numColumns = 2;
		break;
	case 6:
	case 7:
	case 8:
		numRows = 2;
		numColumns = 3;
		break;
	case 9:
	default:
		numRows = numColumns = 3;
		break;					
	}

	return cv::Size2i(numColumns,numRows);
}

//void DirectoryPanel::viewChanged(string viewIdentifier, vector<FBNode*> & viewData)
//{
//	fileGroup->clearChildren();
//	for (auto it = viewData.begin(); it != viewData.end(); it++)
//	{
//		FBNode * childNode = (*it);
//		View * item = ViewOrchestrator::getInstance()->requestView(childNode->getId(), this);
//
//		if (item == NULL)
//		{
//			PicturePanel * p = new PicturePanel();
//			p->show(childNode);
//			p->setVisible(true);
//			item = p;
//			ViewOrchestrator::getInstance()->registerView(childNode->getId(),item, this);
//		}
//		item->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(2,2,2,2)));
//		((PicturePanel*)item)->setDataPriority(currentDataPriority);
//		fileGroup->addChild(item);					
//	}
//
//	((UniformGrid*)fileGroup)->resize(calculatePanelSize(fileGroup->getChildren()->size()));
//
//	layoutDirty = true;
//}


void DirectoryPanel::setDataPriority(float dataPriority)
{
	if (currentDataPriority != dataPriority)
	{
		currentDataPriority = dataPriority;

		for (auto it = fileGroup->getChildren()->begin(); it != fileGroup->getChildren()->end();it++)
		{
			FileImagePanel * imagePanel = dynamic_cast<FileImagePanel*>(*it);
			if (imagePanel != NULL)
				imagePanel->setDataPriority(dataPriority);
		}
	}
}

void DirectoryPanel::viewOwnershipChanged(View * view, ViewOwner * newOwner)
{
	if (this->fileGroup != NULL)
		this->fileGroup->remove(view);
}