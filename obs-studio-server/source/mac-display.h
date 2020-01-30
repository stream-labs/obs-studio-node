/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
******************************************************************************/

#import "Foundation/Foundation.h"
#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import "MyObject-C-Interface.h"
#include "MyCPPClass.h"
#include "nodeobs_display.h"
#include <map>


@interface MyObject : NSObject
@end

@interface CustomView: NSView
@property (atomic, strong) NSDate *previousTimeMoved;
@property (atomic, strong) NSDate *previousTimeDragged;
@property (atomic, strong) NSTrackingArea* trackingArea;
@property bool mouseIn;
- (MouseEvent) translateEvent:(NSEvent *)event;
@end

@interface WinDel : NSWindow <NSApplicationDelegate, NSWindowDelegate>
- (void)applicationDidFinishLaunching:(NSNotification *)notification;
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app;
- (void)windowWillClose:(NSNotification *)notification;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
@end

struct DisplayInfo{
    CustomView *view;
    NSWindow *win;
};


NSArray *_array;
WinDel *del;
std::map<OBS::Display*, DisplayInfo*> displayWindows;
