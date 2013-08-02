#ifndef CEFALOPOD_H_
#define CEFALOPOD_H_

#include <include/cef_client.h>
#include "Logger.hpp"
#ifdef WIN_32_
#include <Windows.h>
#endif


class Cefalopod : public CefClient, public CefLifeSpanHandler, public CefLoadHandler
{
public:
	bool done, loadedEnded, quit;

	std::string token;

    Cefalopod();
    ~Cefalopod();
		
	CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler();
	bool DoClose(CefRefPtr<CefBrowser> browser);	
	void OnAfterCreated(CefRefPtr<CefBrowser> browser);

	CefRefPtr<CefLoadHandler> GetLoadHandler();
	void OnLoadError(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,ErrorCode errorCode,const CefString& errorText,const CefString& failedUrl);
	void OnLoadStart(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame);
	void OnLoadEnd(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,int httpStatusCode);

	bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             const CefString& target_url,
                             const CefString& target_frame_name,
                             const CefPopupFeatures& popupFeatures,
                             CefWindowInfo& windowInfo,
                             CefRefPtr<CefClient>& client,
                             CefBrowserSettings& settings,
                             bool* no_javascript_access);
	
    IMPLEMENT_REFCOUNTING(Cefalopod);
};

#endif