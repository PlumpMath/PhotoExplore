#include "TexturePool.h"

TexturePool::TexturePool(int size)
{
	initPool(size);
}

void TexturePool::releaseAll()
{
	while (!textureQueue.empty())
	{
		glDeleteTextures(1,&textureQueue.front());
		textureQueue.pop();
	}
}

void TexturePool::initPool(int poolSize)
{
	GLuint textId;
	this->poolSize = poolSize;
	for (int i=0;i<poolSize;i++)
	{
		glGenTextures(1,&textId);	
		textureQueue.push(textId);
	}
}

GLuint TexturePool::getTexture(int requiredSize)
{
	GLuint textId;

	if (textureQueue.size() == 0)
	{
		glGenTextures(1,&textId);
	}
	else
	{
		textId = textureQueue.front();
		textureQueue.pop();
	}		

	return textId;
}

int TexturePool::releaseTexture(GLuint texture, int textureSize)
{
	if (texture != NULL)
		textureQueue.push(texture);
	return NULL;
}