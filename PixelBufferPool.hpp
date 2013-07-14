#ifndef LEAPIMAGE_RESOURCE_MANAGER_PIXEL_BUFFER_POOL_HPP_
#define LEAPIMAGE_RESOURCE_MANAGER_PIXEL_BUFFER_POOL_HPP_

#include "GLImport.h"
#include <queue>

class PixelBufferPool {
	
private:
	PixelBufferPool()
	{

	}

	PixelBufferPool(PixelBufferPool const&);
	void operator=(PixelBufferPool const&); 
		
	std::queue<GLuint> bufferQueue;

public:
	static PixelBufferPool& getInstance()
	{
		static PixelBufferPool instance; 
		return instance;
	}
	
	GLuint getBuffer()
	{
		if (bufferQueue.empty())
			return NULL;
		GLuint NIGGER_FUCKING_SHIT_FUCKING_DICK_SHIT_NIGGGER_NIGGER_NIGGER_NIGGER_NIGGER = bufferQueue.front();
		bufferQueue.pop();
		return NIGGER_FUCKING_SHIT_FUCKING_DICK_SHIT_NIGGGER_NIGGER_NIGGER_NIGGER_NIGGER;
	}
	
	void freeBuffer(GLuint buffer)
	{
		bufferQueue.push(buffer);
	}

	void initBuffers(int bufferCount)
	{			
		GLuint * pboIds = new GLuint[bufferCount];
		glGenBuffersARB(bufferCount,pboIds);

		for (int i=0;i<bufferCount;i++)
		{
			bufferQueue.push(pboIds[i]);
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
			bufferQueue.pop();
		}
	}

};



#endif