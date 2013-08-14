#import "ns_hack_objc.h"
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#define GLFW_EXPOSE_NATIVE_NSGL 1
#define GLFW_EXPOSE_NATIVE_COCOA 1

#import <GLFW/glfw3native.h>

//- (void)fullscreenAction:(id)sender
//{
//    [[NSApplication sharedApplication] orderFrontStandardAboutPanel:self];
//}


void makeFullscreen(GLFWwindow * glWindow)
{
	NSWindow * window = glfwGetCocoaWindow(glWindow);
	
	
	NSSize currentSize = [[window contentView] frame].size;
	[window setMinSize:currentSize];
	
	if ([window  respondsToSelector:@selector(toggleFullScreen:)])
	{	
		[window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
		[window toggleFullScreen:window];
	}
	
//	NSMenu *mainMenu = [[NSApplication sharedApplication] mainMenu];
//	NSMenu *appMenu = [[mainMenu itemAtIndex:0] submenu];
//	NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"Fullscreen"
	//										   keyEquivalent:@""];
//action:[window toggleFullScreen:window]
//	[item autorelease];
//	//[item setTarget:self];
//	[appMenu addItem:item];
}


void focusWindow(GLFWwindow * glWindow)
{
	[NSApp activateIgnoringOtherApps:YES];
	NSWindow * window = glfwGetCocoaWindow(glWindow);
	[window makeKeyAndOrderFront:window];
}


/*
int main(int argc, char *argv[])
{
    // Creates an autorelease pool
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
    // Loads the application
    NSApplicationLoad();
	
    // Creates a simple window
    NSRect MainWindowRect = NSMakeRect(300, 300, 300, 500);
	
    NSWindow* MainWindow = [[NSWindow alloc] initWithContentRect: MainWindowRect
													   styleMask: NSTitledWindowMask | NSClosableWindowMask |
							NSMiniaturizableWindowMask | NSResizableWindowMask
														 backing: NSBackingStoreBuffered
														   defer: NO];
	
    [MainWindow orderFrontRegardless];
	
	makeFullscreen(MainWindow);
	
	[MainWindow toggleFullScreen:MainWindow];
	
    // Creates the application object
	
    [NSApplication sharedApplication];
	
    // Enters main message loop
	
    [NSApp run];
	
    // Call release method from object pool
    [pool release];
	
    return 0;
}*/