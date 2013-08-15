#include "FileDetailView.hpp"

using namespace FileSystem;

void FileDetailView::setPicturePanel(FileImagePanel * picturePanel)
{
	imageNode = picturePanel->getNode();
	setImagePanel(picturePanel);
}

