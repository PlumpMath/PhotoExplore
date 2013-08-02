// Copyright (c) 2013 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#include <sstream>
#include "cefclient/cefclient.h"
#include "include/cef_app.h"
#import "include/cef_application_mac.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_runnable.h"
#include "cefclient/client_handler.h"
#include "cefclient/client_switches.h"
#include "cefclient/resource_util.h"
#include "cefclient/scheme_test.h"
#include "cefclient/string_util.h"

// The global ClientHandler reference.
extern CefRefPtr<ClientHandler> g_handler;


char szWorkingDir[512];   // The current working directory

// Content area size for newly created windows.
const int kWindowWidth = 800;
const int kWindowHeight = 600;

// Provide the CefAppProtocol implementation required by CEF.
@interface ClientApplication : NSApplication<CefAppProtocol> {
@private
  BOOL handlingSendEvent_;
}
@end

@implementation ClientApplication
- (BOOL)isHandlingSendEvent {
  return handlingSendEvent_;
}

- (void)setHandlingSendEvent:(BOOL)handlingSendEvent {
  handlingSendEvent_ = handlingSendEvent;
}

- (void)sendEvent:(NSEvent*)event {
  CefScopedSendingEvent sendingEventScoper;
  [super sendEvent:event];
}
@end


// Receives notifications from controls and the browser window. Will delete
// itself when done.
@interface ClientWindowDelegate : NSObject <NSWindowDelegate>
- (IBAction)goBack:(id)sender;
- (IBAction)goForward:(id)sender;
- (IBAction)reload:(id)sender;
- (IBAction)stopLoading:(id)sender;
- (IBAction)takeURLStringValueFrom:(NSTextField *)sender;
- (void)alert:(NSString*)title withMessage:(NSString*)message;
- (void)notifyConsoleMessage:(id)object;
- (void)notifyDownloadComplete:(id)object;
- (void)notifyDownloadError:(id)object;
@end

@implementation ClientWindowDelegate

- (IBAction)goBack:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId())
    g_handler->GetBrowser()->GoBack();
}

- (IBAction)goForward:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId())
    g_handler->GetBrowser()->GoForward();
}

- (IBAction)reload:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId())
    g_handler->GetBrowser()->Reload();
}

- (IBAction)stopLoading:(id)sender {
  if (g_handler.get() && g_handler->GetBrowserId())
    g_handler->GetBrowser()->StopLoad();
}

- (IBAction)takeURLStringValueFrom:(NSTextField *)sender {
  if (!g_handler.get() || !g_handler->GetBrowserId())
    return;

  NSString *url = [sender stringValue];

  // if it doesn't already have a prefix, add http. If we can't parse it,
  // just don't bother rather than making things worse.
  NSURL* tempUrl = [NSURL URLWithString:url];
  if (tempUrl && ![tempUrl scheme])
    url = [@"http://" stringByAppendingString:url];

  std::string urlStr = [url UTF8String];
  g_handler->GetBrowser()->GetMainFrame()->LoadURL(urlStr);
}

- (void)alert:(NSString*)title withMessage:(NSString*)message {
  NSAlert *alert = [NSAlert alertWithMessageText:title
                                   defaultButton:@"OK"
                                 alternateButton:nil
                                     otherButton:nil
                       informativeTextWithFormat:message];
  [alert runModal];
}

- (void)notifyConsoleMessage:(id)object {
//  std::stringstream ss;
//  ss << "Console messages will be written to " << g_handler->GetLogFile();
//  NSString* str = [NSString stringWithUTF8String:(ss.str().c_str())];
//  [self alert:@"Console Messages" withMessage:str];
}

- (void)notifyDownloadComplete:(id)object {
//  std::stringstream ss;
//  ss << "File \"" << g_handler->GetLastDownloadFile() <<
//      "\" downloaded successfully.";
//  NSString* str = [NSString stringWithUTF8String:(ss.str().c_str())];
//  [self alert:@"File Download" withMessage:str];
}

- (void)notifyDownloadError:(id)object {
//  std::stringstream ss;
//  ss << "File \"" << g_handler->GetLastDownloadFile() <<
//      "\" failed to download.";
//  NSString* str = [NSString stringWithUTF8String:(ss.str().c_str())];
//  [self alert:@"File Download" withMessage:str];
}

- (void)windowDidBecomeKey:(NSNotification*)notification {
  if (g_handler.get() && g_handler->GetBrowserId()) {
    // Give focus to the browser window.
    g_handler->GetBrowser()->GetHost()->SetFocus(true);
  }
}

// Called when the window is about to close. Perform the self-destruction
// sequence by getting rid of the window. By returning YES, we allow the window
// to be removed from the screen.
- (BOOL)windowShouldClose:(id)window {
  if (g_handler.get() && !g_handler->IsClosing()) {
    CefRefPtr<CefBrowser> browser = g_handler->GetBrowser();
    if (browser.get()) {
      // Notify the browser window that we would like to close it. This
      // will result in a call to ClientHandler::DoClose() if the
      // JavaScript 'onbeforeunload' event handler allows it.
      browser->GetHost()->CloseBrowser(false);

      // Cancel the close.
      return NO;
    }
  }

  // Try to make the window go away.
  [window autorelease];

  // Clean ourselves up after clearing the stack of anything that might have the
  // window on it.
  [self performSelectorOnMainThread:@selector(cleanup:)
                         withObject:window
                      waitUntilDone:NO];

  // Allow the close.
  return YES;
}

// Deletes itself.
- (void)cleanup:(id)window {
  [self release];
}

@end


// Receives notifications from the application. Will delete itself when done.
@interface ClientAppDelegate : NSObject
- (void)createApp:(id)object;
@end

@implementation ClientAppDelegate

// Create the application on the UI thread.
- (void)createApp:(id)object {
  [NSApplication sharedApplication];
  [NSBundle loadNibNamed:@"MainMenu" owner:NSApp];

  // Set the delegate for application events.
  [NSApp setDelegate:self];

  // Add the Tests menu.


  // Create the delegate for control and browser window events.
  ClientWindowDelegate* delegate = [[ClientWindowDelegate alloc] init];

  // Create the main application window.
  NSRect screen_rect = [[NSScreen mainScreen] visibleFrame];
  NSRect window_rect = { {0, screen_rect.size.height - kWindowHeight},
    {kWindowWidth, kWindowHeight} };
  NSWindow* mainWnd = [[UnderlayOpenGLHostingWindow alloc]
                       initWithContentRect:window_rect
                       styleMask:(NSTitledWindowMask |
                                  NSClosableWindowMask |
                                  NSMiniaturizableWindowMask |
                                  NSResizableWindowMask )
                       backing:NSBackingStoreBuffered
                       defer:NO];
  [mainWnd setTitle:@"cefclient"];
  [mainWnd setDelegate:delegate];

  // Rely on the window delegate to clean us up rather than immediately
  // releasing when the window gets closed. We use the delegate to do
  // everything from the autorelease pool so the window isn't on the stack
  // during cleanup (ie, a window close from javascript).
  [mainWnd setReleasedWhenClosed:NO];

  NSView* contentView = [mainWnd contentView];
	
  // Create the handler.
  g_handler = new ClientHandler();
  g_handler->SetMainHwnd(contentView);
//  g_handler->SetEditHwnd(editWnd);

  // Create the browser view.
  CefWindowInfo window_info;
  CefBrowserSettings settings;


    // Initialize window info to the defaults for a child window.
    window_info.SetAsChild(contentView, 0, 0, kWindowWidth, kWindowHeight);
 

  CefBrowserHost::CreateBrowser(window_info, g_handler.get(),
                                g_handler->GetStartupURL(), settings);

  // Show the window.
  [mainWnd makeKeyAndOrderFront: nil];

  // Size the window.
  NSRect r = [mainWnd contentRectForFrameRect:[mainWnd frame]];
  r.size.width = kWindowWidth;
	r.size.height = kWindowHeight;
  [mainWnd setFrame:[mainWnd frameRectForContentRect:r] display:YES];
}

// Called when the application's Quit menu item is selected.
- (NSApplicationTerminateReply)applicationShouldTerminate:
      (NSApplication *)sender {
  // Request that all browser windows close.
  if (g_handler.get())
    g_handler->CloseAllBrowsers(false);

  // Cancel the termination. The application will exit after all windows have
  // closed.
  return NSTerminateCancel;
}

// Sent immediately before the application terminates. This signal should not
// be called because we cancel the termination.
- (void)applicationWillTerminate:(NSNotification *)aNotification {
  ASSERT(false);  // Not reached.
}

@end


int main(int argc, char* argv[]) {
  CefMainArgs main_args(argc, argv);
  CefRefPtr<ClientApp> app(new ClientApp);
	
	printf("Hello sir how are you today?\n");

  // Execute the secondary process, if any.
//  int exit_code = CefExecuteProcess(main_args, app.get());
//  if (exit_code >= 0)
//    return exit_code;

  // Retrieve the current working directory.
  getcwd(szWorkingDir, sizeof(szWorkingDir));

  // Initialize the AutoRelease pool.
  NSAutoreleasePool* autopool = [[NSAutoreleasePool alloc] init];

  // Initialize the ClientApplication instance.
  [ClientApplication sharedApplication];

  // Parse command line arguments.
  AppInitCommandLine(argc, argv);

  CefSettings settings;

  // Populate the settings based on command line arguments.
  AppGetSettings(settings);

  // Initialize CEF.
  CefInitialize(main_args, settings, app.get());

  // Register the scheme handler.
//  scheme_test::InitTest();

	printf("Delegating delegates\n");
	
  // Create the application delegate and window.
  NSObject* delegate = [[ClientAppDelegate alloc] init];
  [delegate performSelectorOnMainThread:@selector(createApp:) withObject:nil
                          waitUntilDone:NO];

  // Run the application message loop.
  CefRunMessageLoop();
	
	
	printf("Gooday. Please come again.");


  // Shut down CEF.
  CefShutdown();

  // Release the handler.
  g_handler = NULL;

  // Release the delegate.
  [delegate release];

  // Release the AutoRelease pool.
  [autopool release];

  return 0;
}


// Global functions

std::string AppGetWorkingDirectory() {
  return szWorkingDir;
}

void AppQuitMessageLoop() {
  CefQuitMessageLoop();
}
