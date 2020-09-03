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


Napi::FunctionReference osn::Video::constructor;

Napi::Object osn::Video::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);
	Napi::Function func =
		DefineClass(env,
		"Video",
		{
			StaticAccessor("skippedFrames", &osn::Video::skippedFrames, nullptr),
			StaticAccessor("encodedFrames", &osn::Video::encodedFrames, nullptr),
		});
	exports.Set("Video", func);
	osn::Video::constructor = Napi::Persistent(func);
	osn::Video::constructor.SuppressDestruct();
	return exports;
}

osn::Video::Video(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<osn::Video>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
}

Napi::Value osn::Video::skippedFrames(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Video", "GetSkippedFrames", {});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), response[1].value_union.ui32);
}

Napi::Value osn::Video::encodedFrames(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Video", "GetTotalFrames", {});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), response[1].value_union.ui32);
}
