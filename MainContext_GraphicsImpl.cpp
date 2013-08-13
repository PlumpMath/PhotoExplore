#include "MainContext.hpp"


void MainContext::disposeShaders()
{
	glDeleteProgram(blurPrograms[0]);
	glDeleteProgram(blurPrograms[1]);
}


void MainContext::destroyFrameBuffers()
{	
	glDeleteTextures(2,fbo_texture);
	glDeleteFramebuffers(2, fbo);
	glDeleteRenderbuffers(2, rbo_depth);

	glDeleteBuffers(1, &vbo_fbo_vertices);
}


int MainContext::createFrameBuffers(cv::Size2i frameBufferSize)
{
	GLfloat fbo_vertices[] = {
		-1, -1,
		1, -1,
		-1,  1,
		1,  1,
	};

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(2, fbo_texture);
	glGenFramebuffers(2, fbo);
	glGenRenderbuffers(2, rbo_depth);

	for (int i=0;i<2;i++)
	{
		glBindTexture(GL_TEXTURE_2D, fbo_texture[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,frameBufferSize.width,frameBufferSize.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, frameBufferSize.width,frameBufferSize.height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture[i], 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth[i]);

		GLenum status;
		if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
			fprintf(stderr, "glCheckFramebufferStatus: error %p", status);
			return 0;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenBuffers(1, &vbo_fbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

int MainContext::initShaders()
{	
	
	string shaderDir = GlobalConfig::tree()->get<string>("GraphicsSettings.ShaderDirectory");

	if ((verticalBlur = createShader(shaderDir + "/VBlurVertexShader.glsl", GL_VERTEX_SHADER))   == 0) return 0;
	if ((horizontalBlur = createShader(shaderDir + "/HBlurVertexShader.glsl", GL_VERTEX_SHADER)) == 0) return 0;
	if ((blurFragment = createShader(shaderDir + "/BlurFragmentShader.glsl", GL_FRAGMENT_SHADER)) == 0) return 0;

	blurPrograms[0] = glCreateProgram();
	glAttachShader(blurPrograms[0], verticalBlur);
	glAttachShader(blurPrograms[0], blurFragment);

	blurPrograms[1] = glCreateProgram();
	glAttachShader(blurPrograms[1], horizontalBlur);
	glAttachShader(blurPrograms[1], blurFragment);

	for (int i=0;i<2;i++)
	{
		glLinkProgram(blurPrograms[i]);
		glGetProgramiv(blurPrograms[i], GL_LINK_STATUS, &link_ok);
		if (!link_ok) {
			fprintf(stderr, "glLinkProgram failed: %d \n",blurPrograms[i]);
		}

		glValidateProgram(blurPrograms[i]);
		glGetProgramiv(blurPrograms[i], GL_VALIDATE_STATUS, &validate_ok); 
		if (!validate_ok) {
			fprintf(stderr, "glValidateProgram failed: %d \n",blurPrograms[i]);
			cout << blurPrograms[i];
		}

		const char * attribute_name = "a_texCoord";
		attribute_v_coord_postproc[i] = glGetAttribLocation(blurPrograms[i], attribute_name);
		if (attribute_v_coord_postproc[i] == -1) {
			fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
		}

		const char * uniform_name = "s_texture";
		uniform_fbo_texture[i] = glGetUniformLocation(blurPrograms[i], uniform_name);
		if (uniform_fbo_texture[i] == -1) {
			fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
		}

		const char * gauss_name = "gauss_scale";
		uniformGaussScale[i] = glGetUniformLocation(blurPrograms[i], gauss_name);
		if (uniformGaussScale[i] == -1) {
			fprintf(stderr, "Could not bind uniform %s\n", gauss_name);
		}

		uniformColorScale[i] = glGetUniformLocation(blurPrograms[i], "colorScale");
		if (uniformColorScale[i] == -1) {
			fprintf(stderr, "Could not bind uniform %s\n", "colorScale");
		}
	}
}


GLuint MainContext::createShader(string filename,GLenum type)
{	
	GLuint shader = glCreateShader(type);

	std::ifstream inf;
	inf.open(filename,std::ios::in);	
	stringstream ss;
	ss << inf.rdbuf();
	ss << "\0";
	inf.close();

	Logger::stream("MAIN","INFO") << "Compiling shader: " << ss.str() << endl;
	fprintf(stderr, "Compiling shader %s\n",ss.str().c_str());

	string vsStr = ss.str();
	int vsLength = vsStr.length();
	const GLchar * vs_char = (const GLchar*)vsStr.c_str();

	glShaderSource(shader,1,&vs_char,NULL);
	glCompileShader(shader);

	GLsizei length;
	GLchar * infoLog = new GLchar[1000];
	glGetShaderInfoLog(shader,1000,&length,infoLog);

	stringstream shaderInfo;
	for (int i=0;i<length;i++)
	{
		shaderInfo << infoLog[i];
	}
	string infoString = shaderInfo.str();
	fprintf(stderr,"Shader compilation log %s",infoString.c_str());

	return shader;
}



void MainContext::destroyGraphics()
{
	disposeShaders();
	glfwDestroyWindow(GraphicsContext::getInstance().MainWindow);
	GraphicsContext::getInstance().MainWindow = NULL;
}

void MainContext::initGraphics()
{
	if (!GlobalConfig::tree()->get<bool>("GraphicsSettings.OverrideResolution"))
	{
		GlobalConfig::ScreenWidth  = glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
		GlobalConfig::ScreenHeight = glfwGetVideoMode(glfwGetPrimaryMonitor())->height;
	}
	else
	{
		GlobalConfig::ScreenWidth  = GlobalConfig::tree()->get<int>("GraphicsSettings.OverrideWidth");
		GlobalConfig::ScreenHeight = GlobalConfig::tree()->get<int>("GraphicsSettings.OverrideHeight");
	}

	if(!initializeWindow(GlobalConfig::ScreenWidth, GlobalConfig::ScreenHeight, GlobalConfig::tree()->get<bool>("GraphicsSettings.Fullscreen"))) 
	{
		throw new std::runtime_error("OpenGL didn't initialize!");
	}
	
	int fbw,fbh;
	glfwGetFramebufferSize(GraphicsContext::getInstance().MainWindow,&fbw,&fbh);

	createFrameBuffers(cv::Size2i(fbw,fbh));
	initShaders();

	InputEventHandler::getInstance().init(GraphicsContext::getInstance().MainWindow);

#ifndef _WIN32
	makeFullscreen(GraphicsContext::getInstance().MainWindow);
#endif
}



bool MainContext::initializeWindow( int window_width, int window_height, bool isFull)
{
	auto conf = GlobalConfig::tree()->get_child("GraphicsSettings");

	glfwWindowHint(GLFW_SAMPLES,conf.get<int>("FSAASamples"));
	glfwWindowHint(GLFW_RESIZABLE,conf.get<bool>("ResizableWindow"));

	GLFWwindow * handle = glfwCreateWindow(window_width, window_height,conf.get<string>("WindowTitle").c_str(),(isFull) ? glfwGetPrimaryMonitor() : NULL, NULL);

	if (handle == NULL)
	{	
		handleFatalError("Unable to initialize OpenGL. Please ensure your graphics drivers are up to date.",0);
		return false;
	}

	glfwMakeContextCurrent(handle);
	GraphicsContext::getInstance().MainWindow = handle;

	glfwSwapInterval(1);

	glewExperimental=true;
	glewInit();
	if (conf.get<bool>("EnableLineSmoothing"))
	{
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	}
	else
	{
		glDisable(GL_LINE_SMOOTH);
	}

	glEnable(GL_TEXTURE_2D);  
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glHint(GL_TEXTURE_COMPRESSION_HINT,  GL_NICEST);

	glDepthFunc(GL_LESS);

	glAlphaFunc(GL_NEVER + conf.get<int>("AlphaFunc"), conf.get<float>("AlphaFuncVal"));

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Color bg(GlobalConfig::tree()->get_child("GraphicsSettings.ClearColor"));
	float * color = bg.getFloat();
	glClearColor(color[0],color[1], color[2], 0.0f);

	glfwGetFramebufferSize(handle,&frameBufferWidth,&frameBufferHeight);

	glViewport(0, 0, frameBufferWidth, frameBufferHeight);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, frameBufferWidth, frameBufferHeight, 0.0f, -100, 256.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	return true;
} 


void MainContext::updateWindowSize(cv::Size2i newSize)
{
	Logger::stream("INFO","Main") << "Window size is " << newSize.width << "," << newSize.height << endl;

	GlobalConfig::ScreenWidth = newSize.width;
	GlobalConfig::ScreenHeight = newSize.height;
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, newSize.width, newSize.height, 0.0f, -100, 256.0);
}

void MainContext::updateFrameBufferSize(cv::Size2i newSize)
{	
	Logger::stream("INFO","Main") << "FrameBuffer size is " << newSize.width << "," << newSize.height << endl;
	glViewport(0, 0, newSize.width, newSize.height);
	frameBufferHeight = newSize.height;
	frameBufferWidth = newSize.width;

	destroyFrameBuffers();
	createFrameBuffers(newSize);
}