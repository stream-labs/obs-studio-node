#include "window-osx.h"

#include <iostream>

@implementation WindowImplObj

WindowObjCInt::WindowObjCInt(void)
    : self(NULL)
{   }

WindowObjCInt::~WindowObjCInt(void)
{
    [(id)self dealloc];
}

void WindowObjCInt::init(void)
{
    self = [[WindowImplObj alloc] init];
}

void WindowObjCInt::createWindow(void)
{
    // NSLog(@"Creating a window inside the client");
    // CGWindowListOption listOptions;
    // CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionAll, kCGNullWindowID);
    // int count = [windowList count];
    // std::cout << "COUNT WINDOWS :" << count << std::endl;

    // for (CFIndex idx=0; idx<CFArrayGetCount(windowList); idx++) {
    //     CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, idx);
    //     CFStringRef windowName = (CFStringRef)CFDictionaryGetValue(dict, kCGWindowName);
    //     NSString* nsWindowName = (NSString*)windowName;
    //     NSLog(@"Window name: %@", nsWindowName);
    // }

    // CFRelease(windowList);

    // try {
    //     NSLog(@"HERE 0");
    //     NSRect content_rect = NSMakeRect(500, 500, 1000, 1000);
    //     NSLog(@"HERE 2");
    //     NSWindow* win = [
    //             [NSWindow alloc]
    //             initWithContentRect:content_rect
    //             styleMask:NSBorderlessWindowMask // movable
    //             backing:NSBackingStoreBuffered
    //             defer:NO
    //         ];
    //     NSLog(@"HERE 3");
    //     if (!win)
    //         throw "Could not create window";

    //     // info->win = win;
    //     del = [[WinDel alloc] init];
    //     win.delegate = del;
    //     // NSWindowController *windowController = [[NSWindowController alloc] initWithWindow:win];
    //     // [windowController showWindow:del];
    //     // [_array addObject:windowController];

    //     NSLog(@"HERE 4");
    //     view = [[NSView alloc] initWithFrame:content_rect];
    //     NSLog(@"HERE 5");
    //     if (!view)
    //         throw "Could not create view";
    //     // info->view = view;
    // } catch (char const *error) {
    //     NSLog(@"could not create display");
    //     NSLog(@"%s\n", error);

    //     [NSApp terminate:nil];
    // }

    // NSLog(@"HERE 6");
    // win.contentView = view;
    // NSLog(@"HERE 7");
    // [win orderFrontRegardless];
}

@end

@implementation WinDel
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    // UNUSED_PARAMETER(notification);
    NSLog(@"Application finished launching");
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
    // UNUSED_PARAMETER(app);

    return NO;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    // UNUSED_PARAMETER(sender);
    return NSTerminateNow;
}

@end