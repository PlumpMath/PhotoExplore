#ifndef LEAPIMAGE_RESOURCE_MANAGER_TEXTURE_LOAD_TASK_HPP_
#define LEAPIMAGE_RESOURCE_MANAGER_TEXTURE_LOAD_TASK_HPP_

#include "GLImport.h"
#include <opencv2/opencv.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>

struct TextureInfo {

public:
	GLuint textureId;
	GLenum format;
	GLsizei width, height;
	int size, bytesPerPixel;
	int textureType;

	double textureSizeMB;

	TextureInfo()
	{

	}

	TextureInfo(GLuint _textureId) :
		textureId(_textureId)
	{

	}

	TextureInfo(GLuint _textureId, GLenum _textureFormat, GLint bytesPerPixel, GLsizei _textureWidth, GLsizei _textureHeight)
	{
		this->textureId = _textureId;
		this->format = _textureFormat;
		this->height = _textureHeight;
		this->width = _textureWidth;
		this->bytesPerPixel = bytesPerPixel;
		
		this->size = width*height*bytesPerPixel;
		this->textureSizeMB = ((double)(size))/(1048576.0);
	}
};

namespace LoadTaskState {
		
	static const int Error = -1;
	static const int New = 0;
	static const int Initialized = 6;
	static const int LoadingBuffer = 1;
	static const int BufferLoaded = 3;
	static const int LoadingTexture = 7;
	static const int Waiting = 4;
	static const int Complete = 5;
}

class TextureLoadTask {
	
private:	
	TextureInfo textureInfo;
	GLuint sourceBuffer;
	cv::Mat cvImage;
	int dataLoaded, state;
	
	unsigned char* loading_source;
	GLubyte * loading_target;
	
	bool useCompression;
	bool asyncLoading;
	
	volatile bool active;
	
	

	void startTask();

	void initializeTexture();
	
	void beginCopyMemoryToBuffer();
	void copyMemoryToBuffer_TM_update();
	void copyMemoryToBuffer_TM_start();
	
	void copyMemoryToBuffer_async_update();
	void copyMemoryToBuffer_async_start();
	
	void copyBufferToTexture_TM_start();
	void copyBufferToTexture_TM_update();

	void copyMemoryToBuffer();

	void copyBufferToTexture();

	int MaxBytesPerFrame;
	static int taskId;

public:
		
	float priority;
	std::string resourceId;
	boost::function<void(GLuint textureId, int taskStatus)> callback;

	TextureLoadTask(std::string resourceId, float priority, cv::Mat cvImage, boost::function<void(GLuint textureId, int taskStatus)> callback);
	~TextureLoadTask();
	
	void setPriority(float priority);
	
	void cleanup();
	void cancel();	
	void update();

	int getState();	
	GLuint getTextureId();

};

#endif