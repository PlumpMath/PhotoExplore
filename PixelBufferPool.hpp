#ifndef LEAPIMAGE_RESOURCE_MANAGER_PIXEL_BUFFER_POOL_HPP_
#define LEAPIMAGE_RESOURCE_MANAGER_PIXEL_BUFFER_POOL_HPP_

#include "GLImport.h"
#include <list>
#include "Logger.hpp"

class PixelBufferPool {
	
private:
	PixelBufferPool()
	{
		if (GlobalConfig::tree()->get<bool>("GraphicsSettings.DisablePixelBuffers"))
		{
			pixelBufferSupportExists = glewIsSupported("GL_ARB_pixel_buffer_object");
			if (!pixelBufferSupportExists)
				Logger::stream("PixelBufferPool","ERROR") << "PBO support does not exist. Using sychronous texture loading." << endl;
		}
		else
		{
			pixelBufferSupportExists = false;
		}
	}

	PixelBufferPool(PixelBufferPool const&);
	void operator=(PixelBufferPool const&); 
		
	std::list<GLuint> bufferQueue;
	bool pixelBufferSupportExists;

public:
	static PixelBufferPool& getInstance()
	{
		static PixelBufferPool instance; 
		return instance;
	}
	
	bool isEnabled()
	{
		return pixelBufferSupportExists;
	}

	GLuint getBuffer()
	{
		if (bufferQueue.empty())
			return NULL;

		GLuint pbo = bufferQueue.front();
		bufferQueue.pop_front();
		return pbo;
	}
	
	void freeBuffer(GLuint buffer)
	{
		if (std::find(bufferQueue.begin(), bufferQueue.end(),buffer) == bufferQueue.end())
			bufferQueue.push_back(buffer);
	}

	void initBuffers(int bufferCount)
	{			
		GLuint * pboIds = new GLuint[bufferCount];
		glGenBuffersARB(bufferCount,pboIds);

		for (int i=0;i<bufferCount;i++)
		{
			bufferQueue.push_back(pboIds[i]);
		}

		GLenum errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			Logger::getInstance().logstream << "PixelBuffer generation error: " << errorCode << "\n";;
		}
		else
		{
			Logger::getInstance().logstream << "Initialized " << bufferCount << " pixel buffers " << "\n";;
		}
	}

	void dispose()
	{
		while (!bufferQueue.empty())
		{
			glDeleteBuffersARB(1,&bufferQueue.front());
			bufferQueue.pop_front();
		}
	}

};



#endif