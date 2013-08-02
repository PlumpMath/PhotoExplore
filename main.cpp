#define NDEBUG 1
#define TEST_MODE 0
#define LEAPIMAGE_DEBUG 1

#include "GLImport.h"

#include <string>
#include "FileManager.h"
#include "SDLTimer.h"
#include <sstream>
#include "Leap.h"
#include "PointableElementManager.h"
#include "LeapDebug.h"
#include "LeapListenerImpl.h"
#include "LeapStartScreen.h"

#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/cef_app.h>
#include <include/cef_urlrequest.h>

#include "FBNode.h"
#include "FacebookLoader.h"

#include <boost/filesystem.hpp>
#include "ShakeGestureDetector.hpp"

#include "GraphicContext.hpp"

#include "GlobalConfig.hpp"
#include "SwipeGestureDetector.hpp"
#include "FakeDataSource.hpp"
#include "FacebookDataDisplay.hpp"
#include "FacebookBrowser.hpp"

#include "FBDataCursor.hpp"

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>

using namespace Leap;


int GlobalConfig::ScreenWidth = 1920;
int GlobalConfig::ScreenHeight = 1080;
float GlobalConfig::SteadyVelocity = 100;
bool GlobalConfig::LeftHanded = false;
bool GlobalConfig::AllowSingleHandInteraction = !true;
float GlobalConfig::SelectCircleMinRadius = 50.0f;
float GlobalConfig::MinimumInteractionScreenDistance = 400.0f;
std::string GlobalConfig::TestingToken = std::string("");

HandProcessor * HandProcessor::instance = NULL;
FileManager * FileManager::instance = NULL;
FBDataSource * FBDataSource::instance = NULL;
FacebookDataDisplay * FacebookDataDisplay::instance = NULL;
PointableElementManager * PointableElementManager::instance = NULL;

LeapDebug * LeapDebug::instance = NULL;
 

GLuint fbo[2], fbo_texture[2], rbo_depth[2];
GLuint vbo_fbo_vertices;
GLuint blurPrograms[2];
GLint link_ok,validate_ok;
GLuint verticalBlur,horizontalBlur, blurFragment;	
GLfloat fbo_vertices[] = {
	-1, -1,
	1, -1,
	-1,  1,
	1,  1,
};
GLuint attribute_v_coord_postproc[2], uniform_fbo_texture[2], uniformGaussScale[2],uniformColorScale[2];




#ifdef _WIN32

void handleFatalError(string errorText, int sig)
{
	string error = "Photo Explorer encountered a fatal error :(";
	MessageBox(NULL,errorText.c_str(),error.c_str(),0);
}

#else

void handle(int sig) {
	void *array[30];
	size_t size;
	
	// get void*'s for all entries on the stack
	size = backtrace(array, 30);
	
	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

void handleFatalError(string errorText, int sig) {
	handle(sig);
}

#endif


bool init( int window_width, int window_height, bool isFull)
{

	auto conf = GlobalConfig::tree()->get_child("GraphicsSettings");

	glfwOpenWindowHint(GLFW_FSAA_SAMPLES,conf.get<int>("FSAASamples"));

	int handle = glfwOpenWindow(window_width, window_height, 8, 8, 8,8,8,0, (isFull) ? GLFW_FULLSCREEN : GLFW_WINDOW);

	if (handle != GL_TRUE)
	{	
		handleFatalError("Unable to initialize OpenGL. Please ensure your graphics drivers are up to date.",0);
		return false;
	}

	glfwSetWindowTitle(conf.get<string>("WindowTitle").c_str());

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

	Color bg = Colors::WhiteSmoke;
	float * color = bg.getFloat();
	glClearColor(color[0],color[1], color[2], 0.0f);

	glViewport(0, 0, window_width, window_height);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, window_width, window_height, 0.0f, -300.0, 300.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


    return true;
} 

void clean_up()
{
	//glfwCloseWindow();
}

void configureController(const Controller & controller)
{

	Config con = controller.config();
	float f1 = GlobalConfig::tree()->get<float>("Leap.Gesture.Swipe.MinVelocity");
	con.setFloat("Gesture.Swipe.MinVelocity",f1);
	float f2 = GlobalConfig::tree()->get<float>("Leap.Gesture.Swipe.MinLength");	
	con.setFloat("Gesture.Swipe.MinLength",f2);
	
	float f1_a = con.getFloat("Gesture.Swipe.MinVelocity");
	float f2_a = con.getFloat("Gesture.Swipe.MinLength");

	con.save();
	//if (f1_a != f1 || f2_a != f2)
	//{
		Logger::stream("Main","ERROR") << "Configured. MinLength =" << f2_a << ", MinVel= " << f1_a << endl;
	//}
	
	controller.enableGesture(Gesture::Type::TYPE_SWIPE);
}

GLuint createShader(string filename,GLenum type)
{	
	GLuint shader = glCreateShader(type);

	std::ifstream inf;
	inf.open(filename,std::ios::in);	
    stringstream ss;
    ss << inf.rdbuf();
	inf.close();

	Logger::stream("MAIN","INFO") << "Compiling shader: " << ss.str() << endl;
	fprintf(stderr, "Compiling shader %s\n",ss.str().c_str());
	
	string vsStr = ss.str();
	int vsLength = vsStr.length();
	const GLchar * vs_char = (const GLchar*)vsStr.c_str();

	glShaderSource(shader,1,&vs_char,&vsLength);

	return shader;
}

void disposeShaders()
{
	glDeleteTextures(2,fbo_texture);
	glDeleteFramebuffers(2, fbo);
	glDeleteRenderbuffers(2, rbo_depth);
	
	glDeleteBuffers(1, &vbo_fbo_vertices);

	glDeleteProgram(blurPrograms[0]);
	glDeleteProgram(blurPrograms[1]);
}

int initShaders()
{
	

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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GlobalConfig::ScreenWidth , GlobalConfig::ScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, GlobalConfig::ScreenWidth, GlobalConfig::ScreenHeight);
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
//			s << blurPrograms[i];
			//return 0;
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
			//return 0;
		}

		const char * uniform_name = "s_texture";
		uniform_fbo_texture[i] = glGetUniformLocation(blurPrograms[i], uniform_name);
		if (uniform_fbo_texture[i] == -1) {
			fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
			//return 0;
		}

		const char * gauss_name = "gauss_scale";
		uniformGaussScale[i] = glGetUniformLocation(blurPrograms[i], gauss_name);
		if (uniformGaussScale[i] == -1) {
			fprintf(stderr, "Could not bind uniform %s\n", gauss_name);
			//return 0;
		}

		uniformColorScale[i] = glGetUniformLocation(blurPrograms[i], "colorScale");
		if (uniformColorScale[i] == -1) {
			fprintf(stderr, "Could not bind uniform %s\n", "colorScale");
			//return 0;
		}
	}
}

class MyVisitor : public CefCookieVisitor
{
public:
	bool Visit( const CefCookie& cookie, int count, int total, bool& deleteCookie )
	{
		deleteCookie = true;
		//cout << "Cookie[" << count << "]: Value = " << CefString(cookie.value.str).ToString() << " Domain = " << CefString(cookie.path.str).ToString() << "  Path = " << CefString(cookie.path.str).ToString() <<  "\n";
		return true;
	}

	IMPLEMENT_REFCOUNTING(MyVisitor);
};

class DeleteComplete : public CefCompletionHandler
{
public:
	void OnComplete()
	{
		CefShutdown();		
		GraphicsContext::getInstance().invokeApplicationExitCallback();
	}

	IMPLEMENT_REFCOUNTING(DeleteComplete);
};

class DeleteCookieTask : public CefTask {

public:
	void Execute()
	{		
		CefCookieManager::GetGlobalManager()->DeleteCookies("","");		
		CefRefPtr<DeleteComplete> cc = new DeleteComplete();
		CefCookieManager::GetGlobalManager()->FlushStore(cc.get());
	}

	IMPLEMENT_REFCOUNTING(DeleteCookieTask);

};

class ShutdownTask : public CefTask {

public:
	void Execute()
	{		
		CefShutdown();
		GraphicsContext::getInstance().invokeApplicationExitCallback();
	}

	IMPLEMENT_REFCOUNTING(ShutdownTask);

};

void runTests()
{

	FBNode * root =new FBNode("human");
	root->setNodeType("me");
	FBFriendsCursor * friendCursor = new FBFriendsCursor(root);

	int * count = new int[1];

	friendCursor->cursorChangedCallback = [friendCursor,count](){

		int itCount = 0;
		string lastId ="";
		while (friendCursor->state != FBDataCursor::Ended)
		{
			FBNode * result = friendCursor->getNext();
			if (result != NULL)
			{
				itCount++;
				(*count)++;
				string thisId = result->getId();

				if (lastId.compare(thisId)==0)
				{
					Logger::stream("test-mt","ERROR") << itCount << "-" << "Ids same!" << thisId << endl;
					break;
				}
				Logger::stream("test-mt","INFO") << itCount << "-" << thisId << endl;
				lastId = thisId;
			}
			else
				break;
		}
	};
	
	while (friendCursor->state == FBDataCursor::Local || friendCursor->state == FBDataCursor::Loading)
	{
		FBNode * result = friendCursor->getNext();
		if (result != NULL)
		{
			(*count)++;
			Logger::stream("test","INFO") << result->getId() << endl;
		}
		else
			break;
	}
	
	while ((*count) < 400)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}

}


#ifdef _WIN32

int WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow) {
	
#else
	
int main(int argc, char * argv[]){
		
	signal(SIGSEGV, handle);
	signal(SIGBUS, handle);
	
#endif
 
	
	
	CefMainArgs mainArgs;
	CefSettings settings;
	
#if defined(_WIN32)
	settings.multi_threaded_message_loop = true;
#endif
	settings.command_line_args_disabled = true;
	//settings.single_process = true;
	
	
	
	try
	{
		
	GlobalConfig::getInstance().loadConfigFile("config.json");
		
	CefInitialize(mainArgs, settings, NULL);
	
		
	glfwInit();
	
	GLFWvidmode vidMode;
	
	glfwGetDesktopMode(&vidMode);
	
	if (!GlobalConfig::tree()->get<bool>("GraphicsSettings.OverrideResolution"))
	{
		GlobalConfig::ScreenWidth  = vidMode.Width;
		GlobalConfig::ScreenHeight = vidMode.Height;
	}
	else
	{
		GlobalConfig::ScreenWidth  = GlobalConfig::tree()->get<int>("GraphicsSettings.OverrideWidth");
		GlobalConfig::ScreenHeight = GlobalConfig::tree()->get<int>("GraphicsSettings.OverrideHeight");
	}
	
		
		
	if(!init(GlobalConfig::ScreenWidth, GlobalConfig::ScreenHeight, GlobalConfig::tree()->get<bool>("GraphicsSettings.Fullscreen"))) return 1;
	
		
	initShaders();
		

	
	if (GlobalConfig::tree()->get<bool>("Cef.PersistentCookiesEnabled"))
		CefCookieManager::GetGlobalManager()->SetStoragePath(".",true);

	if (GlobalConfig::tree()->get<bool>("FakeDataMode.Enable")) 
		FBDataSource::instance = new FakeDataSource(GlobalConfig::tree()->get<string>("FakeDataMode.SourceDataDirectory"));
	else
		FBDataSource::instance = new FacebookLoader();

	FacebookDataDisplay::instance = new FacebookBrowser();

    bool * quit = new bool[1];
	quit[0] = false;
    Timer delta;
	int frameCount = 0, errorCount = 0;
	long lastFrameId = -1;

	std::string startDir = "."; 


	//Configure leap and attach listeners
	Leap::Controller controller = Leap::Controller();	
	configureController(controller);	
	controller.addListener(SwipeGestureDetector::getInstance());
	controller.addListener(ShakeGestureDetector::getInstance());

	HandProcessor * handProcessor = HandProcessor::getInstance();
	LeapDebug * leapDebug = new LeapDebug(handProcessor);	
	LeapDebug::instance = leapDebug;

	LeapStartScreen startScreen(startDir);
	startScreen.setFinishedCallback([quit](){
		quit[0] = true;
	});

	GraphicsContext::getInstance().applicationExitCallback = [quit](){ quit[0] = true; };
	
	Timer totalTime;
	totalTime.start();	
	delta.start();
	long frameId = 0;		
	int sampleCount = 2000;
		

	
	GraphicsContext::getInstance().globalActionCallback = [&](string s){ 
		
		if (s.compare("logout") == 0)
		{
			CefRefPtr<CefTaskRunner> runner = CefTaskRunner::GetForThread(TID_IO);
			CefRefPtr<DeleteCookieTask> dlTask = new DeleteCookieTask();
			runner->PostTask(dlTask.get());
		}
		else if (s.compare("exit") == 0)
		{
			CefRefPtr<CefTaskRunner> runner = CefTaskRunner::GetForThread(TID_IO);
			CefRefPtr<ShutdownTask> dlTask = new ShutdownTask();
			runner->PostTask(dlTask.get());
		}
		else if (s.compare("hide_tutorial") == 0)
		{
			GlobalConfig::tree()->put<bool>("Tutorial.Enabled",false);
			vector<string> x;
			LeapDebug::instance->setTutorialImages(x);
		}
	};

	try
	{
		
#if TEST_MODE
		runTests();
#else
		
			while(!(*quit))
			{
				stringstream frameOut;
				double deltaTime = delta.get_ticks();			
				delta.start();

				Timer itemTimer;
				itemTimer.start();

				if (glfwGetKey(GLFW_KEY_ESC) == GLFW_PRESS)
					quit[0] = true;
			
			
				HandProcessor::getInstance()->processFrame(controller.frame());
				PointableElementManager::getInstance()->processInputEvents();
				PointableElementManager::getInstance()->processFrame(controller,controller.frame());
				frameOut  << "PointableEvents = " << itemTimer.millis() << "ms \n";
					
				startScreen.onFrame(controller);
				leapDebug->onFrame(controller);
			
				ResourceManager::getInstance().update();
				frameOut  << "RMan = " << itemTimer.millis() << "ms \n";
				itemTimer.start();

				startScreen.update(deltaTime);
				frameOut  << "StartScreen = " << itemTimer.millis() << "ms \n";
				itemTimer.start();

			 
				if (GraphicsContext::getInstance().BlurRenderEnabled)
				{
					glClearColor(.4f,.4f,.4f, 1);

					GraphicsContext::getInstance().IsBlurCurrentPass = true;
					glUseProgram(0);
					glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
					glMatrixMode( GL_MODELVIEW );	
				
					startScreen.draw();

					for (int i=0;i<2;i++)
					{
						glUseProgram(blurPrograms[i]);

						if (i==1)
						{
							glBindFramebuffer(GL_FRAMEBUFFER, 0);
						}
						else
						{
							glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);				
						}
				
						glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

						glBindTexture(GL_TEXTURE_2D, fbo_texture[i]);
						glUniform1i(uniform_fbo_texture[i], 0);
						glUniform1f(uniformGaussScale[i],GraphicsContext::getInstance().getBlurScale());
						glUniform1f(uniformColorScale[i],0.9f);//GraphicsContext::getInstance().getDrawHint("ColorScale",false));
						glEnableVertexAttribArray(attribute_v_coord_postproc[i]);

						glBindBuffer(GL_ARRAY_BUFFER, vbo_fbo_vertices);
						glVertexAttribPointer(
							attribute_v_coord_postproc[i],  // attribute
							2,                  // number of elements per vertex, here (x,y)
							GL_FLOAT,           // the type of each element
							GL_FALSE,           // take our values as-is
							0,                  // no extra data between each position
							0                   // offset of first element
							);
						glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
						glDisableVertexAttribArray(attribute_v_coord_postproc[i]);
					}
				
					GraphicsContext::getInstance().IsBlurCurrentPass = false;			
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					glUseProgram(0);
					GraphicsContext::getInstance().doClearDraw();
				}
				else
				{		
					Color bg = Colors::WhiteSmoke;
					float * color = bg.getFloat();
					glClearColor(color[0],color[1], color[2], 1);

					GraphicsContext::getInstance().IsBlurCurrentPass = false;
					glUseProgram(0);			
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);			

					glMatrixMode( GL_MODELVIEW );			
					ShakeGestureDetector::getInstance().draw();		
					SwipeGestureDetector::getInstance().draw();
					if (GlobalConfig::tree()->get<bool>("Leap.HandModel.DrawDebug"))
						HandProcessor::getInstance()->draw();
					startScreen.draw();
				

				}
				//End
			
				frameOut << "MainDraw= " << itemTimer.millis() << "ms \n";
				itemTimer.start();
	 
				leapDebug->draw();
				frameOut << "DebugDraw = " << itemTimer.millis() << "ms \n";
				itemTimer.start();
					
				itemTimer.start();
				glFinish();
				frameOut  << "glFinish = " << itemTimer.millis() << "ms \n";
				itemTimer.start();
				glfwSwapBuffers();
				frameOut << "SwapBuffers = " << itemTimer.millis() << "ms \n";
				itemTimer.start();
			
			
				if (delta.millis() - (1000.0/60.0) > 2)
				{
					Logger::stream("MAIN","TIME") << "[" << frameId << "] START \n";
					Logger::stream("MAIN","TIME") << frameOut.str();
					Logger::stream("MAIN","TIME") << "[" << frameId << "] = " << delta.millis() << "ms \n"; //@ " << totalTime.seconds() << "s \n";
				}
				frameId++;
			}
	#endif
		}	
		catch( cv::Exception& e )
		{
			Logger::stream("MAIN","FATAL") << "Unhandled CV exception in render loop: " << e.what() << endl;
			stringstream error;
			error << "Unhandled CV exception in loop :" << e.what();
			handleFatalError(error.str(),3);
		}
		catch  (std::exception & e)
		{
			Logger::stream("MAIN","FATAL") << "Unhandled exception: " << e.what() << endl;
			stringstream error;
			error << "Unhandled exception during render loop :" << e.what();			
			handleFatalError(error.str(),3);
		}	
		
		startScreen.shutdown();

	}
	catch( cv::Exception& e )
	{
		Logger::stream("MAIN","FATAL") << "Unhandled CV exception: " << e.what() << endl;
		stringstream error;
		error << "Unhandled CV exception :" << e.what();
		handleFatalError(error.str(),3);
	} catch (std::exception & e)
	{
		Logger::stream("MAIN","FATAL") << "Unhandled exception: " << e.what() << endl;
		stringstream error;
		error << "Unhandled exception :" << e.what();
		handleFatalError(error.str(),3);
	}
	catch (...)
	{
		Logger::stream("MAIN","FATAL") << "Unhandled exception?? " << endl;
		handleFatalError("Unhandled and unknown exception",3);
	}
		
	clean_up();
	glfwCloseWindow();
	
	std::exit(0);

    return 0;
}
