/******************************************************************************
    Copyright (C) 2016-2022 by Streamlabs (General Workings Inc)

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

#include "osn-recording.hpp"
#include "osn-encoder.hpp"
#include "osn-error.hpp"
#include "shared.hpp"

void osn::IRecording::GetPath(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Recording reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(recording->path));
	AUTO_DEBUG;
}

void osn::IRecording::SetPath(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Recording reference is not valid.");
	}

    recording->path = args[1].value_str;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}


void osn::IRecording::GetFormat(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Recording reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(recording->format));
	AUTO_DEBUG;
}

void osn::IRecording::SetFormat(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Recording reference is not valid.");
	}

    recording->format = args[1].value_str;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::IRecording::GetMuxerSettings(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Recording reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(recording->muxerSettings));
	AUTO_DEBUG;
}

void osn::IRecording::SetMuxerSettings(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Recording reference is not valid.");
	}

    recording->muxerSettings = args[1].value_str;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::IRecording::GetVideoEncoder(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

	uint64_t uid =
        osn::Encoder::Manager::GetInstance().find(recording->videoEncoder);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::IRecording::SetVideoEncoder(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

    obs_encoder_t* encoder =
        osn::Encoder::Manager::GetInstance().find(args[1].value_union.ui64);
	if (!encoder) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Encoder reference is not valid.");
	}

    recording->videoEncoder = encoder;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::IRecording::Query(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Recording reference is not valid.");
	}

	std::unique_lock<std::mutex> ulock(recording->signalsMtx);
	if (recording->signalsReceived.empty()) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));

	auto signal = recording->signalsReceived.front();
	rval.push_back(ipc::value("recording"));
	rval.push_back(ipc::value(signal.signal));
	rval.push_back(ipc::value(signal.code));
	rval.push_back(ipc::value(signal.errorMessage));

	recording->signalsReceived.pop();

	AUTO_DEBUG;
}

void osn::IRecording::GetFileFormat(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Recording reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(recording->fileFormat));
	AUTO_DEBUG;
}

void osn::IRecording::SetFileFormat(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Recording reference is not valid.");
	}

    recording->fileFormat = args[1].value_str;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::IRecording::GetOverwrite(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Recording reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(recording->overwrite));
	AUTO_DEBUG;
}

void osn::IRecording::SetOverwrite(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Recording reference is not valid.");
	}

    recording->overwrite = args[1].value_union.ui32;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::IRecording::GetNoSpace(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Recording reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(recording->noSpace));
	AUTO_DEBUG;
}

void osn::IRecording::SetNoSpace(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Recording reference is not valid.");
	}

    recording->noSpace = args[1].value_union.ui32;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

osn::IRecording::Manager& osn::IRecording::Manager::GetInstance()
{
	static osn::IRecording::Manager _inst;
	return _inst;
}