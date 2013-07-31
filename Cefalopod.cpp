#include "Cefalopod.h"

#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/cef_app.h>
#include <include/cef_urlrequest.h>

Cefalopod::Cefalopod()
{	
	quit = loadedEnded = done = false;
	token = std::string("");			
}

Cefalopod::~Cefalopod() 
{

}

CefRefPtr<CefLifeSpanHandler> Cefalopod::GetLifeSpanHandler()
{
	return CefRefPtr<CefLifeSpanHandler>(this);
}

CefRefPtr<CefLoadHandler> Cefalopod::GetLoadHandler()
{
	return CefRefPtr<Cefalopod>(this);
}

bool Cefalopod::OnBeforePopup(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             const CefString& target_url,
                             const CefString& target_frame_name,
                             const CefPopupFeatures& popupFeatures,
                             CefWindowInfo& windowInfo,
                             CefRefPtr<CefClient>& client,
                             CefBrowserSettings& settings,
                             bool* no_javascript_access) 
{
	return true;
}

HWND Cefalopod::getBrowserHandle()
{
	//return browser->GetHost()->GetWindowHandle();
	return browserHandle;
}

bool Cefalopod::DoClose(CefRefPtr<CefBrowser> browser)
{
	quit = true;
	//browser->GetHost()->CloseBrowser(false);
	return false; 
}

void Cefalopod::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	Logger::stream("Cefalopod","INFO") << "OnAfterCreated" << endl;	
#ifdef _WIN32
	browserHandle = browser->GetHost()->GetWindowHandle();
	SwitchToThisWindow(browser->GetHost()->GetWindowHandle(),false);
	ShowWindow(browserHandle,3);
	browser->GetHost()->SetFocus(true);
#endif
}

class SaveComplete : public CefCompletionHandler
{
public:
	CefRefPtr<CefBrowserHost> hostToClose;
	void OnComplete()
	{
		if (hostToClose != NULL)
			hostToClose->CloseBrowser(true);
	}

	IMPLEMENT_REFCOUNTING(SaveComplete);
};

class SaveCookies : public CefTask {


public:
	CefRefPtr<CefBrowserHost> hostToClose;
	
	void Execute()
	{		
		CefRefPtr<SaveComplete> cc = new SaveComplete();
		cc->hostToClose = hostToClose;
		CefCookieManager::GetGlobalManager()->FlushStore(cc.get());
	}

	IMPLEMENT_REFCOUNTING(SaveCookies);

};

static string extractToken(const CefString & urlString, bool & success)
{	
	string token = "";
	success = false;
	int tokenIndex = urlString.ToString().find("access_token=");
	int tokenEnd = urlString.ToString().find_first_of("&");
	if (tokenIndex > 0)
	{
		tokenIndex += 13;
		token = urlString.ToString().substr(tokenIndex,tokenEnd-tokenIndex);
		success = true;
	}
	return token;
}

void Cefalopod::OnLoadError(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,ErrorCode errorCode,const CefString& errorText,const CefString& failedUrl)
{		
	Logger::stream("Cefalopod","INFO") << "OnLoadError" << endl;
	
	if (!done)
		token = extractToken(failedUrl,done);

	if (done)
	{
		CefRefPtr<CefTaskRunner> runner = CefTaskRunner::GetForThread(TID_IO);
		CefRefPtr<SaveCookies> dlTask = new SaveCookies();
		dlTask->hostToClose = browser->GetHost();
		runner->PostTask(dlTask.get());
	}
}

void Cefalopod::OnLoadStart(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame) 
{
	Logger::stream("Cefalopod","INFO") << "OnLoadStart. URL=" << frame->GetURL().ToString() << endl;
	
	if (!done)
	{
		token = extractToken(frame->GetURL(),done);
	}

	if (done)
	{
		browser->StopLoad();
		CefRefPtr<CefTaskRunner> runner = CefTaskRunner::GetForThread(TID_IO);
		CefRefPtr<SaveCookies> dlTask = new SaveCookies();
		dlTask->hostToClose = browser->GetHost();
		runner->PostTask(dlTask.get());
	}
}

void Cefalopod::OnLoadEnd(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame, int httpStatusCode)
{
	Logger::stream("Cefalopod","INFO") << "OnLoadEnd. Code = " << httpStatusCode << " URL=" << frame->GetURL().ToString() << endl;
	
	if (!done)
		token = extractToken(frame->GetURL(),done);

	if (done)
	{
		browser->StopLoad();
		CefRefPtr<CefTaskRunner> runner = CefTaskRunner::GetForThread(TID_IO);
		CefRefPtr<SaveCookies> dlTask = new SaveCookies();
		dlTask->hostToClose = browser->GetHost();
		runner->PostTask(dlTask.get());
	}
}

