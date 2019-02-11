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

#include "video.hpp"
#include <error.hpp>
#include "controller.hpp"
#include "shared.hpp"
#include "utility-v8.hpp"
#include "utility.hpp"

void osn::Video::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
{
	auto ObsVideo = Nan::New<v8::Object>();

	utilv8::SetObjectAccessorProperty(ObsVideo, "skippedFrames", skippedFrames);
	utilv8::SetObjectAccessorProperty(ObsVideo, "encodedFrames", encodedFrames);

	Nan::Set(target, FIELD_NAME("Video"), ObsVideo);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Video::skippedFrames(Nan::NAN_METHOD_ARGS_TYPE info)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Video", "GetSkippedFrames", {});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.ui32));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Video::encodedFrames(Nan::NAN_METHOD_ARGS_TYPE info)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Video", "GetTotalFrames", {});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.ui32));
}
