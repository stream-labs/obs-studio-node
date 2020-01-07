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

#ifndef __MYOBJECT_C_INTERFACE_H__
#define __MYOBJECT_C_INTERFACE_H__

typedef unsigned long long uint64_t;

class MyClassImpl
{
public:
    MyClassImpl ( void );
    ~MyClassImpl( void );

    void init( void );
    void createDisplay(void);
    void destroyDisplay(void *displayObj);
    void startDrawing(void *displayObj);
    void resizeDisplay(void *displayObj, int width, int height);
    void moveDisplay(void *displayObj, int x, int y);
    void setFocused(void *displayObj, bool focused);
    int  getCurrentScaleFactor(void);

private:
    void * self;
};

#endif
