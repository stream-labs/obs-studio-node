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

#ifndef __MYCPP_CLASS_H__
#define __MYCPP_CLASS_H__

#include <queue>
#include <mutex>

typedef unsigned long long uint64_t;

class MyClassImpl;

enum MouseEventType: uint8_t {
    mouseDown = 0,
    mouseUp = 1,
    mouseDragged = 2,
    mouseMoved = 3,
    mouseEntered = 4
};

struct MouseEvent {
    float x;
    float y;
    bool altKey;
    bool ctrlKey;
    bool shiftKey;
    int button;
    int buttons;
};

extern std::queue<std::pair<MouseEventType, MouseEvent>> mouseEvents;
extern std::mutex mouseEvents_mtx;

void addEvent(MouseEventType type, MouseEvent mouse_event);

class MyCPPClass
{
    enum { cANSWER_TO_LIFE_THE_UNIVERSE_AND_EVERYTHING = 42 };
public:
    MyCPPClass ( void );
    ~MyCPPClass( void );

    void init( void );

    void createDisplay(void);
    void destroyDisplay(void);
    void startDrawing(void *displayObj);
    void resizeDisplay(void *displayObj, int width, int height);
    void moveDisplay(int x, int y);
    void setFocused(bool focused);

private:
    MyClassImpl * _impl;
};

#endif
