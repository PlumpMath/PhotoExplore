#ifndef LEAPIMAGE_FILESYSTEM_DIRECTORY_VIEW_HPP_
#define LEAPIMAGE_FILESYSTEM_DIRECTORY_VIEW_HPP_

#include "DataListActivity.hpp"
#include "FileNode.hpp"
#include "ViewOrchestrator.hpp"
#include "FileDetailView.hpp"
#include "FileImagePanel.hpp"

namespace FileSystem {

	class DirectoryView : public DataListActivity, public ViewOwner {

	private:
		FileNode * directoryNode;
		//DirectoryCursor * directoryCursor;

		FileDetailView * imageDetailView;

	public:
		DirectoryView();

		void setDirectory(FileNode * node);
		void showPhoto(FileNode * photoNode);

		FileNode * getDirectory();

		virtual View * getDataView(DataNode * photoNode);
		virtual void setItemPriority(float priority, View * itemView);

		void viewOwnershipChanged(View * view, ViewOwner * newOwner);
		void getTutorialDescriptor(vector<string> & tutorial);
		void setFinishedCallback(const boost::function<void(std::string)> & callback);

		void onGlobalGesture(const Controller & controller, std::string gestureType);

		void layout(Vector position, cv::Size2f size);

	};

}


#endif