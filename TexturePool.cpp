#include "TexturePool.h"
#include "GlobalConfig.hpp"

TexturePool::TexturePool()
{
	initPool(GlobalConfig::tree()->get<int>("TextureLoading.InitialTextureCount"));
}

void TexturePool::releaseAll()
{
	//while (!textureQueue.empty())
	//{
	//	glDeleteTextures(1,&textureQueue.front());
	//	textureQueue.pop_back();
	//}
}

void TexturePool::initPool(int poolSize)
{
	//GLuint textId;
	//this->poolSize = poolSize;
	//for (int i=0;i<poolSize;i++)
	//{
	//	glGenTextures(1,&textId);	
	//	textureQueue.push_back(textId);
	//}
}

GLuint TexturePool::getTexture(int requiredSize)
{
	GLuint textId;

	/*if (textureQueue.size() == 0)
	{*/
		glGenTextures(1,&textId);
	//	//return NULL;
	//}
	//else
	//{
	//	textId = textureQueue.back();
	//	textureQueue.pop_back();
	//}		

	return textId;
}

int TexturePool::releaseTexture(GLuint texture, int textureSize)
{
	if (texture != NULL)
	{		
		glDeleteTextures(1,&texture);
		//glGenTextures(1,&texture);
		//textureQueue.push_front(texture);
	}
	return NULL;
}