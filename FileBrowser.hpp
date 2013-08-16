#ifndef LEAPIMAGE_FILESYSTEM_FILE_BROWSER_HPP_
#define LEAPIMAGE_FILESYSTEM_FILE_BROWSER_HPP_

#include "FileNode.hpp"
#include "DirectoryView.hpp"
#include "View.hpp"

namespace FileSystem
{

	class FileBrowser : public View {

	private:
		DirectoryView * directoryView;
		ViewGroup * menuGroup;

	public:
		FileBrowser();

		void draw();
		void layout(Vector position, cv::Size2f size);

		void update();

		LeapElement * elementAtPoint(int x, int y, int & elementStateFlags);


	};

}



#endif
