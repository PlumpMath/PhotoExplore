#ifndef LEAPIMAGE_GL_IMPORT_H_
#define LEAPIMAGE_GL_IMPORT_H_

#include <GL/glew.h>
#include <GL/glfw.h>

#include <sstream>

#include "Logger.hpp"

class OpenGLHelper {

public:
	static bool LogOpenGLErrors(std::string tag)
	{
		int count = 0;
		GLenum glError;
		while ((glError = glGetError()) != GL_NO_ERROR)
		{
			std::stringstream error;
			error << "GL ERROR [" << count++ << "] = " << glError;
			Logger::log(tag,error.str());
		}
		return count > 0;
	}
};

#endif
