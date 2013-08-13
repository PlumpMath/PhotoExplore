#import "ns_hack.h"
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#define GLFW_EXPOSE_NATIVE_NSGL 1
#define GLFW_EXPOSE_NATIVE_COCOA 1

#import <GLFW/glfw3native.h>

void makeFullscreen(GLFWwindow * glWindow)
{
	NSWindow * window = glfwGetCocoaWindow(glWindow);
	
	[window setCollectionBehavior:
	 NSWindowCollectionBehaviorFullScreenPrimary];
	
	
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