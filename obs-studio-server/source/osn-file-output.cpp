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

#include "osn-file-output.hpp"
#include "osn-error.hpp"
#include "shared.hpp"

void osn::IFileOutput::GetPath(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	FileOutput* fileOutput =
		osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!fileOutput) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "File output reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(fileOutput->path));
	AUTO_DEBUG;
}

void osn::IFileOutput::SetPath(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	FileOutput* fileOutput =
		osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!fileOutput) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "File output reference is not valid.");
	}

    fileOutput->path = args[1].value_str;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}


void osn::IFileOutput::GetFormat(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	FileOutput* fileOutput =
		osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!fileOutput) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "File output reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(fileOutput->format));
	AUTO_DEBUG;
}

void osn::IFileOutput::SetFormat(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	FileOutput* fileOutput =
		osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!fileOutput) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "File output reference is not valid.");
	}

    fileOutput->format = args[1].value_str;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::IFileOutput::GetMuxerSettings(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	FileOutput* fileOutput =
		osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!fileOutput) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "File output reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(fileOutput->muxerSettings));
	AUTO_DEBUG;
}

void osn::IFileOutput::SetMuxerSettings(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	FileOutput* fileOutput =
		osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!fileOutput) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "File output reference is not valid.");
	}

    fileOutput->muxerSettings = args[1].value_str;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::IFileOutput::GetFileFormat(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	FileOutput* fileOutput =
		osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!fileOutput) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "ReplayBuffer reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(fileOutput->fileFormat));
	AUTO_DEBUG;
}

void osn::IFileOutput::SetFileFormat(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	FileOutput* fileOutput =
		osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!fileOutput) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "ReplayBuffer reference is not valid.");
	}

    fileOutput->fileFormat = args[1].value_str;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::IFileOutput::GetOverwrite(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	FileOutput* fileOutput =
		osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!fileOutput) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "ReplayBuffer reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(fileOutput->overwrite));
	AUTO_DEBUG;
}

void osn::IFileOutput::SetOverwrite(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	FileOutput* fileOutput =
		osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!fileOutput) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "ReplayBuffer reference is not valid.");
	}

    fileOutput->overwrite = args[1].value_union.ui32;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::IFileOutput::GetNoSpace(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	FileOutput* fileOutput =
		osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!fileOutput) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "ReplayBuffer reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(fileOutput->noSpace));
	AUTO_DEBUG;
}

void osn::IFileOutput::SetNoSpace(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	FileOutput* fileOutput =
		osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!fileOutput) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "ReplayBuffer reference is not valid.");
	}

    fileOutput->noSpace = args[1].value_union.ui32;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

osn::IFileOutput::Manager& osn::IFileOutput::Manager::GetInstance()
{
	static osn::IFileOutput::Manager _inst;
	return _inst;
}