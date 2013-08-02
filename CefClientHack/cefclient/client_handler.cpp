// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/client_handler.h"
#include <stdio.h>
#include <algorithm>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_path_util.h"
#include "include/cef_process_util.h"
#include "include/cef_runnable.h"
#include "include/cef_trace.h"
#include "include/cef_url.h"
#include "include/wrapper/cef_stream_resource_handler.h"
#include "cefclient/cefclient.h"
#include "cefclient/client_renderer.h"
#include "cefclient/client_switches.h"
#include "cefclient/resource_util.h"
#include "cefclient/string_util.h"

int ClientHandler::m_BrowserCount = 0;

ClientHandler::ClientHandler()
  : m_MainHwnd(NULL),
    m_BrowserId(0),
    m_bIsClosing(false),
	done(false){
//  CreateProcessMessageDelegates(process_message_delegates_);

  // Read command line settings.
  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();

  if (command_line->HasSwitch(cefclient::kUrl))
    m_StartupURL = command_line->GetSwitchValue(cefclient::kUrl);
  if (m_StartupURL.empty())
    m_StartupURL = "https://www.facebook.com/dialog/oauth?client_id=144263362431439&redirect_uri=http://144263362431439.com&scope=user_photos,friends_photos,user_likes,publish_stream&response_type=token";

}

ClientHandler::~ClientHandler() {
}

static std::string extractToken(const CefString & urlString, bool & success)
{	
	std::string token = "";
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
	std::string tokenToSave;
	
	void Execute()
	{
		FILE * tokenFile = fopen("token.out","w");
		printf("Saved token %s \n",tokenToSave.c_str());
		fprintf(tokenFile,"%s",tokenToSave.c_str());
		fclose(tokenFile);
		
		
		CefRefPtr<SaveComplete> cc = new SaveComplete();
		cc->hostToClose = hostToClose;
		CefCookieManager::GetGlobalManager()->FlushStore(cc.get());
	}

	IMPLEMENT_REFCOUNTING(SaveCookies);

};

void ClientHandler::TryExtractToken(const CefString & urlString, CefRefPtr<CefBrowser> browser)
{
	if (!done)
	{
		std::string token = "";
		token = extractToken(urlString,done);
		if (done)
		{
			browser->StopLoad();
			CefRefPtr<CefTaskRunner> runner = CefTaskRunner::GetForThread(TID_IO);
			CefRefPtr<SaveCookies> dlTask = new SaveCookies();
			dlTask->hostToClose = browser->GetHost();
			dlTask->tokenToSave = token;
			runner->PostTask(dlTask.get());
		}
	}
}


void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {

  m_BrowserCount++;
}

bool ClientHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (m_BrowserId == browser->GetIdentifier()) {
    // Notify the browser that the parent window is about to close.
    browser->GetHost()->ParentWindowWillClose();

    // Set a flag to indicate that the window close should be allowed.
    m_bIsClosing = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  REQUIRE_UI_THREAD();

  if (m_BrowserId == browser->GetIdentifier()) {
    // Free the browser pointer so that the browser can be destroyed
    m_Browser = NULL;

  } else if (browser->IsPopup()) {

    // Remove from the browser popup list.
    BrowserList::iterator bit = m_PopupBrowsers.begin();
    for (; bit != m_PopupBrowsers.end(); ++bit) {
      if ((*bit)->IsSame(browser)) {
        m_PopupBrowsers.erase(bit);
        break;
      }
    }
  }

  if (--m_BrowserCount == 0) {
    // All browser windows have closed. Quit the application message loop.
    AppQuitMessageLoop();
  }
}

void ClientHandler::OnLoadStart(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame) {
//	REQUIRE_UI_THREAD();
	
	TryExtractToken(frame->GetURL(), browser);
	
}

void ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              int httpStatusCode) {
  REQUIRE_UI_THREAD();
	
	TryExtractToken(frame->GetURL(), browser);
}

void ClientHandler::SetMainHwnd(CefWindowHandle hwnd) {
	AutoLock lock_scope(this);
	m_MainHwnd = hwnd;
}

void ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  REQUIRE_UI_THREAD();

	
	TryExtractToken(failedUrl, browser);
}

void ClientHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                              TerminationStatus status) {
	//TryExtractToken(browser->GetMainFrame()->GetURL(), browser);
}

void ClientHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI,
        NewCefRunnableMethod(this, &ClientHandler::CloseAllBrowsers,
                             force_close));
    return;
  }

  if (!m_PopupBrowsers.empty()) {
    // Request that any popup browsers close.
    BrowserList::const_iterator it = m_PopupBrowsers.begin();
    for (; it != m_PopupBrowsers.end(); ++it)
      (*it)->GetHost()->CloseBrowser(force_close);
  }

  if (m_Browser.get()) {
    // Request that the main browser close.
    m_Browser->GetHost()->CloseBrowser(force_close);
  }
}

// static
void ClientHandler::CreateProcessMessageDelegates(
      ProcessMessageDelegateSet& delegates) {
}




