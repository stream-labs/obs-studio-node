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

#include "MyCPPClass.h"
#include "MyObject-C-Interface.h"
#include <obs.h>

std::queue<std::pair<MouseEventType, MouseEvent>> mouseEvents;
std::mutex mouseEvents_mtx;

MyCPPClass::MyCPPClass( void )
    : _impl ( nullptr )
{   }

void MyCPPClass::init( void )
{
    _impl = new MyClassImpl();
}

MyCPPClass::~MyCPPClass( void )
{
    if ( _impl ) { delete _impl; _impl = nullptr; }
}

void MyCPPClass::createDisplay(void)
{
    return _impl->createDisplay();
}

void MyCPPClass::destroyDisplay(void *displayObj)
{
    return _impl->destroyDisplay(displayObj);
}

void MyCPPClass::startDrawing(void *displayObj)
{
    return _impl->startDrawing(displayObj);
}

void MyCPPClass::resizeDisplay(void *displayObj, int width, int height)
{
    return _impl->resizeDisplay(displayObj, width, height);
}

void MyCPPClass::moveDisplay(void *displayObj, int x, int y)
{
    return _impl->moveDisplay(displayObj, x, y);
}

void MyCPPClass::setFocused(void *displayObj, bool focused)
{
    return _impl->setFocused(displayObj, focused);
}

int MyCPPClass::getCurrentScaleFactor(void)
{
    return _impl->getCurrentScaleFactor();
}

void MyCPPClass::destroyWindow(void)
{
    _impl->destroyWindow();
}

void addEvent(MouseEventType type, MouseEvent mouse_event)
{
    std::unique_lock<std::mutex> ulock(mouseEvents_mtx);
    mouseEvents.emplace(std::make_pair(type, mouse_event));
}