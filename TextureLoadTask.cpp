#include "TextureManager.h"		


	void TextureLoadTask::copyBufferToTexture()
	{		
		static Timer timer;		
		timer.start();
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, sourceBuffer);  
		glBindTexture(GL_TEXTURE_2D, textureInfo.textureId);		

		glTexSubImage2D(GL_TEXTURE_2D,0,0,0, textureInfo.width,textureInfo.height,textureInfo.format,GL_UNSIGNED_BYTE, 0);			
		//LeapDebug::out  << "[" << Timer::frameId << "] Loaded TEX[" << textureInfo.textureId << "] FROM PBO[" << sourceBuffer << "] in " << timer.millis() << "ms @ " << textureInfo.textureSizeMB/timer.seconds() << " MB/s \n";		
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,NULL);

		int count = 0;
		GLenum glError;
		while ((glError = glGetError()) != GL_NO_ERROR)
		{
			cout << "[" << count << "]" << "copyBufferToTexture ERROR:" << glError << endl;
		}

		state  = Waiting;
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
		
		int count = 0;
		GLenum glError;
		while ((glError = glGetError()) != GL_NO_ERROR)
		{
			cout << "[" << count << "]" << "initializeTexture ERROR:" << glError << endl;
		}

		//LeapDebug::out << "[" << Timer::frameId << "] Initialized TEX[" << textureInfo.textureId << "] in " << initTimer.millis() << "ms \n";

		if (AsyncMemCopy)
		{
			beginCopyMemoryToBuffer();
		}
		else if (TimeMultiplexedCopy)
		{
			dataLoaded = 0;
			copyMemoryToBuffer_TM_start();
		}
		else
		{
			state = LoadingBuffer;
		}
		
	/*	totalTime += initTimer.millis();
		notifyCount++;

		if (notifyCount % 5 == 0)
		{
			LeapImageOut << "Average texture init time = " << (totalTime/((double)notifyCount)) << " ms \n";
		}*/
	}

	void TextureLoadTask::asyncLoadingThread(unsigned char* source, GLubyte * target)
	{		
		//Timer memtimer;
		//memtimer.start();

		memcpy(target,source,textureInfo.size);	

		//LeapDebug::out.setf(std::ios_base::fixed);
		//LeapDebug::out.precision(2);	
		//LeapDebug::out  << "[" << Timer::frameId << "] - TEX["<< textureInfo.textureId << "] from MEM to PBO[" << sourceBuffer << "]  in " << memtimer.millis() << "ms, @ " << (textureInfo.textureSizeMB/memtimer.seconds()) << " MB/s \n";	
	}

	void TextureLoadTask::beginCopyMemoryToBuffer()
	{

		//ilBindImage(sourceImage);		
		//unsigned char * imageData = ilGetData();			
		//ilBindImage(NULL);
		//
		//glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, sourceBuffer);  			
		//glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, textureInfo.size, NULL, GL_STREAM_DRAW_ARB);	
		//GLubyte* bufferTarget = (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);	

		//if(bufferTarget)
		//{					
		//	state = LoadingBufferAsync;
		//	loadThread = boost::thread(asyncLoadingThread,imageData,bufferTarget);
		//	//loadThread.start_thread();
		//}
		//else
		//{
		//	LeapDebug::out << "Couldn't access pixel buffer! \n";			
		//	state = Error;
		//}
	}

	void TextureLoadTask::updateLoadTask()
	{
		if (AsyncMemCopy)
		{
			//if isDone
			//if (loadThread.joinable())
			//{
			//	glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); 	
			//	state = BufferLoaded;
			//}
		}
		else if (TimeMultiplexedCopy)
		{
			copyMemoryToBuffer_TM_update();

			if (state == BufferLoaded)
			{
				copyBufferToTexture();
			}
		}
	}

	
	void TextureLoadTask::copyMemoryToBuffer_TM_update()
	{	
		Timer memTimer;
		memTimer.start();

		int loadCount = (textureInfo.size - dataLoaded);
		
		loadCount = min(loadCount,MaxBytesPerFrame);
		
		memcpy(loading_target+dataLoaded,loading_source+dataLoaded,loadCount);	

		dataLoaded += loadCount;

		if (dataLoaded >= textureInfo.size)
		{
			//LeapDebug::out << "Finished loading PBO[" << sourceBuffer << "] \n";
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, sourceBuffer);  	
			glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); 			
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,NULL);
						
			state = BufferLoaded;

			int count = 0;
			GLenum glError;
			while ((glError = glGetError()) != GL_NO_ERROR)
			{
				cout << "[" << count << "]" << "copyMemoryToBuffer_TM_update ERROR:" << glError << endl;
			}
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
			state = LoadingBufferAsync;
		}
		else
		{
			//LeapDebug::out << "Couldn't access pixel buffer! \n";			
			state = Error;
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
			
			GLenum error;
			if ((error = glGetError()) != GL_NO_ERROR)
			{
				cout << "copyMemoryToBuffer ERROR = " << error << endl;
			}
		}
		else
		{
			//LeapDebug::out << "Couldn't access pixel buffer! \n";			
			state = Error;
		}
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, NULL);
	}
		
	TextureLoadTask::TextureLoadTask(GLuint sourceBuffer, cv::Mat cvImage, TextureInfo textureInfo, bool _matrixDisposable, int _textureType)
	{
		this->sourceBuffer = sourceBuffer;
		this->textureInfo = textureInfo;
		this->sourceImage  = -1;
		this->cvImage = cvImage;
		this->matrixDisposable = _matrixDisposable;
		this->textureType = _textureType;
		
		state = New;
	}

	GLuint TextureLoadTask::getSourceBuffer()
	{
		return sourceBuffer;
	}

	GLuint TextureLoadTask::getTextureId()
	{
		 return textureInfo.textureId;
	}

	void TextureLoadTask::cancel()
	{
		state = Error;
	}

	void TextureLoadTask::cleanup()
	{
		if (matrixDisposable)
			this->cvImage.release();
	}

	void TextureLoadTask::onFrame()
	{
		switch (state)
		{
		case Error:
			cleanup();
			break;
		case New:
			initializeTexture();
			break;
		case LoadingBuffer:
			copyMemoryToBuffer();
			break;
		case LoadingBufferAsync:
			updateLoadTask();
			break;
		case BufferLoaded:
			copyBufferToTexture();	
			break;
		case Waiting:			
			state = Complete;
			break;
		case Complete:
			cleanup();
			break;
		}
	}

	bool TextureLoadTask::isComplete()
	{
		return state == Complete || state == Error;
	}

	void TextureLoadTask::forceComplete()
	{
		while (!isComplete())
		{
			//LeapDebug::out << "Waiting to complete: State = " << state << "\n";
			onFrame();
		}
	}