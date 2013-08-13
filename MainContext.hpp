#ifndef LEAPIMAGE_MAIN_CONTEXT_HPP_
#define LEAPIMAGE_MAIN_CONTEXT_HPP_

#define NDEBUG 1
#define LEAPIMAGE_DEBUG 1

#include "GLImport.h"

#include <string>
#include <sstream>
#include <Leap.h>

#include <boost/filesystem.hpp>

#include <opencv2/opencv.hpp>

#include "LeapInput.hpp"
#include "LeapDebug.h"
#include "LeapStartScreen.h"
#include "FBNode.h"
#include "FacebookLoader.h"
#include "ShakeGestureDetector.hpp"
#include "GraphicContext.hpp"
#include "GlobalConfig.hpp"
#include "SwipeGestureDetector.hpp"
#include "FakeDataSource.hpp"
#include "FacebookDataDisplay.hpp"
#include "SDLTimer.h"
#include "FBDataCursor.hpp"
#include "FacebookBrowser.hpp"
#include "CefHelper.hpp"
#include "InputEventHandler.hpp"
#include "LeapStatusOverlay.hpp"

#ifndef _WIN32
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include "ns_hack.h"
#endif

using namespace Leap;

struct MainContext {

	//Shader members
	GLuint fbo[2], fbo_texture[2], rbo_depth[2];
	GLuint vbo_fbo_vertices;
	GLuint blurPrograms[2];
	GLint link_ok,validate_ok;
	GLuint verticalBlur,horizontalBlur, blurFragment;	
	GLuint attribute_v_coord_postproc[2], uniform_fbo_texture[2], uniformGaussScale[2],uniformColorScale[2];

	int frameBufferWidth,frameBufferHeight;

	//Other members	
	LeapStatusOverlay leapStatusOverlay;
	LeapStartScreen startScreen;
	bool * quit;
	cv::Point2i topLeftCorner;
	bool currentlyDrawing;

	MainContext()
	{
		currentlyDrawing = false;
	}

	#ifdef _WIN32

	void handleFatalError(string errorText, int sig)
	{
		string error = "PhotoExplore encountered a fatal error :(";
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
		
	GLuint createShader(string filename,GLenum type);
	void disposeShaders();
	int initShaders();
	void destroyFrameBuffers();
	int createFrameBuffers(cv::Size2i);
	void destroyGraphics();
	void initGraphics();
	bool initializeWindow( int window_width, int window_height, bool isFull);

	void updateWindowSize(cv::Size2i newSize);
	void updateFrameBufferSize(cv::Size2i newSize);

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
		controller.enableGesture(Gesture::Type::TYPE_SWIPE);
	}


	void initializeUI()
	{
		FacebookDataDisplay::instance = new FacebookBrowser();

		quit = new bool[1];
		quit[0] = false;
		
		leapStatusOverlay.layout(Vector(),cv::Size2f(GlobalConfig::ScreenWidth,GlobalConfig::ScreenHeight));
		startScreen.init();
	}

	void drawStage()
	{	
		if (currentlyDrawing)
			return;

		currentlyDrawing = true;
		bool doFinish = GlobalConfig::tree()->get<bool>("GraphicsSettings.ExecuteGLFinish");
		Timer itemTimer;
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

			startScreen.draw();
		}
		//End

		LeapDebug::getInstance().draw();
		LeapDebug::getInstance().plotValue("Draw",Colors::HotPink,itemTimer.millis() * 20);

		leapStatusOverlay.draw();

		itemTimer.start();
		if (doFinish)
			glFinish();

		LeapDebug::getInstance().plotValue("Finish",Colors::Magenta,itemTimer.millis() * 20);

		itemTimer.start();
		glfwSwapBuffers(GraphicsContext::getInstance().MainWindow);
		LeapDebug::getInstance().plotValue("Swap",Colors::PrettyPurple, itemTimer.millis() * 20);
		currentlyDrawing = false;
	}

	static void onWindowShouldClose(GLFWwindow * window)
	{
		GraphicsContext::getInstance().invokeGlobalAction("exit");
	}
	
	void initCallbacks()
	{

		GraphicsContext::getInstance().applicationExitCallback = [this](){quit[0] = true;};

		InputEventHandler::getInstance().addKeyCallback([&](GLFWwindow * window, int key, int scancode, int action, int mods) -> bool {

			if (key == GLFW_KEY_ESCAPE)
			{
				quit[0] = true;
				return true;
			}
			return false;
		});

		InputEventHandler::getInstance().addWindowPositionCallback([this](GLFWwindow * window, int xPos, int yPos) -> bool{
	
			this->drawStage();	
			return false;

		});

		InputEventHandler::getInstance().addWindowSizeCallback([this](GLFWwindow * window, int newWidth, int newHeight) -> bool{
	
			if (newWidth != GlobalConfig::ScreenWidth || newHeight != GlobalConfig::ScreenHeight)
			{
				updateWindowSize(cv::Size2i(newWidth,newHeight));

				GraphicsContext::getInstance().setDrawHint("SkipLayoutAnimation",1);
				startScreen.layout(Vector(),cv::Size2f(newWidth,newHeight));
				leapStatusOverlay.layout(Vector(),cv::Size2f(newWidth,newHeight));
				LeapDebug::getInstance().layoutTutorial();			
				GraphicsContext::getInstance().setDrawHint("SkipLayoutAnimation",0);

				this->drawStage();

				return true;
			}
			return false;
		});

		
		InputEventHandler::getInstance().addFrameBufferSizeCallback([this](GLFWwindow * window, int newWidth, int newHeight) -> bool{
	
			if (frameBufferWidth != newWidth || frameBufferHeight != newHeight)
			{
				updateFrameBufferSize(cv::Size2i(newWidth,newHeight));
				return true;
			}
			return false;
		});

		glfwSetWindowCloseCallback(GraphicsContext::getInstance().MainWindow,MainContext::onWindowShouldClose);

		GraphicsContext::getInstance().globalActionCallback = [&](string s){ 

			if (s.compare("full") == 0)
			{
				bool full = GlobalConfig::tree()->get<bool>("GraphicsSettings.Fullscreen");
				GlobalConfig::tree()->put<bool>("GraphicsSettings.Fullscreen",!full);

				ResourceManager::getInstance().unloadTextures();
				destroyGraphics();

				initGraphics();
				ResourceManager::getInstance().reloadTextures();				
			}
			else if (s.compare("hide") == 0)
			{
				ResourceManager::getInstance().unloadTextures();
				destroyGraphics();
			}
			else if (s.compare("show") == 0)
			{				
				initGraphics();
				ResourceManager::getInstance().reloadTextures();
			}
			else if (s.compare("logout") == 0)
			{
	#ifdef _WIN32
				CefRefPtr<CefTaskRunner> runner = CefTaskRunner::GetForThread(TID_IO);
				CefRefPtr<DeleteCookieTask> dlTask = new DeleteCookieTask();
				runner->PostTask(dlTask.get());
	#else
				remove("Cookies");
				remove("Cookies-Journal");
				quit[0] = true;
	#endif
			}
			else if (s.compare("exit") == 0)
			{
	#ifdef _WIN32
				CefRefPtr<CefTaskRunner> runner = CefTaskRunner::GetForThread(TID_IO);
				CefRefPtr<ShutdownTask> dlTask = new ShutdownTask();
				runner->PostTask(dlTask.get());
	#else
				quit[0] = true;
	#endif
			}
			else if (s.compare("hide_tutorial") == 0)
			{
				GlobalConfig::tree()->put<bool>("Tutorial.Enabled",false);
				vector<string> x;
				LeapDebug::getInstance().setTutorialImages(x);
			}
		};
	}



	int run()
	{			
		try
		{	
			//Configure leap and attach listeners
			Leap::Controller controller = Leap::Controller();	
			configureController(controller);	
			controller.addListener(SwipeGestureDetector::getInstance());
			controller.addListener(ShakeGestureDetector::getInstance());
			controller.addListener(LeapDebug::getInstance());

			LeapInput::getInstance()->setTopLevelElement(&startScreen);
		
			initCallbacks();		
				
			try
			{	
				Timer delta,itemTimer;
				while(!(*quit))
				{				
					delta.start();
				
					itemTimer.start();
					HandProcessor::getInstance()->processFrame(controller.frame());
					LeapInput::getInstance()->processInputEvents();
					LeapInput::getInstance()->processFrame(controller,controller.frame());			
					LeapDebug::getInstance().plotValue("Input",Colors::White,itemTimer.millis() * 20);

					itemTimer.start();
					ResourceManager::getInstance().update();
					LeapDebug::getInstance().plotValue("RMan",Colors::Red,itemTimer.millis() * 20);

					itemTimer.start();

					startScreen.update(0);				
					itemTimer.start();
								
					leapStatusOverlay.onFramePoll(controller);
					leapStatusOverlay.update();				

					GLFWwindow * mainWindow = GraphicsContext::getInstance().MainWindow;
					if (mainWindow != NULL)
					{
						glfwMakeContextCurrent(mainWindow);
						glfwPollEvents();

						drawStage();
					
						LeapDebug::getInstance().plotValue("aFPS",Colors::HoloBlueBright, delta.millis() * 20);		
					}
				}
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


		destroyGraphics();

		glfwTerminate();
		std::exit(0);

		return 0;
	}

};

#endif