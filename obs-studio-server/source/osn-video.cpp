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

#include "osn-video.hpp"
#include <ipc-server.hpp>
#include <obs.h>
#include "error.hpp"
#include "shared.hpp"

video_t* handler;

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
    // TODO: Remove me and refers to the video context as a global instance, we don't need uids for this
	uint64_t uid = 0;

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
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(video_output_get_skipped_frames(obs_get_video())));
	AUTO_DEBUG;
}

void osn::Video::GetTotalFrames(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(video_output_get_total_frames(obs_get_video())));
	AUTO_DEBUG;
}