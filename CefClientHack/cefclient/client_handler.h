// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_CLIENT_HANDLER_H_
#define CEF_TESTS_CEFCLIENT_CLIENT_HANDLER_H_
#pragma once

#include <list>
#include <map>
#include <set>
#include <string>
#include "include/cef_client.h"
#include "cefclient/util.h"


// Define this value to redirect all popup URLs to the main application browser
// window.
// #define TEST_REDIRECT_POPUP_URLS


// ClientHandler implementation.
class ClientHandler : public CefClient,
                      public CefLifeSpanHandler,
                      public CefLoadHandler{
 public:
  // Interface for process message delegates. Do not perform work in the
  // RenderDelegate constructor.
  class ProcessMessageDelegate : public virtual CefBase {
   public:
    // Called when a process message is received. Return true if the message was
    // handled and should not be passed on to other handlers.
    // ProcessMessageDelegates should check for unique message names to avoid
    // interfering with each other.
    virtual bool OnProcessMessageReceived(
        CefRefPtr<ClientHandler> handler,
        CefRefPtr<CefBrowser> browser,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) {
      return false;
    }
  };

  typedef std::set<CefRefPtr<ProcessMessageDelegate> >
      ProcessMessageDelegateSet;
						  
  ClientHandler();
  virtual ~ClientHandler();

  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE {
    return this;
  }
  // virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                        // CefProcessId source_process,
                                        // CefRefPtr<CefProcessMessage> message)
                                        // OVERRIDE;
										


  // CefLifeSpanHandler methods
  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

  // CefLoadHandler methods
  virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame) OVERRIDE;
  virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         int httpStatusCode) OVERRIDE;
  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& errorText,
                           const CefString& failedUrl) OVERRIDE;
  virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                         TerminationStatus status) OVERRIDE;

  void SetMainHwnd(CefWindowHandle hwnd);
  CefWindowHandle GetMainHwnd() { return m_MainHwnd; }

  CefRefPtr<CefBrowser> GetBrowser() { return m_Browser; }
  int GetBrowserId() { return m_BrowserId; }

  void TryExtractToken(const CefString & urlString,CefRefPtr<CefBrowser> browser);
  // Request that all existing browser windows close.
  void CloseAllBrowsers(bool force_close);

  // Returns true if the main browser window is currently closing. Used in
  // combination with DoClose() and the OS close notification to properly handle
  // 'onbeforeunload' JavaScript events during window close.
  bool IsClosing() { return m_bIsClosing; }

  // Returns the startup URL.
  std::string GetStartupURL() { return m_StartupURL; }

 protected:

  // Create all of ProcessMessageDelegate objects.
  static void CreateProcessMessageDelegates(
      ProcessMessageDelegateSet& delegates);


  // The child browser window
  CefRefPtr<CefBrowser> m_Browser;

  // List of any popup browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
  BrowserList m_PopupBrowsers;

  // The main frame window handle
  CefWindowHandle m_MainHwnd;

  // The child browser id
  int m_BrowserId;

  // True if the main browser window is currently closing.
  bool m_bIsClosing;
  bool done;

  // True if an editable field currently has focus.
  bool m_bFocusOnEditableField;

  // Registered delegates.
  ProcessMessageDelegateSet process_message_delegates_;

  // The startup URL.
  std::string m_StartupURL;

  // Number of currently existing browser windows. The application will exit
  // when the number of windows reaches 0.
  static int m_BrowserCount;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(ClientHandler);
  // Include the default locking implementation.
  IMPLEMENT_LOCKING(ClientHandler);
};

#endif  // CEF_TESTS_CEFCLIENT_CLIENT_HANDLER_H_
