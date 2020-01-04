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

WinDel *del;

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

void createWindow(void)
{
    NSLog(@"Creating delegate display"); 

    try {
        NSRect content_rect = NSMakeRect(0, 0, 0, 0);
        win = [
                [NSWindow alloc]
                initWithContentRect:content_rect
                styleMask:NSBorderlessWindowMask // movable
                backing:NSBackingStoreBuffered
                defer:NO
            ];
        if (!win)
            throw "Could not create window";
        
        win.delegate = del;
        NSWindowController *windowController = [[NSWindowController alloc] initWithWindow:win];
        [windowController showWindow:del];
        [_array addObject:windowController];

        view = [[CustomView alloc] initWithFrame:content_rect];
        if (!view)
            throw "Could not create view";
    } catch (char const *error) {
        NSLog(@"could not create display");
        NSLog(@"%s\n", error);

        [NSApp terminate:nil];
    }

    win.contentView = view;
    [win orderFrontRegardless];
    // [win setHidesOnDeactivate:YES];
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
    NSLog(@"WindowController close");
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

    return 1;
}

void MyClassImpl::destroyDisplay(void)
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        NSLog(@"Destroy display");
        [win close];
    });
}

void MyClassImpl::startDrawing(void *displayObj)
{
    NSLog(@"Start drawing");
    dispatch_sync(dispatch_get_main_queue(), ^{
        createWindow();
        OBS::Display* dp = static_cast<OBS::Display*>(displayObj);
        dp->m_gsInitData.window.view = (id)view;
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
        [win setContentSize:NSMakeSize(width, height)];
        // [win setContentSize:NSMakeSize(0, 0)];
    });
}

void MyClassImpl::moveDisplay(int x, int y)
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        NSScreen *mainScreen = [[NSScreen screens] firstObject];
        NSRect screenRect = [mainScreen visibleFrame];
        [win setFrame:CGRectMake(x, 
            screenRect.size.height - y - [win frame].size.height,
            [win frame].size.width ,
            [win frame].size.height) display:YES];

        // Update tracking area
        NSView *currentView = [win contentView];
        NSTrackingArea* trackingArea = [[NSTrackingArea alloc]
                                initWithRect:[currentView bounds]
                                options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveAlways
                                owner:currentView userInfo:nil];
        [currentView addTrackingArea:trackingArea];
        view.previousTimeMoved = nil;
        view.previousTimeDragged = nil;
    });
}

void MyClassImpl::setFocused(bool focused)
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        if (focused) {
            [win setLevel:NSNormalWindowLevel + 1];
            [win setLevel:NSNormalWindowLevel];
        }
        else {
            // [win setLevel:NSNormalWindowLevel];
        }
    });
}

@end
