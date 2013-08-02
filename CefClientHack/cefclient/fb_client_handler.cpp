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
#include "cefclient/binding_test.h"
#include "cefclient/cefclient.h"
#include "cefclient/client_renderer.h"
#include "cefclient/client_switches.h"
#include "cefclient/dialog_test.h"
#include "cefclient/dom_test.h"
#include "cefclient/resource_util.h"
#include "cefclient/string_util.h"
#include "cefclient/window_test.h"

int ClientHandler::m_BrowserCount = 0;

ClientHandler::ClientHandler()
  : m_MainHwnd(NULL),
    m_BrowserId(0),
    m_bIsClosing(false),
    m_EditHwnd(NULL),
    m_BackHwnd(NULL),
    m_ForwardHwnd(NULL),
    m_StopHwnd(NULL),
    m_ReloadHwnd(NULL),
    m_bFocusOnEditableField(false) {
  CreateProcessMessageDelegates(process_message_delegates_);

  // Read command line settings.
  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();

  if (command_line->HasSwitch(cefclient::kUrl))
    m_StartupURL = command_line->GetSwitchValue(cefclient::kUrl);
  if (m_StartupURL.empty())
    m_StartupURL = "http://www.google.com/";

  // Also use external dev tools if off-screen rendering is enabled since we
  // disallow popup windows.
  m_bExternalDevTools =
      command_line->HasSwitch(cefclient::kExternalDevTools) ||
      AppIsOffScreenRenderingEnabled();

  m_bMouseCursorChangeDisabled =
      command_line->HasSwitch(cefclient::kMouseCursorChangeDisabled);
}

ClientHandler::~ClientHandler() {
}

bool ClientHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
  // Check for messages from the client renderer.
  std::string message_name = message->GetName();
  if (message_name == client_renderer::kFocusedNodeChangedMessage) {
    // A message is sent from ClientRenderDelegate to tell us whether the
    // currently focused DOM node is editable. Use of |m_bFocusOnEditableField|
    // is redundant with CefKeyEvent.focus_on_editable_field in OnPreKeyEvent
    // but is useful for demonstration purposes.
    m_bFocusOnEditableField = message->GetArgumentList()->GetBool(0);
    return true;
  }

  bool handled = false;

  // Execute delegate callbacks.
  ProcessMessageDelegateSet::iterator it = process_message_delegates_.begin();
  for (; it != process_message_delegates_.end() && !handled; ++it) {
    handled = (*it)->OnProcessMessageReceived(this, browser, source_process,
                                              message);
  }

  return handled;
}

bool ClientHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  const CefString& target_url,
                                  const CefString& target_frame_name,
                                  const CefPopupFeatures& popupFeatures,
                                  CefWindowInfo& windowInfo,
                                  CefRefPtr<CefClient>& client,
                                  CefBrowserSettings& settings,
                                  bool* no_javascript_access) {
  if (browser->GetHost()->IsWindowRenderingDisabled()) {
    // Cancel popups in off-screen rendering mode.
    return true;
  }
  return false;
}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  REQUIRE_UI_THREAD();

  // Disable mouse cursor change if requested via the command-line flag.
  if (m_bMouseCursorChangeDisabled)
    browser->GetHost()->SetMouseCursorChangeDisabled(true);

  AutoLock lock_scope(this);
  if (!m_Browser.get())   {
    // We need to keep the main child window, but not popup windows
    m_Browser = browser;
    m_BrowserId = browser->GetIdentifier();
  } else if (browser->IsPopup()) {
    // Add to the list of popup browsers.
    m_PopupBrowsers.push_back(browser);
  }

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

void ClientHandler::OnLoadStart(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame) {
  REQUIRE_UI_THREAD();

  if (m_BrowserId == browser->GetIdentifier() && frame->IsMain()) {
    // We've just started loading a page
    SetLoading(true);
  }
}

void ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              int httpStatusCode) {
  REQUIRE_UI_THREAD();

  if (m_BrowserId == browser->GetIdentifier() && frame->IsMain()) {
    // We've just finished loading a page
    SetLoading(false);

    // Continue the DOM test.
    if (frame->GetURL() == dom_test::kTestUrl)
      dom_test::OnLoadEnd(browser);
  }
}

void ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  REQUIRE_UI_THREAD();

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED)
    return;

  // Don't display an error for external protocols that we allow the OS to
  // handle. See OnProtocolExecution().
  if (errorCode == ERR_UNKNOWN_URL_SCHEME) {
    std::string urlStr = frame->GetURL();
    if (urlStr.find("spotify:") == 0)
      return;
  }

  // Display a load error message.
  std::stringstream ss;
  ss << "<html><body><h2>Failed to load URL " << std::string(failedUrl) <<
        " with error " << std::string(errorText) << " (" << errorCode <<
        ").</h2></body></html>";
  frame->LoadString(ss.str(), failedUrl);
	
	FILE * pFile;	
	pFile = fopen ("result.txt","w");
	fprintf(pFile, "URL=%s",failedUrl.ToString().c_str());
}

void ClientHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                              TerminationStatus status) {
  // Load the startup URL if that's not the website that we terminated on.
  CefRefPtr<CefFrame> frame = browser->GetMainFrame();
  std::string url = frame->GetURL();
  std::transform(url.begin(), url.end(), url.begin(), tolower);

  std::string startupURL = GetStartupURL();
  if (url.find(startupURL) != 0)
    frame->LoadURL(startupURL);
}
void ClientHandler::SetEditHwnd(CefWindowHandle hwnd) {
  AutoLock lock_scope(this);
  m_EditHwnd = hwnd;
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
  // Create the binding test delegates.
  binding_test::CreateProcessMessageDelegates(delegates);

  // Create the dialog test delegates.
  dialog_test::CreateProcessMessageDelegates(delegates);

  // Create the window test delegates.
  window_test::CreateProcessMessageDelegates(delegates);
}


