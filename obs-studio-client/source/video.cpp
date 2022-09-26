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
			StaticMethod("create", &osn::Video::Create),
			StaticMethod("destroy", &osn::Video::Destroy),

			InstanceAccessor("video", &osn::Video::get, &osn::Video::set),

			InstanceAccessor("skippedFrames", &osn::Video::skippedFrames, nullptr),
			InstanceAccessor("encodedFrames", &osn::Video::encodedFrames, nullptr),
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

Napi::Value osn::Video::Create(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Video", "AddVideoContext", {});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

    auto instance =
        osn::Video::constructor.New({
            Napi::Number::New(info.Env(), response[1].value_union.ui64)
        });

	return instance;
}

void osn::Video::Destroy(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return;

    auto video =
        Napi::ObjectWrap<osn::Video>::Unwrap(info[0].ToObject());

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Video", "RemoveVideoContext", {ipc::value((uint64_t)video->canvasId)});

	if (!ValidateResponse(info, response))
		return;

	return;
}

Napi::Value osn::Video::get(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Video", "GetVideoContext", {ipc::value((uint64_t)this->canvasId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	if (response.size() != 11)
		return info.Env().Undefined();

	Napi::Object video = Napi::Object::New(info.Env());
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

void osn::Video::set(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	Napi::Object video = value.ToObject();
	if (!video || video.IsUndefined()) {
		Napi::Error::New(
			info.Env(),
			"The video context object passed is invalid.").ThrowAsJavaScriptException();
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

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Video", "SetVideoContext", {
		ipc::value(fpsNum), ipc::value(fpsDen), ipc::value(baseWidth), ipc::value(baseHeight),
		ipc::value(outputWidth), ipc::value(outputHeight), ipc::value(outputFormat),
		ipc::value(colorspace), ipc::value(range), ipc::value(scaleType), ipc::value(this->canvasId)
	});
	
	if (!ValidateResponse(info, response))
		return;

}
