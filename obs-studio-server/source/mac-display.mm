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

#include "mac-display.h"
#include <obs.h>
#include "MyCPPClass.h"

@implementation CustomView
- (MouseEvent)translateEvent:(NSEvent *)event
{
    MouseEvent mouse_event;

    NSUInteger flags = [event modifierFlags];
    mouse_event.altKey = flags & NSEventModifierFlagOption;
    mouse_event.ctrlKey = flags &  NSEventModifierFlagControl;
    mouse_event.shiftKey = flags & NSEventModifierFlagShift;

    mouse_event.button = event.buttonNumber;

    NSUInteger button_mask = [NSEvent pressedMouseButtons];
    int buttons = 0;
    if ((button_mask & (1 << 0)) != 0)
        buttons += 1;
    if ((button_mask & (1 << 1)) != 0)
        buttons += 2;
    mouse_event.buttons = buttons;

    NSPoint mouse_location = event.locationInWindow;
    mouse_event.x = mouse_location.x;
    mouse_event.y = mouse_location.y;

    return mouse_event;
}
- (void)mouseDown:(NSEvent *)event
{
    addEvent(mouseDown, [self translateEvent: event]);
}

- (void)mouseUp:(NSEvent *)event {
    addEvent(mouseUp, [self translateEvent: event]);
}

- (void)mouseDragged:(NSEvent *)event {
    if (!self.mouseIn)
        return;

    if (!self.previousTimeDragged) {
        self.previousTimeDragged = [NSDate date];
    }

    NSDate *timeNow = [NSDate date];
    NSTimeInterval executionTime = [timeNow timeIntervalSinceDate:self.previousTimeDragged];

    if (executionTime * 1000 > 20) {
        self.previousTimeDragged = timeNow;
        addEvent(mouseDragged, [self translateEvent: event]);
    }
}

- (void)mouseMoved:(NSEvent *)event {
    if (!self.mouseIn)
        return;

    if (!self.previousTimeMoved) {
        self.previousTimeMoved = [NSDate date];
    }

    NSDate *timeNow = [NSDate date];
    NSTimeInterval executionTime = [timeNow timeIntervalSinceDate:self.previousTimeMoved];

    if (executionTime * 1000 > 20) {
        self.previousTimeMoved = timeNow;
        addEvent(mouseMoved, [self translateEvent: event]);
    }
}

- (void)mouseEntered:(NSEvent *)event {
    if (self.mouseIn)
        return;

    addEvent(mouseEntered, [self translateEvent: event]);
    self.mouseIn = true;
}

- (void)mouseExited:(NSEvent *)event {
    if (!self.mouseIn)
        return;

    self.mouseIn = false;
}

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];

    if (self) {
        self.previousTimeMoved = nil;
        self.previousTimeDragged = nil;
        self.mouseIn = false;
    }
    return self;
}

- (void)setItemPropertiesToDefault:sender
{
}
@end

DisplayInfo* createWindow(void)
{
    NSLog(@"Creating delegate display"); 
    DisplayInfo *info = new DisplayInfo;

    try {
        NSRect content_rect = NSMakeRect(0, 0, 0, 0);
        NSWindow* win = [
                [NSWindow alloc]
                initWithContentRect:content_rect
                styleMask:NSBorderlessWindowMask // movable
                backing:NSBackingStoreBuffered
                defer:NO
            ];
        if (!win)
            throw "Could not create window";

        info->win = win;
        
        win.delegate = del;
        NSWindowController *windowController = [[NSWindowController alloc] initWithWindow:win];
        [windowController showWindow:del];
        [_array addObject:windowController];

        CustomView *view = [[CustomView alloc] initWithFrame:content_rect];
        if (!view)
            throw "Could not create view";
        info->view = view;
    } catch (char const *error) {
        NSLog(@"could not create display");
        NSLog(@"%s\n", error);

        [NSApp terminate:nil];
    }

    info->win.contentView = info->view;
    [info->win orderFrontRegardless];

    return info;
}

@implementation WinDel
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    UNUSED_PARAMETER(notification);
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
    UNUSED_PARAMETER(app);

    return NO;
}

- (void)windowWillClose:(NSNotification *)notification
{
    NSWindow *window = notification.object;
    [_array removeObject:window.windowController];
}

@end

@implementation MyObject

MyClassImpl::MyClassImpl( void )
    : self( NULL )
{   }

MyClassImpl::~MyClassImpl( void )
{
    [(id)self dealloc];
}

void MyClassImpl::init( void )
{
    self = [[MyObject alloc] init];
}

void MyClassImpl::createDisplay(void)
{
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        del = [[WinDel alloc] init];
        app.delegate = del;
        [app run];
    }
}

void MyClassImpl::destroyDisplay(void *displayObj)
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        OBS::Display* dp = static_cast<OBS::Display*>(displayObj);
        auto it = displayWindows.find(dp);
        if (it != displayWindows.end()) {
            DisplayInfo *info = it->second;
            [info->win close];
        }
    });
}

void MyClassImpl::startDrawing(void *displayObj)
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        OBS::Display* dp = static_cast<OBS::Display*>(displayObj);
        DisplayInfo *info = createWindow();
        displayWindows.emplace(dp, info);
        dp->m_gsInitData.window.view = (id)info->view;
        dp->m_display = obs_display_create(&dp->m_gsInitData, 0x0);

        obs_display_add_draw_callback(
            dp->m_display,
            [](void *displayPtr, uint32_t cx, uint32_t cy) {
                OBS::Display::DisplayCallback(displayPtr, cx, cy);
            },
            dp
        );
    });
}

void MyClassImpl::resizeDisplay(void *displayObj, int width, int height)
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        OBS::Display* dp = static_cast<OBS::Display*>(displayObj);
        auto it = displayWindows.find(dp);
        if (it != displayWindows.end()) {
            DisplayInfo *info = it->second;
            [info->win setContentSize:NSMakeSize(width, height)];
        }
    });
}

void MyClassImpl::moveDisplay(void *displayObj, int x, int y)
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        OBS::Display* dp = static_cast<OBS::Display*>(displayObj);
        auto it = displayWindows.find(dp);
        if (it != displayWindows.end()) {
            DisplayInfo *info = it->second;
            NSScreen *mainScreen = [[NSScreen screens] firstObject];
            NSRect screenRect = [mainScreen visibleFrame];
            [info->win setFrame:CGRectMake(x, 
                screenRect.size.height - y,
                [info->win frame].size.width ,
                [info->win frame].size.height) display:YES];

            if (dp->m_enableMouseEvents) {
                // Update tracking area
                NSView *currentView = [info->win contentView];
                NSTrackingArea* trackingArea = [[NSTrackingArea alloc]
                                        initWithRect:[currentView bounds]
                                        options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveAlways
                                        owner:currentView userInfo:nil];
                [currentView addTrackingArea:trackingArea];
            }
            info->view.previousTimeMoved = nil;
            info->view.previousTimeDragged = nil;
        }
    });
}

void MyClassImpl::setFocused(void *displayObj, bool focused)
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        OBS::Display* dp = static_cast<OBS::Display*>(displayObj);
        auto it = displayWindows.find(dp);
        if (it != displayWindows.end()) {
            DisplayInfo *info = it->second;
            if (focused) {
                [info->win setLevel:NSNormalWindowLevel + 1];
                [info->win setLevel:NSNormalWindowLevel];
            }
        }
    });
}

int MyClassImpl::getCurrentScaleFactor(void)
{
    return [[NSScreen mainScreen] backingScaleFactor];
}

@end
