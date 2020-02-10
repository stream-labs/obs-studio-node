#import "Foundation/Foundation.h"
#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>

#include "window-osx-obj-c-int.h"
#include "window-osx-int.hpp"

@interface WindowImplObj : NSObject
@end

@interface WinDel : NSWindow <NSApplicationDelegate, NSWindowDelegate>
- (void)applicationDidFinishLaunching:(NSNotification *)notification;
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
@end

NSView *view;
NSWindow *win;
WinDel *del;