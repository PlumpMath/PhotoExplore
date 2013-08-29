#include "MainContext.hpp"

using namespace Leap;

HandProcessor * HandProcessor::instance = NULL;
FBDataSource * FBDataSource::instance = NULL;
FacebookDataDisplay * FacebookDataDisplay::instance = NULL;
LeapInput * LeapInput::instance = NULL;


#ifdef _WIN32

int WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow) {
	
	CefMainArgs mainArgs;
	CefSettings settings;
	
	settings.multi_threaded_message_loop = true;
	settings.single_process = true;
	settings.command_line_args_disabled = true;
	
	CefInitialize(mainArgs, settings, NULL);

	glfwInit();

	GlobalConfig::getInstance().loadConfigFile("./config.json");

	MainContext mainContext;

	mainContext.initGraphics();

	if (GlobalConfig::tree()->get<bool>("FakeDataMode.Enable")) 
		FBDataSource::instance = new FakeDataSource();
	else
	{
		FBDataSource::instance = new FacebookLoader();
		if (GlobalConfig::tree()->get<bool>("Cef.PersistentCookiesEnabled"))
		{
			CefCookieManager::GetGlobalManager()->SetStoragePath(".",true);
		}
	}
	mainContext.initializeUI();
	return mainContext.run();
}
#else

int main(int argc, char * argv[]){

	//signal(SIGSEGV, handle);
	//signal(SIGBUS, handle);
	
	CefMainArgs mainArgs;
	CefSettings settings;

	settings.command_line_args_disabled = true;

	glfwInit();

	GlobalConfig::getInstance().loadConfigFile("./config.json");
//#ifdef _WIN32	
//	GlobalConfig::getInstance().loadConfigFile("./win_config.json");
//#else
//	GlobalConfig::getInstance().loadConfigFile("./mac_config.json");
//#endif

	MainContext mainContext;
	
	mainContext.initGraphics();

	if (GlobalConfig::tree()->get<bool>("FakeDataMode.Enable")) 
		FBDataSource::instance = new FakeDataSource();
	else
	{
		FBDataSource::instance = new FacebookLoader();
	}
	
	CefInitialize(mainArgs, settings, NULL);
	
	mainContext.initializeUI();
	return mainContext.run();

}

#endif