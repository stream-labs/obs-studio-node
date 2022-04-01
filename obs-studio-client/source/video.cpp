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

			StaticMethod("get", &osn::Video::get),
			StaticMethod("set", &osn::Video::set),
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

Napi::Value osn::Video::get(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Video", "GetVideoContext", {});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	Napi::Object video = Napi::Object::New(info.Env());
	if (response.size() != 11)
		return info.Env().Undefined();

	video.Set("fpsNum", response[1].value_union.ui32);
	video.Set("fpsDen", response[2].value_union.ui32);
	video.Set("baseWidth", response[3].value_union.ui32);
	video.Set("baseHeight", response[4].value_union.ui32);
	video.Set("outputWidth", response[5].value_union.ui32);
	video.Set("outputHeight", response[6].value_union.ui32);
	video.Set("outputFormat", response[7].value_union.ui32);
	video.Set("colorspace", response[8].value_union.ui32);
	video.Set("range", response[9].value_union.ui32);
	video.Set("scaleType", response[10].value_union.ui32);

	return video;
}

void osn::Video::set(const Napi::CallbackInfo& info)
{
	Napi::Object video = info[0].ToObject();
	if (!video || !video.IsObject()) {
		Napi::Error::New(
			info.Env(),
			"The video context object passed in invalid.").ThrowAsJavaScriptException();
		return;
	}

	uint32_t fpsNum = video.Get("fpsNum").ToNumber().Uint32Value();
	uint32_t fpsDen = video.Get("fpsDen").ToNumber().Uint32Value();
	uint32_t baseWidth = video.Get("baseWidth").ToNumber().Uint32Value();
	uint32_t baseHeight = video.Get("baseHeight").ToNumber().Uint32Value();
	uint32_t outputWidth = video.Get("outputWidth").ToNumber().Uint32Value();
	uint32_t outputHeight = video.Get("outputHeight").ToNumber().Uint32Value();
	uint32_t outputFormat = video.Get("outputFormat").ToNumber().Uint32Value();
	uint32_t colorspace = video.Get("colorspace").ToNumber().Uint32Value();
	uint32_t range = video.Get("range").ToNumber().Uint32Value();
	uint32_t scaleType = video.Get("scaleType").ToNumber().Uint32Value();

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("Video", "SetVideoContext", {
		ipc::value(fpsNum), ipc::value(fpsDen), ipc::value(baseWidth), ipc::value(baseHeight),
		ipc::value(outputWidth), ipc::value(outputHeight), ipc::value(outputFormat),
		ipc::value(colorspace), ipc::value(range), ipc::value(scaleType)
	});
}
