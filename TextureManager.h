#include "TexturePool.h"
#include "TypographyManager.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include "SDLTimer.h"
#include "ResourceManagerTypes.h"
#include "ImageManager.h"
#include <map>

using boost::multi_index::multi_index_container;
using boost::multi_index::composite_key;
using boost::multi_index::hashed_unique;
using boost::multi_index::ordered_unique;
using boost::multi_index::indexed_by;
using boost::multi_index::member;

#ifndef TextureManager_H_
#define TextureManager_H_


#define  MaxBytesPerFrame 1048576
#define AsyncMemCopy false
#define TimeMultiplexedCopy true

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

struct PendingLoadTask {

	PendingLoadTask(MultiResolutionImage * _image, TextureInfo _textureInfo, int _levelOfDetail, int _textureType):
		image(_image),
		levelOfDetail(_levelOfDetail),
		textureType(_textureType),
		textureInfo(_textureInfo)
	{

	}

	MultiResolutionImage* image;
	int levelOfDetail;
	int textureType;
	TextureInfo textureInfo;
};


class TextureLoadTask {
	
private:	
	TextureInfo textureInfo;
	GLuint sourceBuffer;
	int sourceImage;
	bool matrixDisposable;

	cv::Mat cvImage;

	int dataLoaded;
	unsigned char* loading_source;
	GLubyte * loading_target;

			
	void copyBufferToTexture();

	void initializeTexture();

	void asyncLoadingThread(unsigned char* source, GLubyte * target);

	void beginCopyMemoryToBuffer();

	void updateLoadTask();

	
	void copyMemoryToBuffer_TM_update();

	void copyMemoryToBuffer_TM_start();


	void copyMemoryToBuffer();

public:
	int state;
	int textureType;

	static const int Error = -1;
	static const int New = 0;
	static const int LoadingBuffer = 1;
	static const int LoadingBufferAsync = 2;
	static const int BufferLoaded = 3;
	static const int Waiting = 4;
	static const int Complete = 5;
		
	TextureLoadTask(GLuint sourceBuffer, cv::Mat cvImage, TextureInfo textureInfo, bool _matrixDisposable, int _textureType);
	GLuint getSourceBuffer();

	GLuint getTextureId();

	void cancel();

	void cleanup();

	void onFrame();

	bool isComplete();

	void forceComplete();

};

struct CachedTexture
{
	const static int LoadState_Complete = 2;
	const static int LoadState_Ongoing = 1;
	const static int LoadState_Pending = 0;
	const static int LoadState_Empty = -1;

	 CachedTexture (GLuint _textureId, string _imageFileName, LevelOfDetail _levelOfDetail, int _textureWidth,int _textureHeight, int _loaded, float _priority)
        : textureId(_textureId),
          imageFileName(_imageFileName),
		  levelOfDetail(_levelOfDetail), 
		  loaded(_loaded),
		  textureWidth(_textureWidth),
		  textureHeight(_textureHeight),
		  priority(_priority)
    {
    }

	 CachedTexture (GLuint _textureId, string _imageFileName, LevelOfDetail _levelOfDetail, float _priority)
        : textureId(_textureId),
          imageFileName(_imageFileName),
		  levelOfDetail(_levelOfDetail), 
		  loaded(0),
		  priority(_priority)
    {
    }
	
    GLuint textureId;
    string imageFileName;
	LevelOfDetail levelOfDetail;

	int loaded;
	int textureWidth;
	int textureHeight;
	float priority;

	bool isLoaded() const {
		return loaded >= 0;
	}

};

struct PriorityIndex{};
struct NameDetailIndex{};
struct TextureIdIndex{};

typedef boost::multi_index_container
	<
		CachedTexture,
		boost::multi_index::indexed_by
		<
			boost::multi_index::hashed_non_unique
			<
				boost::multi_index::tag<TextureIdIndex>, 
				boost::multi_index::member<CachedTexture, GLuint, &CachedTexture::textureId>
			>,
			boost::multi_index::ordered_non_unique
			<
				boost::multi_index::tag<PriorityIndex>, 
				boost::multi_index::member<CachedTexture,float,&CachedTexture::priority>
			>,
			boost::multi_index::hashed_unique
			<
				boost::multi_index::tag<NameDetailIndex>, 
				boost::multi_index::composite_key
				<
					CachedTexture, 
					boost::multi_index::member<CachedTexture, string, &CachedTexture::imageFileName>,
					boost::multi_index::member<CachedTexture, LevelOfDetail, &CachedTexture::levelOfDetail> 
				> 
			>
		>
    > TextureCache;


struct PendingTask
{
	string resourceId;
	LevelOfDetail levelOfDetail;	
	float priority;

public:
	PendingTask(string _resourceId, LevelOfDetail _levelOfDetail, float _priority) :
		resourceId(_resourceId),
		levelOfDetail(_levelOfDetail),
		priority(_priority)
	{ }
};



typedef boost::multi_index_container
	<
		PendingTask,
		boost::multi_index::indexed_by
		<
			boost::multi_index::ordered_non_unique
			<
				boost::multi_index::tag<PriorityIndex>,
				boost::multi_index::member<PendingTask,float, &PendingTask::priority>
			>,
			boost::multi_index::ordered_unique
			<
				boost::multi_index::tag<NameDetailIndex>, 
				boost::multi_index::composite_key
				<
					PendingTask, 
					boost::multi_index::member<PendingTask, string, &PendingTask::resourceId>,
					boost::multi_index::member<PendingTask, LevelOfDetail, &PendingTask::levelOfDetail> 
				> 
			>
		>
	> PendingTaskQueue;



class TextureManager {

private:
	std::map<int,TexturePool*> texturePoolMap;
	std::queue<GLuint> bufferQueue;
	std::deque<TextureLoadTask*> textureTaskQueue;
	std::vector<PendingLoadTask> pendingTextures;
	TextureCache textureCache, fontTextureCache;
	PendingTaskQueue overflowQueue;
	
	boost::function<void(string,LevelOfDetail,GLuint)> resourceChangedCallback;

	int bufferCount;
	int MaxTextureCount;
	static TextureManager * instance;
	//bool cacheDirty;
	int dirtyItems;
	bool fontCacheDirty;

	void initBuffers(int bufferCount);

	void freeBuffers(int count = 1);

	void checkTaskComplete();
	bool startLoadTask(PendingLoadTask & loadTask, int taskImportance);
	TextureInfo getNewTextureInfo(MultiResolutionImage * imageInfo, LevelOfDetail levelOfDetail, float priority, int textureType);
	TextureCache * getCacheForTextureType(int textureType);
	bool loadTexture(string imageURI, LevelOfDetail levelOfDetail, float loadImportance, float priority);
	
	void unloadTexture(GLuint textureId, int textureType);
	void loadTextureFromImage(MultiResolutionImage * imageInfo, LevelOfDetail levelOfDetail, int loadImportance, float priority, int textureType);

	void cleanupTextureCache();
	void cleanupFontCache();
	
public:

	const static int TextureType_Image = 2;
	const static int TextureType_Font = 3;

	static TextureManager * getInstance()
	{
		if (instance == NULL)
			instance = new TextureManager();
		return instance;
	}

	TextureManager();

	void update(int stepCount = 1);

	//void cleanupCache();

	void setTexturePriority(string textureURI, int levelOfDetail, int textureType, float priority);
	
	bool isTextureLoaded(string imageURI, LevelOfDetail levelOfDetail);
	bool isTextureLoaded(GLuint textureId, int textureType);
				
	void destroyTexture(string textureURI, int levelOfDetail, int textureType);
	void loadTextureFromImage(string key, LevelOfDetail levelOfDetail, int textureType, float priority, cv::Mat & imgMat);

	GLuint getLoadedTexture(string textureURI, LevelOfDetail levelOfDetail, int textureType);
	
	void setResourceChangedCallback(boost::function<void(string,LevelOfDetail,GLuint)> resourceChangedCallback);	

	void initialize();
	void dispose();
};



#endif