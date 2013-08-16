#include "FileDetailView.hpp"
#include "ViewOrchestrator.hpp"

using namespace FileSystem;

void FileDetailView::setPicturePanel(FileImagePanel * picturePanel)
{
	imageNode = picturePanel->getNode();

	parentNode = imageNode->Parent;
	
	setImagePanel(picturePanel);

	if (parentNode != NULL)
	{
		auto childIt = parentNode->Files.get<FileNameIndex>().find(imageNode->filename);

		if (childIt != parentNode->Files.get<FileNameIndex>().end())
		{
			auto randIt = parentNode->Files.project<RandFileIndex>(childIt);
			containerOffset = std::distance(parentNode->Files.get<RandFileIndex>().begin(),randIt);			
		}
	}
}


DynamicImagePanel * FileDetailView::getSiblingByOffset(int offset)
{
	if (parentNode != NULL)
	{
		int index = containerOffset + offset;
		if (index >= 0 && index < parentNode->Files.size())
		{
			FileNode * file = parentNode->Files.get<RandFileIndex>().at(index).Node;

			View * v = ViewOrchestrator::getInstance()->requestView(file->filePath.string(),NULL);

			DynamicImagePanel * dip = dynamic_cast<DynamicImagePanel*>(v);
			
			return dip;
		}
	}
	return NULL;
}
