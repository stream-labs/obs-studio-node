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
#include "nodeobs_api.h"
#include "nodeobs_display.h"

struct SourceInfo
{
	uint32_t fader;
	uint32_t volmeter;
};

extern std::map<std::string, SourceInfo*> sourceInfo;
extern std::vector<std::string>           tabScenes;
extern std::string                        currentTransition;
extern std::map<std::string, obs_source_t*>    transitions;

class OBS_content
{
	public:
	OBS_content();
	~OBS_content();

	static void Register(ipc::server&);

	static void OBS_content_createDisplay(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_content_destroyDisplay(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_content_getDisplayPreviewOffset(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_content_getDisplayPreviewSize(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_content_createSourcePreviewDisplay(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_content_resizeDisplay(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_content_moveDisplay(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_content_setPaddingSize(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_content_setPaddingColor(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_content_setBackgroundColor(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_content_setOutlineColor(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_content_setShouldDrawUI(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_content_setDrawGuideLines(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
};
