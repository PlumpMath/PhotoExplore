#ifndef LEAPIMAGE_RESOURCE_MANAGER_TEXTURE_LOADER_HPP_
#define LEAPIMAGE_RESOURCE_MANAGER_TEXTURE_LOADER_HPP_

#include <opencv2/opencv.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/function.hpp>
#include <vector>

#include "TextureLoadTask.hpp"


struct NameIndex{};
struct PriorityIndex{};

typedef boost::multi_index_container
	<
		TextureLoadTask,
		boost::multi_index::indexed_by
		<	
			boost::multi_index::ordered_non_unique
			<
				boost::multi_index::tag<PriorityIndex>,
				boost::multi_index::member<TextureLoadTask,float,&TextureLoadTask::priority>,
				std::less<float>
			>,
			boost::multi_index::hashed_unique
			<
				boost::multi_index::tag<NameIndex>,
				boost::multi_index::member<TextureLoadTask, std::string, &TextureLoadTask::resourceId>
			>	
		>
	> TextureLoadQueue;

class TextureLoader {
	
private:
	TextureLoader();
	TextureLoader(TextureLoader const&);
	void operator=(TextureLoader const&); 
	
	TextureLoadQueue loadQueue;
	std::vector<TextureLoadTask> activeTasks;
	int maxConcurrentTasks;
	
public:
	static TextureLoader& getInstance()
	{
		static TextureLoader instance; 
		return instance;
	}

	void loadTextureFromImage(std::string resourceId, float priority, cv::Mat & image, boost::function<void(GLuint textureId, int taskStatus)> callback);
	void updateTask(std::string resourceId, float priority);
	void update();
	void cancelTask(std::string resourceId);

};


#endif