#ifndef CEFALOPOD_H_
#define CEFALOPOD_H_

#include <include/cef_client.h>
#include "Logger.hpp"
#ifdef _WIN32
#include <Windows.h>
#endif


class Cefalopod : public CefClient, public CefLifeSpanHandler, public  CefRequestHandler, public CefLoadHandler
{
public:
	bool done, loadedEnded, quit;

#ifdef _WIN32
	HWND browserHandle;
#endif

	std::string token;

    Cefalopod();
    ~Cefalopod();
		
	CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler();
	bool DoClose(CefRefPtr<CefBrowser> browser);	
	void OnAfterCreated(CefRefPtr<CefBrowser> browser);

	CefRefPtr<CefRequestHandler> GetRequestHandler();
	CefRefPtr<CefLoadHandler> GetLoadHandler();
	void OnLoadError(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,ErrorCode errorCode,const CefString& errorText,const CefString& failedUrl);
	void OnLoadStart(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame);
	void OnLoadEnd(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,int httpStatusCode);

	bool OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,CefRefPtr<CefRequest> request);
	
    IMPLEMENT_REFCOUNTING(Cefalopod);
};

#endif