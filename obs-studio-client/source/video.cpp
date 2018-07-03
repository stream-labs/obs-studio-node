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

#include "video.hpp"
#include "shared.hpp"
#include "utility.hpp"
#include "utility-v8.hpp"
#include "controller.hpp"
#include <error.hpp>

Nan::Persistent<v8::FunctionTemplate> osn::Video::prototype = Nan::Persistent<v8::FunctionTemplate>();

void osn::Video::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target) {
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->SetClassName(Nan::New<v8::String>("Video").ToLocalChecked());
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);

	utilv8::SetTemplateField(fnctemplate, "getGlobal", GetGlobal);
	utilv8::SetTemplateAccessorProperty(fnctemplate->InstanceTemplate(), "skippedFrames", GetSkippedFrames);
	utilv8::SetTemplateAccessorProperty(fnctemplate->InstanceTemplate(), "totalFrames", GetTotalFrames);

	utilv8::SetObjectField(target, "Video", fnctemplate->GetFunction());
	prototype.Reset(fnctemplate);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Video::GetGlobal(Nan::NAN_METHOD_ARGS_TYPE info) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Video", "GetGlobal",
		{});

	if (!ValidateResponse(response)) return;
		
	osn::Video* obj = new osn::Video(response[1].value_union.ui64);
	info.GetReturnValue().Set(osn::Video::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Video::GetSkippedFrames(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::Video* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Video", "GetSkippedFrames",
		{ ipc::value(obj->handler) });

	if (!ValidateResponse(response)) return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.ui32));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Video::GetTotalFrames(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::Video* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Video", "GetTotalFrames",
		{ ipc::value(obj->handler) });

	if (!ValidateResponse(response)) return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.ui32));
}
