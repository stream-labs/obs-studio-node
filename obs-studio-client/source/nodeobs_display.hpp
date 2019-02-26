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

#include <nan.h>
#include <node.h>

namespace display
{
	static void OBS_content_createDisplay(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_destroyDisplay(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_getDisplayPreviewOffset(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_getDisplayPreviewSize(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_createSourcePreviewDisplay(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_resizeDisplay(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_moveDisplay(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_setPaddingSize(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_setPaddingColor(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_setOutlineColor(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_setShouldDrawUI(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_content_setDrawGuideLines(const v8::FunctionCallbackInfo<v8::Value>& args);
} // namespace display
