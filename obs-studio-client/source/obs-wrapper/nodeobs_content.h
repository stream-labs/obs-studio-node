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

#pragma once
#include <chrono>
#include <iostream>
#include <mutex>
#include <obs.h>
#include <sstream>
#include <stdio.h>
#include <string>
#include <thread>
#include <vector>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <ctime>
#include <fstream>
#include <map>
#include "obs-nodeobs_api.h"
#include "obs-nodeobs_display.h"

struct SourceInfo
{
	uint32_t fader;
	uint32_t volmeter;
};

class OBS_content
{
	public:
	OBS_content();
	~OBS_content();

	static void OBS_content_createDisplay(uint64_t windowHandle, std::string key, int32_t displayMode);
	static void OBS_content_destroyDisplay(std::string key);
	static void OBS_content_shutdownDisplays();
	static std::pair<int32_t, int32_t> OBS_content_getDisplayPreviewOffset(std::string key);
	static std::pair<int32_t, int32_t> OBS_content_getDisplayPreviewSize(std::string key);
	static void OBS_content_createSourcePreviewDisplay(uint64_t windowHandle, std::string sourceName, std::string key);
	static void OBS_content_resizeDisplay(std::string key, uint32_t width, uint32_t height);
	static void OBS_content_moveDisplay(std::string key, uint32_t x, uint32_t y);
	static void OBS_content_setPaddingSize(std::string key, uint32_t paddingSize);
	static void OBS_content_setPaddingColor(std::string key, uint32_t r, uint32_t g, uint32_t b, uint32_t a);
	static void OBS_content_setOutlineColor(std::string key, uint32_t r, uint32_t g, uint32_t b, uint32_t a);
	static void OBS_content_setShouldDrawUI(std::string key, bool drawUI);
	static void OBS_content_setDrawGuideLines(std::string key, bool drawGuideLines);
	// static void OBS_content_createIOSurface(
	//     void*                          data,
	//     const int64_t                  id,
	//     const std::vector<ipc::value>& args,
	//     std::vector<ipc::value>&       rval);
};
