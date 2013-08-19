#include "FileDetailView.hpp"

using namespace FileSystem;


DynamicImagePanel * FileDetailView::getDetailedDataView(DataNode * node)
{
	FileNode * file = (FileNode*)node;

	if (file != NULL)
	{
		View * v = ViewOrchestrator::getInstance()->requestView(file->filePath.string(),this);
		FileImagePanel * filePanel = dynamic_cast<FileImagePanel*>(v);

		if (filePanel == NULL)
		{
			filePanel = new FileImagePanel();
			filePanel->show(file);

			filePanel->setLayoutParams(LayoutParams(cv::Size2f(),cv::Vec4f(5,5,5,5)));
			filePanel->layout(Vector(),cv::Size2f(100,100));
		}
		
		return filePanel;
	}
	return NULL;
}
