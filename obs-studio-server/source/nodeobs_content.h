#pragma once
#include <obs.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <thread>
#include <chrono>
#include <thread>
#include <mutex>
#ifndef _WIN32
	#include <unistd.h>
#endif
#include <ctime>
#include <fstream>
#include <map>
#include "nodeobs_display.h"
#include "nodeobs_api.h"
using namespace std;

struct SourceInfo {
	uint32_t fader;
	uint32_t volmeter;
};

extern map<std::string, SourceInfo*> sourceInfo;
extern vector<std::string> tabScenes;
extern string currentTransition;
extern map<string, obs_source_t*> transitions;

class OBS_content
{
public:
	OBS_content();
	~OBS_content();

	static void Register(ipc::server&);

	static void OBS_content_createDisplay(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_destroyDisplay(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_getDisplayPreviewOffset(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_getDisplayPreviewSize(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_createSourcePreviewDisplay(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_resizeDisplay(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_moveDisplay(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_setPaddingSize(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_setPaddingColor(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_setBackgroundColor(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_setOutlineColor(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_setGuidelineColor(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_setResizeBoxOuterColor(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_setResizeBoxInnerColor(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_setShouldDrawUI(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_selectSource(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_selectSources(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_dragSelectedSource(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_getDrawGuideLines(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_content_setDrawGuideLines(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
};
