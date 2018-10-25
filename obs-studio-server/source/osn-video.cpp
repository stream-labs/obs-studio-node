// Client module for the OBS Studio node module.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#include "osn-video.hpp"
#include <ipc-server.hpp>
#include <obs.h>
#include "error.hpp"
#include "shared.hpp"

video_t* s_handler;

void osn::Video::Register(ipc::server& srv)
{
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("Video");
	cls->register_function(std::make_shared<ipc::function>("GetGlobal", std::vector<ipc::type>{}, GetGlobal));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetSkippedFrames", std::vector<ipc::type>{ipc::type::UInt64}, GetSkippedFrames));
	cls->register_function(
	    std::make_shared<ipc::function>("GetTotalFrames", std::vector<ipc::type>{ipc::type::UInt64}, GetTotalFrames));

	srv.register_collection(cls);
}

void osn::Video::GetGlobal(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	uint64_t uid;

	if (s_handler) {
		uid = osn::Video::Manager::GetInstance().find(s_handler);
	} else {
		s_handler = obs_get_video();
		uid       = osn::Video::Manager::GetInstance().allocate(s_handler);
	}

	if (uid == UINT64_MAX) {
		// No further Ids left, leak somewhere.
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Index list is full."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));

	AUTO_DEBUG;
}

void osn::Video::GetSkippedFrames(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	video_t* video = osn::Video::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!video) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Video context is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(video_output_get_skipped_frames(video)));
	AUTO_DEBUG;
}

void osn::Video::GetTotalFrames(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	video_t* video = osn::Video::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!video) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Video context is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(video_output_get_total_frames(video)));
	AUTO_DEBUG;
}

osn::Video::Manager& osn::Video::Manager::GetInstance()
{
	static osn::Video::Manager _inst;
	return _inst;
}