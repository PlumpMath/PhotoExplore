#include "TextureLoadTask.hpp"		

#include "GLImport.h"
#include "SDLTimer.h"
#include <math.h>
#include "TexturePool.h"
#include "PixelBufferPool.hpp"
#include "Logger.hpp"

using namespace std;
using namespace LoadTaskState;

#define  MaxBytesPerFrame 1048576

TextureLoadTask::TextureLoadTask(std::string _resourceId, float _priority, cv::Mat _cvImage, boost::function<void(GLuint textureId, int taskStatus)> _callback):
	cvImage(_cvImage),
	priority(_priority),
	resourceId(_resourceId),
	callback(_callback)
{
	if (cvImage.data == NULL || cvImage.size().area() == 0)
		throw new std::exception("Invalid image");

	sourceBuffer = NULL;
	textureInfo.textureId = NULL;
	state = New;
}


TextureLoadTask::~TextureLoadTask()
{
	cleanup();
}

void TextureLoadTask::setPriority(float _priority)
{
	this->priority = _priority;
}

GLuint TextureLoadTask::getTextureId()
{
	return textureInfo.textureId;
}

int TextureLoadTask::getState()
{
	return state;
}

void TextureLoadTask::cancel()
{	
	state = Error;
	cleanup();
}


void TextureLoadTask::update()
{
	switch (state)
	{
	case New:
		startTask();
		break;
	case Initialized:
		initializeTexture();
		break;
	case LoadingBuffer:
		copyMemoryToBuffer();
		break;
	case BufferLoaded:
		copyBufferToTexture();	
		break;
	case Waiting:
		cleanup();	
		state = Complete;
		break;
	case Complete:				
		break;
	case Error:
		break;
	}
}



void TextureLoadTask::startTask()
{
	if (textureInfo.textureId == NULL)
	{
		GLuint textureId = TexturePool::getInstance().getTexture();
		if (textureId != NULL)
			textureInfo = TextureInfo(textureId,GL_RGBA,4,cvImage.size().width,cvImage.size().height);
		else
			return;
	}

	sourceBuffer = PixelBufferPool::getInstance().getBuffer();
	if (sourceBuffer != NULL)
	{
		//Logger::stream("TextureLoadTask","INFO") << "Load task started. TexID = " << textureInfo.textureId << " BufferID = " << sourceBuffer << endl;
		state = Initialized;
		update();
	}
}


void TextureLoadTask::initializeTexture()
{
	static double totalTime;
	static int notifyCount;
	static Timer initTimer;
	
	initTimer.start();
	glBindTexture(GL_TEXTURE_2D, textureInfo.textureId);
	glTexImage2D(GL_TEXTURE_2D,0,textureInfo.bytesPerPixel,textureInfo.width,textureInfo.height,0,textureInfo.format,GL_UNSIGNED_BYTE,0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, NULL);
		
	//Logger::stream("TextureLoadTask","INFO") << "Initialized texture. TexID = " << textureInfo.textureId << " W= " << textureInfo.width << " H = " << textureInfo.height << endl;;
	OpenGLHelper::LogOpenGLErrors("TextureLoadTask-initializeTexture");

	//LeapDebug::out << "[" << Timer::frameId << "] Initialized TEX[" << textureInfo.textureId << "] in " << initTimer.millis() << "ms \n";

	dataLoaded = 0;
	copyMemoryToBuffer_TM_start();
}



void TextureLoadTask::copyBufferToTexture()
{		
	static Timer timer;		
	timer.start();
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, sourceBuffer);  
	glBindTexture(GL_TEXTURE_2D, textureInfo.textureId);		

	glTexSubImage2D(GL_TEXTURE_2D,0,0,0, textureInfo.width,textureInfo.height,textureInfo.format,GL_UNSIGNED_BYTE, 0);			
	//LeapDebug::out  << "[" << Timer::frameId << "] Loaded TEX[" << textureInfo.textureId << "] FROM PBO[" << sourceBuffer << "] in " << timer.millis() << "ms @ " << textureInfo.textureSizeMB/timer.seconds() << " MB/s \n";		
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,NULL);

	
	//Logger::stream("TextureLoadTask","INFO") << "Copied to texture. TexID = " << textureInfo.textureId << endl;
	
	state  = Waiting;
}

void TextureLoadTask::copyMemoryToBuffer_TM_update()
{	
	Timer memTimer;
	memTimer.start();

	int loadCount = (textureInfo.size - dataLoaded);

	loadCount = min<int>(loadCount,MaxBytesPerFrame);

	memcpy(loading_target+dataLoaded,loading_source+dataLoaded,loadCount);	

	dataLoaded += loadCount;

	if (dataLoaded >= textureInfo.size)
	{
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, sourceBuffer);  	
		glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); 			
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,NULL);

		state = BufferLoaded;
		OpenGLHelper::LogOpenGLErrors("TextureLoadTask-copyMemoryToBuffer_TM_update");
	}
	//if (memTimer.millis() > 2)
	//LeapDebug::out  << "[" << Timer::frameId << "] - TEX["<< textureInfo.textureId << "] from MEM to PBO[" << sourceBuffer << "]  in " << memTimer.millis() << "ms, @ " << ((loadCount/1048576)/memTimer.seconds()) << " MB/s \n";	
}

void TextureLoadTask::copyMemoryToBuffer_TM_start()
{
	Timer memTimer;
	memTimer.start();

	unsigned char * data = cvImage.data;

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, sourceBuffer);  			
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, textureInfo.size, NULL, GL_STREAM_DRAW_ARB);
	GLubyte* ptr = (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);	
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, NULL);

	if(ptr && data)
	{				
		loading_source = data;
		loading_target = ptr;
		dataLoaded = 0;
		state = LoadingBuffer;
	}
	else
	{
		//Logger::stream("TextureLoadTask","ERROR") << "copyMemoryToBuffer_TM_start - Couldn't access pixel buffer. ID = " << sourceBuffer << endl;
		cancel();
	}
	//LeapDebug::out  << "[" << Timer::frameId << "] Prepared to copy TEX["<< textureInfo.textureId << "] from MEM to PBO[" << sourceBuffer << "] in " << memTimer.millis() << "ms \n";
}


void TextureLoadTask::copyMemoryToBuffer()
{
	Timer memtimer;

	unsigned char * data = cvImage.data;

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, sourceBuffer);  	

	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, textureInfo.size, NULL, GL_STREAM_DRAW_ARB);	

	GLubyte* ptr = (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);	

	if(ptr)
	{				
		memtimer.start();
		memcpy(ptr,data,textureInfo.size);	
		//LeapDebug::out.setf(std::ios_base::fixed);
		//LeapDebug::out.precision(2);	
		//LeapDebug::out  << "[" << Timer::frameId << "] - TEX["<< textureInfo.textureId << "] from MEM to PBO[" << sourceBuffer << "]  in " << memtimer.millis() << "ms, @ " << (textureInfo.textureSizeMB/memtimer.seconds()) << " MB/s \n";	
		glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); 	
		state = BufferLoaded;
		
		//Logger::stream("TextureLoadTask","INFO") << "Buffer load complete. TexID = " << textureInfo.textureId << " BufferID = " << sourceBuffer << endl;
		OpenGLHelper::LogOpenGLErrors("TextureLoadTask-copyMemoryToBuffer");
	}
	else
	{
		OpenGLHelper::LogOpenGLErrors("TextureLoadTask-copyMemoryToBuffer_ERROR");
		//Logger::stream("TextureLoadTask","ERROR") << "Pixel buffer not acessible. TexID = " << textureInfo.textureId << " BufferID = " << sourceBuffer << endl;
		cancel();
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, NULL);
}


void TextureLoadTask::cleanup()
{
	if (sourceBuffer != NULL)
	{
		//Logger::stream("TextureLoadTask","INFO") << "Cleaning up. TexID = " << textureInfo.textureId << " BufferID = " << sourceBuffer << endl;
		PixelBufferPool::getInstance().freeBuffer(sourceBuffer);
		sourceBuffer = NULL;
	}
}