// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#include "cefclient/client_handler.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "cefclient/cefclient.h"

//
//void ClientHandler::SendNotification(NotificationType type) {
//  SEL sel = nil;
//  switch(type) {
//    case NOTIFY_CONSOLE_MESSAGE:
//      sel = @selector(notifyConsoleMessage:);
//      break;
//    case NOTIFY_DOWNLOAD_COMPLETE:
//      sel = @selector(notifyDownloadComplete:);
//      break;
//    case NOTIFY_DOWNLOAD_ERROR:
//      sel = @selector(notifyDownloadError:);
//      break;
//  }
//
//  if (sel == nil)
//    return;
//
//  NSWindow* window = [AppGetMainHwnd() window];
//  NSObject* delegate = [window delegate];
//  [delegate performSelectorOnMainThread:sel withObject:nil waitUntilDone:NO];
//}

