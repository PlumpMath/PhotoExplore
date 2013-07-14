#include "GLImport.h"

#include <queue>
#include <map>

#ifndef TexturePool_H_
#define TexturePool_H_

using namespace std;

class TexturePool {

private:
	queue<GLuint> textureQueue;
	int poolSize;
	
	TexturePool(TexturePool const&);
	void operator=(TexturePool const&); 
		
public:
	static TexturePool& getInstance()
	{
		static TexturePool instance; 
		return instance;
	}
	
	TexturePool();
	void releaseAll();

	void initPool(int poolSize);

	GLuint getTexture(int requiredSize = 0);
	int releaseTexture(GLuint texture, int textureSize = 0);
	
};
#endif